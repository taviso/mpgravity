#ifndef SECURITY_WIN32
# define SECURITY_WIN32
#endif
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>
#include <winsock.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <schannel.h>
#include <security.h>
#include <sspi.h>
#include <assert.h>

#include "sockssl.h"

// Best guess about how much data will be read for a handshake
// doesn't matter if it's wrong.
#define INIT_IO_BUFSIZE (1 << 12)

#if !defined(NDEBUG) && !defined(TRACE)
# include <stdio.h>
// You can only use TRACE() from C++, here is a quick replacement for C. Could
// use OutputDebugString() or RPT() instead, but it doesn't really bother me.
// Note that in debug builds you need to AllocConsole() and then freopen()
// stdout to see printf() output.
# define TRACE(msg, ...) do {       \
    printf("%s:", __func__);        \
    printf(msg, __VA_ARGS__);       \
    printf("\n");                   \
    fflush(stdout);                 \
  } while (false)
# define AfxDebugBreak __debugbreak
#else
# define TRACE(msg, ...)
# define AfxDebugBreak()
#endif // !NDEBUG

#if defined(TEST_EINTR)
static INT sendfault(SOCKET socket, PVOID buf, DWORD len, DWORD flags)
{
    static i;
    if (i++ & 1 && i > 1) {
        WSASetLastError(WSAEINTR);
        return SOCKET_ERROR;
    }
    return send(socket, buf, len, flags);
}
static INT recvfault(SOCKET socket, PVOID buf, DWORD len, DWORD flags)
{
    static i;
    if (i++ & 1 && i > 2) {
        WSASetLastError(WSAEINTR);
        return SOCKET_ERROR;
    }
    return recv(socket, buf, len, flags);
}
# define send sendfault
# define recv recvfault
#endif // TEST_EINTR

typedef struct _SEC_LAYER {
    HANDLE Heap;
    CredHandle Credentials;
    CtxtHandle Context;
    SecPkgContext_StreamSizes StreamSizes;
    // Receive Buffers
    SecBuffer CipherCache;
    SecBuffer PlainCache;
    // Send Buffer
    SecBuffer SendCache;
    // Dont need to save WSAError, because the system does.
    DWORD LastError;
} SEC_LAYER, *PSEC_LAYER;

// I dunno if these are all necessary.
static const kSSPIFlags = ISC_REQ_SEQUENCE_DETECT
                        | ISC_REQ_REPLAY_DETECT
                        | ISC_REQ_CONFIDENTIALITY
                        | ISC_RET_EXTENDED_ERROR
                        | ISC_REQ_ALLOCATE_MEMORY
                        | ISC_REQ_STREAM;

// This send() wrapper handles WSAEINTR by accepting and caching all data
// sent. If you call with Len == 0, any existing data is sent, this way
// you can check if it's safe to EncryptMessage() before committing to
// completing the operation.
//
// Q. Why not just loop until send() accepts it?
// A. Gravity uses this technique to stay responsive during long blocking
//    operations, so if we don't return after an EINTR, the GUI will freeze.
static INT SafeSend(PSEC_LAYER SecLayer, SOCKET Socket, PVOID Buf, DWORD Len)
{
    INT Result;
    DWORD CurrSize;
    PBYTE CurrAddr;

    TRACE("%u cached bytes", SecLayer->SendCache.cbBuffer);

    CurrSize = SecLayer->SendCache.cbBuffer;
    CurrAddr = SecLayer->SendCache.pvBuffer;

    // Caller just wants to clear buffer, we can return now if we
    // have no data to send.
    if (Len == 0 && CurrSize == 0) {
        return 0;
    }

    // Add all of this new data to our send buffer.
    SecLayer->SendCache.cbBuffer += Len;
    SecLayer->SendCache.pvBuffer = HeapReAlloc(SecLayer->Heap,
                                               HEAP_ZERO_MEMORY,
                                               CurrAddr,
                                               SecLayer->SendCache.cbBuffer);

    // Pointer to end of current buffer.
    CurrAddr = SecLayer->SendCache.pvBuffer;
    CurrAddr += CurrSize;

    CopyMemory(CurrAddr, Buf, Len);

    // Try to send some of the data we have.
    Result = send(Socket,
                  SecLayer->SendCache.pvBuffer,
                  SecLayer->SendCache.cbBuffer,
                  0);

    TRACE("send() returns %d", Result);

    if (Result == SOCKET_ERROR) {
        // Gravity called WSACancelBlockingCall() while we were blocked on
        // send(), we just swallow the error and pretend it worked, then
        // will try again next time.
        if (WSAGetLastError() == WSAEINTR) {
            TRACE("send interrupted, %d bytes saved in cache", Len);
            return Len ? Len : SOCKET_ERROR;
        } else {
            TRACE("send WinSock error was %#x", WSAGetLastError());
            // Our only option is to discard the data in the cache, this will
            // require some renegotiation by schannel.
            SecLayer->SendCache.cbBuffer = 0;
            return SOCKET_ERROR;
        }
    }

    // We got rid of some data, fixup the cache.
    SecLayer->SendCache.cbBuffer -= Result;

    MoveMemory(SecLayer->SendCache.pvBuffer,
       (PBYTE) SecLayer->SendCache.pvBuffer + Result,
               SecLayer->SendCache.cbBuffer);

    TRACE("send cache now contains %u", SecLayer->SendCache.cbBuffer);

    // Note that we always claim we sent everything requested.
    return Len;
}

static INT DisconnectFromServer(PSEC_LAYER SecLayer, SOCKET Socket)
{
    INT Result;
    DWORD dwSSPIOutFlags;
    DWORD dwType = SCHANNEL_SHUTDOWN;
    SecBuffer OutBuffers[] = {
        {
            .pvBuffer   = &dwType,
            .BufferType = SECBUFFER_TOKEN,
            .cbBuffer   = sizeof(dwType),
        },
    };
    SecBufferDesc OutBuffer = {
        .cBuffers   = _countof(OutBuffers),
        .pBuffers   = OutBuffers,
        .ulVersion  = SECBUFFER_VERSION,
    };

    // Notify schannel that we are about to close the connection.
    SecLayer->LastError = ApplyControlToken(&SecLayer->Context, &OutBuffer);

    if (SecLayer->LastError != SEC_E_OK) {
        TRACE("ApplyControlToken() => %#x", SecLayer->LastError);
        return -1;
    }

    // Build an SSL close notify message.
    OutBuffers[0].pvBuffer   = NULL;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer   = 0;

    SecLayer->LastError = InitializeSecurityContext(&SecLayer->Credentials,
                                                    &SecLayer->Context,
                                                    NULL,
                                                    kSSPIFlags,
                                                    0,
                                                    0,
                                                    NULL,
                                                    0,
                                                    &SecLayer->Context,
                                                    &OutBuffer,
                                                    &dwSSPIOutFlags,
                                                    NULL);

    if (SecLayer->LastError != SEC_E_OK) {
        TRACE("InitializeSecurityContext() => %#x", SecLayer->LastError);
        return -1;
    }

    TRACE("close notify is %d bytes", OutBuffers[0].cbBuffer);

    // Send the close notify message to the server.
    Result = SafeSend(SecLayer,
                      Socket,
                      OutBuffers[0].pvBuffer,
                      OutBuffers[0].cbBuffer);

    TRACE("send returned %d", Result);

    FreeContextBuffer(OutBuffers[0].pvBuffer);

    if (Result == SOCKET_ERROR) {
        TRACE("failed to send close notify, %#x", WSAGetLastError());
        return -1;
    }

    return 0;
}


static SECURITY_STATUS ClientHandshakeLoop(PSEC_LAYER SecLayer,
                                           SOCKET Socket,
                                           PCredHandle phCreds,
                                           CtxtHandle * phContext,
                                           BOOL fDoInitialRead)
{
    SecBufferDesc OutBuffer, InBuffer;
    SecBuffer OutBuffers[1], InBuffers[2];
    DWORD dwSSPIOutFlags, cbData;
    DWORD cbIoBuffer;
    DWORD IoBufferSz;
    PBYTE IoBuffer;
    SECURITY_STATUS scRet;
    BOOL fDoRead;

    // Take ownership of the CipherCache during handshake.
    cbIoBuffer = SecLayer->CipherCache.cbBuffer;
    IoBuffer = SecLayer->CipherCache.pvBuffer;
    IoBufferSz = cbIoBuffer;

    // This is FALSE for renegotiation requests.
    fDoRead = fDoInitialRead;

    // Loop until the handshake is finished or an error occurs.
    scRet = SEC_I_CONTINUE_NEEDED;

    while (scRet == SEC_I_CONTINUE_NEEDED || scRet == SEC_E_INCOMPLETE_MESSAGE) {
        // Read data from server.
        if (SecLayer->CipherCache.cbBuffer == 0 || scRet == SEC_E_INCOMPLETE_MESSAGE) {
            if (fDoRead) {
                // Guesstimate how much more room we need.
                IoBufferSz = IoBufferSz
                    ? (IoBufferSz << 1)
                    : INIT_IO_BUFSIZE;

                // Make more space.
                SecLayer->CipherCache.pvBuffer = HeapReAlloc(
                    SecLayer->Heap,
                    HEAP_ZERO_MEMORY,
                    SecLayer->CipherCache.pvBuffer,
                    IoBufferSz);

                IoBuffer = SecLayer->CipherCache.pvBuffer;

                // First make sure the sendbuffer is empty...
                if (SafeSend(SecLayer, Socket, 0, 0) != 0) {
                    // There's no way to recover from this, we'll desync
                    // and never get connected, just return error
                    // and let caller resume if possible.
                    return SOCKET_ERROR;
                }

                cbData = recv(Socket, IoBuffer + cbIoBuffer, IoBufferSz - cbIoBuffer, 0);

                if (cbData == SOCKET_ERROR) {
                    TRACE("error %d reading data from server", WSAGetLastError());
                    scRet = SEC_E_INTERNAL_ERROR;
                    break;
                } else if (cbData == 0) {
                    TRACE("server unexpectedly disconnected");
                    scRet = SEC_E_INTERNAL_ERROR;
                    break;
                }
                TRACE("%d bytes of handshake data received", cbData);
                SecLayer->CipherCache.cbBuffer += cbData;
                cbIoBuffer = SecLayer->CipherCache.cbBuffer;
            } else {
              fDoRead = TRUE;
            }
        }

        // Set up the input buffers. Buffer 0 is used to pass in data
        // received from the server. Schannel will consume some or all
        // of this. Leftover data (if any) will be placed in buffer 1 and
        // given a buffer type of SECBUFFER_EXTRA.
        InBuffers[0].pvBuffer   = IoBuffer;
        InBuffers[0].cbBuffer   = cbIoBuffer;
        InBuffers[0].BufferType = SECBUFFER_TOKEN;

        InBuffers[1].pvBuffer   = NULL;
        InBuffers[1].cbBuffer   = 0;
        InBuffers[1].BufferType = SECBUFFER_EMPTY;

        InBuffer.cBuffers       = 2;
        InBuffer.pBuffers       = InBuffers;
        InBuffer.ulVersion      = SECBUFFER_VERSION;


        // Set up the output buffers. These are initialized to NULL
        // so as to make it less likely we'll attempt to free random
        // garbage later.
        OutBuffers[0].pvBuffer  = NULL;
        OutBuffers[0].BufferType= SECBUFFER_TOKEN;
        OutBuffers[0].cbBuffer  = 0;

        OutBuffer.cBuffers      = 1;
        OutBuffer.pBuffers      = OutBuffers;
        OutBuffer.ulVersion     = SECBUFFER_VERSION;

        scRet = InitializeSecurityContextA(phCreds,
                                           phContext,
                                           NULL,
                                           kSSPIFlags,
                                           0,
                                           0,
                                           &InBuffer,
                                           0,
                                           NULL,
                                           &OutBuffer,
                                           &dwSSPIOutFlags,
                                           NULL);


        // If InitializeSecurityContext was successful (or if the error was
        // one of the special extended ones), send the contends of the output
        // buffer to the server.
        if (scRet == SEC_E_OK
         || scRet == SEC_I_CONTINUE_NEEDED
         || FAILED(scRet)
         && (dwSSPIOutFlags & ISC_RET_EXTENDED_ERROR)) {
            if (OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL) {
                cbData = SafeSend(SecLayer, Socket, OutBuffers[0].pvBuffer, OutBuffers[0].cbBuffer);

                if (cbData == SOCKET_ERROR || cbData == 0) {
                    TRACE("error %d sending data to server", WSAGetLastError());
                    FreeContextBuffer(OutBuffers[0].pvBuffer);
                    DeleteSecurityContext(phContext);
                    return SEC_E_INTERNAL_ERROR;
                }

                TRACE("%d bytes of handshake data sent", cbData);

                // Free output buffer.
                FreeContextBuffer(OutBuffers[0].pvBuffer);
                OutBuffers[0].pvBuffer = NULL;
            }
        }

        // If InitializeSecurityContext returned SEC_E_INCOMPLETE_MESSAGE,
        // then we need to read more data from the server and try again.
        if (scRet == SEC_E_INCOMPLETE_MESSAGE) continue;

        // If InitializeSecurityContext returned SEC_E_OK, then the
        // handshake completed successfully.
        if (scRet == SEC_E_OK) {
            // If the "extra" buffer contains data, this is encrypted application
            // protocol layer stuff. It needs to be saved. The application layer
            // will later decrypt it with DecryptMessage.
            TRACE("handshake was successful");

            if (InBuffers[1].BufferType == SECBUFFER_EXTRA) {
                TRACE("%d extra bytes with handshake", InBuffers[1].cbBuffer);

                MoveMemory(IoBuffer,
                           IoBuffer + (cbIoBuffer - InBuffers[1].cbBuffer),
                           InBuffers[1].cbBuffer);

                SecLayer->CipherCache.cbBuffer = InBuffers[1].cbBuffer;

                TRACE("CipherCache @%lu", SecLayer->CipherCache.cbBuffer);
            } else {
                // All the data was used.
                TRACE("handshake used all data in cache");
                SecLayer->CipherCache.cbBuffer = 0;
            }
            break; // Bail out to quit
        }

        // Check for fatal error.
        if (FAILED(scRet)) {
            TRACE("error %#x returned by InitializeSecurityContext", scRet);
            break;
        }

        // If InitializeSecurityContext returned SEC_I_INCOMPLETE_CREDENTIALS,
        // then the server just requested client authentication.
        if (scRet == SEC_I_INCOMPLETE_CREDENTIALS) {
            break;
        }

        // Copy any leftover data from the "extra" buffer, and go around again.
        if (InBuffers[1].BufferType == SECBUFFER_EXTRA) {
            MoveMemory(IoBuffer,
                       IoBuffer + (cbIoBuffer - InBuffers[1].cbBuffer),
                       InBuffers[1].cbBuffer);
            cbIoBuffer = InBuffers[1].cbBuffer;
        } else {
          cbIoBuffer = 0;
        }

        // Keep cache data updated.
        SecLayer->CipherCache.cbBuffer = cbIoBuffer;
    }

    // Delete the security context in the case of a fatal error.
    if (FAILED(scRet))
        DeleteSecurityContext(phContext);

    return scRet;
}

static SECURITY_STATUS PerformClientHandshake(PSEC_LAYER      SecLayer,
                                              SOCKET          Socket,        // in
                                              LPSTR           pszServerName, // in
                                              PCredHandle     phCreds,
                                              CtxtHandle *    phContext)
{

    SecBufferDesc   OutBuffer;
    SecBuffer       OutBuffers[1];
    DWORD           dwSSPIOutFlags, cbData;
    SECURITY_STATUS scRet;

    //  Initiate a ClientHello message and generate a token.
    OutBuffers[0].pvBuffer   = NULL;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer   = 0;

    OutBuffer.cBuffers  = 1;
    OutBuffer.pBuffers  = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    scRet = InitializeSecurityContextA(phCreds,
                                       NULL,
                                       pszServerName,
                                       kSSPIFlags,
                                       0,
                                       0,
                                       NULL,
                                       0,
                                       phContext,
                                       &OutBuffer,
                                       &dwSSPIOutFlags,
                                       NULL);

    if (scRet != SEC_I_CONTINUE_NEEDED) {
        TRACE("error %d returned by InitializeSecurityContext", scRet);
        return scRet;
    }

    // Send response to server if there is one.
    if (OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL) {

        cbData = SafeSend(SecLayer, Socket, OutBuffers[0].pvBuffer, OutBuffers[0].cbBuffer);

        if (cbData == SOCKET_ERROR || cbData == 0)
        {
            TRACE("error %d sending data to server", WSAGetLastError());
            FreeContextBuffer(OutBuffers[0].pvBuffer);
            DeleteSecurityContext(phContext);
            return SEC_E_INTERNAL_ERROR;
        }

        TRACE("%d bytes of handshake data sent", cbData);
        FreeContextBuffer(OutBuffers[0].pvBuffer);
        OutBuffers[0].pvBuffer = NULL;
    }

    return ClientHandshakeLoop(SecLayer, Socket, phCreds, phContext, TRUE);
}

static DWORD EncryptSend(PSEC_LAYER SecLayer,
                         SOCKET Socket,
                         PBYTE pbIoBuffer,
                         DWORD cbMessage)
{
    DWORD cbData;
    PBYTE pbMessage = pbIoBuffer + SecLayer->StreamSizes.cbHeader;
    SecBuffer Buffers[] = {
        {
          .pvBuffer   = pbIoBuffer,
          .cbBuffer   = SecLayer->StreamSizes.cbHeader,
          .BufferType = SECBUFFER_STREAM_HEADER
        }, {
          .pvBuffer   = pbMessage,
          .cbBuffer   = cbMessage,
          .BufferType = SECBUFFER_DATA
        }, {
          .pvBuffer   = pbMessage + cbMessage,
          .cbBuffer   = SecLayer->StreamSizes.cbTrailer,
          .BufferType = SECBUFFER_STREAM_TRAILER
        }, {
          .BufferType = SECBUFFER_EMPTY,
          .pvBuffer   = NULL,
          .cbBuffer   = 0
        },
    };
    SecBufferDesc Message = {
        .ulVersion       = SECBUFFER_VERSION,
        .cBuffers        = _countof(Buffers),
        .pBuffers        = Buffers,
    };

    TRACE("sending %d bytes of plaintext.", cbMessage);

    // Before we commit to sending this, check if it will succeed.
    if (SafeSend(SecLayer, Socket, NULL, 0) != 0) {
        TRACE("not commiting to send(), interrupted message in cache");
        return SOCKET_ERROR;
    }

    SecLayer->LastError = EncryptMessage(&SecLayer->Context, 0, &Message, 0);

    if (SecLayer->LastError != SEC_E_OK) {
        TRACE("error %#x returned by EncryptMessage", SecLayer->LastError);
        return SecLayer->LastError;
    }

    // It would be nice if we could just send() here, but we need to handle
    // WSACancelBlockingCall. We cannot simply abandon the message because it
    // will look like a message suppression attack.

    // Regardless, once we've encryped, it *must* be sent or the socket will
    // break.
    cbData = SafeSend(SecLayer,
                      Socket,
                      pbIoBuffer,
                      Buffers[0].cbBuffer
                    + Buffers[1].cbBuffer
                    + Buffers[2].cbBuffer);

    TRACE("%d bytes of encrypted data sent", cbData);

    // We can pass this error through to caller.
    if (cbData == SOCKET_ERROR || cbData == 0)
        return cbData;

    // We don't want to return the actual number through if it worked, because
    // we added some protocol data to it.
    //
    // Note that SafeSend() always accepts all the data we give it.
    return cbMessage;
}

static LONG FillRequestFromCache(PSEC_LAYER SecLayer, PBYTE Buf, DWORD Size)
{
    TRACE("plaintext cache @%lu bytes.", SecLayer->PlainCache.cbBuffer);

    if (SecLayer->PlainCache.cbBuffer) {
        SIZE_T FilledFromCache = min(Size, SecLayer->PlainCache.cbBuffer);

        // We can fill at least part of this request from the cache.
        memcpy(Buf, SecLayer->PlainCache.pvBuffer, FilledFromCache);

        // Discard the used data in cache.
        MoveMemory(SecLayer->PlainCache.pvBuffer,
           (PBYTE) SecLayer->PlainCache.pvBuffer + FilledFromCache,
                   SecLayer->PlainCache.cbBuffer - FilledFromCache);

        SecLayer->PlainCache.cbBuffer -= FilledFromCache;

        // Cool, you've got all the data we have.
        return FilledFromCache;
    }

    return 0;
}

// This is slightly more complicated than it should be because the way Gravity
// prevents blocking is not using async sockets or select() queries like a
// normal person, it just makes potentially blocking recv() calls, and then
// calls WSACancelBlocking() periodically from another thread!
//
// That's not really a problem with plaintext, an interrupted recv can just be
// resumed.. but we have to deal with messages that can be partially
// read but not ready to be processed! To handle this we need to cache
// ciphertext and plaintext between calls.
LONG SecureRecv(PSEC_LAYER SecLayer, SOCKET Socket, PBYTE Buf, DWORD Size)
{
    SECURITY_STATUS scRet;
    SecBuffer Buffers[4];
    DWORD RecvOffset = 0;
    DWORD Refills = 0;
    DWORD DataProvided = 0;
    SecBufferDesc Message = {
        .ulVersion        = SECBUFFER_VERSION,
        .cBuffers         = _countof(Buffers),
        .pBuffers         = Buffers,
    };

    TRACE("SecureRecv() => %#x bytes from %#x into %p", Size, Socket, Buf);

    if (!SecLayer || Socket == INVALID_SOCKET) {
        return SOCKET_ERROR;
    }

    // First make sure the sendbuffer is empty or we'll get desynched.
    if (SafeSend(SecLayer, Socket, 0, 0) != 0) {
        return SOCKET_ERROR;
    }

    // You cannot simply read any amount of data, you need a whole message
    // to decrypt. We can cache any data not wanted and return it on the next
    // recv().
    while (TRUE) {
        // Return any data we already have.
        if ((DataProvided = FillRequestFromCache(SecLayer, Buf, Size))) {
            return DataProvided;
        }

        // We don't have any plaintext, we need to collect some ciphertext.
        // First, how much ciphertext do we _already_ have?
        if ((RecvOffset = SecLayer->CipherCache.cbBuffer)) {
            // The only way we can reach here is if Gravity called
            // WSACancelBlocking() in another thread, so resume a
            // previous operation.
            TRACE("resume interrupted operation with %d bytes", RecvOffset);
            goto resume;
        }

        // How much data (approx) would we _like_? If we're expecting
        // a message of Size bytes, then we can probably expect at least that
        // much ciphertext, with additional protocol header overhead.
        SecLayer->CipherCache.cbBuffer = Size
            + SecLayer->StreamSizes.cbHeader
            + SecLayer->StreamSizes.cbTrailer;

        for (Refills = 0;; Refills++) {
            LONG AmountRead;

            TRACE("attempt %d to refill ciphertext cache", Refills);

            // Allocate the space we think we might need, we can ask schannel
            // how short we are. RecvOffset is how much actually contains
            // data, the rest we are about to fill from recv();
            SecLayer->CipherCache.pvBuffer = HeapReAlloc(
                SecLayer->Heap,
                HEAP_ZERO_MEMORY,
                SecLayer->CipherCache.pvBuffer,
                SecLayer->CipherCache.cbBuffer);

            AmountRead = recv(Socket,
                      (PBYTE) SecLayer->CipherCache.pvBuffer + RecvOffset,
                              SecLayer->CipherCache.cbBuffer - RecvOffset,
                              0);

            TRACE("%d bytes of application data received", AmountRead);

            // Even if there was en error, attempt a decrypt operation.
            // The reason is that we may have been blocking and Gravity woke us
            // up from a different thread with WSACancelBlocking(), and I may
            // have overestimated the read size.
            if (AmountRead <= 0) {
                TRACE("error %d from recv()", WSAGetLastError());

                // Fixup cache size to record what we still have.
                SecLayer->CipherCache.cbBuffer = RecvOffset;
                return AmountRead;
            }

            // Okay, trim off any excess cache buffer.
            SecLayer->CipherCache.cbBuffer = RecvOffset += AmountRead;

          resume:

            // Now setup the buffers to attempt a decryption.
            ZeroMemory(Buffers, sizeof Buffers);
            CopyMemory(Buffers, &SecLayer->CipherCache, sizeof(Buffers[0]));

            scRet = DecryptMessage(&SecLayer->Context, &Message, 0, NULL);

            TRACE("DecryptMessage() => %#x", scRet);

            // These should never be modified.
            assert(Message.cBuffers == _countof(Buffers));
            assert(Message.pBuffers == Buffers);

            // Huh, I guess that just worked, cleanup and then see if we can
            // now fill this request from the cache.
            if (scRet == SEC_E_OK) {
                DWORD ExtraBytes = 0;

                // First, find out if there was a SECBUFFER_EXTRA, this tells
                // us about unprocessed data.
                for (int i = 0; i < _countof(Buffers); i++) {
                    if (Message.pBuffers[i].BufferType == SECBUFFER_EXTRA) {
                        ExtraBytes = Message.pBuffers[i].cbBuffer;
                        break;
                    }
                }

                TRACE("there are %u bytes still in ciphertext", ExtraBytes);

                assert(SecLayer->PlainCache.cbBuffer == 0);

                // Second, copy all the plaintext to the plaintext cache.
                SecLayer->PlainCache.cbBuffer = SecLayer->CipherCache.cbBuffer
                                              - ExtraBytes;

                SecLayer->PlainCache.pvBuffer = HeapReAlloc(
                    SecLayer->Heap,
                    HEAP_ZERO_MEMORY,
                    SecLayer->PlainCache.pvBuffer,
                    SecLayer->PlainCache.cbBuffer);

                MoveMemory(SecLayer->PlainCache.pvBuffer,
                           SecLayer->CipherCache.pvBuffer,
                           SecLayer->PlainCache.cbBuffer);

                // XXX: Am I guaranteed this?
                assert(SecLayer->CipherCache.cbBuffer
                    >= SecLayer->StreamSizes.cbTrailer
                    + SecLayer->StreamSizes.cbHeader);

                // Thirdly we need to pull the protocol headers out of the
                // Plaintext.
                MoveMemory(SecLayer->PlainCache.pvBuffer,
                   (PBYTE) SecLayer->PlainCache.pvBuffer
                            + SecLayer->StreamSizes.cbHeader,
                           SecLayer->PlainCache.cbBuffer
                            - SecLayer->StreamSizes.cbHeader);

                // Adjust buffer sizes accodingly.
                SecLayer->PlainCache.cbBuffer -= SecLayer->StreamSizes.cbHeader;
                SecLayer->PlainCache.cbBuffer -= SecLayer->StreamSizes.cbTrailer;

                // Fourthly (yeesh), we need to fix the CipherCache to
                // remove all the Plaintext.
                MoveMemory(SecLayer->CipherCache.pvBuffer,
                   (PBYTE) SecLayer->CipherCache.pvBuffer
                            + SecLayer->CipherCache.cbBuffer
                            - ExtraBytes,
                           ExtraBytes);

                SecLayer->CipherCache.cbBuffer = ExtraBytes;

                TRACE("CipherCache now contains %d", ExtraBytes);

                // Now break to let the plaintext cache handle the request.
                break;
            }

            // More data is required to decrypt the last message. We can find
            // out how much data is needed by looking for SECBUFFER_MISSING
            // buffers.
            if (scRet == SEC_E_INCOMPLETE_MESSAGE) {
                DWORD MissingBytes = 0;

                TRACE("We need more ciphertext to make progress");

                for (unsigned long i = 0; i < Message.cBuffers; i++) {
                    // The amount needed is Message.pBuffers[i].cbBuffer
                    if (Message.pBuffers[i].BufferType == SECBUFFER_MISSING) {
                        assert(Message.pBuffers[i].cbBuffer);
                        MissingBytes = Message.pBuffers[i].cbBuffer;
                        break;
                    }
                }

                SecLayer->CipherCache.cbBuffer += MissingBytes;

                TRACE("system says we need %d more bytes", MissingBytes);

                // Try another recv now that we know the size required.
                continue;
            }

            // XXX: If this happens, there's some memory management error.
            // this is a bug in my code.
            if (scRet == SEC_E_INVALID_TOKEN) {
                TRACE("oof, buffers are not correct");
                AfxDebugBreak();
            }

            // Either there was a network error, or a bug in my code managing
            // ciphertext.
            if (scRet == SEC_E_DECRYPT_FAILURE) {
                TRACE("system reports that message decryption failed");
                AfxDebugBreak();
            }

            // FIXME: I've never seen this happen...
            if (scRet == SEC_I_CONTEXT_EXPIRED) {
                TRACE("SEC_I_CONTEXT_EXPIRED what should I do here?");
                AfxDebugBreak();
            }

            // Server wants to renegotiate, we may need to shuffle
            // SECBUFFER_EXTRA buffers around.
            // FIXME: This probably doesnt handle WSAEINTR yet.
            if (scRet == SEC_I_RENEGOTIATE) {
                TRACE("server requested renegotiate");
                scRet = ClientHandshakeLoop(SecLayer,
                                            Socket,
                                            &SecLayer->Credentials,
                                            &SecLayer->Context,
                                            FALSE);

                TRACE("ClientHandshakeLoop => %#x", scRet);

                if (scRet == SEC_E_OK) {
                    // Try another recv now that worked.
                    continue;
                }

            }

            // FIXME: handle all the documented return codes.
            TRACE("unhandled scRet %#x", scRet);
            AfxDebugBreak();

            // Maybe invalidate the cipher cache and try again?
            SecLayer->CipherCache.cbBuffer = 0;
            return SOCKET_ERROR;
        }
    }

    // Unreachable.
    AfxDebugBreak();
    return SOCKET_ERROR;
}

LONG SecureSend(PSEC_LAYER SecurityLayer, SOCKET Socket, PVOID Buf, SIZE_T Size)
{
    SECURITY_STATUS Status;
    PBYTE pbIoBuffer;
    DWORD cbIoBufferLength, cbData;

    TRACE("%lu %p %lu", Socket, Buf, Size);

    if (!SecurityLayer || Socket == INVALID_SOCKET) {
        return SOCKET_ERROR;
    }

    if (Size > SecurityLayer->StreamSizes.cbMaximumMessage)
        Size = SecurityLayer->StreamSizes.cbMaximumMessage;

    // Create a buffer.
    cbIoBufferLength = SecurityLayer->StreamSizes.cbHeader
                        + SecurityLayer->StreamSizes.cbTrailer
                        + Size;
    pbIoBuffer       = HeapAlloc(SecurityLayer->Heap, 0, cbIoBufferLength);

    memcpy(pbIoBuffer+SecurityLayer->StreamSizes.cbHeader, Buf, Size);

    cbData = EncryptSend(SecurityLayer, Socket, pbIoBuffer, Size);

    HeapFree(SecurityLayer->Heap, 0, pbIoBuffer);

    return cbData;
}

PSEC_LAYER AddSecurityLayer(LPCTSTR HostAddress, SOCKET Socket) {
    SECURITY_STATUS Status;
    PSEC_LAYER SecurityLayer;
    TimeStamp Lifetime;
    SCHANNEL_CRED SchannelCreds = {
        .dwVersion = SCHANNEL_CRED_VERSION,
    };

    SecurityLayer = HeapAlloc(GetProcessHeap(),
                              HEAP_ZERO_MEMORY,
                              sizeof *SecurityLayer);

    if (!SecurityLayer) {
        TRACE("memory allocation failure");
        return NULL;
    }

    // Create a heap to make cleanup easier.
    SecurityLayer->Heap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 0, 0);

    SecurityLayer->PlainCache.BufferType = SECBUFFER_DATA;
    SecurityLayer->CipherCache.BufferType = SECBUFFER_DATA;
    SecurityLayer->SendCache.BufferType = SECBUFFER_DATA;

    // Create the buffers used for decrypting messages.
    SecurityLayer->PlainCache.pvBuffer = HeapAlloc(SecurityLayer->Heap,
                                                   HEAP_ZERO_MEMORY,
                                                   0);
    SecurityLayer->CipherCache.pvBuffer = HeapAlloc(SecurityLayer->Heap,
                                                    HEAP_ZERO_MEMORY,
                                                    0);
    SecurityLayer->SendCache.pvBuffer = HeapAlloc(SecurityLayer->Heap,
                                                  HEAP_ZERO_MEMORY,
                                                  0);

    // https://docs.microsoft.com/en-us/windows/win32/secauthn/getting-schannel-credentials
    Status = AcquireCredentialsHandle(NULL,
                                      UNISP_NAME,
                                      SECPKG_CRED_OUTBOUND,
                                      NULL,
                                      &SchannelCreds,
                                      NULL,
                                      NULL,
                                      &SecurityLayer->Credentials,
                                      &Lifetime);

    if (Status != SEC_E_OK) {
        TRACE("AcquireCredentialsHandle => %#x", Status);
        goto error;
    }

    if (PerformClientHandshake(SecurityLayer,
                               Socket,
                               HostAddress,
                               &SecurityLayer->Credentials,
                               &SecurityLayer->Context) != 0) {
        // Note: if it was WSAEINTR, you can retry!
        TRACE("perform handshake failed");
        goto error;
    }

    Status = QueryContextAttributes(&SecurityLayer->Context,
                                    SECPKG_ATTR_STREAM_SIZES,
                                    &SecurityLayer->StreamSizes);

    if (Status != SEC_E_OK) {
        TRACE("Failed to query sizes");
        goto error;
    }

    return SecurityLayer;

  error:
    DeleteSecurityContext(&SecurityLayer->Context);
    FreeCredentialsHandle(&SecurityLayer->Credentials);
    HeapDestroy(SecurityLayer->Heap);
    HeapFree(GetProcessHeap(), 0, SecurityLayer);
    return NULL;
}

void RemoveSecureLayer(PSEC_LAYER SecurityLayer, SOCKET Socket)
{
    if (SecurityLayer == NULL)
        return;

    // Send a close_notify alert to the server
    DisconnectFromServer(SecurityLayer, Socket);
    DeleteSecurityContext(&SecurityLayer->Context);
    FreeCredentialsHandle(&SecurityLayer->Credentials);
    HeapDestroy(SecurityLayer->Heap);
    HeapFree(GetProcessHeap(), 0, SecurityLayer);
    return;
}

LONG SockAvailable(PSEC_LAYER SecurityLayer, SOCKET Socket, PULONG Available)
{
    ULONG ProtoOverhead;

    // If there is no SecurityLayer, this is an error.
    if (!SecurityLayer) {
        return SOCKET_ERROR;
    }

    // This is a generic replacement for FIONREAD that adjusts for SSL.
    if (SecurityLayer->PlainCache.cbBuffer) {
        *Available = SecurityLayer->PlainCache.cbBuffer;
        return 0;
    }

    // OK, what's on the socket?
    if (ioctlsocket(Socket, FIONREAD, Available) == SOCKET_ERROR)
        return SOCKET_ERROR;

    // Add anything already in the Ciphertext buffer?
    if (SecurityLayer->CipherCache.cbBuffer) {
        *Available += SecurityLayer->CipherCache.cbBuffer;
    }

    // But that includes overhead, so subtract that.
    ProtoOverhead = SecurityLayer->StreamSizes.cbHeader
        + SecurityLayer->StreamSizes.cbTrailer;

    // If there's data left, that's what is available.
    if (*Available >= ProtoOverhead) {
        *Available -= ProtoOverhead;
    } else {
        *Available = 0;
    }

    return 0;
}
