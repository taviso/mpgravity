#ifndef __SOCKSSL_H
# define __SOCKSSL_H
#pragma once
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct _SEC_LAYER *PSEC_LAYER;

PSEC_LAYER AddSecurityLayer(LPCTSTR hostAddress, SOCKET Socket);
void RemoveSecureLayer(PSEC_LAYER SecurityLayer, SOCKET Socket);
LONG SecureSend(PSEC_LAYER SecurityLayer, SOCKET Socket, PVOID Buf, SIZE_T Size);
LONG SecureRecv(PSEC_LAYER SecurityLayer, SOCKET Socket, PVOID Buf, SIZE_T Size);
LONG SockAvailable(PSEC_LAYER SecurityLayer, SOCKET Socket, PULONG Available);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __SOCKSSL_H
