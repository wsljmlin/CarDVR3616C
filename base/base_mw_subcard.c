#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "base_mw_subcard.h"
#include <assert.h>

/***************************************************
	macro / enum
***************************************************/
//#define BASE_MW_SUBCARD_DEBUG
#ifdef BASE_MW_SUBCARD_DEBUG
#define BMS_Debug(fmt, arg...) fprintf(stdout, "[ BMS ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define BMS_Debug(fmt, arg...)
#endif

#define BMS_FuncIn() BMS_Debug("in\n")
#define BMS_FuncOut() BMS_Debug("out\n")
#define BMS_iVal(iVal) BMS_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define BMS_lVal(lVal) BMS_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define BMS_PtVal(ptVal) BMS_Debug("pt %s @ %p\n", #ptVal, ptVal)

typedef enum {
	BASE_MW_SUBCARD_FEEDBACKSTATUS_SYSTEMNOTSTART,
	BASE_MW_SUBCARD_FEEDBACKSTATUS_FREE,
	BASE_MW_SUBCARD_FEEDBACKSTATUS_BUSY,
	BASE_MW_SUBCARD_FEEDBACKSTATUS_VERYBUSY,
} BASE_MW_SUBCARD_FeedbackStatus;

/***************************************************
	variable
***************************************************/
#ifdef SUBCARD_CHECK_DEBUG
#else
extern LONG glFolderIndex;
extern sCOMM_SPACESAFER_Info gsTwoFolder[TWO_FOLDER];
extern CHAR gcMountInfo[TWO_FOLDER][32];
extern CHAR gcTargetFileInfo[TWO_FOLDER][32];
#endif /* SUBCARD_CHECK_DEBUG */
/***************************************************
	prototype
***************************************************/
void* pvBASE_MW_SUBCARD_SigProcess(void* pvArg);
void vBASE_MW_SUBCARD_Deinit_Cleanup(void* pvArg);
eBASE_MW_SUBCARD_Ret eBASE_MW_SUBCARD_FeedBack(sBASE_MW_SUBCARD_Info* psInfo, BASE_MW_SUBCARD_FeedbackStatus eStatus);


/***************************************************
	function
***************************************************/

/*********************************************
* func : eBASE_MW_SUBCARD_Init(sBASE_MW_SUBCARD_Info* psInfo, LONG lPoolId, LONG lTimeTh, CHAR* pcPath)
* arg : sBASE_MW_SUBCARD_Info* psInfo, LONG lPoolId, LONG lTimeTh, CHAR* pcPath, sCOMM_CONDFLAG* psConFlag
* ret : eBASE_MW_SUBCARD_Ret
* note : init subcard
*********************************************/
eBASE_MW_SUBCARD_Ret eBASE_MW_SUBCARD_Init(sBASE_MW_SUBCARD_Info* psInfo, LONG lPoolId, LONG lTimeTh, CHAR* pcPath) {
	eBASE_MW_SUBCARD_Ret eRet = BASE_MW_SUBCARD_SUCCESS;
	eBASE_MW_SUBCARD_REC_Ret eRecRet = BASE_MW_SUBCARD_REC_SUCCESS;

	BMS_FuncIn();

	{
		eRecRet = eBASE_MW_SUBCARD_REC_Init(&psInfo->sRecInfo, lPoolId, lTimeTh, pcPath);
		if(eRecRet != BASE_MW_SUBCARD_REC_SUCCESS) {
			BMS_Debug("wrong with subcard rec\n");
			return BASE_MW_SUBCARD_REC;
		}

		pthread_mutex_init(&psInfo->mutStatus, NULL);
		pthread_cond_init(&psInfo->condStatus, NULL);

		psInfo->eStatus = BASE_MW_SUBCARD_Invalid;
	}

	BMS_FuncOut();

	return eRet;
}

/*********************************************
* func : pvBASE_MW_SUBCARD_SigProcess(void* pvArg)
* arg : void* pvArg
* ret : void*
* note : signal process main thread
*********************************************/
void* pvBASE_MW_SUBCARD_SigProcess(void* pvArg) {
	sBASE_MW_SUBCARD_Info* psInfo = (sBASE_MW_SUBCARD_Info*) pvArg;
	
	BMS_FuncIn();

	{
		
		/* register clean up function */
		pthread_cleanup_push(vBASE_MW_SUBCARD_Deinit_Cleanup, pvArg);
		
		while(!(psInfo->eStatus & BASE_MW_SUBCARD_NeedToQuit)) {
			pthread_mutex_lock(&psInfo->mutStatus);
			while(!(psInfo->eStatus & BASE_MW_SUBCARD_HaveWork)) {
				pthread_cond_wait(&psInfo->condStatus, &psInfo->mutStatus);
			}

			BMS_Debug("get signal, begin to rec\n");
			psInfo->eStatus &= ~ BASE_MW_SUBCARD_VeryBusy;
			psInfo->eStatus &= ~ BASE_MW_SUBCARD_HaveWork;
			psInfo->eStatus |= BASE_MW_SUBCARD_Busy;
			pthread_mutex_unlock(&psInfo->mutStatus);

			/* here, we are sure we recieve at least one signal */
			eBASE_MW_SUBCARD_REC_Consume(&psInfo->sRecInfo);

			/* toggle the status */
			pthread_mutex_lock(&psInfo->mutStatus);
			psInfo->eStatus &= ~ BASE_MW_SUBCARD_Busy;
			pthread_mutex_unlock(&psInfo->mutStatus);
			
		}
		
#ifdef SUBCARD_CHECK_DEBUG
#else
		{
			LONG lCnt;
			for(lCnt = 0;lCnt < TWO_FOLDER;lCnt++) {
				eCOMM_SPACESAFER_DeInit(&gsTwoFolder[lCnt]);
			}	
		}
#endif /* SUBCARD_CHECK_DEBUG */

		/* execute clean up function */
		pthread_cleanup_pop(0);
	}

	BMS_FuncOut();

	return NULL;
}

/*********************************************
* func : eBASE_MW_SUBCARD_StartSigProcess(sBASE_MW_SUBCARD_Info* psInfo)
* arg : sBASE_MW_SUBCARD_Info* psInfo
* ret : eBASE_MW_SUBCARD_Ret
* note : start signal process
*********************************************/
eBASE_MW_SUBCARD_Ret eBASE_MW_SUBCARD_StartSigProcess(sBASE_MW_SUBCARD_Info* psInfo) {
	eBASE_MW_SUBCARD_Ret eRet = BASE_MW_SUBCARD_SUCCESS;
	LONG lComRet;

	BMS_FuncIn();

	{
		/* set the status */
		psInfo->eStatus = BASE_MW_SUBCARD_Valid;
		
		/* create signal process thread */
		lComRet = pthread_create(&psInfo->tid, NULL, pvBASE_MW_SUBCARD_SigProcess, (void*)psInfo);
		if(lComRet != 0) {
			BMS_Debug("create thread error\n");
			return BASE_MW_SUBCARD_CREATETHREAD;
		}
	}

	BMS_FuncOut();

	return eRet;
}


/*********************************************
* func : eBASE_MW_SUBCARD_NewSignal(sBASE_MW_SUBCARD_Info* psInfo)
* arg : sBASE_MW_SUBCARD_Info* psInfo
* ret : eBASE_MW_SUBCARD_Ret
* note : new signal request
*********************************************/
eBASE_MW_SUBCARD_Ret eBASE_MW_SUBCARD_NewSignal(sBASE_MW_SUBCARD_Info* psInfo) {
	eBASE_MW_SUBCARD_Ret eRet = BASE_MW_SUBCARD_SUCCESS;
	BASE_MW_SUBCARD_FeedbackStatus eFeedbackStatus;
	LONG lBusyToVeryBusy = 0;

	BMS_FuncIn();

	{
		/* pre-condition check */
		if (psInfo->eStatus == BASE_MW_SUBCARD_Invalid) {
			/* indicate the thread is not start yet */
			BMS_Debug("the subcard system have not start yet\n");
			return BASE_MW_SUBCARD_SYSTEMNOTSTART;
		}
		
		/* send a new signal */
		eFeedbackStatus = BASE_MW_SUBCARD_FEEDBACKSTATUS_FREE;
		pthread_mutex_lock(&psInfo->mutStatus);
		if(psInfo->eStatus & BASE_MW_SUBCARD_Busy) {
			if(psInfo->eStatus & BASE_MW_SUBCARD_VeryBusy) {
				BMS_Debug("warning, signal handle not finished, this request may be discard\n");
				eFeedbackStatus = BASE_MW_SUBCARD_FEEDBACKSTATUS_VERYBUSY;
			} else {
				BMS_Debug("warning, signal handle not finished, this request will be handle later\n");
				eFeedbackStatus = BASE_MW_SUBCARD_FEEDBACKSTATUS_BUSY;
				lBusyToVeryBusy = 1;
			}
		}

		/* feedback */
		eRet = eBASE_MW_SUBCARD_FeedBack(psInfo, eFeedbackStatus);
		if (eRet != BASE_MW_SUBCARD_SUCCESS) {
			/* indicate not enough space */
			pthread_mutex_unlock(&psInfo->mutStatus);
			return eRet;
		}

		/* toggle very busy flag */
		if (lBusyToVeryBusy) {
			/* indicate enough space, so status can change to very busy mode */
			psInfo->eStatus |= BASE_MW_SUBCARD_VeryBusy;
		}

		/* toggle the status */
		psInfo->eStatus |= BASE_MW_SUBCARD_HaveWork;
		
		pthread_mutex_unlock(&psInfo->mutStatus);

		pthread_cond_signal(&psInfo->condStatus);
		
	}

	BMS_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_MW_SUBCARD_Deinit(sBASE_MW_SUBCARD_Info* psInfo)
* arg : sBASE_MW_SUBCARD_Info* psInfo
* ret : eBASE_MW_SUBCARD_Ret
* note : deinit the subcard
*********************************************/
eBASE_MW_SUBCARD_Ret eBASE_MW_SUBCARD_Deinit(sBASE_MW_SUBCARD_Info* psInfo) {
	eBASE_MW_SUBCARD_Ret eRet = BASE_MW_SUBCARD_SUCCESS;
	eBASE_MW_SUBCARD_REC_Ret eSubcardRecRet = BASE_MW_SUBCARD_REC_SUCCESS;
	void* pvThreadStat = NULL;
	eBASE_MW_SUBCARD_Status eSubCardStatus;

	BMS_FuncIn();

	{
		/* only valid status should stop the thread */
		if (psInfo->eStatus & BASE_MW_SUBCARD_Valid) {

			/* toggle the status */
			pthread_mutex_lock(&psInfo->mutStatus);
			psInfo->eStatus |= BASE_MW_SUBCARD_NeedToQuit;
			eSubCardStatus = psInfo->eStatus;
			pthread_mutex_unlock(&psInfo->mutStatus);

			/* if current not busy, then cancel the thread */
			if(!(eSubCardStatus & BASE_MW_SUBCARD_Busy))  {
				/* cancel the thread */
				pthread_cancel(psInfo->tid);
			}

			/* recieve the thread */
			pthread_join(psInfo->tid, &pvThreadStat);
			if(pvThreadStat == PTHREAD_CANCELED) {
				BMS_Debug("thread have been canceled\n");
			}

		}
		
		/* destroy the mutex and condi flag */
		pthread_mutex_destroy(&psInfo->mutStatus);
		pthread_cond_destroy(&psInfo->condStatus);

		/* deinit subcard */
		eSubcardRecRet = eBASE_MW_SUBCARD_REC_Deinit(&psInfo->sRecInfo);
		if(eSubcardRecRet != BASE_MW_SUBCARD_REC_SUCCESS) {
			BMS_Debug("wrong with subcard rec\n");
			return BASE_MW_SUBCARD_REC;
		}

		psInfo->eStatus = BASE_MW_SUBCARD_Invalid;
	}

	BMS_FuncOut();

	return eRet;
}

/*********************************************
* func : vBASE_MW_SUBCARD_Deinit_Cleanup(void* pvArg)
* arg : void* pvArg
* ret : void
* note : deinit clean up function
*********************************************/
void vBASE_MW_SUBCARD_Deinit_Cleanup(void* pvArg) {
	sBASE_MW_SUBCARD_Info* psInfo = (sBASE_MW_SUBCARD_Info*) pvArg;
	LONG lRet = 0;

	BMS_FuncIn();

	{	
		/* unlock the mutex */
		lRet = pthread_mutex_unlock(&psInfo->mutStatus);
		if(lRet != 0) {
			BMS_Debug("warning lock error\n");
		}
	}

	BMS_FuncOut();

	return;
}

/*********************************************
* func : eBASE_MW_SUBCARD_UpdateTimeTh(sBASE_MW_SUBCARD_Info* psInfo, LONG lNewTimeTh)
* arg : sBASE_MW_SUBCARD_Info* psInfo, LONG lNewTimeTh
* ret : eBASE_MW_SUBCARD_Ret
* note : update sub card time threshold
*********************************************/
eBASE_MW_SUBCARD_Ret eBASE_MW_SUBCARD_UpdateTimeTh(sBASE_MW_SUBCARD_Info* psInfo, LONG lNewTimeTh) {
	eBASE_MW_SUBCARD_Ret eRet = BASE_MW_SUBCARD_SUCCESS;

	BMS_FuncIn();

	{
		eBASE_MW_SUBCARD_REC_UpdateRecDuration(&psInfo->sRecInfo, lNewTimeTh);
	}

	BMS_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_MW_SUBCARD_UpdateOverlayType(sBASE_MW_SUBCARD_Info* psInfo, eBASE_MW_SUBCARD_OVERLAY_Type lNewOverlayType)
* arg : sBASE_MW_SUBCARD_Info* psInfo, eBASE_MW_SUBCARD_OVERLAY_Type lNewOverlayType
* ret : eBASE_MW_SUBCARD_Ret
* note : update sub card overlay type
*********************************************/
eBASE_MW_SUBCARD_Ret eBASE_MW_SUBCARD_UpdateOverlayType(sBASE_MW_SUBCARD_Info* psInfo, eBASE_MW_SUBCARD_OVERLAY_Type eNewOverlayType) {
	eBASE_MW_SUBCARD_Ret eRet = BASE_MW_SUBCARD_SUCCESS;

	BMS_FuncIn();

	{
		eBASE_MW_SUBCARD_REC_UpdateOverlayType(&psInfo->sRecInfo, eNewOverlayType);
	}

	BMS_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_MW_SUBCARD_GetOverlayType(sBASE_MW_SUBCARD_Info* psInfo, eBASE_MW_SUBCARD_OVERLAY_Type* peOverlayType)
* arg : sBASE_MW_SUBCARD_Info* psInfo, eBASE_MW_SUBCARD_OVERLAY_Type peOverlayType
* ret : eBASE_MW_SUBCARD_Ret
* note : update sub card overlay type
*********************************************/
eBASE_MW_SUBCARD_Ret eBASE_MW_SUBCARD_GetOverlayType(sBASE_MW_SUBCARD_Info* psInfo, eBASE_MW_SUBCARD_OVERLAY_Type* peOverlayType) {
	eBASE_MW_SUBCARD_Ret eRet = BASE_MW_SUBCARD_SUCCESS;

	BMS_FuncIn();

	{
		eBASE_MW_SUBCARD_REC_GetOverlayType(&psInfo->sRecInfo, peOverlayType);
	}

	BMS_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_MW_SUBCARD_FeedBack(sBASE_MW_SUBCARD_Info* psInfo, BASE_MW_SUBCARD_FeedbackStatus eStatus)
* arg : sBASE_MW_SUBCARD_Info* psInfo, BASE_MW_SUBCARD_FeedbackStatus eStatus
* ret : eBASE_MW_SUBCARD_Ret
* note : feed back
*********************************************/
eBASE_MW_SUBCARD_Ret eBASE_MW_SUBCARD_FeedBack(sBASE_MW_SUBCARD_Info* psInfo, BASE_MW_SUBCARD_FeedbackStatus eStatus) {
	eBASE_MW_SUBCARD_Ret eRet = BASE_MW_SUBCARD_SUCCESS;
	eBASE_MW_SUBCARD_OVERLAY_Type eOverlayType;
#ifdef SUBCARD_CHECK_DEBUG
	eBASE_MW_SD_ERROR eSdRet = BASE_MW_SD_SUCCESS;
#else
	eCOMM_SPACESAFER_Ret eSubCardRet = COMM_SPACESAFER_SUCCESS;
#endif /* SUBCARD_CHECK_DEBUG */

	BMS_FuncIn();

	{

		/* system not start */
		if (eStatus == BASE_MW_SUBCARD_FEEDBACKSTATUS_SYSTEMNOTSTART) {
			BMS_Debug("system not start\n");
			return BASE_MW_SUBCARD_SYSTEMNOTSTART;
		}

		/* judge overlay type */
		eBASE_MW_SUBCARD_REC_GetOverlayType(&psInfo->sRecInfo, &eOverlayType);
		if (eOverlayType == BASE_MW_SUBCARD_OVERLAY_FORCE) {
			BMS_Debug("force overlay mode\n");
			return BASE_MW_SUBCARD_SUCCESS;
		}

		/* overlay type is not force, should judge whether have enough space */
		switch (eStatus) {
			case BASE_MW_SUBCARD_FEEDBACKSTATUS_SYSTEMNOTSTART :
				/* system not start, should handle before */
				assert(0);
				break;

			case BASE_MW_SUBCARD_FEEDBACKSTATUS_BUSY :
				/* busy, should check whether have extra space */
				BMS_Debug("feedback busy\n");
#ifdef SUBCARD_CHECK_DEBUG
				eSdRet = eBASE_SD_MW_CheckSpace(NANDFLASH, 2 * BASE_MW_NAND_FILE_SIZEKB);
				if (eSdRet != BASE_MW_SD_SUCCESS) {
					BMS_Debug("not enough space\n");
					eRet = BASE_MW_SUBCARD_NOTENOUGHSPACE;
				}
#else
				eSubCardRet = eCOMM_SPACESAFER_CheckSpace(&gsTwoFolder[glFolderIndex],2 * COMM_NAND_FILE_SIZEKB);
				if (eSubCardRet != COMM_SPACESAFER_SUCCESS) {
					BMS_Debug("not enough space\n");
					eRet = BASE_MW_SUBCARD_NOTENOUGHSPACE;
				}
#endif /* SUBCARD_CHECK_DEBUG */
				break;
				
			case BASE_MW_SUBCARD_FEEDBACKSTATUS_FREE :
				/* free, check whether have enough space */
				BMS_Debug("feedback free\n");
#ifdef SUBCARD_CHECK_DEBUG
				eSdRet = eBASE_SD_MW_CheckSpace(NANDFLASH, BASE_MW_NAND_FILE_SIZEKB);
				if (eSdRet != BASE_MW_SD_SUCCESS) {
					eRet = BASE_MW_SUBCARD_NOTENOUGHSPACE;
				}
#else
				eSubCardRet = eCOMM_SPACESAFER_CheckSpace(&gsTwoFolder[glFolderIndex],COMM_NAND_FILE_SIZEKB);
				if (eSubCardRet != COMM_SPACESAFER_SUCCESS) {
					BMS_Debug("not enough space\n");
					eRet = BASE_MW_SUBCARD_NOTENOUGHSPACE;
				}
#endif /* SUBCARD_CHECK_DEBUG */
				break;
				
			case BASE_MW_SUBCARD_FEEDBACKSTATUS_VERYBUSY :
				/* already have work, return success, space is enough */
				BMS_Debug("feedback very busy\n");
				break;
				
			default :
				assert(0);
				break;
		}

	}

	BMS_FuncOut();

	return eRet;
}

#ifdef __cplusplus
}
#endif
