#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "ctrl_uartReq_parseCommand.h"
#include "CTRL_UARTREQ_PARSECOMMAND_TIME.h"
#include "CTRL_UARTREQ_PARSECOMMAND_EMPTY.h"
#include "CTRL_UARTREQ_PARSECOMMAND_INTER.h"
#include "CTRL_UARTREQ_PARSECOMMAND_QUIT.h"
#include "CTRL_UARTREQ_PARSECOMMAND_WIFI.h"
#include "CTRL_UARTREQ_PARSECOMMAND_LED.h"
#include "CTRL_UARTREQ_PARSECOMMAND_SNAP.h"

#include <assert.h>

/***************************************************
	macro / enum
***************************************************/
//#define CTRL_UARTREQ_PARSECOMMAND_DEBUG
#ifdef CTRL_UARTREQ_PARSECOMMAND_DEBUG
#define CUP_Debug(fmt, arg...) fprintf(stdout, "[ CUP ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CUP_Debug(fmt, arg...)
#endif

#define CUP_FuncIn() CUP_Debug("in\n")
#define CUP_FuncOut() CUP_Debug("out\n")
#define CUP_iVal(iVal) CUP_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CUP_lVal(lVal) CUP_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CUP_PtVal(ptVal) CUP_Debug("pt %s @ %p\n", #ptVal, ptVal)

typedef enum {
	CTRL_UARTREQ_PARSECOMMAND_ErrorStatus_NoError,
	CTRL_UARTREQ_PARSECOMMAND_ErrorStatus_CommandError,
	CTRL_UARTREQ_PARSECOMMAND_ErrorStatus_FunctionError,
} eCTRL_UARTREQ_PARSECOMMAND_ErrorStatus;

typedef enum {
	CTRL_UARTREQ_PARSECOMMAND_Type,
	CTRL_UARTREQ_PARSECOMMAND_FirstCommand,
} eCTRL_UARTREQ_PARSECOMMAND_Entry;

#define CTRL_UARTREQ_PARSECOMMAND_FUNCTIONMAX 47
#define CTRL_UARTREQ_PARSECOMMAND_GETARRAYSIZE(array) (sizeof(array) / sizeof((array)[0]))
#define CTRL_UARTREQ_PARSECOMMAND_GETFUNCTION(func) (pvCTRL_UARTREQ_PARSECOMMAND_ ## func)

/***************************************************
	struct
***************************************************/
typedef struct {
	UCHAR ucKey;
	vFunc pvFunction;
} sCTRL_UARTREQ_PARSECOMMAND_CommandEntry;

/***************************************************
	variable
***************************************************/
sCTRL_UARTREQ_PARSECOMMAND_CommandEntry sCommandSet[] = {
 	{'C', CTRL_UARTREQ_PARSECOMMAND_GETFUNCTION(TIME)},
 	{'I', CTRL_UARTREQ_PARSECOMMAND_GETFUNCTION(INTER)},
 	{'Q', CTRL_UARTREQ_PARSECOMMAND_GETFUNCTION(QUIT)},
 	{'E', CTRL_UARTREQ_PARSECOMMAND_GETFUNCTION(EMPTY)},
 	{'W', CTRL_UARTREQ_PARSECOMMAND_GETFUNCTION(WIFI)},
 	{'L', CTRL_UARTREQ_PARSECOMMAND_GETFUNCTION(LED)},
 	{'S', CTRL_UARTREQ_PARSECOMMAND_GETFUNCTION(SNAP)},
};

/***************************************************
	prototype
***************************************************/
eCTRL_UARTREQ_PARSECOMMAND_Ret eCTRL_UARTREQ_PARSECOMMAND_InitCommand(sCTRL_UARTREQ_PARSECOMMAND_Info* psInfo, UCHAR* pucCommand, UCHAR* pucFeedback, sCTRL_UARTREQ_SAVEREQ_Info* psReqSaver);
eCTRL_UARTREQ_PARSECOMMAND_Ret eCTRL_UARTREQ_PARSECOMMAND_GetCommandFunction(sCTRL_UARTREQ_PARSECOMMAND_Info* psInfo);

/***************************************************
	function
***************************************************/

/*********************************************
* func : eCTRL_UARTREQ_PARSECOMMAND_InitCommand(sCTRL_UARTREQ_PARSECOMMAND_Info* psInfo, UCHAR* pucCommand, sCTRL_UARTREQ_SAVEREQ_Info* psReqSaver)
* arg : sCTRL_UARTREQ_PARSECOMMAND_Info* psInfo, UCHAR* pucCommand, sCTRL_UARTREQ_SAVEREQ_Info* psReqSaver
* ret : eCTRL_UARTREQ_PARSECOMMAND_Ret
* note : init the command
*********************************************/
eCTRL_UARTREQ_PARSECOMMAND_Ret eCTRL_UARTREQ_PARSECOMMAND_InitCommand(sCTRL_UARTREQ_PARSECOMMAND_Info* psInfo, UCHAR* pucCommand, UCHAR* pucFeedback, sCTRL_UARTREQ_SAVEREQ_Info* psReqSaver) {
	eCTRL_UARTREQ_PARSECOMMAND_Ret eRet = CTRL_UARTREQ_PARSECOMMAND_SUCCESS;

	CUP_FuncIn();

	{
		psInfo->pucCommand = pucCommand;
		psInfo->pucFeedback = pucFeedback;
		psInfo->psReqSaver = psReqSaver;
		psInfo->vCommandHandleFunc = NULL;

		/* reset the request saver */
		eCTRL_UARTREQ_SAVEREQ_Reset(psInfo->psReqSaver);
	}

	CUP_FuncOut();

	return eRet;
}

/*********************************************
* func : eCTRL_UARTREQ_PARSECOMMAND_HandleCommand(sCTRL_UARTREQ_PARSECOMMAND_Info* psInfo, UCHAR* pucCommand, sCTRL_UARTREQ_SAVEREQ_Info* psReqSaver)
* arg : sCTRL_UARTREQ_PARSECOMMAND_Info* psInfo, UCHAR* pucCommand, sCTRL_UARTREQ_SAVEREQ_Info* psReqSaver
* ret : eCTRL_UARTREQ_PARSECOMMAND_Ret
* note :
*********************************************/
eCTRL_UARTREQ_PARSECOMMAND_Ret eCTRL_UARTREQ_PARSECOMMAND_HandleCommand(sCTRL_UARTREQ_PARSECOMMAND_Info* psInfo, UCHAR* pucCommand, UCHAR* pucFeedback, sCTRL_UARTREQ_SAVEREQ_Info* psReqSaver) {
	eCTRL_UARTREQ_PARSECOMMAND_Ret eRet = CTRL_UARTREQ_PARSECOMMAND_SUCCESS;
	sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg = &psInfo->sFuncArg;

	CUP_FuncIn();

	{
		eCTRL_UARTREQ_PARSECOMMAND_InitCommand(psInfo, pucCommand, pucFeedback, psReqSaver);

		/* get the current command handle function */
		eRet = eCTRL_UARTREQ_PARSECOMMAND_GetCommandFunction(psInfo);
		if(eRet != CTRL_UARTREQ_PARSECOMMAND_SUCCESS) {
			
			CUP_Debug("get command function error\n");
			psThreadArg->eRet = CTRL_UARTREQ_PARSECOMAMND_Function_ErrorCommand;
			eRet = CTRL_UARTREQ_PARSECOMMAND_COMMANDNOTFOUND;
			
		} else {
		
			/* handle the current command */
			psThreadArg->eRet = CTRL_UARTREQ_PARSECOMMAND_Function_Success;
			psThreadArg->psReqSaver = psInfo->psReqSaver;
			psInfo->vCommandHandleFunc((void*)(&psInfo->sFuncArg));

			/* parse the function return */
			if(psThreadArg->eRet != CTRL_UARTREQ_PARSECOMMAND_Function_Success) {
				CUP_Debug("command handle function error with <%d>\n", psThreadArg->eRet);
				eRet = CTRL_UARTREQ_PARSECOMMAND_FUNCPARSE;
			}
			
		}

		/* get feedback */
		vGetFeedback(psThreadArg, (CHAR*) psInfo->pucFeedback);
		
	}

	CUP_FuncOut();

	return eRet;
}

/*********************************************
* func : eCTRL_UARTREQ_PARSECOMMAND_GetCommandFunction(sCTRL_UARTREQ_PARSECOMMAND_Info* psInfo)
* arg : sCTRL_UARTREQ_PARSECOMMAND_Info* psInfo
* ret : eCTRL_UARTREQ_PARSECOMMAND_Ret
* note : get the command handle function
*********************************************/
eCTRL_UARTREQ_PARSECOMMAND_Ret eCTRL_UARTREQ_PARSECOMMAND_GetCommandFunction(sCTRL_UARTREQ_PARSECOMMAND_Info* psInfo) {
	eCTRL_UARTREQ_PARSECOMMAND_Ret eRet = CTRL_UARTREQ_PARSECOMMAND_SUCCESS;
	sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg = &psInfo->sFuncArg;
	UCHAR* pucHeader = psInfo->pucCommand;
	LONG lCommandEntryCnt = 0;
	UCHAR ucCommandKey;
	sCTRL_UARTREQ_PARSECOMMAND_CommandEntry* psCommandEntry;
	LONG lCommandSetLen = 0;

	CUP_FuncIn();

	{

		/* search for handle function */
		ucCommandKey = pucHeader[ CTRL_UARTREQ_PARSECOMMAND_Type ];
		lCommandSetLen = CTRL_UARTREQ_PARSECOMMAND_GETARRAYSIZE(sCommandSet);
		for(; lCommandEntryCnt != lCommandSetLen; lCommandEntryCnt++) {
			psCommandEntry = sCommandSet + lCommandEntryCnt;
			if(ucCommandKey == psCommandEntry->ucKey) {
				/* find the command */
				psInfo->vCommandHandleFunc = psCommandEntry->pvFunction;
				break;
			}
		}

		if(lCommandEntryCnt == lCommandSetLen) {
			/* indicate not found the command */
			CUP_Debug("can not find command key %c\n", ucCommandKey);
			return CTRL_UARTREQ_PARSECOMMAND_FUNCPARSE;
		}

		/* prepare the function argument */
		psThreadArg->ucCommandKey = ucCommandKey;
		psThreadArg->pucCommandFirst = pucHeader + CTRL_UARTREQ_PARSECOMMAND_FirstCommand;
		psThreadArg->lCommandLen = strlen((CHAR*) psThreadArg->pucCommandFirst);

	}

	CUP_FuncOut();

	return eRet;
}

#ifdef __cplusplus
}
#endif
