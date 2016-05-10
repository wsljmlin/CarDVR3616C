#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "CTRL_UARTREQ_PARSECOMMAND_TIME.h"
#include "comm_command.h"
#include "comm_mw_strpool.h"
#include "comm_sharemem.h"
#include <assert.h>

/***************************************************
	macro / enum
***************************************************/
//#define CTRL_UARTREQ_PARSECOMMAND_TIME_DEBUG
#ifdef CTRL_UARTREQ_PARSECOMMAND_TIME_DEBUG
#define CUPT_Debug(fmt, arg...) fprintf(stdout, "[ CUPT ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CUPT_Debug(fmt, arg...)
#endif

#define CUPT_FuncIn() CUPT_Debug("in\n")
#define CUPT_FuncOut() CUPT_Debug("out\n")
#define CUPT_iVal(iVal) CUPT_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CUPT_lVal(lVal) CUPT_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CUPT_PtVal(ptVal) CUPT_Debug("pt %s @ %p\n", #ptVal, ptVal)

#define CTRL_UARTREQ_PARSECOMMAND_TIME_TIMEHOLDER 32
#define CTRL_UARTREQ_PARSECOMMAND_TIME_COMMANDLEN 16

#define CTRL_UARTREQ_PARSECOMMAND_TIME_REGIONSHOWTYPETIME 2
#define CTRL_UARTREQ_PARSECOMMAND_TIME_REGIONNUM0 0
#define CTRL_UARTREQ_PARSECOMMAND_TIME_REGIONRESOLUTION1080P 1
#define CTRL_UARTREQ_PARSECOMMAND_TIME_REGIONRESOLUTION720P 2

#ifdef RECONFIGTIME
#define CTRL_UARTREQ_PARSECOMMAND_DELTA_TIME 10
#include "base_mw_time.h"
#endif

#define BEFORE_TIME_MAX 5
#define AFTER_TIME_MAX 60

typedef enum {
	CTRL_UARTREQ_PARSECOMMAND_TIME_CommandContent_Year = 0,
	CTRL_UARTREQ_PARSECOMMAND_TIME_CommandContent_Month = 2,
	CTRL_UARTREQ_PARSECOMMAND_TIME_CommandContent_Day = 4,
	CTRL_UARTREQ_PARSECOMMAND_TIME_CommandContent_Hour = 6,
	CTRL_UARTREQ_PARSECOMMAND_TIME_CommandContent_Minute = 8,
	CTRL_UARTREQ_PARSECOMMAND_TIME_CommandContent_Second = 10,
	CTRL_UARTREQ_PARSECOMMAND_TIME_CommandContent_OverlayType = 12,
	CTRL_UARTREQ_PARSECOMMAND_TIME_CommandContent_BeforeTime = 13,
	CTRL_UARTREQ_PARSECOMMAND_TIME_CommandContent_AfterTime = 14,
} eCTRL_UARTREQ_PARSECOMMAND_TIME_CommandContent;

typedef enum {
	CTRL_UARTREQ_PARSECOMMAND_TIME_CommandLen_Year = 2,
	CTRL_UARTREQ_PARSECOMMAND_TIME_CommandLen_Month = 2,
	CTRL_UARTREQ_PARSECOMMAND_TIME_CommandLen_Day = 2,
	CTRL_UARTREQ_PARSECOMMAND_TIME_CommandLen_Hour = 2,
	CTRL_UARTREQ_PARSECOMMAND_TIME_CommandLen_Minute = 2,
	CTRL_UARTREQ_PARSECOMMAND_TIME_CommandLen_Second = 2,
	CTRL_UARTREQ_PARSECOMMAND_TIME_CommandLen_OverlayType = 1,
	CTRL_UARTREQ_PARSECOMMAND_TIME_CommandLen_BeforeTime = 1,
	CTRL_UARTREQ_PARSECOMMAND_TIME_CommandLen_AfterTime = 2,
} eCTRL_UARTREQ_PARSECOMMAND_TIME_CommandLen;

/***************************************************
	variable
***************************************************/
extern sCOMM_SHAREMEM gsShareMem;
extern const LONG SM_OFFSET[];
static CHAR cShowBmpStr[] = "start";

/***************************************************
	prototype
***************************************************/
void vCTRL_UDARTREQ_PARSECOMMAND_TIME_GetTimeString(UCHAR* pucTime, UCHAR* pucCommand, LONG lLen);
void vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteTimeReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg);
void vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteOverlayTypeReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg);
void vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteBeforeTimeReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg);
void vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteAfterTimeReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg);
void vCTRL_UARTREQ_PARSECOMMAND_TIME_WriteThreadStartReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg);
void vCTRL_UARTREQ_PARSECOMMAND_TIME_WriteTimeOsdReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg);

/***************************************************
	function
***************************************************/

/*********************************************
* func : pvCTRL_UARTREQ_PARSECOMMAND_TIME(void* pvArg)
* arg : void* pvArg
* ret : void
* note : parse time command
*********************************************/
void pvCTRL_UARTREQ_PARSECOMMAND_TIME(void* pvArg) {
	sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg = (sCTRL_UARTREQ_PARSECOMMAND_ThreadArg*) pvArg;
#ifdef RECONFIGTIME
	static int configed = 0;;
#endif
	CUPT_FuncIn();

	{
		/* check the valid */
		if(psThreadArg->lCommandLen != CTRL_UARTREQ_PARSECOMMAND_TIME_COMMANDLEN) {
			psThreadArg->eRet = CTRL_UARTREQ_PARSECOMMAND_Function_Time;
			CUPT_Debug("error config command len <%d, %ld>\n", 
				CTRL_UARTREQ_PARSECOMMAND_TIME_COMMANDLEN, 
				psThreadArg->lCommandLen);
			return;
		}
#ifdef RECONFIGTIME
		if(configed) {
			/* after first configured, others will only set system time  */
			if(lBASE_MW_TIME_CompareNowTime(PREVSET_TIMESTAMP) < CTRL_UARTREQ_PARSECOMMAND_DELTA_TIME ) {
				psThreadArg->eRet = CTRL_UARTREQ_PARSECOMMAND_Function_Time;
				return;
			}
			
			vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteTimeReq(psThreadArg);
		} else {
			/* write request */
			vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteTimeReq(psThreadArg);
			vCTRL_UARTREQ_PARSECOMMAND_TIME_WriteTimeOsdReq(psThreadArg);
			vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteOverlayTypeReq(psThreadArg);
			vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteBeforeTimeReq(psThreadArg);
			vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteAfterTimeReq(psThreadArg);
			vCTRL_UARTREQ_PARSECOMMAND_TIME_WriteThreadStartReq(psThreadArg); 
			configed = 1;
		}
#else
		/* write request */
		vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteTimeReq(psThreadArg);
		vCTRL_UARTREQ_PARSECOMMAND_TIME_WriteTimeOsdReq(psThreadArg);
		vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteOverlayTypeReq(psThreadArg);
		vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteBeforeTimeReq(psThreadArg);
		vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteAfterTimeReq(psThreadArg);
		vCTRL_UARTREQ_PARSECOMMAND_TIME_WriteThreadStartReq(psThreadArg);
#endif
	}

	CUPT_FuncOut();

	return;
}

/*********************************************
* func : vCTRL_UDARTREQ_PARSECOMMAND_TIME_GetTimeString(UCHAR* pucTime, UCHAR* pucCommand, LONG lLen)
* arg : UCHAR* pucTime, UCHAR* pucCommand, LONG lLen
* ret : void
* note : get the time string 
*********************************************/
void vCTRL_UDARTREQ_PARSECOMMAND_TIME_GetTimeString(UCHAR* pucTime, UCHAR* pucCommand, LONG lLen) {
	CUPT_FuncIn();
	LONG lYear;
	LONG lMonth;
	LONG lDay;
	LONG lHour;
	LONG lMin;
	LONG lSec;

	{

		/* get time data */
		lYear = lNUCharToLong(pucCommand, 
			CTRL_UARTREQ_PARSECOMMAND_TIME_CommandContent_Year,
			CTRL_UARTREQ_PARSECOMMAND_TIME_CommandLen_Year);
		lYear += 2000;
		
		lMonth = lNUCharToLong(pucCommand,
			CTRL_UARTREQ_PARSECOMMAND_TIME_CommandContent_Month,
			CTRL_UARTREQ_PARSECOMMAND_TIME_CommandLen_Month);
		
		lDay = lNUCharToLong(pucCommand,
			CTRL_UARTREQ_PARSECOMMAND_TIME_CommandContent_Day,
			CTRL_UARTREQ_PARSECOMMAND_TIME_CommandLen_Day);
		
		lHour = lNUCharToLong(pucCommand,
			CTRL_UARTREQ_PARSECOMMAND_TIME_CommandContent_Hour,
			CTRL_UARTREQ_PARSECOMMAND_TIME_CommandLen_Hour);
		
		lMin = lNUCharToLong(pucCommand,
			CTRL_UARTREQ_PARSECOMMAND_TIME_CommandContent_Minute,
			CTRL_UARTREQ_PARSECOMMAND_TIME_CommandLen_Minute);
		
		lSec = lNUCharToLong(pucCommand,
			CTRL_UARTREQ_PARSECOMMAND_TIME_CommandContent_Second,
			CTRL_UARTREQ_PARSECOMMAND_TIME_CommandLen_Second);

		/* contruct time string */
		sprintf((CHAR*)pucTime, "%ld:%ld:%ld:%ld:%ld:%ld", lYear, lMonth, lDay, lHour, lMin, lSec);
		CUPT_Debug("the time string : %s\n", pucTime);
	}
	
	CUPT_FuncOut();

	return;
}

/*********************************************
* func : vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteTimeReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg)
* arg : sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg
* ret : void
* note : write the time command 
*********************************************/
void vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteTimeReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg) {

	UCHAR ucTimeHolder[CTRL_UARTREQ_PARSECOMMAND_TIME_TIMEHOLDER];
	UCHAR* pucTimeHolder = ucTimeHolder;
	sCOMM_CMD sCommand;
	LONG lStrPoolKey = 0;
	CHAR pcRequest[CTRL_UARTREQ_PARSECOMMAND_REQMAX];
	CHAR* ppcRequest = pcRequest;

	CUPT_FuncIn();

	{

		/* get command data */
		memset(ucTimeHolder, 0, sizeof(ucTimeHolder));
		vCTRL_UDARTREQ_PARSECOMMAND_TIME_GetTimeString(pucTimeHolder, psThreadArg->pucCommandFirst, psThreadArg->lCommandLen);

		/* get time string strpool key */
		memset(pcRequest, 0, CTRL_UARTREQ_PARSECOMMAND_REQMAX);
		COMM_MW_StrPool_WriteEntry(COMM_GET_STRPOOL(), (CHAR*)pucTimeHolder, &lStrPoolKey, &mutStringPoolWt);
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_SYS, COMM_SUBCMD_SETTIME, lStrPoolKey); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);

		/* construct command */
		vCOMM_CMD_AddReqInfo(pcRequest, COMM_UI_UART);

		/* write to the request saver */
		eCTRL_UARTREQ_SAVEREQ_WriteReq(psThreadArg->psReqSaver, pcRequest);

	}

	CUPT_FuncOut();
}

/*********************************************
* func : vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteOverlayTypeReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg)
* arg : sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg
* ret : void
* note : write the overlay type command 
*********************************************/
void vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteOverlayTypeReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg) {

	sCOMM_CMD sCommand;
	CHAR pcRequest[CTRL_UARTREQ_PARSECOMMAND_REQMAX];
	CHAR* ppcRequest = pcRequest;
	LONG lOverlayType;
	
	CUPT_FuncIn();

	{
		/* clear the buffer */
		memset(pcRequest, 0, CTRL_UARTREQ_PARSECOMMAND_REQMAX);
		
		/* construct command */
		lOverlayType = lNUCharToLong(psThreadArg->pucCommandFirst,
			CTRL_UARTREQ_PARSECOMMAND_TIME_CommandContent_OverlayType,
			CTRL_UARTREQ_PARSECOMMAND_TIME_CommandLen_OverlayType);
		
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_RECORD, COMM_SUBCMD_OVERLAY_TYPE, lOverlayType); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);
		vCOMM_CMD_AddReqInfo(pcRequest, COMM_UI_UART);

		/* write to the request saver */
		eCTRL_UARTREQ_SAVEREQ_WriteReq(psThreadArg->psReqSaver, pcRequest);

	}

	CUPT_FuncOut();
}

/*********************************************
* func : vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteBeforeTimeReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg)
* arg : sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg
* ret : void
* note : write the before time command 
*********************************************/
void vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteBeforeTimeReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg) {

	sCOMM_CMD sCommand;
	CHAR pcRequest[CTRL_UARTREQ_PARSECOMMAND_REQMAX];
	CHAR* ppcRequest = pcRequest;
	LONG lBeforeTime;
	
	CUPT_FuncIn();

	{
		/* clear the buffer */
		memset(pcRequest, 0, CTRL_UARTREQ_PARSECOMMAND_REQMAX);

		/* construct command */
		lBeforeTime = lNUCharToLong(psThreadArg->pucCommandFirst,
			CTRL_UARTREQ_PARSECOMMAND_TIME_CommandContent_BeforeTime,
			CTRL_UARTREQ_PARSECOMMAND_TIME_CommandLen_BeforeTime);

		/* regular the before time */
		lBeforeTime = ( lBeforeTime > BEFORE_TIME_MAX ) ? BEFORE_TIME_MAX : lBeforeTime;
		
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_RECORD, COMM_SUBCMD_BUFFER_TIME, lBeforeTime); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);
		vCOMM_CMD_AddReqInfo(pcRequest, COMM_UI_UART);

		/* write to the request saver */
		eCTRL_UARTREQ_SAVEREQ_WriteReq(psThreadArg->psReqSaver, pcRequest);

	}

	CUPT_FuncOut();
}

/*********************************************
* func : vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteAfterTimeReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg)
* arg : sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg
* ret : void
* note : write the after time command 
*********************************************/
void vCTRL_UDARTREQ_PARSECOMMAND_TIME_WriteAfterTimeReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg) {

	sCOMM_CMD sCommand;
	CHAR pcRequest[CTRL_UARTREQ_PARSECOMMAND_REQMAX];
	CHAR* ppcRequest = pcRequest;
	LONG lAfterTime;
	LONG lBeforeTime;
	LONG lWholeTime;
	
	CUPT_FuncIn();

	{
		/* clear the buffer */
		memset(pcRequest, 0, CTRL_UARTREQ_PARSECOMMAND_REQMAX);

		/* construct command */
		lBeforeTime = lNUCharToLong(psThreadArg->pucCommandFirst,
			CTRL_UARTREQ_PARSECOMMAND_TIME_CommandContent_BeforeTime,
			CTRL_UARTREQ_PARSECOMMAND_TIME_CommandLen_BeforeTime);
		
		lAfterTime = lNUCharToLong(psThreadArg->pucCommandFirst,
			CTRL_UARTREQ_PARSECOMMAND_TIME_CommandContent_AfterTime,
			CTRL_UARTREQ_PARSECOMMAND_TIME_CommandLen_AfterTime);

		/* regular the before and after time */
		lBeforeTime = ( lBeforeTime > BEFORE_TIME_MAX ) ? BEFORE_TIME_MAX : lBeforeTime;
		lAfterTime = ( lAfterTime > AFTER_TIME_MAX ) ? AFTER_TIME_MAX : lAfterTime;

		/* get whole time */
		lWholeTime = lBeforeTime + lAfterTime;
		
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_RECORD, COMM_SUBCMD_WHOLE_TIME, lWholeTime); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);
		vCOMM_CMD_AddReqInfo(pcRequest, COMM_UI_UART);
#if 1
		extern LONG slDelterTimeMax;
		slDelterTimeMax = lAfterTime;
#endif

		/* write to the request saver */
		eCTRL_UARTREQ_SAVEREQ_WriteReq(psThreadArg->psReqSaver, pcRequest);

	}

	CUPT_FuncOut();
}

/*********************************************
* func : vCTRL_UARTREQ_PARSECOMMAND_TIME_WriteThreadStartReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg)
* arg : sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg
* ret : void
* note : write the thread start request
*********************************************/
void vCTRL_UARTREQ_PARSECOMMAND_TIME_WriteThreadStartReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg) {

	sCOMM_CMD sCommand;
	CHAR pcRequest[CTRL_UARTREQ_PARSECOMMAND_REQMAX];
	CHAR* ppcRequest = pcRequest;
	LONG lCmdVal = 1;
	
	CUPT_FuncIn();

	{
		/* clear the buffer */
		memset(pcRequest, 0, CTRL_UARTREQ_PARSECOMMAND_REQMAX);
		
		/* construct command */
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_SYS, COMM_SUBCMD_THREADSTART, lCmdVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);
		vCOMM_CMD_AddReqInfo(pcRequest, COMM_UI_UART);

		/* write to the request saver */
		eCTRL_UARTREQ_SAVEREQ_WriteReq(psThreadArg->psReqSaver, pcRequest);

	}

	CUPT_FuncOut();
}

/*********************************************
* func : vCTRL_UARTREQ_PARSECOMMAND_TIME_WriteTimeOsdReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg)
* arg : sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg
* ret : void
* note : write the time osd request
*********************************************/
void vCTRL_UARTREQ_PARSECOMMAND_TIME_WriteTimeOsdReq(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg) {

	sCOMM_CMD sCommand;
	CHAR pcRequest[CTRL_UARTREQ_PARSECOMMAND_REQMAX];
	CHAR* ppcRequest = pcRequest;
	LONG lCmdVal;
	
	CUPT_FuncIn();

	{
		/* clear the buffer */
		memset(pcRequest, 0, CTRL_UARTREQ_PARSECOMMAND_REQMAX);

		/** construct command **/

		/* region resolution */
#ifdef USE_720P
		lCmdVal = CTRL_UARTREQ_PARSECOMMAND_TIME_REGIONRESOLUTION720P;
#else
		lCmdVal = CTRL_UARTREQ_PARSECOMMAND_TIME_REGIONRESOLUTION1080P;
#endif
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_REG, COMM_SUBCMD_OVERLAYRESOLUTION, lCmdVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);

		/* region number */
		ppcRequest += strlen(ppcRequest);
		*ppcRequest++ = ',';
		lCmdVal = CTRL_UARTREQ_PARSECOMMAND_TIME_REGIONNUM0;
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_REG, COMM_SUBCMD_OVERLAYNUM, lCmdVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);

		/* region showtype */
		ppcRequest += strlen(ppcRequest);
		*ppcRequest++ = ',';
		lCmdVal = CTRL_UARTREQ_PARSECOMMAND_TIME_REGIONSHOWTYPETIME;
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

	CUPT_FuncOut();
}

#ifdef __cplusplus
}
#endif
