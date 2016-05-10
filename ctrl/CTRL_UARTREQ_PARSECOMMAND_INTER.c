#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "CTRL_UARTREQ_PARSECOMMAND_INTER.h"
#include "comm_command.h"
#include "comm_mw_time.h"
#ifdef INTER_CMD_REDEFINE
#include "comm_mw_strpool.h"
#include "comm_sharemem.h"
#include <assert.h>
#endif
/***************************************************
	macro / enum
***************************************************/
//#define CTRL_UARTREQ_PARSECOMMAND_INTER_DEBUG
#ifdef CTRL_UARTREQ_PARSECOMMAND_INTER_DEBUG
#define CUPI_Debug(fmt, arg...) fprintf(stdout, "[ CUPI ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CUPI_Debug(fmt, arg...)
#endif

#define CUPI_FuncIn() CUPI_Debug("in\n")
#define CUPI_FuncOut() CUPI_Debug("out\n")
#define CUPI_iVal(iVal) CUPI_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CUPI_lVal(lVal) CUPI_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CUPI_PtVal(ptVal) CUPI_Debug("pt %s @ %p\n", #ptVal, ptVal)

#ifdef INTER_CMD_REDEFINE
#define CTRL_UARTREQ_PARSECOMMAND_INTER_REGIONRESOLUTION1080P 1
#define CTRL_UARTREQ_PARSECOMMAND_INTER_REGIONRESOLUTION720P 2

#define CTRL_UARTREQ_PARSECOMMAND_INTER_REGIONSHOWTYPESTRING 1
#define CTRL_UARTREQ_PARSECOMMAND_INTER_REGIONNUM1 1
#define CTRL_UARTREQ_PARSECOMMAND_INTER_REGIONNUM2 2
#define CTRL_UARTREQ_PARSECOMMAND_INTER_GPSXDATALEN (20)
#define CTRL_UARTREQ_PARSECOMMAND_INTER_GPSYDATALEN (20)
#endif

typedef enum {
	CTRL_UARTREQ_PARSECOMMAND_INTER_RET_SUCCESS,
	CTRL_UARTREQ_PARSECOMMAND_INTER_RET_FAIL,
	CTRL_UARTREQ_PARSECOMMAND_INTER_RET_TIME,
} eCTRL_UARTREQ_PARSECOMMAND_INTER_Ret;

/***************************************************
	variable
***************************************************/
static CHAR* INTERCOMMAND = "INTER";

#if 1
LONG slDelterTimeMax = 10;
#else
static LONG slDelterTimeMax = 10;
#endif

#ifdef INTER_CMD_REDEFINE
extern sCOMM_SHAREMEM gsShareMem;
extern const LONG SM_OFFSET[];
static CHAR cShowBmpStr[128];
#endif
/***************************************************
	prototype
***************************************************/
eCTRL_UARTREQ_PARSECOMMAND_INTER_Ret eCTRL_UARTREQ_PARSECOMMAND_INTER_DeltTimeCheck(LONG lDeltTimeMin);
#ifdef INTER_CMD_REDEFINE
void vCTRL_UDARTREQ_PARSECOMMAND_IINTER_WriteInterReq(void* pvArg, LONG lVal) ;
void vCTRL_UDARTREQ_PARSECOMMAND_IINTER_WriteLongitudeReq(void* pvArg, LONG lVal) ;
void vCTRL_UDARTREQ_PARSECOMMAND_IINTER_WriteLatitudeReq(void* pvArg, LONG lVal);
void vCTRL_UARTREQ_PARSECOMMAND_INTER_WriteGPSOsdGPSXReq(void* pvArg);
void vCTRL_UARTREQ_PARSECOMMAND_INTER_WriteGPSOsdGPSYReq(void* pvArg);
#endif
/***************************************************
	function
***************************************************/
/*********************************************
* func : pvCTRL_UARTREQ_PARSECOMMAND_INTER(void* pvArg)
* arg : void* pvArg
* ret : void
* note : parse inter command
*********************************************/
void pvCTRL_UARTREQ_PARSECOMMAND_INTER(void* pvArg) {
	sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg = (sCTRL_UARTREQ_PARSECOMMAND_ThreadArg*) pvArg;	
	LONG lCommandVal = 1;
	
#ifdef RECONFIGTIME
#else
	sCOMM_CMD sCommand;
	CHAR pcRequest[CTRL_UARTREQ_PARSECOMMAND_REQMAX];
	CHAR* ppcRequest = pcRequest;
#endif
	CUPI_FuncIn();

	{
		/* check whether config yet */
		if(!(psThreadArg->eStatus & CTRL_UARTREQ_PARSECOMMAND_STATUS_Configed)) {
			CUPI_Debug("not config yet !\n");
			psThreadArg->eRet = CTRL_UARTREQ_PARSECOMMAND_Function_NotConfigYet;
			return;
		}
		
		/* parse the inter command */
		if (strncmp((CHAR*)psThreadArg->pucCommandFirst, INTERCOMMAND, 5) != 0) {
			CUPI_Debug("the inter command < %s > error\n", psThreadArg->pucCommandFirst);
			psThreadArg->eRet = CTRL_UARTREQ_PARSECOMMAND_Function_Inter;
			return;
		}
#ifdef INTER_CMD_REDEFINE
		sscanf((CHAR*)psThreadArg->pucCommandFirst+5, "%s", cShowBmpStr);
#endif

		/* check delter time */
		if (eCTRL_UARTREQ_PARSECOMMAND_INTER_DeltTimeCheck(slDelterTimeMax)) {
			CUPI_Debug("wrong delter time\n");
			psThreadArg->eRet = CTRL_UARTREQ_PARSECOMMAND_Function_InterDelterTime;
			return;
		}
#ifdef INTER_CMD_REDEFINE
		vCTRL_UDARTREQ_PARSECOMMAND_IINTER_WriteInterReq(pvArg, lCommandVal);
		if(strlen(psThreadArg->pucCommandFirst) > 5) {
			vCTRL_UARTREQ_PARSECOMMAND_INTER_WriteGPSOsdGPSXReq(pvArg);
			vCTRL_UARTREQ_PARSECOMMAND_INTER_WriteGPSOsdGPSYReq(pvArg); 
		}
		if(CTRL_UARTREQ_PARSECOMMAND_Function_Success != psThreadArg->eRet ) {
			return;
		}
#else
		/* construct the request */
		memset(pcRequest, 0, CTRL_UARTREQ_PARSECOMMAND_REQMAX);
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_RECORD, COMM_SUBCMD_SUBCARDREC, lCommandVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);	
		vCOMM_CMD_AddReqInfo(pcRequest, COMM_UI_UART);

		/* write to the request saver */
		eCTRL_UARTREQ_SAVEREQ_WriteReq(psThreadArg->psReqSaver, pcRequest);
#endif
	}
	
	CUPI_FuncOut();

	return;
}

#ifdef INTER_CMD_REDEFINE
void vCTRL_UDARTREQ_PARSECOMMAND_IINTER_WriteInterReq(void* pvArg, LONG lVal) {
	sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg = (sCTRL_UARTREQ_PARSECOMMAND_ThreadArg*) pvArg;
	sCOMM_CMD sCommand;
	CHAR pcRequest[CTRL_UARTREQ_PARSECOMMAND_REQMAX];
	CHAR* ppcRequest = pcRequest;

	/* construct the request */
	memset(pcRequest, 0, CTRL_UARTREQ_PARSECOMMAND_REQMAX);
	vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_RECORD, COMM_SUBCMD_SUBCARDREC, lVal); 
	vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);	
	vCOMM_CMD_AddReqInfo(pcRequest, COMM_UI_UART);

	/* write to the request saver */
	eCTRL_UARTREQ_SAVEREQ_WriteReq(psThreadArg->psReqSaver, pcRequest);
}

void vCTRL_UARTREQ_PARSECOMMAND_INTER_WriteGPSOsdGPSXReq(void* pvArg) {
	sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg = (sCTRL_UARTREQ_PARSECOMMAND_ThreadArg*) pvArg;
	sCOMM_CMD sCommand;
	CHAR pcRequest[CTRL_UARTREQ_PARSECOMMAND_REQMAX];
	CHAR* ppcRequest = pcRequest;
	LONG lCmdVal;
	CHAR cGpsXbuf[CTRL_UARTREQ_PARSECOMMAND_INTER_GPSXDATALEN+1];
	CHAR* pcGpsXbufIndexEnd = NULL;
	CHAR* pcGpsXbuf = NULL;
	LONG lGpsBufDataLen = 0;
	
	CUPI_FuncIn();

	{
		/* clear the buffer */
		memset(pcRequest, 0, CTRL_UARTREQ_PARSECOMMAND_REQMAX);
		memset(cGpsXbuf, 0 , sizeof(cGpsXbuf));
		/* err check */
		if(strlen(cShowBmpStr) > (CTRL_UARTREQ_PARSECOMMAND_INTER_GPSXDATALEN + CTRL_UARTREQ_PARSECOMMAND_INTER_GPSYDATALEN)) {
			memset(cGpsXbuf, ' ', CTRL_UARTREQ_PARSECOMMAND_INTER_GPSXDATALEN);
			goto constructcmd;
		}
		if((!strstr(cShowBmpStr, "X:") ) || (!strstr(cShowBmpStr, "Y:")) ) {
			memset(cGpsXbuf, ' ', CTRL_UARTREQ_PARSECOMMAND_INTER_GPSXDATALEN);
			goto constructcmd;
		}
		/* get position of GPS X data */
		pcGpsXbuf = index(cShowBmpStr, 'X');
		if(!pcGpsXbuf) {
			CUPI_Debug("error inter gps command\n", __FUNCTION__, __LINE__);
			//psThreadArg->eRet = CTRL_UARTREQ_PARSECOMMAND_Function_GPSformat;
			memset(cGpsXbuf, ' ', CTRL_UARTREQ_PARSECOMMAND_INTER_GPSXDATALEN);
			goto constructcmd;
			//return;
		}
		/* get end position of GPS X data */
		pcGpsXbufIndexEnd = index(pcGpsXbuf, 'Y');
		if(!pcGpsXbufIndexEnd) {
			CUPI_Debug("error inter gps command\n", __FUNCTION__, __LINE__);
			//psThreadArg->eRet = CTRL_UARTREQ_PARSECOMMAND_Function_GPSformat;
			memset(cGpsXbuf, ' ', CTRL_UARTREQ_PARSECOMMAND_INTER_GPSXDATALEN);
			goto constructcmd;
			//return;
		}
		if(((pcGpsXbufIndexEnd - pcGpsXbuf) > CTRL_UARTREQ_PARSECOMMAND_INTER_GPSXDATALEN) ||
			(strlen(pcGpsXbufIndexEnd) > CTRL_UARTREQ_PARSECOMMAND_INTER_GPSYDATALEN)) {
			CUPI_Debug(" error inter gps command\n", __FUNCTION__, __LINE__);
			//psThreadArg->eRet = CTRL_UARTREQ_PARSECOMMAND_Function_GPSformat;memset(cGpsXbuf, ' ', CTRL_UARTREQ_PARSECOMMAND_INTER_GPSXDATALEN);
			memset(cGpsXbuf, ' ', CTRL_UARTREQ_PARSECOMMAND_INTER_GPSXDATALEN);
			goto constructcmd;
			//return;
		}

		/* calculate gps data lenght */
		lGpsBufDataLen = pcGpsXbufIndexEnd - pcGpsXbuf;
		memcpy(cGpsXbuf, pcGpsXbuf, lGpsBufDataLen);
		memset(cGpsXbuf+lGpsBufDataLen, ' ', CTRL_UARTREQ_PARSECOMMAND_INTER_GPSXDATALEN-lGpsBufDataLen);

		/** construct command **/
constructcmd:
		/* region resolution */
#ifdef USE_720P
		lCmdVal = CTRL_UARTREQ_PARSECOMMAND_INTER_REGIONRESOLUTION720P;
#else
		lCmdVal = CTRL_UARTREQ_PARSECOMMAND_INTER_REGIONRESOLUTION1080P;
#endif
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_REG, COMM_SUBCMD_OVERLAYRESOLUTION, lCmdVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);

		/* region number */
		ppcRequest += strlen(ppcRequest);
		*ppcRequest++ = ',';
		lCmdVal = CTRL_UARTREQ_PARSECOMMAND_INTER_REGIONNUM1;
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_REG, COMM_SUBCMD_OVERLAYNUM, lCmdVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);

		/* region showtype */
		ppcRequest += strlen(ppcRequest);
		*ppcRequest++ = ',';
		lCmdVal = CTRL_UARTREQ_PARSECOMMAND_INTER_REGIONSHOWTYPESTRING;
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_REG, COMM_SUBCMD_OVERLAYSHOWTYPE, lCmdVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);

		/* region showbmp */
		ppcRequest += strlen(ppcRequest);
		*ppcRequest++ = ',';
		COMM_MW_StrPool_WriteEntry(COMM_GET_STRPOOL(), cGpsXbuf, &lCmdVal, &mutStringPoolWt);
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_REG, COMM_SUBCMD_OVERLAYSHOWBMP, lCmdVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);

		/* write to the request saver */
		vCOMM_CMD_AddReqInfo(pcRequest, COMM_UI_UART);
		eCTRL_UARTREQ_SAVEREQ_WriteReq(psThreadArg->psReqSaver, pcRequest);

	}

	CUPI_FuncOut();
}


void vCTRL_UARTREQ_PARSECOMMAND_INTER_WriteGPSOsdGPSYReq(void* pvArg) {
	sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg = (sCTRL_UARTREQ_PARSECOMMAND_ThreadArg*) pvArg;
	sCOMM_CMD sCommand;
	CHAR pcRequest[CTRL_UARTREQ_PARSECOMMAND_REQMAX];
	CHAR* ppcRequest = pcRequest;
	LONG lCmdVal;
	CHAR cGpsYbuf[CTRL_UARTREQ_PARSECOMMAND_INTER_GPSYDATALEN+1];
	CHAR* pcGpsYbuf = NULL;
	CHAR* pcGpsData = NULL;
	LONG lGpsBufDataLen = 0;
	
	
	CUPI_FuncIn();

	{
		/* clear the buffer */
		memset(pcRequest, 0, CTRL_UARTREQ_PARSECOMMAND_REQMAX);
		memset(cGpsYbuf, 0 , sizeof(cGpsYbuf));
		pcGpsData = cShowBmpStr;
		/* err check */
		if(strlen(cShowBmpStr) > (CTRL_UARTREQ_PARSECOMMAND_INTER_GPSXDATALEN + CTRL_UARTREQ_PARSECOMMAND_INTER_GPSYDATALEN)) {
			memset(cGpsYbuf, ' ', CTRL_UARTREQ_PARSECOMMAND_INTER_GPSYDATALEN);
			goto constructcmd;
		}
		if((!strstr(cShowBmpStr, "X:") ) || (!strstr(cShowBmpStr, "Y:")) ) {
			memset(cGpsYbuf, ' ', CTRL_UARTREQ_PARSECOMMAND_INTER_GPSYDATALEN);
			goto constructcmd;
		}
			
		/* get position of GPS Y data */
		pcGpsYbuf = index(pcGpsData, 'Y');
		if(!pcGpsYbuf) {
			CUPI_Debug(" error inter gps command\n", __FUNCTION__, __LINE__);
			//psThreadArg->eRet = CTRL_UARTREQ_PARSECOMMAND_Function_GPSformat;
			memset(cGpsYbuf, ' ', CTRL_UARTREQ_PARSECOMMAND_INTER_GPSYDATALEN);
			goto constructcmd;
			//return;
		}

		if((strlen(pcGpsYbuf) > CTRL_UARTREQ_PARSECOMMAND_INTER_GPSYDATALEN) ||
			((pcGpsYbuf - pcGpsData) > CTRL_UARTREQ_PARSECOMMAND_INTER_GPSXDATALEN)) {
			CUPI_Debug("error inter gps command\n", __FUNCTION__, __LINE__);
			//psThreadArg->eRet = CTRL_UARTREQ_PARSECOMMAND_Function_GPSformat;
			memset(cGpsYbuf, ' ', CTRL_UARTREQ_PARSECOMMAND_INTER_GPSYDATALEN);
			goto constructcmd;
			//return;
		}
		/* calculate gps data lenght */
		lGpsBufDataLen = strlen(pcGpsYbuf);
		memcpy(cGpsYbuf, pcGpsYbuf, lGpsBufDataLen);
		memset(cGpsYbuf+lGpsBufDataLen, ' ', CTRL_UARTREQ_PARSECOMMAND_INTER_GPSYDATALEN-lGpsBufDataLen);

		/** construct command **/
constructcmd:
		/* region resolution */
#ifdef USE_720P
		lCmdVal = CTRL_UARTREQ_PARSECOMMAND_INTER_REGIONRESOLUTION720P;
#else
		lCmdVal = CTRL_UARTREQ_PARSECOMMAND_INTER_REGIONRESOLUTION1080P;
#endif
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_REG, COMM_SUBCMD_OVERLAYRESOLUTION, lCmdVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);

		/* region number */
		ppcRequest += strlen(ppcRequest);
		*ppcRequest++ = ',';
		lCmdVal = CTRL_UARTREQ_PARSECOMMAND_INTER_REGIONNUM2;
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_REG, COMM_SUBCMD_OVERLAYNUM, lCmdVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);

		/* region showtype */
		ppcRequest += strlen(ppcRequest);
		*ppcRequest++ = ',';
		lCmdVal = CTRL_UARTREQ_PARSECOMMAND_INTER_REGIONSHOWTYPESTRING;
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_REG, COMM_SUBCMD_OVERLAYSHOWTYPE, lCmdVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);

		/* region showbmp */
		ppcRequest += strlen(ppcRequest);
		*ppcRequest++ = ',';
		COMM_MW_StrPool_WriteEntry(COMM_GET_STRPOOL(), cGpsYbuf, &lCmdVal, &mutStringPoolWt);
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_REG, COMM_SUBCMD_OVERLAYSHOWBMP, lCmdVal); 
		vCOMM_CMD_CmdToChar(&sCommand, &ppcRequest);

		/* write to the request saver */
		vCOMM_CMD_AddReqInfo(pcRequest, COMM_UI_UART);
		eCTRL_UARTREQ_SAVEREQ_WriteReq(psThreadArg->psReqSaver, pcRequest);

	}

	CUPI_FuncOut();
}

#endif
/*********************************************
* func : eCTRL_UARTREQ_PARSECOMMAND_INTER_DeltTimeCheck(LONG lDeltTimeMin)
* arg : LONG lDeltTimeMin
* ret : eCTRL_UARTREQ_PARSECOMMAND_INTER_Ret
* note : parse inter command send delter time
*********************************************/
eCTRL_UARTREQ_PARSECOMMAND_INTER_Ret eCTRL_UARTREQ_PARSECOMMAND_INTER_DeltTimeCheck(LONG lDeltTimeMin) {
	eCTRL_UARTREQ_PARSECOMMAND_INTER_Ret eRet = CTRL_UARTREQ_PARSECOMMAND_INTER_RET_SUCCESS;
	static LONG slFirstTime = 1;
	static sCOMM_MW_TIME_Info sTimeInfo;
	LONG lDeltTime = 0;

	CUPI_FuncIn();

	{

		if (slFirstTime) {
			/* indicate the first time */
			eCOMM_MW_TIME_Init(&sTimeInfo, "interDelterTime");
			slFirstTime = 0;
			return eRet;
		}

		/* indicate not the first one, check the time */
		eCOMM_MW_TIME_Update(&sTimeInfo);

		/* check the delter time */
		lDeltTime = lCOMM_MW_TIME_GetDeltTime(&sTimeInfo);
		if (lDeltTime < lDeltTimeMin) {
			CUPI_lVal(lDeltTime);
			CUPI_lVal(lDeltTimeMin);
			return CTRL_UARTREQ_PARSECOMMAND_INTER_RET_TIME;
		} else {
			eCOMM_MW_TIME_Save(&sTimeInfo);
		}
	}

	CUPI_FuncOut();

	return eRet;
}

#ifdef __cplusplus
}
#endif
