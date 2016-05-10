#ifndef __NET_COM_H__
#define __NET_COM_H__


#ifdef __cplusplus
extern "C"{
#endif

#include "comm_common.h"

typedef struct {
	/* socket fd */
	LONG lSockFd;
	/* server port */
	LONG lServerPort;
	/* type, server or client */
	LONG lType;
} sCOMM_NET;

typedef enum {
	COMM_NET_SERVER,
	COMM_NET_CLIENT
} eCOMM_NET_TYPE;

typedef enum {
	COMM_NET_SUCCESS,
	COMM_NET_FAILURE,
	COMM_NET_SOCKETINVALID,
} eCOMM_NET_RETURN;

/* funtion declare */

/*------------------------------------------------------------------------------*/
/*!
    @brief			init the socket of the server
    @param			sCOMM_NET* psNet		[out]				the pointer struct of sCOMM_NET
    @param			LONG lPort				[in]					the port
    @param			LONG lType				[in]					COMM_NET_SERVER or COMM_NET_CLIENT
    @param			LONG lMaxCnt			[in]					max count connect
 
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_Net_Init(sCOMM_NET* psNet, LONG lPort, LONG lType, LONG lMaxCnt);

/*------------------------------------------------------------------------------*/
/*!
    @brief			connect to the server
    @param			sCOMM_NET* psNet		[in]				the pointer struct of sCOMM_NET
    @param			CHAR* pcIp				[in]				the server ip address

*/
/* -----------------------------------------------------------------------------*/
void vCOMM_Net_Connect(sCOMM_NET* psNet, CHAR* pcIp);

/*------------------------------------------------------------------------------*/
/*!
    @brief			read from the socket
    @param			LONG lAcceptFd		[in]				the pointer struct of sCOMM_NET
    @param 		LONG lLen				[in]				the length to be read
    @param			CHAR* pcBuf			[out]			the buffer to be read
    @ret													COMM_NET_SUCCESS, or COMM_NET_FAILURE
 
*/
/* -----------------------------------------------------------------------------*/
LONG lCOMM_Net_Read(LONG lAcceptFd, CHAR* pcBuf, LONG lLen);

/*------------------------------------------------------------------------------*/
/*!
    @brief			write to the socket
    @param			LONG lAcceptFd		[in]				the pointer struct of sCOMM_NET
    @param			LONG lLen				[in]				the length to be write
    @param			CHAR* pcBuf			[out]			the buffer to be read
    @ret													COMM_NET_SUCCESS or COMM_NET_FAILURE
*/
/* -----------------------------------------------------------------------------*/
LONG vCOMM_Net_Write(LONG lAcceptFd, CHAR* pcBuf, LONG lLen);

/*------------------------------------------------------------------------------*/
/*!
    @brief			close the socket
    @param			sCOMM_NET* psNet		[in]				the pointer struct of sCOMM_NET
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_Net_DeInit(sCOMM_NET* psNet);

#ifdef __cplusplus
}
#endif

#endif /* __NET_COM_H__ */