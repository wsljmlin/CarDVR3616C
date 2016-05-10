#ifndef __CTRL_UARTREQ_SAVEREQ_H__
#define __CTRL_UARTREQ_SAVEREQ_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************
	include
*********************************************************/
#include "comm_common.h"

/*********************************************************
	macro / enum
*********************************************************/
#define CTRL_UARTREQ_SAVEREQ_REQMAX 8
typedef enum {
	CTRL_UARTREQ_SAVEREQ_SUCCESS,
	CTRL_UARTREQ_SAVEREQ_FAIL,
	CTRL_UARTREQ_SAVEREQ_READEND,
	CTRL_UARTREQ_SAVEREQ_TOOMUCHREQ,
	CTRL_UARTREQ_SAVEREQ_FULL,
} eCTRL_UARTREQ_SAVEREQ_Ret;

/*********************************************************
	struct
*********************************************************/
typedef struct {
	UCHAR* pucRequestHolder;
	CHAR* pcRequest[CTRL_UARTREQ_SAVEREQ_REQMAX];
	CHAR* pcWriter;
	CHAR* pcTail;
	LONG lCurReadCnt;
	LONG lCurWriteCnt;
	LONG lRequestHolderMax;
} sCTRL_UARTREQ_SAVEREQ_Info;

/*********************************************************
	variable
*********************************************************/

/*********************************************************
	prototype
*********************************************************/
eCTRL_UARTREQ_SAVEREQ_Ret eCTRL_UARTREQ_SAVEREQ_Init(sCTRL_UARTREQ_SAVEREQ_Info* psInfo, UCHAR* pucRequestHolder, LONG lRequestHolderMax);
eCTRL_UARTREQ_SAVEREQ_Ret eCTRL_UARTREQ_SAVEREQ_Reset(sCTRL_UARTREQ_SAVEREQ_Info* psInfo);
eCTRL_UARTREQ_SAVEREQ_Ret eCTRL_UARTREQ_SAVEREQ_WriteReq(sCTRL_UARTREQ_SAVEREQ_Info* psInfo, CHAR* pcRequest);
eCTRL_UARTREQ_SAVEREQ_Ret eCTRL_UARTREQ_SAVEREQ_ReadReq(sCTRL_UARTREQ_SAVEREQ_Info* psInfo, CHAR** ppcRequest);
eCTRL_UARTREQ_SAVEREQ_Ret eCTRL_UARTREQ_SAVEREQ_Show(sCTRL_UARTREQ_SAVEREQ_Info* psInfo);

#ifdef __cplusplus
}
#endif

#endif
