#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "ctrl_uartReq_saveReq.h"

/***************************************************
	macro / enum
***************************************************/
//#define CTRL_UARTREQ_SAVEREQ_DEBUG
#ifdef CTRL_UARTREQ_SAVEREQ_DEBUG
#define CUS_Debug(fmt, arg...) fprintf(stdout, "[ CUS ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CUS_Debug(fmt, arg...)
#endif

#define CUS_FuncIn() CUS_Debug("in\n")
#define CUS_FuncOut() CUS_Debug("out\n")
#define CUS_iVal(iVal) CUS_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CUS_lVal(lVal) CUS_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CUS_PtVal(ptVal) CUS_Debug("pt %s @ %p\n", #ptVal, ptVal)


/***************************************************
	variable
***************************************************/

/***************************************************
	prototype
***************************************************/

/***************************************************
	function
***************************************************/

/*********************************************
* func : eCTRL_UARTREQ_SAVEREQ_Init(sCTRL_UARTREQ_SAVEREQ_Info* psInfo, UCHAR* pucRequestHolder, LONG lRequestHolderMax)
* arg : sCTRL_UARTREQ_SAVEREQ_Info* psInfo, UCHAR* pucRequestHolder, LONG lRequestHolderMax
* ret : eCTRL_UARTREQ_SAVEREQ_Ret
* note :
*********************************************/
eCTRL_UARTREQ_SAVEREQ_Ret eCTRL_UARTREQ_SAVEREQ_Init(sCTRL_UARTREQ_SAVEREQ_Info* psInfo, UCHAR* pucRequestHolder, LONG lRequestHolderMax) {
	eCTRL_UARTREQ_SAVEREQ_Ret eRet = CTRL_UARTREQ_SAVEREQ_SUCCESS;

	CUS_FuncIn();

	{
		psInfo->pucRequestHolder = pucRequestHolder;
		psInfo->lRequestHolderMax = lRequestHolderMax;
		psInfo->pcTail = (CHAR*)pucRequestHolder + lRequestHolderMax;
		eCTRL_UARTREQ_SAVEREQ_Reset(psInfo);
	}

	CUS_FuncOut();

	return eRet;
}

/*********************************************
* func : eCTRL_UARTREQ_SAVEREQ_Reset(sCTRL_UARTREQ_SAVEREQ_Info* psInfo)
* arg : sCTRL_UARTREQ_SAVEREQ_Info* psInfo
* ret : eCTRL_UARTREQ_SAVEREQ_Ret
* note :
*********************************************/
eCTRL_UARTREQ_SAVEREQ_Ret eCTRL_UARTREQ_SAVEREQ_Reset(sCTRL_UARTREQ_SAVEREQ_Info* psInfo) {
	eCTRL_UARTREQ_SAVEREQ_Ret eRet = CTRL_UARTREQ_SAVEREQ_SUCCESS;

	CUS_FuncIn();

	{
		psInfo->lCurReadCnt = 0;
		psInfo->lCurWriteCnt = 0;

		LONG lRequestEntry;
		for (lRequestEntry = 0; lRequestEntry != CTRL_UARTREQ_SAVEREQ_REQMAX; lRequestEntry++) {
			psInfo->pcRequest[lRequestEntry] = NULL;
		}

		psInfo->pcWriter = (CHAR*) psInfo->pucRequestHolder;
	}

	CUS_FuncOut();

	return eRet;
}

/*********************************************
* func : eCTRL_UARTREQ_SAVEREQ_WriteReq(sCTRL_UARTREQ_SAVEREQ_Info* psInfo, CHAR* pcRequest)
* arg : sCTRL_UARTREQ_SAVEREQ_Info* psInfo, CHAR* pcRequest
* ret : eCTRL_UARTREQ_SAVEREQ_Ret
* note :
*********************************************/
eCTRL_UARTREQ_SAVEREQ_Ret eCTRL_UARTREQ_SAVEREQ_WriteReq(sCTRL_UARTREQ_SAVEREQ_Info* psInfo, CHAR* pcRequest) {
	eCTRL_UARTREQ_SAVEREQ_Ret eRet = CTRL_UARTREQ_SAVEREQ_SUCCESS;

	CUS_FuncIn();

	{
		/* judge whether have space to write */
		if (psInfo->lCurWriteCnt == CTRL_UARTREQ_SAVEREQ_REQMAX) {
			CUS_Debug("too much request !\n");
			return CTRL_UARTREQ_SAVEREQ_TOOMUCHREQ;
		}

		if ((psInfo->pcWriter + strlen(pcRequest) + 1) > psInfo->pcTail) {
			CUS_Debug("the request is full !\n");
			return CTRL_UARTREQ_SAVEREQ_FULL;
		}

		/* write the request to the holder */
		strcpy(psInfo->pcWriter, pcRequest);
		psInfo->pcRequest[psInfo->lCurWriteCnt] = psInfo->pcWriter;

		/* update the write count */
		psInfo->lCurWriteCnt ++;
		psInfo->pcWriter += strlen(pcRequest) + 1;

	}

	CUS_FuncOut();

	return eRet;
}

/*********************************************
* func : eCTRL_UARTREQ_SAVEREQ_ReadReq(sCTRL_UARTREQ_SAVEREQ_Info* psInfo, CHAR** ppcRequest)
* arg : sCTRL_UARTREQ_SAVEREQ_Info* psInfo, CHAR** ppcRequest
* ret : eCTRL_UARTREQ_SAVEREQ_Ret
* note :
*********************************************/
eCTRL_UARTREQ_SAVEREQ_Ret eCTRL_UARTREQ_SAVEREQ_ReadReq(sCTRL_UARTREQ_SAVEREQ_Info* psInfo, CHAR** ppcRequest) {
	eCTRL_UARTREQ_SAVEREQ_Ret eRet = CTRL_UARTREQ_SAVEREQ_SUCCESS;

	CUS_FuncIn();

	{
		/* judge whether reach the max */
		if(psInfo->lCurReadCnt == psInfo->lCurWriteCnt) {
			*ppcRequest = NULL;
			return CTRL_UARTREQ_SAVEREQ_READEND;
		}

		/* return the request */
		*ppcRequest = psInfo->pcRequest[psInfo->lCurReadCnt];
		psInfo->lCurReadCnt ++;

	}

	CUS_FuncOut();

	return eRet;
}

/*********************************************
* func : eCTRL_UARTREQ_SAVEREQ_Show(sCTRL_UARTREQ_SAVEREQ_Info* psInfo)
* arg : sCTRL_UARTREQ_SAVEREQ_Info* psInfo
* ret : eCTRL_UARTREQ_SAVEREQ_Ret
* note :
*********************************************/
eCTRL_UARTREQ_SAVEREQ_Ret eCTRL_UARTREQ_SAVEREQ_Show(sCTRL_UARTREQ_SAVEREQ_Info* psInfo) {
	eCTRL_UARTREQ_SAVEREQ_Ret eRet = CTRL_UARTREQ_SAVEREQ_SUCCESS;

	CUS_FuncIn();

	{
		printf("save request information : \n");
		printf("Header : < %p > Len : < %ld > write cnt : < %ld > read cnt : < %ld >\n", 
			psInfo->pucRequestHolder, psInfo->lRequestHolderMax, psInfo->lCurWriteCnt, psInfo->lCurReadCnt);

		printf("consumerNo  pointer     content\n");

		LONG lConsumer;
		for (lConsumer = 0; lConsumer != psInfo->lCurWriteCnt; lConsumer++) {
			printf("%-10ld  %p  %s\n", lConsumer, psInfo->pcRequest[lConsumer], psInfo->pcRequest[lConsumer]);
		}
	}

	CUS_FuncOut();

	return eRet;
}

#ifdef __cplusplus
}
#endif
