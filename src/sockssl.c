#ifndef SECURITY_WIN32
# define SECURITY_WIN32
#endif
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock.h>
#include <wincrypt.h>
#include <wintrust.h>
#include <schannel.h>
#include <security.h>
#include <sspi.h>

#undef NDEBUG
#include <assert.h>

#include "sockssl.h"

#define IO_BUFFER_SIZE 0x10000

typedef struct _SEC_LAYER {
    CredHandle Credentials;
    CtxtHandle Context;
    HANDLE Heap;
    DWORD RecvOffset;
    SecBuffer CipherCache;
    SecBuffer PlainCache;
    SecPkgContext_StreamSizes StreamSizes;
    // Dont need to save WSAError, because the system does.
    DWORD LastSecError;
} SEC_LAYER, *PSEC_LAYER;

// I dunno if these are all necessary.
static const kSSPIFlags = ISC_REQ_SEQUENCE_DETECT
                        | ISC_REQ_REPLAY_DETECT
                        | ISC_REQ_CONFIDENTIALITY
                        | ISC_RET_EXTENDED_ERROR
                        | ISC_REQ_ALLOCATE_MEMORY
                        | ISC_REQ_STREAM;

static LONG DisconnectFromServer(SOCKET Socket, PCredHandle phCreds, CtxtHandle * phContext)
{
    PBYTE pbMessage;
    DWORD dwType, dwSSPIOutFlags, cbMessage, cbData, Status;
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
    dwType = SCHANNEL_SHUTDOWN;

    if (ApplyControlToken(phContext, &OutBuffer) != SEC_E_OK) {
        printf("ApplyControlToken Failed\n");
        goto cleanup;
    }

    // Build an SSL close notify message.
    OutBuffers[0].pvBuffer   = NULL;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer   = 0;

    Status = InitializeSecurityContextA(phCreds,
                                        phContext,
                                        NULL,
                                        kSSPIFlags,
                                        0,
                                        0,
                                        NULL,
                                        0,
                                        phContext,
                                        &OutBuffer,
                                        &dwSSPIOutFlags,
                                        NULL);

    if (FAILED(Status)) {
        printf("**** Error 0x%x returned by InitializeSecurityContext\n", Status);
        goto cleanup;
    }

    pbMessage = OutBuffers[0].pvBuffer;
    cbMessage = OutBuffers[0].cbBuffer;

    // Send the close notify message to the server.
    if (pbMessage != NULL && cbMessage != 0) {
        // XXX NEED TO HANDLE EINTR
        cbData = send(Socket, pbMessage, cbMessage, 0);
        if(cbData == SOCKET_ERROR || cbData == 0)
        {
            Status = WSAGetLastError();
                        printf("**** Error %d sending close notify\n", Status);
            goto cleanup;
        }
        printf("Sending Close Notify\n");
        printf("%d bytes of handshake data sent\n", cbData);
        FreeContextBuffer(pbMessage);
    }

cleanup:
    DeleteSecurityContext(phContext);

    return Status;
}


static SECURITY_STATUS ClientHandshakeLoop(PSEC_LAYER      SecLayer,
                                           SOCKET          Socket,
                                           PCredHandle     phCreds,
                                           CtxtHandle *    phContext,
                                           BOOL            fDoInitialRead,
                                           SecBuffer *     pExtraData)
{
    SecBufferDesc   OutBuffer, InBuffer;
    SecBuffer       InBuffers[2], OutBuffers[1];
    DWORD           dwSSPIOutFlags, cbData, cbIoBuffer;
    SECURITY_STATUS scRet;
    PUCHAR          IoBuffer;
    BOOL            fDoRead;

    // Allocate data buffer.
    IoBuffer = HeapAlloc(SecLayer->Heap, 0, IO_BUFFER_SIZE);
    cbIoBuffer = 0;
    fDoRead = fDoInitialRead;

    // Loop until the handshake is finished or an error occurs.
    scRet = SEC_I_CONTINUE_NEEDED;

    while( scRet == SEC_I_CONTINUE_NEEDED        ||
           scRet == SEC_E_INCOMPLETE_MESSAGE     ||
           scRet == SEC_I_INCOMPLETE_CREDENTIALS )
   {
        if(0 == cbIoBuffer || scRet == SEC_E_INCOMPLETE_MESSAGE) // Read data from server.
        {
            if(fDoRead)
            {
                cbData = recv(Socket, IoBuffer + cbIoBuffer, IO_BUFFER_SIZE - cbIoBuffer, 0 );
                if(cbData == SOCKET_ERROR)
                {
                    printf("**** Error %d reading data from server\n", WSAGetLastError());
                    scRet = SEC_E_INTERNAL_ERROR;
                    break;
                }
                else if(cbData == 0)
                {
                    printf("**** Server unexpectedly disconnected\n");
                    scRet = SEC_E_INTERNAL_ERROR;
                    break;
                }
                printf("%d bytes of handshake data received\n", cbData);
                cbIoBuffer += cbData;
            }
            else
              fDoRead = TRUE;
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
        if(scRet == SEC_E_OK                ||
           scRet == SEC_I_CONTINUE_NEEDED   ||
           FAILED(scRet) && (dwSSPIOutFlags & ISC_RET_EXTENDED_ERROR))
        {
            if(OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL)
            {
                // XXX NEED TO HANDLE EINTR
                cbData = send(Socket, OutBuffers[0].pvBuffer, OutBuffers[0].cbBuffer, 0 );
                if(cbData == SOCKET_ERROR || cbData == 0)
                {
                    printf( "**** Error %d sending data to server (2)\n",  WSAGetLastError() );
                    FreeContextBuffer(OutBuffers[0].pvBuffer);
                    DeleteSecurityContext(phContext);
                    return SEC_E_INTERNAL_ERROR;
                }
                printf("%d bytes of handshake data sent\n", cbData);

                // Free output buffer.
                FreeContextBuffer(OutBuffers[0].pvBuffer);
                OutBuffers[0].pvBuffer = NULL;
            }
        }



        // If InitializeSecurityContext returned SEC_E_INCOMPLETE_MESSAGE,
        // then we need to read more data from the server and try again.
        if(scRet == SEC_E_INCOMPLETE_MESSAGE) continue;


        // If InitializeSecurityContext returned SEC_E_OK, then the
        // handshake completed successfully.
        if(scRet == SEC_E_OK)
        {
            // If the "extra" buffer contains data, this is encrypted application
            // protocol layer stuff. It needs to be saved. The application layer
            // will later decrypt it with DecryptMessage.
            printf("Handshake was successful\n");

            if(InBuffers[1].BufferType == SECBUFFER_EXTRA)
            {
                pExtraData->pvBuffer = HeapAlloc(SecLayer->Heap, 0, InBuffers[1].cbBuffer);
                MoveMemory( pExtraData->pvBuffer,
                            IoBuffer + (cbIoBuffer - InBuffers[1].cbBuffer),
                            InBuffers[1].cbBuffer );

                pExtraData->cbBuffer   = InBuffers[1].cbBuffer;
                pExtraData->BufferType = SECBUFFER_TOKEN;

                printf( "%d bytes of app data was bundled with handshake data\n", pExtraData->cbBuffer );
            }
            else
            {
                pExtraData->pvBuffer   = NULL;
                pExtraData->cbBuffer   = 0;
                pExtraData->BufferType = SECBUFFER_EMPTY;
            }
            break; // Bail out to quit
        }



        // Check for fatal error.
        if(FAILED(scRet)) { printf("**** Error 0x%x returned by InitializeSecurityContext (2)\n", scRet); break; }

        // If InitializeSecurityContext returned SEC_I_INCOMPLETE_CREDENTIALS,
        // then the server just requested client authentication.
        if(scRet == SEC_I_INCOMPLETE_CREDENTIALS)
        {
            break;
        }

        // Copy any leftover data from the "extra" buffer, and go around again.
        if ( InBuffers[1].BufferType == SECBUFFER_EXTRA )
        {
            MoveMemory( IoBuffer, IoBuffer + (cbIoBuffer - InBuffers[1].cbBuffer), InBuffers[1].cbBuffer );
            cbIoBuffer = InBuffers[1].cbBuffer;
        }
        else
          cbIoBuffer = 0;
    }

    // Delete the security context in the case of a fatal error.
    if(FAILED(scRet)) DeleteSecurityContext(phContext);

    HeapFree(SecLayer->Heap, 0, IoBuffer);
    return scRet;
}


static SECURITY_STATUS PerformClientHandshake(PSEC_LAYER      SecLayer,
                                              SOCKET          Socket,        // in
                                              LPSTR           pszServerName, // in
                                              PCredHandle     phCreds,
                                              CtxtHandle *    phContext,     // out
                                              SecBuffer *     pExtraData)   // out
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
        printf("**** Error %d returned by InitializeSecurityContext (1)\n", scRet);
        return scRet;
    }

    // Send response to server if there is one.
    if (OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL)
    {
        // XXX NEED TO HANDLE EINTR
        cbData = send(Socket, OutBuffers[0].pvBuffer, OutBuffers[0].cbBuffer, 0);

        if (cbData == SOCKET_ERROR || cbData == 0)
        {
            printf("**** Error %d sending data to server (1)\n", WSAGetLastError());
            FreeContextBuffer(OutBuffers[0].pvBuffer);
            DeleteSecurityContext(phContext);
            return SEC_E_INTERNAL_ERROR;
        }

        printf("%d bytes of handshake data sent\n", cbData);
        FreeContextBuffer(OutBuffers[0].pvBuffer); // Free output buffer.
        OutBuffers[0].pvBuffer = NULL;
    }

    return ClientHandshakeLoop(SecLayer, Socket, phCreds, phContext, TRUE, pExtraData);
}



static DWORD EncryptSend(SOCKET Socket,
                         CtxtHandle * phContext,
                         PBYTE pbIoBuffer,
                         SecPkgContext_StreamSizes Sizes,
                         DWORD cbMessage)
{
    SECURITY_STATUS scRet;
    SecBufferDesc Message;
    SecBuffer Buffers[4];
    DWORD cbData;
    PBYTE pbMessage;


    pbMessage = pbIoBuffer + Sizes.cbHeader; // Offset by "header size"

    printf("Sending %d bytes of plaintext.\n", cbMessage);

    Buffers[0].pvBuffer     = pbIoBuffer;              // Pointer to buffer 1
    Buffers[0].cbBuffer     = Sizes.cbHeader;          // length of header
    Buffers[0].BufferType   = SECBUFFER_STREAM_HEADER; // Type of the buffer

    Buffers[1].pvBuffer     = pbMessage;               // Pointer to buffer 2
    Buffers[1].cbBuffer     = cbMessage;               // length of the message
    Buffers[1].BufferType   = SECBUFFER_DATA;          // Type of the buffer

    Buffers[2].pvBuffer     = pbMessage + cbMessage;   // Pointer to buffer 3
    Buffers[2].cbBuffer     = Sizes.cbTrailer;         // length of the trailor
    Buffers[2].BufferType   = SECBUFFER_STREAM_TRAILER;// Type of the buffer

        Buffers[3].pvBuffer     = SECBUFFER_EMPTY;     // Pointer to buffer 4
    Buffers[3].cbBuffer     = SECBUFFER_EMPTY;         // length of buffer 4
    Buffers[3].BufferType   = SECBUFFER_EMPTY;         // Type of the buffer 4


    Message.ulVersion       = SECBUFFER_VERSION;        // Version number
    Message.cBuffers        = 4;                        // Number of buffers - must contain four SecBuffer structures.
    Message.pBuffers        = Buffers;                  // Pointer to array of buffers

    scRet = EncryptMessage(phContext, 0, &Message, 0);  // must contain four SecBuffer structures.

    if (FAILED(scRet)) {
        printf("**** Error 0x%x returned by EncryptMessage\n", scRet);
        return scRet;
    }

    // Send the encrypted data to the server.
    // XXX NEED TO HANDLE EINTR
    cbData = send(Socket,
                  pbIoBuffer,
                  Buffers[0].cbBuffer + Buffers[1].cbBuffer + Buffers[2].cbBuffer,
                  0);

    printf("%d bytes of encrypted data sent\n", cbData);

    // Subtract header and trailer size.
    if (cbData != Buffers[0].cbBuffer + Buffers[1].cbBuffer + Buffers[2].cbBuffer) {
        printf("FIXME: send truncated, this probably wont work");
    }

    return cbMessage;

}

static LONG FillRequestFromCache(PSEC_LAYER SecLayer, PBYTE Buf, DWORD Size)
{
    printf("Plaintext cache @%lu bytes.\n", SecLayer->PlainCache.cbBuffer);

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

    printf("SecureRecv() => %#x bytes from %#x into %p\n", Size, Socket, Buf);

    if (!SecLayer || Socket == INVALID_SOCKET) {
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
            printf("Resuming an interrupted operation with %d bytes\n",
                   RecvOffset);
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

            printf("Attempt %d to refill ciphertext cache\n", Refills);

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

            printf("%d bytes of application data received\n", AmountRead);

            // Even if there was en error, attempt a decrypt operation.
            // The reason is that we may have been blocking and Gravity woke us
            // up from a different thread with WSACancelBlocking(), and I may
            // have overestimated the read size.
            if (AmountRead <= 0) {
                printf("Error %d from recv()\n", WSAGetLastError());
                return AmountRead;
            }

            // Okay, trim off any excess cache buffer.
            SecLayer->CipherCache.cbBuffer = RecvOffset += AmountRead;

          resume:

            // Now setup the buffers to attempt a decryption.
            ZeroMemory(Buffers, sizeof Buffers);
            CopyMemory(Buffers, &SecLayer->CipherCache, sizeof(Buffers[0]));

            scRet = DecryptMessage(&SecLayer->Context, &Message, 0, NULL);

            printf("DecryptMessage() returned %#x\n", scRet);

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

                printf("There are %u bytes still in ciphertext\n", ExtraBytes);

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

                printf("CipherCache now contains %d\n", ExtraBytes);

                // Now break to let the plaintext cache handle the request.
                break;
            }

            // More data is required to decrypt the last message. We can find
            // out how much data is needed by looking for SECBUFFER_MISSING
            // buffers.
            if (scRet == SEC_E_INCOMPLETE_MESSAGE) {
                DWORD MissingBytes = 0;

                printf("We need more ciphertext to make progress\n");

                for (unsigned long i = 0; i < Message.cBuffers; i++) {
                    // The amount needed is Message.pBuffers[i].cbBuffer
                    if (Message.pBuffers[i].BufferType == SECBUFFER_MISSING) {
                        assert(Message.pBuffers[i].cbBuffer);
                        MissingBytes = Message.pBuffers[i].cbBuffer;
                        break;
                    }
                }

                SecLayer->CipherCache.cbBuffer += MissingBytes;

                printf("System says we need %d more bytes\n", MissingBytes);

                // Try another recv now that we know the size required.
                continue;
            }

            // XXX: If this happens, there's some memory management error.
            // this is a bug in my code.
            if (scRet == SEC_E_INVALID_TOKEN) {
                printf("This usually means the buffers are not correct.\n");
                __debugbreak();
                return SOCKET_ERROR;
            }

            // Either there was a network error, an attack, or a bug
            // in my code managing ciphertext.
            if (scRet == SEC_E_DECRYPT_FAILURE) {
                printf("System reports that message decryption failed\n");
                __debugbreak();
                // FIXME: Maybe invalidate the cache?
                return SOCKET_ERROR;
            }

            // FIXME: I've never seen this happen, but apparently it can.
            if (scRet == SEC_I_CONTEXT_EXPIRED) {
                printf("SEC_I_CONTEXT_EXPIRED, what should I do here.\n");
                // Is there a SECBUFFER_EXTRA to tell me where to clear to?
                __debugbreak();
                return 0;
            }

            // Server wants to renegotiate, we may need to shuffle
            // SECBUFFER_EXTRA buffers around.
            // FIXME: This probably doesnt handle WSAEINTR yet.
            if (scRet == SEC_I_RENEGOTIATE) {
                printf("Server requested renegotiate!\n");

                // untested, need to fix.
                __debugbreak();

                for (unsigned long i = 0; i < Message.cBuffers; i++) {
                    if (Message.pBuffers[i].BufferType == SECBUFFER_EXTRA) {
                        scRet = ClientHandshakeLoop(SecLayer,
                                                    Socket,
                                                    &SecLayer->Credentials,
                                                    &SecLayer->Context,
                                                    FALSE,
                                                    &Message.pBuffers[i]);
                        if (scRet != SEC_E_OK) {
                            return SOCKET_ERROR;
                        }

                        // Do I really need to do this?
                        // if yes, need to copy whatever is left into cache.a
                        // FIXME: yes, i think i do.
                        assert(Message.pBuffers[i].pvBuffer == NULL);

                        //if (ExtraBuffer.pvBuffer)
                        //{
                        //    MoveMemory(pbIoBuffer, ExtraBuffer.pvBuffer, ExtraBuffer.cbBuffer);
                        //    cbIoBuffer = ExtraBuffer.cbBuffer;
                        //}

                        break;
                    }

                }

                // Try another recv now that worked.
                continue;
            }

            // FIXME: handle all the documented return codes.
            printf("unhandled scRet %#x\n", scRet);
            __debugbreak();
            return SOCKET_ERROR;
        }
    }

    // Unreachable.
    __debugbreak();
    return SOCKET_ERROR;
}

LONG SecureSend(PSEC_LAYER SecurityLayer, SOCKET Socket, PVOID Buf, SIZE_T Size)
{
    SecPkgContext_StreamSizes Sizes;
    SECURITY_STATUS Status;
    PBYTE pbIoBuffer;
    DWORD cbIoBufferLength, cbData;

    printf("SecureSend(%p, %lu, %p, %lu);\n", SecurityLayer, Socket, Buf, Size);

    if (!SecurityLayer) {
        return SOCKET_ERROR;
    }

    Status = QueryContextAttributes(&SecurityLayer->Context,
                                             SECPKG_ATTR_STREAM_SIZES,
                                             &Sizes);

    if (Status != SEC_E_OK) {
        printf("**** Error 0x%x reading SECPKG_ATTR_STREAM_SIZES\n", Status);
        return -1;
    }

    if (Size > Sizes.cbMaximumMessage)
        Size = Sizes.cbMaximumMessage;

    // Create a buffer.
    cbIoBufferLength = Sizes.cbHeader + Size + Sizes.cbTrailer;
    pbIoBuffer       = HeapAlloc(SecurityLayer->Heap, 0, cbIoBufferLength);

    memcpy(pbIoBuffer+Sizes.cbHeader, Buf, Size);

    cbData = EncryptSend(Socket, &SecurityLayer->Context, pbIoBuffer, Sizes, Size);

    HeapFree(SecurityLayer->Heap, 0, pbIoBuffer);

    return cbData;
}

PSEC_LAYER AddSecurityLayer(LPCTSTR HostAddress, SOCKET Socket) {
    SecBuffer  ExtraData;
    SECURITY_STATUS Status;
    PSEC_LAYER SecurityLayer;
    TimeStamp Lifetime;
    SCHANNEL_CRED SchannelCreds = {
        .dwVersion = SCHANNEL_CRED_VERSION,
    };

    SecurityLayer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof *SecurityLayer);

    if (!SecurityLayer) {
        printf("Memory allocation failure\n");
        return NULL;
    }

    // Create a heap to make cleanup easier.
    SecurityLayer->Heap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 0, 0);

    SecurityLayer->PlainCache.BufferType  = SECBUFFER_DATA;
    SecurityLayer->CipherCache.BufferType = SECBUFFER_DATA;

    // Create the buffers used for decrypting messages.
    SecurityLayer->PlainCache.pvBuffer = HeapAlloc(SecurityLayer->Heap,
                                                   HEAP_ZERO_MEMORY,
                                                   0);
    SecurityLayer->CipherCache.pvBuffer = HeapAlloc(SecurityLayer->Heap,
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
        printf("AcquireCredentialsHandle() failed, %#x\n", Status);
        goto error;
    }

    if (PerformClientHandshake(SecurityLayer,
                               Socket,
                               HostAddress,
                               &SecurityLayer->Credentials,
                               &SecurityLayer->Context,
                               &ExtraData) != 0) {
        printf("Perform handshake failed\n");
        goto error;
    }

    Status = QueryContextAttributes(&SecurityLayer->Context,
                                    SECPKG_ATTR_STREAM_SIZES,
                                    &SecurityLayer->StreamSizes);

    if (Status != SEC_E_OK) {
        printf("Failed to query sizes\n");
        goto error;
    }

    return SecurityLayer;

  error:
    DeleteSecurityContext(&SecurityLayer->Context);
    HeapDestroy(SecurityLayer->Heap);
    HeapFree(GetProcessHeap(), 0, SecurityLayer);
    return NULL;
}

void RemoveSecureLayer(PSEC_LAYER SecurityLayer, SOCKET Socket)
{
    if (SecurityLayer == NULL)
        return;

    // Send a close_notify alert to the server
    DisconnectFromServer(Socket, &SecurityLayer->Credentials, &SecurityLayer->Context);
    DeleteSecurityContext(&SecurityLayer->Context);
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
