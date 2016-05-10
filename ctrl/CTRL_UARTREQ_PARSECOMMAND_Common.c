#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "CTRL_UARTREQ_PARSECOMMAND_Common.h"
#include <assert.h>

/***************************************************
	macro / enum
***************************************************/
//#define CTRL_UARTREQ_PARSECOMMAND_COMMON_DEBUG
#ifdef CTRL_UARTREQ_PARSECOMMAND_COMMON_DEBUG
#define CUPC_Debug(fmt, arg...) fprintf(stdout, "[ CUPC ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CUPC_Debug(fmt, arg...)
#endif

#define CUPC_FuncIn() CUPC_Debug("in\n")
#define CUPC_FuncOut() CUPC_Debug("out\n")
#define CUPC_iVal(iVal) CUPC_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CUPC_lVal(lVal) CUPC_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CUPC_PtVal(ptVal) CUPC_Debug("pt %s @ %p\n", #ptVal, ptVal)

#define NUCHARTOLONGMAX 16

/***************************************************
	variable
***************************************************/

/***************************************************
	prototype
***************************************************/
void vGetErrorFeedback(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psInfo, CHAR* pcFeedback);
void vGetCorrectFeedback(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psInfo, CHAR* pcFeedback);

/***************************************************
	function
***************************************************/
/*********************************************
* func : lNUCharToLong(UCHAR* pucHeader, LONG lPosition, LONG lCount)
* arg : UCHAR* pucHeader, LONG lPosition, LONG lCount
* ret : LONG
* note : return the value of the part string
*********************************************/
LONG lNUCharToLong(UCHAR* pucHeader, LONG lPosition, LONG lCount) {

	LONG lRetValue = 0;
	CHAR cHolder[NUCHARTOLONGMAX];
	CHAR* pcPosition = NULL;
	
	CUPC_FuncIn();

	{
		assert(lCount < NUCHARTOLONGMAX);

		/* construct the string */
		pcPosition = (CHAR*)pucHeader + lPosition;
		strncpy(cHolder, pcPosition, lCount);
		cHolder[lCount] = '\0';

		CUPC_Debug("holder : %s\n", cHolder);

		/* get the return value */
		lRetValue = atol(cHolder);

		CUPC_Debug("<%s, %ld, %ld> -> %ld\n", pucHeader, lPosition, lCount, lRetValue);
	}

	CUPC_FuncOut();

	return lRetValue;
}

/*********************************************
* func : vGetCharContent(UCHAR* pucHeader, LONG lPosition, LONG lCount, CHAR* pcContent)
* arg : UCHAR* pucHeader, LONG lPosition, LONG lCount, CHAR* pcContent
* ret : 
* note : return the content of the char
*********************************************/
void vGetCharContent(UCHAR* pucHeader, LONG lPosition, LONG lCount, CHAR* pcContent) {

	CHAR* pcPosition = NULL;
	LONG lCopyCount;
	
	CUPC_FuncIn();

	{
		assert(lCount < NUCHARTOLONGMAX);

		/* construct the string */
		pcPosition = (CHAR*)pucHeader + lPosition;
		lCopyCount = lCount;

		/* skip the leader '0' */
		while(*pcPosition == '0') {
			pcPosition ++;
			lCopyCount --;
		}

		/* copy the string */
		strncpy(pcContent, pcPosition, lCopyCount);
		pcContent[lCopyCount] = '\0';

		CUPC_Debug("<%s, %ld, %ld> -> %s\n", pucHeader, lPosition, lCount, pcContent);
	}

	CUPC_FuncOut();

	return;
}

/*********************************************
* func : vGetErrorFeedback(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psInfo, CHAR* pcFeedback)
* arg : sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psInfo, CHAR* pcFeedback
* ret : 
* note : return the error feed back
*********************************************/
void vGetErrorFeedback(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psInfo, CHAR* pcFeedback) {

	eCTRL_UARTREQ_PARSECOMMAND_Function_Ret eStatus = psInfo->eRet;
	
	CUPC_FuncIn();

	{
		/* check the precondition */
		assert(eStatus < CTRL_UARTREQ_PARSECOMMAND_ERRORMAX);
		assert(eStatus > 0);
		
		CUPC_Debug("construct error code : < %d >\n", eStatus);

		/* construct the feedback */
		sprintf(pcFeedback, "#BBADCODE%02d\n", eStatus);

		CUPC_Debug("error feedback : %s\n", pcFeedback);
	}

	CUPC_FuncOut();

	return;
}

/*********************************************
* func : vGetCorrectFeedback(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psInfo, CHAR* pcFeedback)
* arg : sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psInfo, CHAR* pcFeedback
* ret : 
* note : return the correct feed back
*********************************************/
void vGetCorrectFeedback(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psInfo, CHAR* pcFeedback) {

	CHAR* pcPositionTmp = pcFeedback;
	
	CUPC_FuncIn();

	{
		/* construct the feedback */
		pcPositionTmp += sprintf(pcPositionTmp, "#%c", psInfo->ucCommandKey);
		strncpy(pcPositionTmp, (CHAR*)psInfo->pucCommandFirst, psInfo->lCommandLen);
		pcPositionTmp += psInfo->lCommandLen;
		strcpy(pcPositionTmp, "\n");

		CUPC_Debug("correct feedback : %s\n", pcFeedback);
	}

	CUPC_FuncOut();

	return;
}

/*********************************************
* func : vGetFeedback(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psInfo, CHAR* pcFeedback)
* arg : sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psInfo, CHAR* pcFeedback
* ret : 
* note : return the feed back
*********************************************/
void vGetFeedback(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psInfo, CHAR* pcFeedback) {

	eCTRL_UARTREQ_PARSECOMMAND_Function_Ret eStatus = psInfo->eRet;
	
	CUPC_FuncIn();

	{
		if (eStatus == CTRL_UARTREQ_PARSECOMMAND_Function_Success) {

			/* correct command */
			vGetCorrectFeedback(psInfo, pcFeedback);
			
		} else {

			/* discorrect command */
			vGetErrorFeedback(psInfo, pcFeedback);
			
		}
	}

	CUPC_FuncOut();

	return;
}

#ifdef __cplusplus
}
#endif
