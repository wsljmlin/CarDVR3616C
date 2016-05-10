#ifndef __CTRL_UARTREQ_PARSECOMMAND_H__
#define __CTRL_UARTREQ_PARSECOMMAND_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************
	include
*********************************************************/
#include "comm_common.h"
#include "CTRL_UARTREQ_PARSECOMMAND_Common.h"

/*********************************************************
	macro / enum
*********************************************************/
typedef enum {
	CTRL_UARTREQ_PARSECOMMAND_SUCCESS,
	CTRL_UARTREQ_PARSECOMMAND_FAIL,
	CTRL_UARTREQ_PARSECOMMAND_FUNCPARSE,
	CTRL_UARTREQ_PARSECOMMAND_COMMANDNOTFOUND,
	CTRL_UARTREQ_PARSECOMMAND_PRECONDITION,
} eCTRL_UARTREQ_PARSECOMMAND_Ret;

typedef void(* vFunc)(void*);

/*********************************************************
	struct
*********************************************************/
typedef struct {
	UCHAR* pucCommand;
	UCHAR* pucFeedback;
	sCTRL_UARTREQ_SAVEREQ_Info* psReqSaver;
	vFunc vCommandHandleFunc;
	sCTRL_UARTREQ_PARSECOMMAND_ThreadArg sFuncArg; 
} sCTRL_UARTREQ_PARSECOMMAND_Info;

/*********************************************************
	variable
*********************************************************/

/*********************************************************
	prototype
*********************************************************/
eCTRL_UARTREQ_PARSECOMMAND_Ret eCTRL_UARTREQ_PARSECOMMAND_HandleCommand(sCTRL_UARTREQ_PARSECOMMAND_Info* psInfo, UCHAR* pucCommand, UCHAR* pucFeedback, sCTRL_UARTREQ_SAVEREQ_Info* psReqSaver);

#ifdef __cplusplus
}
#endif

#endif
