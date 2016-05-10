#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************
	include
*****************************************************/
#include "comm_mw_strpool.h"

/*****************************************************
	define / enum
*****************************************************/
// #define COMM_MW_STRPOOL_Debug
#ifdef COMM_MW_STRPOOL_Debug
#define CMS_Debug(fmt, args...) fprintf(stdout, "[ CMS ] : %s <%d> "fmt, __func__, __LINE__, ##args)
#define CMS_FuncIn() fprintf(stdout, "[ CMS ] : %s <%d> in\n", __func__, __LINE__)
#define CMS_FuncOut() fprintf(stdout, "[ CMS ] : %s <%d> out\n", __func__, __LINE__)
#define CMS_iVal(iVal) fprintf(stdout, "[ CMS ] : %s <%d> %s <%d> @ <%p>\n", __func__, __LINE__, #iVal, iVal, &iVal)
#define CMS_lVal(lVal) fprintf(stdout, "[ CMS ] : %s <%d> %s <%ld> @ <%p>\n", __func__, __LINE__, #lVal, lVal, &lVal)
#define CMS_PtVal(PtVal) fprintf(stdout, "[ CMS ] : %s <%d> %s <%p> @ <%p>\n", __func__, __LINE__, #PtVal, PtVal, &PtVal)
#else
#define CMS_Debug(fmt, args...)
#define CMS_FuncIn()
#define CMS_FuncOut()
#define CMS_iVal(iVal)
#define CMS_lVal(lVal)
#define CMS_PtVal(PtVal)
#endif

#define COMM_MW_StrPool_HeaderSize 16

/*****************************************************
	struct
*****************************************************/
typedef struct {
	UCHAR* pucHeader;
	UCHAR* pucWriter;
	LONG lPoolSize;
} sCOMM_MW_StrPool;

enum {
	COMM_MW_StrPoolHeader,
	COMM_MW_StrPoolWriter,
	COMM_MW_StrPoolPoolSize,
	COMM_MW_StrPoolRes,
} eCOMM_MW_StrPoolHeader;

/*****************************************************
	variable
*****************************************************/
static sCOMM_MW_StrPool gsStrPool;

/*****************************************************
	prototype
*****************************************************/
eCOMM_MW_StrPoolRet COMM_MW_StrPool_GetStrPool(UCHAR* pcStrPool, sCOMM_MW_StrPool** ppsStrPool);
eCOMM_MW_StrPoolRet COMM_MW_StrPool_UpdateStrPool(UCHAR* pcStrPool, sCOMM_MW_StrPool* psStrPool);
eCOMM_MW_StrPoolRet COMM_MW_StrPool_WriteEntryPrePare(sCOMM_MW_StrPool* psStrPool, CHAR* pcWriteEntry);

/*****************************************************
	funciton
*****************************************************/

/*****************************************************
 * function :	COMM_MW_StrPool_Init(UCHAR* pcStrPool, LONG lPoolSize)
 * argument :	UCHAR* pcStrPool, LONG lPoolSize
 * return :		eCOMM_MW_StrPoolRet
 * note :		init string pool
 ****************************************************/
eCOMM_MW_StrPoolRet COMM_MW_StrPool_Init(UCHAR* pcStrPool, LONG lPoolSize) {
	eCOMM_MW_StrPoolRet eRet = COMM_MW_StrPoolSuccess;
	LONG* plStrPool = (LONG*) pcStrPool;

	CMS_FuncIn();

	{
		/* clear the memory */
		memset(pcStrPool, 0, lPoolSize);

		/* regular the pool size */
		lPoolSize &= ~0x1;
		
		/* header(4), writer(4), poolsize(4), reserve(4) */
		plStrPool[COMM_MW_StrPoolHeader] = COMM_MW_StrPool_HeaderSize;
		plStrPool[COMM_MW_StrPoolWriter] = COMM_MW_StrPool_HeaderSize;
		plStrPool[COMM_MW_StrPoolPoolSize] = lPoolSize - COMM_MW_StrPool_HeaderSize;
	}
	
	CMS_FuncOut();

	return eRet;
}

/*****************************************************
 * function :	COMM_MW_StrPool_WriteEntry(sCOMM_MW_StrPool* psStrPool, CHAR* pcWriteEntry, LONG* plPosition)
 * argument :	sCOMM_MW_StrPool* psStrPool, CHAR* pcWriteEntry, LONG* plPosition
 * return :		eCOMM_MW_StrPoolRet
 * note :		write a entry to share memory
 ****************************************************/
eCOMM_MW_StrPoolRet COMM_MW_StrPool_WriteEntry(UCHAR* pcStrPool, CHAR* pcWriteEntry, LONG* plPosition, pthread_mutex_t* psMutWt) {
	eCOMM_MW_StrPoolRet eRet = COMM_MW_StrPoolSuccess;
	sCOMM_MW_StrPool* psStrPool = NULL;

	CMS_FuncIn();

	{
		pthread_mutex_lock(psMutWt);
		
		COMM_MW_StrPool_GetStrPool(pcStrPool, &psStrPool);
		eRet = COMM_MW_StrPool_WriteEntryPrePare(psStrPool, pcWriteEntry);
		if(eRet != COMM_MW_StrPoolSuccess) {
			pthread_mutex_unlock(psMutWt);
			return eRet;
		}

		*plPosition = psStrPool->pucWriter - psStrPool->pucHeader;
		strcpy((CHAR*)psStrPool->pucWriter, pcWriteEntry);
		psStrPool->pucWriter += strlen((CHAR*)psStrPool->pucWriter) + 1;

		COMM_MW_StrPool_UpdateStrPool(pcStrPool, psStrPool);
		
		pthread_mutex_unlock(psMutWt);
	}

	CMS_FuncOut();

	return eRet;
}

/*****************************************************
 * function :	COMM_MW_StrPool_ReadEntry(sCOMM_MW_StrPool* psStrPool, CHAR* pcReadEntry, LONG lPosition)
 * argument :	sCOMM_MW_StrPool* psStrPool, CHAR* pcReadEntry, LONG lPosition
 * return :		eCOMM_MW_StrPoolRet
 * note :		read a entry to share memory
 ****************************************************/
eCOMM_MW_StrPoolRet COMM_MW_StrPool_ReadEntry(UCHAR* pcStrPool, CHAR* pcReadEntry, LONG lPosition) {
	eCOMM_MW_StrPoolRet eRet = COMM_MW_StrPoolSuccess;
	sCOMM_MW_StrPool* psStrPool = NULL;
	UCHAR* pucSrc;

	CMS_FuncIn();

	{
		COMM_MW_StrPool_GetStrPool(pcStrPool, &psStrPool);
		
		/* first check the valid of position */
		if(lPosition > psStrPool->lPoolSize) {
			CMS_Debug("wrong position to read\n");
			return COMM_MW_StrPoolFull;
		}

		pucSrc = psStrPool->pucHeader + lPosition;
		strcpy(pcReadEntry, (CHAR*)pucSrc);

	}
	
	CMS_FuncOut();
	
	return eRet;
}

/*****************************************************
 * function :	COMM_MW_StrPool_WriteEntryPrePare(sCOMM_MW_StrPool* psStrPool, CHAR* pcWriteEntry)
 * argument :	sCOMM_MW_StrPool* psStrPool, CHAR* pcWriteEntry
 * return :		eCOMM_MW_StrPoolRet
 * note :		prepare write
 ****************************************************/
eCOMM_MW_StrPoolRet COMM_MW_StrPool_WriteEntryPrePare(sCOMM_MW_StrPool* psStrPool, CHAR* pcWriteEntry) {
	eCOMM_MW_StrPoolRet eRet = COMM_MW_StrPoolSuccess;
	UCHAR* pucHeader = NULL;
	UCHAR* pucNewHeader = NULL;
	UCHAR* pucWp = psStrPool->pucWriter;
	LONG lPoolHalfSize = psStrPool->lPoolSize >> 1;
	LONG lCurPos = psStrPool->pucWriter - psStrPool->pucHeader;
	LONG lWtCnt = strlen(pcWriteEntry) + 1;

	CMS_FuncIn();

	{
		if(lWtCnt > lPoolHalfSize) {
			CMS_Debug("too large to write\n");
			return COMM_MW_StrPoolTooLarge;
		}
		
		if(lCurPos > lPoolHalfSize) {
			pucHeader = psStrPool->pucHeader + lPoolHalfSize;
			pucNewHeader = psStrPool->pucHeader;
		} else {
			pucHeader = psStrPool->pucHeader;
			pucNewHeader = psStrPool->pucHeader + lPoolHalfSize;
		}

		if((pucWp - pucHeader + lWtCnt) > lPoolHalfSize) {
			psStrPool->pucWriter = pucNewHeader;
			memset(pucNewHeader, 0, lPoolHalfSize);
		}

	}
	
	CMS_FuncOut();
	
	return eRet;
}

/*****************************************************
 * function :	COMM_MW_StrPool_GetStrPool(UCHAR* pcStrPool, sCOMM_MW_StrPool* psStrPool)
 * argument :	UCHAR* pcStrPool, sCOMM_MW_StrPool* psStrPool
 * return :		eCOMM_MW_StrPoolRet
 * note :		get a str pool pointer
 ****************************************************/
eCOMM_MW_StrPoolRet COMM_MW_StrPool_GetStrPool(UCHAR* pcStrPool, sCOMM_MW_StrPool** ppsStrPool) {
	eCOMM_MW_StrPoolRet eRet = COMM_MW_StrPoolSuccess;
	LONG* plStrPool = NULL;

	CMS_FuncIn();

	{
		plStrPool = (LONG*) pcStrPool;

		gsStrPool.lPoolSize = plStrPool[ COMM_MW_StrPoolPoolSize ];
		gsStrPool.pucHeader = plStrPool[ COMM_MW_StrPoolHeader ] + pcStrPool;
		gsStrPool.pucWriter = plStrPool[ COMM_MW_StrPoolWriter ] + pcStrPool;
		
		*ppsStrPool = &gsStrPool;
	}
	
	CMS_FuncOut();
	
	return eRet;
}

/*****************************************************
 * function :	COMM_MW_StrPool_UpdateStrPool(UCHAR* pcStrPool, sCOMM_MW_StrPool* psStrPool)
 * argument :	UCHAR* pcStrPool, sCOMM_MW_StrPool* psStrPool
 * return :		eCOMM_MW_StrPoolRet
 * note :		update string pool
 ****************************************************/
eCOMM_MW_StrPoolRet COMM_MW_StrPool_UpdateStrPool(UCHAR* pcStrPool, sCOMM_MW_StrPool* psStrPool) {
	eCOMM_MW_StrPoolRet eRet = COMM_MW_StrPoolSuccess;
	LONG* plStrPool = (LONG*)pcStrPool;

	CMS_FuncIn();

	{
		plStrPool[COMM_MW_StrPoolWriter] = (LONG) (psStrPool->pucWriter - pcStrPool);
	}
	
	CMS_FuncOut();

	return eRet;
}



#ifdef __cplusplus
}
#endif
