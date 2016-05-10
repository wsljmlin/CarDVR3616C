#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "CTRL_UARTREQ_PARSECOMMAND_EMPTY.h"
#include "comm_command.h"

/***************************************************
	macro / enum
***************************************************/
//#define CTRL_UARTREQ_PARSECOMMAND_EMPTY_DEBUG
#ifdef CTRL_UARTREQ_PARSECOMMAND_EMPTY_DEBUG
#define CUPE_Debug(fmt, arg...) fprintf(stdout, "[ CUPE ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CUPE_Debug(fmt, arg...)
#endif

#define CUPE_FuncIn() CUPE_Debug("in\n")
#define CUPE_FuncOut() CUPE_Debug("out\n")
#define CUPE_iVal(iVal) CUPE_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CUPE_lVal(lVal) CUPE_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CUPE_PtVal(ptVal) CUPE_Debug("pt %s @ %p\n", #ptVal, ptVal)


/***************************************************
	variable
***************************************************/
static CHAR* EMPTYCOMMAND = "EMPTY";

/***************************************************
	prototype
***************************************************/

/***************************************************
	function
***************************************************/
/*********************************************
* func : pvCTRL_UARTREQ_PARSECOMMAND_EMPTY(void* pvArg)
* arg : void* pvArg
* ret : void
* note : parse empty subCard command
*********************************************/
void pvCTRL_UARTREQ_PARSECOMMAND_EMPTY(void* pvArg) {
	sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg = (sCTRL_UARTREQ_PARSECOMMAND_ThreadArg*) pvArg;
	sCOMM_CMD sCommand;
	LONG lCommandVal = 1;
	CHAR pcRequest[CTRL_UARTREQ_PARSECOMMAND_REQMAX];
	CHAR* ppcRequest = pcRequest;

	CUPE_FuncIn();

	{

		/* check whether config yet */
		if(!(psThreadArg->eStatus & CTRL_UARTREQ_PARSECOMMAND_STATUS_Configed)) {
			CUPE_Debug("not config yet !\n");
			psThreadArg->eRet = CTRL_UARTREQ_PARSECOMMAND_Function_NotConfigYet;
			return;
		}
		
		/* parse the inter command */
		if (strncmp((CHAR*)psThreadArg->pucCommandFirst, EMPTYCOMMAND, 5) != 0) {
			CUPE_Debug("the empty subCard command < %s > error\n", psThreadArg->pucCommandFirst);
			psThreadArg->eRet = CTRL_UARTREQ_PARSECOMMAND_Function_Empty;
			return;
		}
		
		/* construct the request */
		memset(pcRequest, 0, CTRL_UARTREQ_PARSECOMMAND_REQMAX);
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_SYS, COMM_SUBCMD_CLEARSUBCARD, lCommandVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);	
		vCOMM_CMD_AddReqInfo(pcRequest, COMM_UI_UART);

		/* write to the request saver */
		eCTRL_UARTREQ_SAVEREQ_WriteReq(psThreadArg->psReqSaver, pcRequest);

	}
	
	CUPE_FuncOut();

	return;
}


#ifdef __cplusplus
}
#endif
