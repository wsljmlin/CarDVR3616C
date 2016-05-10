#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "comm_timer.h"

/***************************************************
	macro / enum
***************************************************/
// #define COMM_TIMER_DEBUG
#ifdef COMM_TIMER_DEBUG
#define CT_Debug(fmt, arg...) fprintf(stdout, "[ TIMER ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CT_Debug(fmt, arg...)
#endif
#define CT_FuncIn() CT_Debug("in\n")
#define CT_FuncOut() CT_Debug("out\n")
#define CT_iVal(iVal) CT_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CT_lVal(lVal) CT_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CT_PtVal(ptVal) CT_Debug("pt %s @ %p\n", #ptVal, ptVal)

#define COMM_TIMER_PHYTIMEOUTSEC 1

/***************************************************
	variable
***************************************************/

/***************************************************
	prototype
***************************************************/
eCOMM_TIMER_RET eCOMM_TIMER_GetFreeTimer(sCOMM_TIMER_INFO** ppsTimerInfo, LONG* plId);
eCOMM_TIMER_RET eCOMM_TIMER_Eclipse(sCOMM_TIMER_INFO* psTimerInfo);
eCOMM_TIMER_RET eCOMM_TIMER_GetTimeOutVal(struct itimerval* psTimeOut, const LONG lTimeOutSec);
void vCOMM_TIMER_Handle(int iSignal);

/***************************************************
	function
***************************************************/

/*********************************************
 * func : eCOMM_TIMER_InitAll()
 * arg : void
 * ret : eCOMM_TIMER_RET
 * note : init all the timer, set physical timer
 *********************************************/
eCOMM_TIMER_RET eCOMM_TIMER_InitAll() {
	eCOMM_TIMER_RET eRet = COMM_TIMER_SUCCESS;
	LONG lTimerIdTmp;
	sCOMM_TIMER_INFO* psInfo = NULL;
	sCOMM_TIMER_LIST* psList = NULL;
	struct itimerval sTimeOut;

	CT_FuncIn();

	{
		/* init all the timer */
		for(lTimerIdTmp = 0; lTimerIdTmp != COMM_TIMER_MAXCNT; lTimerIdTmp++) {
			MALLOC(sizeof(sCOMM_TIMER_INFO));
			psTimerNode[lTimerIdTmp] = (sCOMM_TIMER_INFO*) malloc (sizeof(sCOMM_TIMER_INFO));
			psInfo = psTimerNode[lTimerIdTmp];

			pthread_mutex_init(&psInfo->sMutStatus, NULL);
			psInfo->eStatus = COMM_TIMER_VALID;
			psInfo->lTimerId = lTimerIdTmp;

		}

		/* init list */
		psList = &gsTimerList;
		psList->psHeader = NULL;
		psList->psTail = NULL;
		pthread_mutex_init(&psList->mutStatus, NULL);

		/* set physical timer */
		eCOMM_TIMER_GetTimeOutVal(&sTimeOut, COMM_TIMER_PHYTIMEOUTSEC);
		setitimer(ITIMER_REAL, &sTimeOut, NULL);
		signal(SIGALRM, vCOMM_TIMER_Handle);
	}

	CT_FuncOut();

	return eRet;
}

/*********************************************
 * func : eCOMM_TIMER_DeinitAll()
 * arg : void
 * ret : eCOMM_TIMER_RET
 * note : deinit all the timer
 *********************************************/
eCOMM_TIMER_RET eCOMM_TIMER_DeinitAll() {
	eCOMM_TIMER_RET eRet = COMM_TIMER_SUCCESS;
	LONG lTimerIdTmp;
	sCOMM_TIMER_INFO* psInfo = NULL;
	sCOMM_TIMER_LIST* psList = &gsTimerList;
	struct itimerval sTimeOut;

	CT_FuncIn();

	{
		
		/* deinit all the timer */
		for(lTimerIdTmp = 0; lTimerIdTmp != COMM_TIMER_MAXCNT; lTimerIdTmp++) {
			psInfo = psTimerNode[lTimerIdTmp];

			if(psInfo) {
				if(psInfo->eStatus != COMM_TIMER_INVALID) {
					psInfo->eStatus = COMM_TIMER_INVALID;
					psInfo->lCurTime = 0;
					psInfo->lTimeOut = 0;
					psInfo->lTimerId = -1;
					psInfo->psNextTimer = NULL;
					psInfo->pvCBFunc = NULL;
				}

				pthread_mutex_destroy(&psInfo->sMutStatus);
				free(psTimerNode[lTimerIdTmp]);
				psTimerNode[lTimerIdTmp] = NULL;
			}
		}

		/* deinit the timer list */
		psList->psHeader = NULL;
		psList->psTail = NULL;
		pthread_mutex_destroy(&psList->mutStatus);
		/* deinit physical timer */
		eCOMM_TIMER_GetTimeOutVal(&sTimeOut, 0);
		setitimer(ITIMER_REAL, &sTimeOut, NULL);
		signal(SIGALRM, vCOMM_TIMER_Handle);

		/*timer deinit*/
		eCOMM_TIMER_DeinitAll();
	}

	CT_FuncOut();

	return eRet;
}

/*********************************************
 * func : eCOMM_TIMER_Init(const LONG lTimeOut, const pvTimerCallBackFunc pvCBFunc, void* pvCBFuncArg, LONG* plTimerId)
 * arg : const LONG lTimeOut, const pvTimerCallBackFunc pvCBFunc, void* pvCBFuncArg, LONG* plTimerId
 * ret : eCOMM_TIMER_RET
 * note : init the specific logical timer
 *********************************************/
eCOMM_TIMER_RET eCOMM_TIMER_Init(const LONG lTimeOut, const pvTimerCallBackFunc pvCBFunc, void* pvCBFuncArg, LONG* plTimerId) {
	eCOMM_TIMER_RET eRet = COMM_TIMER_SUCCESS;
	sCOMM_TIMER_INFO* psInfo = NULL;
	sCOMM_TIMER_LIST* psList = &gsTimerList;

	CT_FuncIn();

	{
		eRet = eCOMM_TIMER_GetFreeTimer(&psInfo, plTimerId);
		if(!psInfo) {
			CT_Debug("timer reach max\n");
			return COMM_TIMER_TIMERREACHMAX;
		}

		/* init the timer */
		psInfo->lCurTime = 0;
		psInfo->lTimeOut = lTimeOut;
		psInfo->pvCBFunc = pvCBFunc;
		psInfo->pvCBFuncArg = pvCBFuncArg;
		psInfo->psNextTimer = NULL;

		/* insert into timer list */
		/* get lock */
		pthread_mutex_lock(&psList->mutStatus);
		
		if(!psList->psHeader) {
			/* indicate the first timer */
			psList->psTail = psInfo;
			psList->psHeader = psInfo;
		} else {
			/* indicate not the first timer */
			psList->psTail->psNextTimer = psInfo;
			psList->psTail = psInfo;
		}

		/* release lock */
		pthread_mutex_unlock(&psList->mutStatus);
	}

	CT_FuncOut();

	return eRet;
}

/*********************************************
 * func : eCOMM_TIMER_Deinit(const LONG lTimerId)
 * arg : const LONG lTimerId
 * ret : eCOMM_TIMER_RET
 * note : deinit the specific logical timer
 *********************************************/
eCOMM_TIMER_RET eCOMM_TIMER_Deinit(const LONG lTimerId) {
	eCOMM_TIMER_RET eRet = COMM_TIMER_SUCCESS;
	sCOMM_TIMER_INFO* psInfoTmp = NULL;
	sCOMM_TIMER_INFO* psRemove = NULL;
	sCOMM_TIMER_LIST* psList = &gsTimerList;

	CT_FuncIn();

	{
		/* remove from the list */
		
		/* get lock */
		pthread_mutex_lock(&psList->mutStatus);	
		
		psInfoTmp = psList->psHeader;
		if(psInfoTmp->lTimerId == lTimerId) {
			psRemove = psInfoTmp;
			psInfoTmp = psInfoTmp->psNextTimer;
			psList->psHeader=psList->psHeader->psNextTimer;
		}else{
			
			while(1) {
				if(!(psInfoTmp->psNextTimer)) {
					
					/* release lock */
					pthread_mutex_unlock(&psList->mutStatus);	
					
					return COMM_TIMER_NOTUSED;
				}

				if(psInfoTmp->psNextTimer->lTimerId == lTimerId) {
					psRemove=psInfoTmp->psNextTimer;
					psInfoTmp->psNextTimer = psRemove->psNextTimer;

					/* judge whether tail */
					if(!(psInfoTmp->psNextTimer)) {
						psList->psTail = psInfoTmp;
					}

					break;
				}

				psInfoTmp = psInfoTmp->psNextTimer;
			}
		}
		
		/* release lock */
		pthread_mutex_unlock(&psList->mutStatus);
		
		/* remove the timer */
		psRemove->lCurTime = 0;
		psRemove->lTimeOut = 0;
		psRemove->psNextTimer = NULL;
		psRemove->pvCBFunc = NULL;
		psRemove->pvCBFuncArg = NULL;

		pthread_mutex_lock(&psRemove->sMutStatus);
		psRemove->eStatus = COMM_TIMER_VALID;
		pthread_mutex_unlock(&psRemove->sMutStatus);
	}

	CT_FuncOut();

	return eRet;
}

/*********************************************
 * func : eCOMM_TIMER_GetFreeTimer(sCOMM_TIMER_INFO** ppsTimerInfo, LONG* plId)
 * arg : sCOMM_TIMER_INFO** ppsTimerInfo, LONG* plId
 * ret : eCOMM_TIMER_RET
 * note : get a free timer
 *********************************************/
eCOMM_TIMER_RET eCOMM_TIMER_GetFreeTimer(sCOMM_TIMER_INFO** ppsTimerInfo, LONG* plId) {
	eCOMM_TIMER_RET eRet = COMM_TIMER_SUCCESS;
	sCOMM_TIMER_INFO* psTimerTmp = NULL;
	LONG lTimerIdTmp;

	CT_FuncIn();

	{
		/* init */
		*plId = -1;

		for(lTimerIdTmp = 0; lTimerIdTmp != COMM_TIMER_MAXCNT; lTimerIdTmp++) {
			psTimerTmp = psTimerNode[lTimerIdTmp];
			if(psTimerTmp && (psTimerTmp->eStatus == COMM_TIMER_VALID)) {
				*ppsTimerInfo = psTimerTmp;
				*plId = lTimerIdTmp;

				/* toggle the status */
				pthread_mutex_lock(&psTimerTmp->sMutStatus);
				psTimerTmp->eStatus = COMM_TIMER_ISUSE;
				pthread_mutex_unlock(&psTimerTmp->sMutStatus);
				
				break;
			}
		}
	}

	CT_FuncOut();

	return eRet;
}

/*********************************************
 * func : eCOMM_TIMER_Eclipse(sCOMM_TIMER_INFO* psTimerInfo)
 * arg : sCOMM_TIMER_INFO* psTimerInfo
 * ret : eCOMM_TIMER_RET
 * note : eclipse from the timer
 *********************************************/
eCOMM_TIMER_RET eCOMM_TIMER_Eclipse(sCOMM_TIMER_INFO* psTimerInfo) {
	eCOMM_TIMER_RET eRet = COMM_TIMER_SUCCESS;
	

	CT_FuncIn();

	{
		if(++(psTimerInfo->lCurTime) == psTimerInfo->lTimeOut) {
			psTimerInfo->pvCBFunc(psTimerInfo->pvCBFuncArg);
			psTimerInfo->lCurTime = 0;
		}
	}

	CT_FuncOut();

	return eRet;
}

/*********************************************
 * func : vCOMM_TIMER_Handle(LONG lSignal)
 * arg : LONG lSignal
 * ret : void
 * note : the physical timer handle function
 *********************************************/
void vCOMM_TIMER_Handle(int iSignal) {
	sCOMM_TIMER_INFO* psTimerTmp = NULL;
	sCOMM_TIMER_LIST* psList = &gsTimerList;

	CT_FuncIn();

	{
		psTimerTmp = psList->psHeader;
		while(psTimerTmp) {
			eCOMM_TIMER_Eclipse(psTimerTmp);
			psTimerTmp = psTimerTmp->psNextTimer;
		}
	}

	CT_FuncOut();

	return;
}

/*********************************************
 * func : eCOMM_TIMER_GetTimeOutVal(struct itimerval* psTimeOut, const LONG lTimeOutSec)
 * arg : struct itimerval* psTimeOut, const LONG lTimeOutSec
 * ret : eCOMM_TIMER_RET
 * note : get time out value
 *********************************************/
eCOMM_TIMER_RET eCOMM_TIMER_GetTimeOutVal(struct itimerval* psTimeOut, const LONG lTimeOutSec) {
	eCOMM_TIMER_RET eRet = COMM_TIMER_SUCCESS;

	CT_FuncIn();

	{
		psTimeOut->it_value.tv_sec = lTimeOutSec;
		psTimeOut->it_value.tv_usec = 0;
		psTimeOut->it_interval.tv_sec = lTimeOutSec;
		psTimeOut->it_interval.tv_usec = 0;
	}

	CT_FuncOut();

	return eRet;
}



#ifdef __cplusplus
}
#endif
