#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "comm_mw_time.h"

/***************************************************
	macro / enum
***************************************************/
//#define COMM_MW_TIME_DEBUG
#ifdef COMM_MW_TIME_DEBUG
#define CMT_Debug(fmt, arg...) fprintf(stdout, "[ CMT ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CMT_Debug(fmt, arg...)
#endif

#define CMT_FuncIn() CMT_Debug("in\n")
#define CMT_FuncOut() CMT_Debug("out\n")
#define CMT_iVal(iVal) CMT_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CMT_lVal(lVal) CMT_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CMT_PtVal(ptVal) CMT_Debug("pt %s @ %p\n", #ptVal, ptVal)


/***************************************************
	variable
***************************************************/
static pthread_mutex_t mutVisitTime = PTHREAD_MUTEX_INITIALIZER;

/***************************************************
	prototype
***************************************************/

/***************************************************
	function
***************************************************/

/*********************************************
* func : eCOMM_MW_TIME_Init(sCOMM_MW_TIME_Info* psInfo, CHAR* pcName)
* arg  : sCOMM_MW_TIME_Info* psInfo, CHAR* pcName
* ret  : eCOMM_MW_TIME_Ret
* note :
*********************************************/
eCOMM_MW_TIME_Ret eCOMM_MW_TIME_Init(sCOMM_MW_TIME_Info* psInfo, CHAR* pcName) {
	eCOMM_MW_TIME_Ret eRet = COMM_MW_TIME_SUCCESS;

	CMT_FuncIn();

	{
		/* check */
		assert(strlen(pcName) < COMM_MW_TIME_NAME_MAX);
		
		/* clear the buffer */
		memset(psInfo, 0, sizeof(sCOMM_MW_TIME_Info));

		/* init */
		strcpy(psInfo->cName, pcName);
		psInfo->lDeltTimeSec = 0;
		psInfo->sCurrentVisitTime = time(NULL);
		psInfo->sSaveVisitTime = psInfo->sCurrentVisitTime;

	}

	CMT_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_MW_TIME_Reset(sCOMM_MW_TIME_Info* psInfo)
* arg  : sCOMM_MW_TIME_Info* psInfo
* ret  : eCOMM_MW_TIME_Ret
* note :
*********************************************/
eCOMM_MW_TIME_Ret eCOMM_MW_TIME_Reset(sCOMM_MW_TIME_Info* psInfo) {
	eCOMM_MW_TIME_Ret eRet = COMM_MW_TIME_SUCCESS;

	CMT_FuncIn();

	{
		psInfo->sCurrentVisitTime = 0;
		psInfo->sSaveVisitTime = 0;
		psInfo->lDeltTimeSec = 0;
	}

	CMT_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_MW_TIME_UpdatePre(sCOMM_MW_TIME_Info* psInfo)
* arg  : sCOMM_MW_TIME_Info* psInfo
* ret  : eCOMM_MW_TIME_Ret
* note :
*********************************************/
eCOMM_MW_TIME_Ret eCOMM_MW_TIME_Update(sCOMM_MW_TIME_Info* psInfo) {
	eCOMM_MW_TIME_Ret eRet = COMM_MW_TIME_SUCCESS;

	CMT_FuncIn();

	{
		pthread_mutex_lock(&mutVisitTime);

		psInfo->sCurrentVisitTime = time(NULL);
		psInfo->lDeltTimeSec = psInfo->sCurrentVisitTime - psInfo->sSaveVisitTime;

		if (psInfo->lDeltTimeSec < 0) {
			CMT_Debug("something wrong with time, reset now\n");
			eCOMM_MW_TIME_Reset(psInfo);
		}

		pthread_mutex_unlock(&mutVisitTime);
	}

	CMT_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_MW_TIME_Deinit(sCOMM_MW_TIME_Info* psInfo)
* arg  : sCOMM_MW_TIME_Info* psInfo
* ret  : eCOMM_MW_TIME_Ret
* note :
*********************************************/
LONG lCOMM_MW_TIME_GetDeltTime(sCOMM_MW_TIME_Info* psInfo) {
	return psInfo->lDeltTimeSec;
}

/*********************************************
* func : eCOMM_MW_TIME_Save(sCOMM_MW_TIME_Info* psInfo)
* arg  : sCOMM_MW_TIME_Info* psInfo
* ret  : eCOMM_MW_TIME_Ret
* note :
*********************************************/
eCOMM_MW_TIME_Ret eCOMM_MW_TIME_Save(sCOMM_MW_TIME_Info* psInfo) {
	eCOMM_MW_TIME_Ret eRet = COMM_MW_TIME_SUCCESS;

	CMT_FuncIn();

	{
		psInfo->sSaveVisitTime = psInfo->sCurrentVisitTime;
	}

	CMT_FuncOut();

	return eRet;
}


#ifdef __cplusplus
}
#endif
