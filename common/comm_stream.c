#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "comm_stream.h"

/***************************************************
	macro / enum
***************************************************/
//#define COMM_STREAM_DEBUG
#ifdef COMM_STREAM_DEBUG
#define CS_Debug(fmt, arg...) fprintf(stdout, "[ CS ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CS_Debug(fmt, arg...)
#endif

#define CS_FuncIn() CS_Debug("in\n")
#define CS_FuncOut() CS_Debug("out\n")
#define CS_iVal(iVal) CS_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CS_lVal(lVal) CS_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CS_PtVal(ptVal) CS_Debug("pt %s @ %p\n", #ptVal, ptVal)


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
* func : eCOMM_STREAM_FindByte(UCHAR* pucStream, CHAR* pcByte, LONG lStreamLen, UCHAR** ppucPosition)
* arg : UCHAR* pucStream, CHAR* pcByte, LONG lStreamLen, UCHAR** ppucPosition
* ret : eCOMM_STREAM_Ret
* note : find the specific byte in the stream
*********************************************/
eCOMM_STREAM_Ret eCOMM_STREAM_FindByte(UCHAR* pucStream, CHAR* pcByte, LONG lStreamLen, UCHAR** ppucPosition) {
	eCOMM_STREAM_Ret eRet = COMM_STREAM_SUCCESS;
	LONG lEntryTmp;
	LONG lEntryLen = lStreamLen;

	CS_FuncIn();

	{
		*ppucPosition = NULL;
		for (lEntryTmp = 0; lEntryTmp != lEntryLen; lEntryTmp ++) {
			if (pucStream[lEntryTmp] == *pcByte) {
				*ppucPosition = pucStream + lEntryTmp;
				break;
			}
		}

	}

	CS_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_STREAM_Split(UCHAR* pucStream, CHAR* pcDelimi, LONG lStreamLen, UCHAR* pucEntry[], LONG* plEntryLen)
* arg : UCHAR* pucStream, CHAR* pcDelimi, LONG lStreamLen, UCHAR* pucEntry[], LONG* plEntryLen
* ret : eCOMM_STREAM_Ret
* note : split the input stream by the pcDelimi
*********************************************/
eCOMM_STREAM_Ret eCOMM_STREAM_Split(UCHAR* pucStream, CHAR* pcDelimi, LONG lStreamLen, UCHAR* pucEntry[], LONG* plEntryLen) {
	eCOMM_STREAM_Ret eRet = COMM_STREAM_SUCCESS;
	UCHAR* pucDemPos = NULL;
	UCHAR* pucPos = NULL;
	UCHAR* pucTailNextByte = NULL;
	LONG lEntryLen = 0;

	CS_FuncIn();

	{
		pucDemPos = pucStream;
		pucTailNextByte = pucStream + lStreamLen;
		while (lEntryLen < lStreamLen) {
			/* find the first not the delimi */
			eCOMM_STREAM_FindNotByte(pucDemPos, pcDelimi, pucTailNextByte - pucDemPos, &pucPos);
			if (!pucPos) {
				break;
			}

			/* get the entry */
			pucEntry [lEntryLen ++] = pucPos;

			/* find the next delimi */
			eCOMM_STREAM_FindByte(pucPos, pcDelimi, pucTailNextByte - pucPos, &pucDemPos);
			if (!pucDemPos) {
				break;
			}
		}

		/* update entry length */
		*plEntryLen = lEntryLen;
	}

	CS_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_STREAM_FindNotByte(UCHAR* pucStream, CHAR* pcByte, LONG lStreamLen, UCHAR** ppucPosition)
* arg : UCHAR* pucStream, CHAR* pcByte, LONG lStreamLen, UCHAR** ppucPosition
* ret : eCOMM_STREAM_Ret
* note : find the first not the specific byte in the stream
*********************************************/
eCOMM_STREAM_Ret eCOMM_STREAM_FindNotByte(UCHAR* pucStream, CHAR* pcByte, LONG lStreamLen, UCHAR** ppucPosition) {
	eCOMM_STREAM_Ret eRet = COMM_STREAM_SUCCESS;
	LONG lEntryTmp;
	LONG lEntryLen = lStreamLen;

	CS_FuncIn();

	{
		*ppucPosition = NULL;
		for (lEntryTmp = 0; lEntryTmp != lEntryLen; lEntryTmp ++) {
			if (pucStream[lEntryTmp] != *pcByte) {
				*ppucPosition = pucStream + lEntryTmp;
				break;
			}
		}

	}

	CS_FuncOut();

	return eRet;
}

#ifdef __cplusplus
}
#endif
