#ifndef __COMM_POOL_PAYLOAD_H__
#define __COMM_POOL_PAYLOAD_H__

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "comm_common.h"

/***************************************************
	macro / enum
***************************************************/
typedef enum {
	COMM_POOL_PAYLOAD_SUCCESS,
	COMM_POOL_PAYLOAD_RESOURCEERROR,
	COMM_POOL_PAYLOAD_TYPENOTSUPPORT,
	COMM_POOL_PAYLOAD_FAIL,
} eCOMM_POOL_PAYLOAD_Ret;

typedef enum {
	COMM_POOL_PAYLOAD_INVALID = 0x1,
	COMM_POOL_PAYLOAD_NORMAL = 0x2,
	COMM_POOL_PAYLOAD_FULL = 0x4,
	COMM_POOL_PAYLOAD_EMPTY = 0x8,
	COMM_POOL_PAYLOAD_WRAP = 0x10,
} eCOMM_POOL_PAYLOAD_Status;

/***************************************************
	struct
***************************************************/
typedef struct {
	UCHAR* pucHeader;
	UCHAR* pucTail;
	LONG lPayLoadLength;
	LONG lUserExpandLength;
	UCHAR* pucWriter;
	UCHAR* pucReader;
	pthread_mutex_t mutPayload;
	pthread_cond_t condWt;
	pthread_cond_t condRd;
	eCOMM_POOL_PAYLOAD_Status eStatus;
} sCOMM_POOL_PAYLOAD_Info;

/***************************************************
	variable
***************************************************/

/***************************************************
	prototype
***************************************************/
eCOMM_POOL_PAYLOAD_Ret eCOMM_POOL_PAYLOAD_Init(sCOMM_POOL_PAYLOAD_Info* psInfo, LONG lPayloadLen, LONG lUserExpandLength);
eCOMM_POOL_PAYLOAD_Ret eCOMM_POOL_PAYLOAD_Write(sCOMM_POOL_PAYLOAD_Info* psInfo, UCHAR* pucSrc, LONG lWtLen);
eCOMM_POOL_PAYLOAD_Ret eCOMM_POOL_PAYLOAD_Read(sCOMM_POOL_PAYLOAD_Info* psInfo, UCHAR* pucSrc, LONG lRdLen);
eCOMM_POOL_PAYLOAD_Ret eCOMM_POOL_PAYLOAD_Deinit(sCOMM_POOL_PAYLOAD_Info* psInfo);
eCOMM_POOL_PAYLOAD_Ret eCOMM_POOL_PAYLOAD_GetWriter(sCOMM_POOL_PAYLOAD_Info* psInfo, UCHAR** ppucWt);
eCOMM_POOL_PAYLOAD_Ret eCOMM_POOL_PAYLOAD_WriteExtraData(sCOMM_POOL_PAYLOAD_Info* psInfo, UCHAR* pucSrc, LONG lWtLen);

#ifdef __cplusplus
}
#endif

#endif
