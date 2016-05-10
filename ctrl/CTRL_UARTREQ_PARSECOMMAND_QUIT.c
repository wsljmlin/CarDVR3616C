#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "CTRL_UARTREQ_PARSECOMMAND_QUIT.h"
#include "comm_command.h"

/***************************************************
	macro / enum
***************************************************/
//#define CTRL_UARTREQ_PARSECOMMAND_QUIT_DEBUG
#ifdef CTRL_UARTREQ_PARSECOMMAND_QUIT_DEBUG
#define CUPQ_Debug(fmt, arg...) fprintf(stdout, "[ CUPQ ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CUPQ_Debug(fmt, arg...)
#endif

#define CUPQ_FuncIn() CUPQ_Debug("in\n")
#define CUPQ_FuncOut() CUPQ_Debug("out\n")
#define CUPQ_iVal(iVal) CUPQ_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CUPQ_lVal(lVal) CUPQ_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CUPQ_PtVal(ptVal) CUPQ_Debug("pt %s @ %p\n", #ptVal, ptVal)


/***************************************************
	variable
***************************************************/
static CHAR* QUITCOMMAND = "QUIT";

/***************************************************
	prototype
***************************************************/

/***************************************************
	function
***************************************************/
/*********************************************
* func : pvCTRL_UARTREQ_PARSECOMMAND_QUIT(void* pvArg)
* arg : void* pvArg
* ret : void
* note : parse quit system command
*********************************************/
void pvCTRL_UARTREQ_PARSECOMMAND_QUIT(void* pvArg) {
	sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg = (sCTRL_UARTREQ_PARSECOMMAND_ThreadArg*) pvArg;
	sCOMM_CMD sCommand;
	LONG lCommandVal = 1;
	CHAR pcRequest[CTRL_UARTREQ_PARSECOMMAND_REQMAX];
	CHAR* ppcRequest = pcRequest;

	CUPQ_FuncIn();

	{
		
		/* parse the inter command */
		if (strncmp((CHAR*)psThreadArg->pucCommandFirst, QUITCOMMAND, 4) != 0) {
			CUPQ_Debug("the quit command < %s > error\n", psThreadArg->pucCommandFirst);
			psThreadArg->eRet = CTRL_UARTREQ_PARSECOMMAND_Function_Quit;
			return;
		}
		
		/* construct the request */
		memset(pcRequest, 0, CTRL_UARTREQ_PARSECOMMAND_REQMAX);
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_SYS, COMM_SUBCMD_SYSQUIT, lCommandVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);	
		vCOMM_CMD_AddReqInfo(pcRequest, COMM_UI_UART);

		/* write to the request saver */
		eCTRL_UARTREQ_SAVEREQ_WriteReq(psThreadArg->psReqSaver, pcRequest);
	}
	
	CUPQ_FuncOut();

	return;
}

#ifdef __cplusplus
}
#endif
