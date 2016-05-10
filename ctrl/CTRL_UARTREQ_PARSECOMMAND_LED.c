#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "CTRL_UARTREQ_PARSECOMMAND_LED.h"
#include "comm_command.h"
#include "comm_mw_strpool.h"
#include "comm_sharemem.h"

/***************************************************
	macro / enum
***************************************************/
//#define CTRL_UARTREQ_PARSECOMMAND_LED_DEBUG
#ifdef CTRL_UARTREQ_PARSECOMMAND_LED_DEBUG
#define CUPL_Debug(fmt, arg...) fprintf(stdout, "[ CUPL ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CUPL_Debug(fmt, arg...)
#endif

#define CUPL_FuncIn() CUPL_Debug("in\n")
#define CUPL_FuncOut() CUPL_Debug("out\n")
#define CUPL_iVal(iVal) CUPL_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CUPL_lVal(lVal) CUPL_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CUPL_PtVal(ptVal) CUPL_Debug("pt %s @ %p\n", #ptVal, ptVal)

#define CTRL_UARTREQ_PARSECOMMAND_LED_REGIONSHOWTYPETIME 2
#define CTRL_UARTREQ_PARSECOMMAND_LED_REGIONNUM0 0
#define CTRL_UARTREQ_PARSECOMMAND_LED_REGIONRESOLUTION1080P 1
#define CTRL_UARTREQ_PARSECOMMAND_LED_REGIONRESOLUTION720P 2

typedef enum {
	CTRL_UARTREQ_PARSECOMMAND_LED_CommandContent_Red = 4,
	CTRL_UARTREQ_PARSECOMMAND_LED_CommandContent_Green = 6,
} eCTRL_UARTREQ_PARSECOMMAND_LED_CommandContent;

typedef enum {
	CTRL_UARTREQ_PARSECOMMAND_LED_CommandLen_Red = 1,
	CTRL_UARTREQ_PARSECOMMAND_LED_CommandLen_Green = 1,
} eCTRL_UARTREQ_PARSECOMMAND_LED_CommandLen;


/***************************************************
	variable
***************************************************/
static CHAR LEDCOMMAND[] = "LED";
extern sCOMM_SHAREMEM gsShareMem;
extern const LONG SM_OFFSET[];
static CHAR cShowBmpStr[] = "start";

/***************************************************
	prototype
***************************************************/
void vCTRL_UARTREQ_PARSECOMMAND_LED_WriteLedRequest(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg);
void vCTRL_UARTREQ_PARSECOMMAND_LED_WriteThreadStartReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg);
void vCTRL_UARTREQ_PARSECOMMAND_LED_WriteTimeOsdReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg);

/***************************************************
	function
***************************************************/
/*********************************************
* func : pvCTRL_UARTREQ_PARSECOMMAND_LED(void* pvArg)
* arg : void* pvArg
* ret : void
* note : parse led system command
*********************************************/
void pvCTRL_UARTREQ_PARSECOMMAND_LED(void* pvArg) {
	sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg = (sCTRL_UARTREQ_PARSECOMMAND_ThreadArg*) pvArg;

	CUPL_FuncIn();

	{
		
		/* parse the inter command */
		if (strncmp((CHAR*)psThreadArg->pucCommandFirst, LEDCOMMAND, 3) != 0) {
			CUPL_Debug("the led command < %s > error\n", psThreadArg->pucCommandFirst);
			psThreadArg->eRet = CTRL_UARTREQ_PARSECOMMAND_Function_Led;
			return;
		}

		/* write request */
		vCTRL_UARTREQ_PARSECOMMAND_LED_WriteLedRequest(psThreadArg);
		vCTRL_UARTREQ_PARSECOMMAND_LED_WriteTimeOsdReq(psThreadArg);
		vCTRL_UARTREQ_PARSECOMMAND_LED_WriteThreadStartReq(psThreadArg);

	}
	
	CUPL_FuncOut();

	return;
}

/*********************************************
* func : vCTRL_UARTREQ_PARSECOMMAND_LED_WriteLedRequest(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg)
* arg : sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg
* ret : void
* note : write the led request
*********************************************/
void vCTRL_UARTREQ_PARSECOMMAND_LED_WriteLedRequest(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg) {
	sCOMM_CMD sCommand;
	LONG lCommandVal = 0;
	CHAR pcRequest[CTRL_UARTREQ_PARSECOMMAND_REQMAX];
	CHAR* ppcRequest = pcRequest;

	/* the two led status */
	LONG lRedLedStatus;
	LONG lGreenLedStatus;

	CUPL_FuncIn();

	{

		/* get the led status */
		lRedLedStatus = lNUCharToLong(psThreadArg->pucCommandFirst,
			CTRL_UARTREQ_PARSECOMMAND_LED_CommandContent_Red,
			CTRL_UARTREQ_PARSECOMMAND_LED_CommandLen_Red);

		lGreenLedStatus = lNUCharToLong(psThreadArg->pucCommandFirst,
			CTRL_UARTREQ_PARSECOMMAND_LED_CommandContent_Green,
			CTRL_UARTREQ_PARSECOMMAND_LED_CommandLen_Green);

		/* construct the command value */
		lCommandVal = (lRedLedStatus << 4) | lGreenLedStatus;
		
		/* construct the request */
		memset(pcRequest, 0, CTRL_UARTREQ_PARSECOMMAND_REQMAX);
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_SYS, COMM_SUBCMD_LED, lCommandVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);	
		vCOMM_CMD_AddReqInfo(pcRequest, COMM_UI_UART);

		/* write to the request saver */
		eCTRL_UARTREQ_SAVEREQ_WriteReq(psThreadArg->psReqSaver, pcRequest);

	}
	
	CUPL_FuncOut();

	return;
}

/*********************************************
* func : vCTRL_UARTREQ_PARSECOMMAND_LED_WriteThreadStartReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg)
* arg : sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg
* ret : void
* note : write the start thread request
*********************************************/
void vCTRL_UARTREQ_PARSECOMMAND_LED_WriteThreadStartReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg) {
	sCOMM_CMD sCommand;
	LONG lCommandVal = 1;
	CHAR pcRequest[CTRL_UARTREQ_PARSECOMMAND_REQMAX];
	CHAR* ppcRequest = pcRequest;

	CUPL_FuncIn();

	{
		
		/* construct the request */
		memset(pcRequest, 0, CTRL_UARTREQ_PARSECOMMAND_REQMAX);
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_SYS, COMM_SUBCMD_THREADSTART, lCommandVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);	
		vCOMM_CMD_AddReqInfo(pcRequest, COMM_UI_UART);

		/* write to the request saver */
		eCTRL_UARTREQ_SAVEREQ_WriteReq(psThreadArg->psReqSaver, pcRequest);

	}
	
	CUPL_FuncOut();

	return;
}

/*********************************************
* func : vCTRL_UARTREQ_PARSECOMMAND_LED_WriteTimeOsdReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg)
* arg : sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg
* ret : void
* note : write the time osd request
*********************************************/
void vCTRL_UARTREQ_PARSECOMMAND_LED_WriteTimeOsdReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg) {

	sCOMM_CMD sCommand;
	CHAR pcRequest[CTRL_UARTREQ_PARSECOMMAND_REQMAX];
	CHAR* ppcRequest = pcRequest;
	LONG lCmdVal;
	
	CUPL_FuncIn();

	{
		/* clear the buffer */
		memset(pcRequest, 0, CTRL_UARTREQ_PARSECOMMAND_REQMAX);

		/** construct command **/

		/* region resolution */
#ifdef USE_720P
		lCmdVal = CTRL_UARTREQ_PARSECOMMAND_LED_REGIONRESOLUTION720P;
#else
		lCmdVal = CTRL_UARTREQ_PARSECOMMAND_LED_REGIONRESOLUTION1080P;
#endif
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_REG, COMM_SUBCMD_OVERLAYRESOLUTION, lCmdVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);

		/* region number */
		ppcRequest += strlen(ppcRequest);
		*ppcRequest++ = ',';
		lCmdVal = CTRL_UARTREQ_PARSECOMMAND_LED_REGIONNUM0;
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_REG, COMM_SUBCMD_OVERLAYNUM, lCmdVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);

		/* region showtype */
		ppcRequest += strlen(ppcRequest);
		*ppcRequest++ = ',';
		lCmdVal = CTRL_UARTREQ_PARSECOMMAND_LED_REGIONSHOWTYPETIME;
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_REG, COMM_SUBCMD_OVERLAYSHOWTYPE, lCmdVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);

		/* region showbmp */
		ppcRequest += strlen(ppcRequest);
		*ppcRequest++ = ',';
		COMM_MW_StrPool_WriteEntry(COMM_GET_STRPOOL(), cShowBmpStr, &lCmdVal, &mutStringPoolWt);
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_REG, COMM_SUBCMD_OVERLAYSHOWBMP, lCmdVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);

		/* write to the request saver */
		vCOMM_CMD_AddReqInfo(pcRequest, COMM_UI_UART);
		eCTRL_UARTREQ_SAVEREQ_WriteReq(psThreadArg->psReqSaver, pcRequest);

	}

	CUPL_FuncOut();
}


#ifdef __cplusplus
}
#endif
