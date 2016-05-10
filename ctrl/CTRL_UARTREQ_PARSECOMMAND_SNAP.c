#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "CTRL_UARTREQ_PARSECOMMAND_SNAP.h"
#include "comm_command.h"

/***************************************************
	macro / enum
***************************************************/
//#define CTRL_UARTREQ_PARSECOMMAND_SNAP_DEBUG
#ifdef CTRL_UARTREQ_PARSECOMMAND_SNAP_DEBUG
#define CUPS_Debug(fmt, arg...) fprintf(stdout, "[ CUPS ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CUPS_Debug(fmt, arg...)
#endif

#define CUPS_FuncIn() CUPS_Debug("in\n")
#define CUPS_FuncOut() CUPS_Debug("out\n")
#define CUPS_iVal(iVal) CUPS_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CUPS_lVal(lVal) CUPS_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CUPS_PtVal(ptVal) CUPS_Debug("pt %s @ %p\n", #ptVal, ptVal)


/***************************************************
	variable
***************************************************/
static CHAR* SNAPCOMMAND = "SNAP";

/***************************************************
	prototype
***************************************************/

/***************************************************
	function
***************************************************/

/*********************************************
* func : pvCTRL_UARTREQ_PARSECOMMAND_SNAP(void* pvArg)
* arg : void* pvArg
* ret : void
* note : snap command
*********************************************/
void pvCTRL_UARTREQ_PARSECOMMAND_SNAP(void* pvArg) {
	sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg = (sCTRL_UARTREQ_PARSECOMMAND_ThreadArg*) pvArg;
	sCOMM_CMD sCommand;
	LONG lCommandVal = 1;
	CHAR pcRequest[CTRL_UARTREQ_PARSECOMMAND_REQMAX];
	CHAR* ppcRequest = pcRequest;

	CUPS_FuncIn();

	{

		/* check whether config yet */
		if(!(psThreadArg->eStatus & CTRL_UARTREQ_PARSECOMMAND_STATUS_Configed)) {
			CUPS_Debug("not config yet !\n");
			psThreadArg->eRet = CTRL_UARTREQ_PARSECOMMAND_Function_NotConfigYet;
			return;
		}
		
		/* parse the snap command */
		if (strncmp((CHAR*)psThreadArg->pucCommandFirst, SNAPCOMMAND, 4) != 0) {
			CUPS_Debug("the snap command < %s > error\n", psThreadArg->pucCommandFirst);
			psThreadArg->eRet = CTRL_UARTREQ_PARSECOMMAND_Function_Snap;
			return;
		}
		
		/* construct the request */
		memset(pcRequest, 0, CTRL_UARTREQ_PARSECOMMAND_REQMAX);
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_RECORD, COMM_SUBCMD_RECSNAP, lCommandVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);	
		vCOMM_CMD_AddReqInfo(pcRequest, COMM_UI_UART);

		/* write to the request saver */
		eCTRL_UARTREQ_SAVEREQ_WriteReq(psThreadArg->psReqSaver, pcRequest);

	}
	
	CUPS_FuncOut();

	return;
}

#ifdef __cplusplus
}
#endif
