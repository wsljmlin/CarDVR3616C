#ifndef __CTRL_UART_GETCOMMAND_H__
#define __CTRL_UART_GETCOMMAND_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************
	include
*********************************************************/
#include "comm_interface.h"

/*********************************************************
	macro / enum
*********************************************************/
typedef enum {
	CTRL_UART_GETCOMMAND_SUCCESS,
	CTRL_UART_GETCOMMAND_FAIL,
	CTRL_UART_GETCOMMAND_MALLOC,
	CTRL_UART_GETCOMMAND_THREAD,
	CTRL_UART_GETCOMMAND_NULL,
	CTRL_UART_GETCOMMAND_SHORTCOMMAND,
	CTRL_UART_GETCOMMAND_SYNTAX,
	CTRL_UART_GETCOMMAND_LENGTH,
	CTRL_UART_GETCOMMAND_ADDCHECK,
	CTRL_UART_GETCOMMAND_XORCHECK,
	CTRL_UART_GETCOMMAND_FEEDBACKERROR,
	CTRL_UART_GETCOMMAND_GETCONFIGMESSAGE,
	CTRL_UART_GETCOMMAND_NOPREFIX,
	CTRL_UART_GETCOMMAND_NOPOSTFIX,
} eCTRL_UART_GETCOMMAND_Ret;

typedef enum {
	CTRL_UART_GETCOMMAND_Status_End = 0x0,
	CTRL_UART_GETCOMMAND_Status_NotEnd = 0x1,
	CTRL_UART_GETCOMMAND_Status_Configed = 0x10,
} eCTRL_UART_GETCOMMAND_Status;

/*********************************************************
	struct
*********************************************************/
typedef struct {
	sCOMM_QUEUE* psCommandQueue;
	sCOMM_QUEUE* psFeedbackQueue;
	sCOMM_DD_UART_Info* psUart;
	UCHAR* pucPayLoad;
	UCHAR* pucCommand;
	UCHAR* pucFeedBack;
	eCTRL_UART_GETCOMMAND_Status* peStatus;
} sCTRL_UART_GETCOMMAND_ThreadArg;

typedef struct {
	sCOMM_QUEUE* psCommandQueue;
	sCOMM_QUEUE* psFeedbackQueue;
	sCOMM_DD_UART_Info* psUart;
	UCHAR* pucPayLoad;
	UCHAR* pucCommand;
	UCHAR* pucFeedBack;
	eCTRL_UART_GETCOMMAND_Status eStatus;
	sCTRL_UART_GETCOMMAND_ThreadArg* psThreadArg;
	pthread_t tid;
} sCTRL_UART_GETCOMMAND_Info;

/*********************************************************
	variable
*********************************************************/

/*********************************************************
	prototype
*********************************************************/
eCTRL_UART_GETCOMMAND_Ret eCTRL_UART_getCommand_Init(sCTRL_UART_GETCOMMAND_Info* psInfo, sCOMM_QUEUE* psCommandQueue, sCOMM_QUEUE* psFeedbackQueue);
eCTRL_UART_GETCOMMAND_Ret eCTRL_UART_getCommand_Deinit(sCTRL_UART_GETCOMMAND_Info* psInfo);
eCTRL_UART_GETCOMMAND_Ret eCTRL_UART_getCommand_StartThread(sCTRL_UART_GETCOMMAND_Info* psInfo);
eCTRL_UART_GETCOMMAND_Ret eCTRL_UART_getCommand_StopThread(sCTRL_UART_GETCOMMAND_Info* psInfo);


#ifdef __cplusplus
}
#endif

#endif
