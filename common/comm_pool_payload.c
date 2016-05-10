#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "comm_pool_payload.h"
#include <assert.h>

/***************************************************
	macro / enum
***************************************************/
//#define COMM_POOL_PAYLOAD_DEBUG
#ifdef COMM_POOL_PAYLOAD_DEBUG
#define CPP_Debug(fmt, arg...) fprintf(stdout, "[ POOL PAYLOAD ] : %s() : <%d> "fmt, __func__, __LINE__, ##arg)
#else
#define CPP_Debug(fmt, arg...)
#endif

#define CPP_FuncIn() CPP_Debug("in\n")
#define CPP_FuncOut() CPP_Debug("out\n")
#define CPP_iVal(ival) CPP_Debug("%s <%d> @ %p\n", #ival, ival, &ival)
#define CPP_lVal(lval) CPP_Debug("%s <%ld> @ %p\n", #lval, lval, &lval)
#define CPP_PtVal(ptval) CPP_Debug("%s @ %p\n", #ptval, ptval)

typedef enum {
	COMM_POOL_PAYLOAD_WRITER,
	COMM_POOL_PAYLOAD_READER,
} eCOMM_POOL_PAYLOAD_TYPE;

/***************************************************
	struct
***************************************************/

/***************************************************
	variable
***************************************************/

/***************************************************
	prototype
***************************************************/
eCOMM_POOL_PAYLOAD_Ret eCOMM_POOL_PAYLOAD_QueryAndFill(sCOMM_POOL_PAYLOAD_Info* psInfo, UCHAR* pucBuf, LONG lLen, eCOMM_POOL_PAYLOAD_TYPE eType);

/***************************************************
	function
***************************************************/

/***************************************************
 * func : eCOMM_POOL_PAYLOAD_Init(sCOMM_POOL_PAYLOAD_Info* psInfo, LONG lPayloadLen, LONG lUserExpandLength)
 * arg : sCOMM_POOL_PAYLOAD_Info* psInfo, LONG lPayloadLen, LONG lUserExpandLength
 * ret : eCOMM_POOL_PAYLOAD_Ret
 * note : init pool payload
 **************************************************/
eCOMM_POOL_PAYLOAD_Ret eCOMM_POOL_PAYLOAD_Init(sCOMM_POOL_PAYLOAD_Info* psInfo, LONG lPayloadLen, LONG lUserExpandLength) {
	eCOMM_POOL_PAYLOAD_Ret eRet = COMM_POOL_PAYLOAD_SUCCESS;

	CPP_FuncIn();

	{
		/* status and length */
		psInfo->eStatus = COMM_POOL_PAYLOAD_NORMAL;
		psInfo->lPayLoadLength = lPayloadLen;
		psInfo->lUserExpandLength = lUserExpandLength;

		/* header wt, read */
		MALLOC((lPayloadLen + lUserExpandLength) * sizeof(UCHAR));
		psInfo->pucHeader = malloc((lPayloadLen + lUserExpandLength) * sizeof(UCHAR));
		if(!psInfo->pucHeader) {
			CPP_Debug("malloc error\n");
			return COMM_POOL_PAYLOAD_RESOURCEERROR;
		}

		/* init the payload */
		memset(psInfo->pucHeader, 0, lPayloadLen + lUserExpandLength);
		
		psInfo->pucTail = psInfo->pucHeader + lPayloadLen;
		psInfo->pucReader = psInfo->pucHeader;
		psInfo->pucWriter = psInfo->pucHeader;
		
		/* condition mutex */
		pthread_cond_init(&psInfo->condRd, NULL);
		pthread_cond_init(&psInfo->condWt, NULL);
		pthread_mutex_init(&psInfo->mutPayload, NULL);

	}
	
	CPP_FuncOut();
	
	return eRet;
}

/***************************************************
 * func : eCOMM_POOL_PAYLOAD_Write(sCOMM_POOL_PAYLOAD_Info* psInfo, UCHAR* pucSrc, LONG lWtLen)
 * arg : sCOMM_POOL_PAYLOAD_Info* psInfo, UCHAR* pucSrc, LONG lWtLen
 * ret : eCOMM_POOL_PAYLOAD_Ret
 * note : write to the pool payload
 **************************************************/
eCOMM_POOL_PAYLOAD_Ret eCOMM_POOL_PAYLOAD_Write(sCOMM_POOL_PAYLOAD_Info* psInfo, UCHAR* pucSrc, LONG lWtLen) {
	eCOMM_POOL_PAYLOAD_Ret eRet = COMM_POOL_PAYLOAD_SUCCESS;

	CPP_FuncIn();

	{
		/* get lock and wait for resource */
		pthread_mutex_lock(&psInfo->mutPayload);

		while(eCOMM_POOL_PAYLOAD_QueryAndFill(psInfo, pucSrc, lWtLen, COMM_POOL_PAYLOAD_WRITER) != COMM_POOL_PAYLOAD_SUCCESS) {
			
			psInfo->eStatus |= COMM_POOL_PAYLOAD_FULL;
			CPP_Debug("full now\n");	

			CPP_PtVal(psInfo->pucHeader);
			CPP_PtVal(psInfo->pucReader);
			CPP_PtVal(psInfo->pucWriter);
			CPP_PtVal(psInfo->pucTail);
			CPP_lVal(lWtLen);
		
			pthread_cond_wait(&psInfo->condWt, &psInfo->mutPayload);
			
		}
		
		pthread_mutex_unlock(&psInfo->mutPayload);

		if(psInfo->eStatus & COMM_POOL_PAYLOAD_EMPTY) {
			pthread_mutex_lock(&psInfo->mutPayload);
			psInfo->eStatus &= (~COMM_POOL_PAYLOAD_EMPTY);
			pthread_cond_signal(&psInfo->condRd);
			pthread_mutex_unlock(&psInfo->mutPayload);
		}
	
		CPP_PtVal(psInfo->pucHeader);
		CPP_PtVal(psInfo->pucTail);
		CPP_PtVal(psInfo->pucReader);
		CPP_PtVal(psInfo->pucWriter);
	}
	
	CPP_FuncOut();
	
	return eRet;
}

/***************************************************
 * func : eCOMM_POOL_PAYLOAD_Read(sCOMM_POOL_PAYLOAD_Info* psInfo, UCHAR* pucSrc, LONG lRdLen)
 * arg : sCOMM_POOL_PAYLOAD_Info* psInfo, UCHAR* pucSrc, LONG lRdLen
 * ret : eCOMM_POOL_PAYLOAD_Ret
 * note : read from the pool payload
 **************************************************/
eCOMM_POOL_PAYLOAD_Ret eCOMM_POOL_PAYLOAD_Read(sCOMM_POOL_PAYLOAD_Info* psInfo, UCHAR* pucSrc, LONG lRdLen) {
	eCOMM_POOL_PAYLOAD_Ret eRet = COMM_POOL_PAYLOAD_SUCCESS;

	CPP_FuncIn();

	{
		/* we can sure the pucSrc equal psInfo->pucReader */
		assert(pucSrc == psInfo->pucReader);

		/* get lock */
		pthread_mutex_lock(&psInfo->mutPayload);
		while(eCOMM_POOL_PAYLOAD_QueryAndFill(psInfo, NULL, lRdLen, COMM_POOL_PAYLOAD_READER) != COMM_POOL_PAYLOAD_SUCCESS) {
			/* indicate quit */		
			psInfo->eStatus |= COMM_POOL_PAYLOAD_EMPTY;
			CPP_Debug("empty now\n");
			pthread_cond_wait(&psInfo->condRd, &psInfo->mutPayload);
		}
		pthread_mutex_unlock(&psInfo->mutPayload);

		if(psInfo->eStatus & COMM_POOL_PAYLOAD_FULL) {
			pthread_mutex_lock(&psInfo->mutPayload);
			psInfo->eStatus &= (~COMM_POOL_PAYLOAD_FULL);
			pthread_cond_signal(&psInfo->condWt);
			pthread_mutex_unlock(&psInfo->mutPayload);
		}
	}
	
	CPP_FuncOut();
	
	return eRet;
}

/***************************************************
 * func : eCOMM_POOL_PAYLOAD_Deinit(sCOMM_POOL_PAYLOAD_Info* psInfo)
 * arg : sCOMM_POOL_PAYLOAD_Info* psInfo
 * ret : eCOMM_POOL_PAYLOAD_Ret
 * note : deinit the pool payload
 **************************************************/
eCOMM_POOL_PAYLOAD_Ret eCOMM_POOL_PAYLOAD_Deinit(sCOMM_POOL_PAYLOAD_Info* psInfo) {
	eCOMM_POOL_PAYLOAD_Ret eRet = COMM_POOL_PAYLOAD_SUCCESS;

	CPP_FuncIn();

	{

		/* status length */
		psInfo->eStatus = COMM_POOL_PAYLOAD_INVALID;
		psInfo->lPayLoadLength = 0;

		/* pointer */
		psInfo->pucReader = NULL;
		psInfo->pucWriter = NULL;
		psInfo->pucTail = NULL;
		free(psInfo->pucHeader);
		psInfo->pucHeader = NULL;

		/* mutex, condition */
		pthread_cond_destroy(&psInfo->condRd);
		pthread_cond_destroy(&psInfo->condWt);
		pthread_mutex_destroy(&psInfo->mutPayload);

	}
	
	CPP_FuncOut();
	
	return eRet;
}

/***************************************************
 * func : eCOMM_POOL_PAYLOAD_QueryAndFill(sCOMM_POOL_PAYLOAD_Info* psInfo, UCHAR* pucBuf, LONG lLen, eCOMM_POOL_PAYLOAD_TYPE eType)
 * arg : sCOMM_POOL_PAYLOAD_Info* psInfo, UCHAR* pucBuf, LONG lLen, eCOMM_POOL_PAYLOAD_TYPE eType
 * ret : eCOMM_POOL_PAYLOAD_Ret
 * note : query pointer
 **************************************************/
eCOMM_POOL_PAYLOAD_Ret eCOMM_POOL_PAYLOAD_QueryAndFill(sCOMM_POOL_PAYLOAD_Info* psInfo, UCHAR* pucBuf, LONG lLen, eCOMM_POOL_PAYLOAD_TYPE eType) {
	eCOMM_POOL_PAYLOAD_Ret eRet = COMM_POOL_PAYLOAD_FAIL;
	LONG lToTail;
	LONG lRemain;

	CPP_FuncIn();

	{
		CPP_iVal(eType);
		CPP_PtVal(psInfo->pucHeader);
		CPP_PtVal(psInfo->pucReader);
		CPP_PtVal(psInfo->pucWriter);
		CPP_PtVal(psInfo->pucTail);
		CPP_lVal(lLen);

		if(eType == COMM_POOL_PAYLOAD_WRITER) {
			/* writer */
			if(psInfo->pucReader <= psInfo->pucWriter) {
				if(psInfo->pucWriter + lLen <= psInfo->pucTail) {
					assert(!(psInfo->eStatus & COMM_POOL_PAYLOAD_WRAP));
					/* case 1 : head -> rd -> wt -> tail (unwrap) */
					memcpy(psInfo->pucWriter, pucBuf, lLen);
					psInfo->pucWriter += lLen;
					return COMM_POOL_PAYLOAD_SUCCESS;
				} else if(psInfo->pucWriter + lLen - psInfo->lPayLoadLength < psInfo->pucReader) {
					assert(!(psInfo->eStatus & COMM_POOL_PAYLOAD_WRAP));
					/* case 2 : head -> rd -> wt -> tail (unwrap -> wrap) */
					lToTail = psInfo->pucTail - psInfo->pucWriter;
					lRemain = lLen - lToTail;
					memcpy(psInfo->pucWriter, pucBuf, lToTail);
					memcpy(psInfo->pucHeader, pucBuf + lToTail, lRemain);
					psInfo->pucWriter = psInfo->pucHeader + lRemain;
					psInfo->eStatus |= COMM_POOL_PAYLOAD_WRAP;
					return COMM_POOL_PAYLOAD_SUCCESS;
				}
			} else {
				if(psInfo->pucWriter + lLen < psInfo->pucReader) {
					assert(psInfo->eStatus & COMM_POOL_PAYLOAD_WRAP);
					/* case 3 : head -> wt -> rd -> tail (wrap) */
					memcpy(psInfo->pucWriter, pucBuf, lLen);
					psInfo->pucWriter += lLen;
					return COMM_POOL_PAYLOAD_SUCCESS;
				}
			}
		} else {
			/* reader */
			if(psInfo->pucReader <= psInfo->pucWriter) {
				if(psInfo->pucReader + lLen <= psInfo->pucWriter) {
					assert(!(psInfo->eStatus & COMM_POOL_PAYLOAD_WRAP));
					/* case 1 : head -> rd -> wt -> tail (unwrap) */
					memset(psInfo->pucReader, 0, lLen);
					psInfo->pucReader += lLen;
					return COMM_POOL_PAYLOAD_SUCCESS;
				}
			} else {
				if(psInfo->pucReader + lLen <= psInfo->pucTail) {
					assert(psInfo->eStatus & COMM_POOL_PAYLOAD_WRAP);
					/* case 2 : head -> wt -> rd -> tail (wrap) */
					memset(psInfo->pucReader, 0, lLen);
					psInfo->pucReader += lLen;
					return COMM_POOL_PAYLOAD_SUCCESS;
				} else if(psInfo->pucReader + lLen - psInfo->lPayLoadLength <= psInfo->pucWriter) {
					assert(psInfo->eStatus & COMM_POOL_PAYLOAD_WRAP);
					/* case 3 : head -> wt -> rd -> tail (wrap -> unwrap) */
					lToTail = psInfo->pucTail - psInfo->pucReader;
					lRemain = lLen - lToTail;
					memset(psInfo->pucReader, 0, lToTail);
					memset(psInfo->pucHeader, 0, lRemain);
					psInfo->pucReader = psInfo->pucHeader + lRemain;
					psInfo->eStatus &= ~COMM_POOL_PAYLOAD_WRAP;
					return COMM_POOL_PAYLOAD_SUCCESS;
				}
			}
		}
	}
	
	CPP_FuncOut();
	
	return eRet;
}

/***************************************************
 * func : eCOMM_POOL_PAYLOAD_GetWriter(sCOMM_POOL_PAYLOAD_Info* psInfo, UCHAR** ppucWt)
 * arg : sCOMM_POOL_PAYLOAD_Info* psInfo, UCHAR** ppucWt
 * ret : eCOMM_POOL_PAYLOAD_Ret
 * note : get the current writer
 **************************************************/
eCOMM_POOL_PAYLOAD_Ret eCOMM_POOL_PAYLOAD_GetWriter(sCOMM_POOL_PAYLOAD_Info* psInfo, UCHAR** ppucWt) {
	eCOMM_POOL_PAYLOAD_Ret eRet = COMM_POOL_PAYLOAD_SUCCESS;

	CPP_FuncIn();

	{
		*ppucWt = psInfo->pucWriter;
	}
	
	CPP_FuncOut();

	return eRet;
}

/***************************************************
 * func : eCOMM_POOL_PAYLOAD_WriteExtraData(sCOMM_POOL_PAYLOAD_Info* psInfo, UCHAR* pucSrc, LONG lWtLen)
 * arg : sCOMM_POOL_PAYLOAD_Info* psInfo, UCHAR* pucSrc, LONG lWtLen
 * ret : eCOMM_POOL_PAYLOAD_Ret
 * note : writer extra data to payload
 **************************************************/
eCOMM_POOL_PAYLOAD_Ret eCOMM_POOL_PAYLOAD_WriteExtraData(sCOMM_POOL_PAYLOAD_Info* psInfo, UCHAR* pucSrc, LONG lWtLen) {
	eCOMM_POOL_PAYLOAD_Ret eRet = COMM_POOL_PAYLOAD_SUCCESS;
	LONG lWriteLen = lWtLen;

	CPP_FuncIn();

	{
		/* judge whether exceed the threshold */
		assert (lWriteLen <= psInfo->lUserExpandLength) ;

		/* write to the expand space */
		memset(psInfo->pucTail, 0, psInfo->lUserExpandLength);
		memcpy(psInfo->pucTail, pucSrc, lWriteLen);
	}
	
	CPP_FuncOut();
	
	return eRet;
}

#ifdef __cplusplus
}
#endif

