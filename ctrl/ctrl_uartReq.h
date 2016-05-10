#ifndef __CTRL_UARTREQ_H__
#define __CTRL_UARTREQ_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************
	include
*********************************************************/
#include "comm_interface.h"
#include "ctrl_uartReq_saveReq.h"

/*********************************************************
	macro / enum
*********************************************************/
typedef enum {
	CTRL_UARTREQ_SUCCESS,
	CTRL_UARTREQ_FAIL,
	CTRL_UARTREQ_MALLOC,
	CTRL_UARTREQ_THREAD,
	CTRL_UARTREQ_HANDLECOMMAND,
	CTRL_UARTREQ_FEEDBACK,
} eCTRL_UARTREQ_Ret;

typedef enum {
	CTRL_UARTREQ_Status_IsEnd,
	CTRL_UARTREQ_Status_NotEnd,
} eCTRL_UARTREQ_Status;

/*********************************************************
	struct
*********************************************************/
typedef struct {
	sCOMM_QUEUE* psCommandQueue;
	sCOMM_QUEUE* psRequestQueue;
	sCOMM_QUEUE* psFeedbackQueue;
	UCHAR* pucCommand;
	UCHAR* pucRequest;
	UCHAR* pucFeedBack;
	sCTRL_UARTREQ_SAVEREQ_Info sSaveReq;
	eCTRL_UARTREQ_Status* peStatus;
} sCTRL_UARTREQ_ThreadArg;

typedef struct {
	sCOMM_QUEUE* psCommandQueue;
	sCOMM_QUEUE* psFeedbackQueue;
	sCOMM_QUEUE* psRequestQueue;
	UCHAR* pucPayLoad;
	eCTRL_UARTREQ_Status eStatus;
	sCTRL_UARTREQ_ThreadArg* psThreadArg;
	pthread_t tid;
} sCTRL_UARTREQ_Info;

/*********************************************************
	variable
*********************************************************/

/*********************************************************
	prototype
*********************************************************/
eCTRL_UARTREQ_Ret eCTRL_UARTREQ_Init(sCTRL_UARTREQ_Info* psInfo, sCOMM_QUEUE* psCommandQueue, sCOMM_QUEUE* psFeedbackQueue, sCOMM_QUEUE* psRequestQueue);
eCTRL_UARTREQ_Ret eCTRL_UARTREQ_Deinit(sCTRL_UARTREQ_Info* psInfo);
eCTRL_UARTREQ_Ret eCTRL_UARTREQ_StartThread(sCTRL_UARTREQ_Info* psInfo);
eCTRL_UARTREQ_Ret eCTRL_UARTREQ_StopThread(sCTRL_UARTREQ_Info* psInfo);

#ifdef __cplusplus
}
#endif

#endif
