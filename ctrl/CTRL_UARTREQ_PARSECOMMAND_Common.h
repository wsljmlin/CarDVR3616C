#ifndef __CTRL_UARTREQ_PARSECOMMAND_COMMON_H__
#define __CTRL_UARTREQ_PARSECOMMAND_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************
	include
*********************************************************/
#include "comm_common.h"
#include "ctrl_uartReq_saveReq.h"

/*********************************************************
	macro / enum
*********************************************************/
#define CTRL_UARTREQ_PARSECOMMAND_REQMAX 128
#define CTRL_UARTREQ_PARSECOMMAND_ERRORMAX 100

typedef enum {
	CTRL_UARTREQ_PARSECOMMAND_Function_Success,
	CTRL_UARTREQ_PARSECOMMAND_Function_Fail,
	CTRL_UARTREQ_PARSECOMMAND_Function_Time,
	CTRL_UARTREQ_PARSECOMMAND_Function_Inter,
	CTRL_UARTREQ_PARSECOMMAND_Function_Wifi,
	CTRL_UARTREQ_PARSECOMMAND_Function_Quit,
	CTRL_UARTREQ_PARSECOMMAND_Function_Empty,
	CTRL_UARTREQ_PARSECOMMAND_Function_Led,
	CTRL_UARTREQ_PARSECOMMAND_Function_Snap,
	CTRL_UARTREQ_PARSECOMAMND_Function_ErrorCommand,
	CTRL_UARTREQ_PARSECOMMAND_Function_FeedbackError,
	CTRL_UARTREQ_PARSECOMMAND_Function_NotConfigYet,
	CTRL_UARTREQ_PARSECOMMAND_Function_InterDelterTime,
#ifdef INTER_CMD_REDEFINE
	CTRL_UARTREQ_PARSECOMMAND_Function_GPSformat,
#endif
} eCTRL_UARTREQ_PARSECOMMAND_Function_Ret;

typedef enum {
	CTRL_UARTREQ_PARSECOMMAND_STATUS_NotConfig,
	CTRL_UARTREQ_PARSECOMMAND_STATUS_Configed,
} eCTRL_UARTREQ_PARSECOMMAND_Status;

/*********************************************************
	struct
*********************************************************/
typedef struct {
	UCHAR ucCommandKey;
	UCHAR* pucCommandFirst;
	LONG lCommandLen;
	sCTRL_UARTREQ_SAVEREQ_Info* psReqSaver;
	eCTRL_UARTREQ_PARSECOMMAND_Status eStatus;
	eCTRL_UARTREQ_PARSECOMMAND_Function_Ret eRet;
} sCTRL_UARTREQ_PARSECOMMAND_ThreadArg;

/*********************************************************
	variable
*********************************************************/

/*********************************************************
	prototype
*********************************************************/
LONG lNUCharToLong(UCHAR* pucHeader, LONG lPosition, LONG lCount);
void vGetCharContent(UCHAR* pucHeader, LONG lPosition, LONG lCount, CHAR* pcContent);
void vGetFeedback(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psInfo, CHAR* pcFeedback);

#ifdef __cplusplus
}
#endif

#endif
