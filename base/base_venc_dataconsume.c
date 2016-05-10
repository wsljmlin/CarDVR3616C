#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#define _GNU_SOURCE
#include "base_venc_dataconsume.h"
#include "base_mw_sd.h"
#include "base_mw_filesort.h"
#include <assert.h>
#include <fcntl.h>

/***************************************************
	macro / enum
***************************************************/
//#define BASE_VENC_DATACONSUME_DEBUG
#ifdef BASE_VENC_DATACONSUME_DEBUG
#define BVD_Debug(fmt, arg...) fprintf(stdout, "[ BVD ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define BVD_Debug(fmt, arg...)
#endif

#define BVD_FuncIn() BVD_Debug("in\n")
#define BVD_FuncOut() BVD_Debug("out\n")
#define BVD_iVal(iVal) BVD_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define BVD_lVal(lVal) BVD_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define BVD_PtVal(ptVal) BVD_Debug("pt %s @ %p\n", #ptVal, ptVal)

#define BASE_VENC_DATACONSUME_GET_FRAME_FROM_SEC(sec) (sec * 30)
#define BASE_VENC_DATACONSUME_MAXFILENAME 64
#define BASE_VENC_DATACONSUME_ERRORCNTMAX 20

typedef enum {
	BASE_VENC_DATACONSUME_FILENEW,
	BASE_VENC_DATACONSUME_FILECONTINUE,
} eBASE_VENC_DATACONSUME_FILETYPE;

typedef enum {
	BASE_VENC_DATACONSUME_CALLBACK_SUCCESS = 0x0,
	BASE_VENC_DATACONSUME_CALLBACK_OPENFILE = 0x1,
	BASE_VENC_DATACONSUME_CALLBACK_EMPTYFD = 0x2,
	BASE_VENC_DATACONSUME_CALLBACK_WRITE = 0x4,
	BASE_VENC_DATACONSUME_CALLBACK_VFILE = 0x8,
	BASE_VENC_DATACONSUME_CALLBACK_TFCARD= 0x10,
	BASE_VENC_DATACONSUME_CALLBACK_FILESORT= 0x11,
} eBASE_VENC_DATACONSUME_CALLBACK_Ret;

/***************************************************
	variable
***************************************************/
sBASE_MW_FILESORT_Info sFileSortInfo;

/***************************************************
	struct
***************************************************/
typedef struct {
	eBASE_VENC_DATACONSUME_FILETYPE eFileType;
	LONG lFrameTh;
	LONG lCurFrame;
	LONG lCBRet;
	CHAR* pcPathPos;
	CHAR* pcFilePos;
	sCOMM_MW_VFILE_Info* psVfile;
	CHAR cDirName[BASE_VENC_DATACONSUME_MAXNAME/2];
} sBASE_VENC_DATACONSUME_CallBackArg;

/***************************************************
	prototype
***************************************************/
void* pvMConsumer(void* MArg);
void* pvVConsumer(void* VArg);
void BASE_VENC_DATACONSUME_ReadCallback(void* pvArg);
void BASE_VENC_DATACONSUME_ReadCallback_ISliceVersion(void* pvArg);
eBASE_VENC_DATACONSUME_Ret eBASE_VENC_DATACONSUME_OpenNewFile(sBASE_VENC_DATACONSUME_CallBackArg* psDataConsumeArg);
eBASE_VENC_DATACONSUME_Ret eBASE_VENC_DATACONSUME_WriteCurrentFrame(sCOMM_POOL_CBFuncArg* psCBArg);
eBASE_VENC_DATACONSUME_Ret eBASE_VENC_DATACONSUME_CloseFile(sBASE_VENC_DATACONSUME_CallBackArg* psDataConsumeArg);

/***************************************************
	function
***************************************************/

/*********************************************
* func : eBASE_VENC_DATACONSUME_Init(sBASE_VENC_DATACONSUME_Info* psInfo, LONG* plPoolId, LONG lTimeVTh, LONG lTimeMTh, CHAR* pcPathName)
* arg : sBASE_VENC_DATACONSUME_Info* psInfo, LONG* plPoolId, LONG lTimeVTh, LONG lTimeMTh, CHAR* pcPathName
* ret : eBASE_VENC_DATACONSUME_Ret
* note : init data consumer
*********************************************/
eBASE_VENC_DATACONSUME_Ret eBASE_VENC_DATACONSUME_Init(sBASE_VENC_DATACONSUME_Info* psInfo, LONG* plPoolId, LONG lTimeVTh, LONG lTimeMTh, CHAR* pcPathName) {
	eBASE_VENC_DATACONSUME_Ret eRet = BASE_VENC_DATACONSUME_SUCCESS;
	sBASE_VENC_DATACONSUME_Communicate* psCommu;
	sBASE_VENC_DATACONSUME_MArg* psMArg;
	sBASE_VENC_DATACONSUME_VArg* psVArg;

	BVD_FuncIn();

	{

		/* init psInfo */
		psInfo->eStatus = BASE_VENC_DATACONSUME_Invalid;
		psInfo->plPoolId = plPoolId;
		psInfo->lTimeVTh = lTimeVTh;
		psInfo->lTimeMTh = lTimeMTh;

		memset(psInfo->cName, 0, BASE_VENC_DATACONSUME_MAXNAME);
		BVD_Debug("%s\n",pcPathName);
		if(pcPathName) {
			strcpy(psInfo->cName, pcPathName);
			if(access(psInfo->cName, F_OK) != 0) {
				mkdir(psInfo->cName,
					S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
			}
		}
		MALLOC(sizeof(sBASE_VENC_DATACONSUME_MArg));
		MALLOC(sizeof(sBASE_VENC_DATACONSUME_VArg));
		MALLOC(sizeof(sBASE_VENC_DATACONSUME_Communicate));
		psInfo->psMArg = malloc(sizeof(sBASE_VENC_DATACONSUME_MArg));
		psInfo->psVArg = malloc(sizeof(sBASE_VENC_DATACONSUME_VArg));
		psInfo->psCommu = malloc(sizeof(sBASE_VENC_DATACONSUME_Communicate));

		/* init communicate */
		psCommu = psInfo->psCommu;
		pthread_mutex_init(&psCommu->mutPayLoad, NULL);
		pthread_cond_init(&psCommu->condPayLoad, NULL);
		psCommu->lPayLoad = 0;
		psCommu->eCommuStatus = BASE_VENC_DATACONSUME_CommuNoData;

		/* init main argument */
		psMArg = psInfo->psMArg;
		psMArg->peMStatus = &psInfo->eStatus;
		psMArg->psCommu = psInfo->psCommu;
		psMArg->plPoolId = psInfo->plPoolId;
		psMArg->lMFrameCntTh = BASE_VENC_DATACONSUME_GET_FRAME_FROM_SEC(psInfo->lTimeMTh);
		psMArg->pcName = psInfo->cName;
		psMArg->pcDirName = strdup(psInfo->cName);

		/* init virtual argument */
		psVArg = psInfo->psVArg;
		psVArg->peVStatus = &psInfo->eStatus;
		psVArg->psCommu = psInfo->psCommu;
		psVArg->plPoolId = psInfo->plPoolId;
		psVArg->lVFrameCntTh = BASE_VENC_DATACONSUME_GET_FRAME_FROM_SEC(psInfo->lTimeVTh);

		/* toggle the status flag */
		psInfo->eStatus = BASE_VENC_DATACONSUME_Valid;

	}

	BVD_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_VENC_DATACONSUME_StartMConsumer(sBASE_VENC_DATACONSUME_Info* psInfo)
* arg : sBASE_VENC_DATACONSUME_Info* psInfo
* ret : eBASE_VENC_DATACONSUME_Ret
* note : start Main consumer thread
*********************************************/
eBASE_VENC_DATACONSUME_Ret eBASE_VENC_DATACONSUME_StartMConsumer(sBASE_VENC_DATACONSUME_Info* psInfo) {
	eBASE_VENC_DATACONSUME_Ret eRet = BASE_VENC_DATACONSUME_SUCCESS;
	sBASE_VENC_DATACONSUME_MArg* psMArg = psInfo->psMArg;
	LONG lComRet;

	BVD_FuncIn();

	{
		lComRet = pthread_create(&psInfo->sPidM, NULL, pvMConsumer, (void*)psMArg);
		if(lComRet != 0) {
			BVD_Debug("error with create Main thread\n");
			return BASE_VENC_DATACONSUME_NEWTHREAD;
		}

		psInfo->eStatus |= BASE_VENC_DATACONSUME_StartM;
	}

	BVD_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_VENC_DATACONSUME_StartVConsumer(sBASE_VENC_DATACONSUME_Info* psInfo)
* arg : sBASE_VENC_DATACONSUME_Info* psInfo
* ret : eBASE_VENC_DATACONSUME_Ret
* note : start virtual consumer thread
*********************************************/
eBASE_VENC_DATACONSUME_Ret eBASE_VENC_DATACONSUME_StartVConsumer(sBASE_VENC_DATACONSUME_Info* psInfo) {
	eBASE_VENC_DATACONSUME_Ret eRet = BASE_VENC_DATACONSUME_SUCCESS;
	sBASE_VENC_DATACONSUME_VArg* psVArg = psInfo->psVArg;
	LONG lComRet;

	BVD_FuncIn();

	{
		lComRet = pthread_create(&psInfo->sPidV, NULL, pvVConsumer, (void*)psVArg);
		if(lComRet != 0) {
			BVD_Debug("error with create Main thread\n");
			return BASE_VENC_DATACONSUME_NEWTHREAD;
		}

		psInfo->eStatus |= BASE_VENC_DATACONSUME_StartV;
	}

	BVD_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_VENC_DATACONSUME_StopMConsumer(sBASE_VENC_DATACONSUME_Info* psInfo)
* arg : sBASE_VENC_DATACONSUME_Info* psInfo
* ret : eBASE_VENC_DATACONSUME_Ret
* note :
*********************************************/
eBASE_VENC_DATACONSUME_Ret eBASE_VENC_DATACONSUME_StopMConsumer(sBASE_VENC_DATACONSUME_Info* psInfo) {
	eBASE_VENC_DATACONSUME_Ret eRet = BASE_VENC_DATACONSUME_SUCCESS;

	BVD_FuncIn();

	{

		/* join the thread */
		if (psInfo->eStatus & BASE_VENC_DATACONSUME_StartM) {
			pthread_join(psInfo->sPidM, NULL);
		}
	}

	BVD_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_VENC_DATACONSUME_StopVConsumer(sBASE_VENC_DATACONSUME_Info* psInfo)
* arg : sBASE_VENC_DATACONSUME_Info* psInfo
* ret : eBASE_VENC_DATACONSUME_Ret
* note :
*********************************************/
eBASE_VENC_DATACONSUME_Ret eBASE_VENC_DATACONSUME_StopVConsumer(sBASE_VENC_DATACONSUME_Info* psInfo) {
	eBASE_VENC_DATACONSUME_Ret eRet = BASE_VENC_DATACONSUME_SUCCESS;

	BVD_FuncIn();

	{

		/* join the thread */
		if (psInfo->eStatus & BASE_VENC_DATACONSUME_StartV) {
			pthread_join(psInfo->sPidV, NULL);
		}
	}

	BVD_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_VENC_DATACONSUME_Deinit(sBASE_VENC_DATACONSUME_Info* psInfo)
* arg : sBASE_VENC_DATACONSUME_Info* psInfo
* ret : eBASE_VENC_DATACONSUME_Ret
* note :
*********************************************/
eBASE_VENC_DATACONSUME_Ret eBASE_VENC_DATACONSUME_Deinit(sBASE_VENC_DATACONSUME_Info* psInfo) {
	eBASE_VENC_DATACONSUME_Ret eRet = BASE_VENC_DATACONSUME_SUCCESS;
	sBASE_VENC_DATACONSUME_Communicate* psCommu = psInfo->psCommu;

	BVD_FuncIn();

	{
		/* toggle stop flag */
		pthread_mutex_lock(&psCommu->mutPayLoad);
		psInfo->eStatus |= BASE_VENC_DATACONSUME_Stop;
		pthread_mutex_unlock(&psCommu->mutPayLoad);

		/* stop threads */
		eBASE_VENC_DATACONSUME_StopMConsumer(psInfo);
		eBASE_VENC_DATACONSUME_StopVConsumer(psInfo);

		/* release resource */
		psInfo->plPoolId = NULL;
		
		if (NULL != psInfo->psMArg->pcDirName) {
			free(psInfo->psMArg->pcDirName);
			psInfo->psMArg->pcDirName = NULL;
		}
		free(psInfo->psMArg);
		free(psInfo->psVArg);
		pthread_mutex_destroy(&psCommu->mutPayLoad);
		pthread_cond_destroy(&psCommu->condPayLoad);
		free(psInfo->psCommu);

		psInfo->eStatus = BASE_VENC_DATACONSUME_Invalid;
	}

	BVD_FuncOut();

	return eRet;
}

/*********************************************
* func : pvMConsumer(void* MArg)
* arg : void* MArg
* ret : void*
* note : Main consumer thread
*********************************************/
void* pvMConsumer(void* MArg) {
	sBASE_VENC_DATACONSUME_MArg* psMArg = (sBASE_VENC_DATACONSUME_MArg*) MArg;
	sBASE_VENC_DATACONSUME_Communicate* psCommu = psMArg->psCommu;
	eCOMM_POOL_Err ePoolRet = COMM_POOL_SUCCESS;
	eCOMM_MW_VFILE_Ret eVfileRet = COMM_MW_VFILE_SUCC;
	eBASE_MW_FILESORT_RET eRet;
	LONG lEndFlag = 0;
	LONG lCBRet = 0;
	sBASE_MW_SD_STAT* psSDStat = &gsSDStat;
	eBASE_MW_SD_ERROR eSdRet = BASE_MW_SD_SUCCESS;
	sBASE_API_LEDCTL_Info* psLedCtlInfo = &gsLedCtlInfo;
	eBASE_API_LEDCTL_LedNewStatus eStatus = BASE_API_LEDCTL_NORMAL;
	eBASE_MW_FILESORT_RET eFileSortRet = BASE_MW_FILESORT_SUCCESS;
	
	sBASE_VENC_DATACONSUME_CallBackArg sCallBackArg;
	sCOMM_MW_VFILE_Info* psVfile = NULL;

	/* attach the pool */
	ePoolRet = eCOMM_POOL_Attach(*psMArg->plPoolId, &psMArg->lMConsumerKey, "maincard");
	if(ePoolRet != COMM_POOL_SUCCESS) {
		BVD_Debug("virtual attach pool error\n");
		return NULL;
	}

	/* init call back argument */
	sCallBackArg.eFileType = BASE_VENC_DATACONSUME_FILENEW;
	sCallBackArg.lCurFrame = 0;
	sCallBackArg.lFrameTh = psMArg->lMFrameCntTh;
	sCallBackArg.pcPathPos = psMArg->pcName;
	sCallBackArg.pcFilePos = psMArg->pcName + strlen(psMArg->pcName);
	strcpy(sCallBackArg.cDirName, psMArg->pcName);

	/* init vfile */
	MALLOC(sizeof(sCOMM_MW_VFILE_Info));
	sCallBackArg.psVfile = malloc(sizeof(sCOMM_MW_VFILE_Info));
	psVfile = sCallBackArg.psVfile;
	if(!psVfile) {
		BVD_Debug("malloc error\n");
		return NULL;
	}
	
	eVfileRet = eCOMM_MW_VFILE_Init(psVfile, psMArg->pcName, 
		COMM_MW_VFILE_SETFLAG_NOCACHED | COMM_MW_VFILE_SETFLAG_WITH_SYNC);
	if(eVfileRet != COMM_MW_VFILE_SUCC) {
		BVD_Debug("vfile init error with <%d>\n", eVfileRet);
		return NULL;
	}

	{
		// SD is not init, need to update the SD space parammeter
#ifdef SD_CHECK_DEBUG			
		pthread_mutex_lock(&psSDStat->sMutStatus);
		
		if (BASE_MW_SD_SUCCESS != psSDStat->lSdState ) 
		{
			/* wait the sd card at most 20s */
			int iThreshold = 0;
			do {
				//printf("[ SD CARD ] : not ready, wait for %d sec ..\n", iThreshold);
				eSdRet = eBASE_SD_MW_GETSPACE(psSDStat, TFCARD, ".h264", 1);
				pthread_mutex_unlock(&psSDStat->sMutStatus);
				usleep(1000 * 1000);
				pthread_mutex_lock(&psSDStat->sMutStatus);
			} while(iThreshold++ < BASE_VENC_DATACONSUME_ERRORCNTMAX
				&& eSdRet != BASE_MW_SD_SUCCESS);
			
			if(eSdRet != BASE_MW_SD_SUCCESS) {
				//printf("[ SD CARD ] : there probably no sd card at all\n");
				psSDStat->lSdState = BASE_MW_SD_INVALID;
				psSDStat->eVideoStat = BASE_MW_VIDEO_NOREC;						
				BVD_Debug("get space error <%d>\n", eSdRet);
				//pthread_mutex_unlock(&psSDStat->sMutStatus);	
				
				eStatus = BASE_API_LEDCTL_SDFAIL;
				eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);		
			} else {
				//printf("[ SD CARD ] : find the sd card !\n");
				psSDStat->lSdState = BASE_MW_SD_SUCCESS;
				psSDStat->eVideoStat= BASE_MW_VIDEO_REC;						
			}
		} 
		
		//Init  the file list of current audio directory
		if (BASE_MW_SD_SUCCESS == psSDStat->lSdState && BASE_MW_FILESORT_Valid != sFileSortInfo.lState) {
			sFileSortInfo.lFileMaxLen = TFCARD_MAX_FILE_NUM;
			eFileSortRet = eBASE_MW_FILESORT_Init(
				&sFileSortInfo, sFileSortInfo.lFileMaxLen, BASE_MW_SD_MAINCARDPATH);
			/*if (eFileSortRet == BASE_MW_FILESORT_SUCCESS) {
				printf("--- Main Card:eBASE_MW_FILESORT_Init() OK!---\n");
			} else {
				printf("--- Main Card:eBASE_MW_FILESORT_Init() Fail!---\n");
			}*/		
		}
		pthread_mutex_unlock(&psSDStat->sMutStatus);
#endif			
	}	

	/* main process handle */
	while(!lEndFlag) {
#ifdef SD_CHECK_DEBUG			
		pthread_mutex_lock(&psSDStat->sMutStatus);
		if (BASE_MW_SD_SUCCESS == psSDStat->lSdState ) 
		{
			eStatus = BASE_API_LEDCTL_REC;
			eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);			
		}
		pthread_mutex_unlock(&psSDStat->sMutStatus);		
#endif			
		/* read from the pool */
		ePoolRet = eCOMM_POOL_Read(*psMArg->plPoolId, 
			BASE_VENC_DATACONSUME_ReadCallback_ISliceVersion, (void*) &sCallBackArg, psMArg->lMConsumerKey);	
		if(ePoolRet != COMM_POOL_SUCCESS) {
			if(ePoolRet == COMM_POOL_NEEDTOQUIT) {
				/* indicate quit */
				BVD_Debug("M thread will quiting...\n");
				lEndFlag = 1;
				break;
			} else {
				/* indicate a error occur */
				BVD_Debug("error to read\n");
				return NULL;
			}
		}
#ifdef SD_CHECK_DEBUG		
		lCBRet = sCallBackArg.lCBRet;
		if (lCBRet != BASE_VENC_DATACONSUME_CALLBACK_SUCCESS) 
		{
			if (BASE_VENC_DATACONSUME_CALLBACK_TFCARD == lCBRet || 
					BASE_VENC_DATACONSUME_CALLBACK_FILESORT == lCBRet) 
			{
				pthread_mutex_lock(&psSDStat->sMutStatus);
				psSDStat->lSdState = BASE_MW_SD_INVALID;
				pthread_mutex_unlock(&psSDStat->sMutStatus);
				
				if (BASE_MW_FILESORT_Valid == sFileSortInfo.lState) 
				{
					//printf("mainCard vfile error: deinit the filesort resource!\n");
					eFileSortRet = eBASE_MW_FILESORT_DeInit(&sFileSortInfo);
					/*if (eFileSortRet == BASE_MW_FILESORT_SUCCESS) {
						printf("--- Main Card:eBASE_MW_FILESORT_DeInit() OK!---\n");
					} else {
						printf("--- Main Card:eBASE_MW_FILESORT_DeInit() Fail!---\n");
					}*/	
				}						
			}
		}
#endif		
		/* signal the V consumer */
		pthread_mutex_lock(&psCommu->mutPayLoad);
		if(psCommu->eCommuStatus != BASE_VENC_DATACONSUME_CommuHaveData) {
			/* indicate V consumer is free now */
			eCOMM_POOL_QueryConsumerOrder(*psMArg->plPoolId, psMArg->lMConsumerKey, &psCommu->lPayLoad);
			psCommu->eCommuStatus = BASE_VENC_DATACONSUME_CommuHaveData;
			pthread_cond_signal(&psCommu->condPayLoad);
		}
		pthread_mutex_unlock(&psCommu->mutPayLoad);

	}

	/* send last message to V consumer */
	pthread_mutex_lock(&psCommu->mutPayLoad);
	psCommu->lPayLoad = -1;
	psCommu->eCommuStatus = BASE_VENC_DATACONSUME_CommuHaveData;
	pthread_cond_signal(&psCommu->condPayLoad);
	pthread_mutex_unlock(&psCommu->mutPayLoad);

	/* deinit call back argument */
	eVfileRet = eCOMM_MW_VFILE_Deinit(psVfile);
	if(eVfileRet != COMM_MW_VFILE_SUCC) {
		BVD_Debug("vfile deinit error with <%d>\n", eVfileRet);
		return NULL;
	}
	free(psVfile);
	
	/* detach the pool */
	ePoolRet = eCOMM_POOL_Dettach(*psMArg->plPoolId, psMArg->lMConsumerKey);
	if(ePoolRet != COMM_POOL_SUCCESS) {
		BVD_Debug("virtual dettach pool error\n");
		return NULL;
	}

	//add by yudong
	if (sFileSortInfo.lState == BASE_MW_FILESORT_Valid)
	{
		eRet = eBASE_MW_FILESORT_DeInit(&sFileSortInfo);
		/*if (eRet == BASE_MW_FILESORT_SUCCESS) {
			printf("--- eBASE_MW_FILESORT_DeInit() OK -- \n");
		} else {
			printf("--- eBASE_MW_FILESORT_DeInit() Failed -- \n");	
		}*/
	}

	return NULL;
}

/*********************************************
* func : pvVConsumer
* arg : void* VArg
* ret : void*
* note : Virtual consumer thread
*********************************************/
void* pvVConsumer(void* VArg) {
	sBASE_VENC_DATACONSUME_VArg* psVArg = (sBASE_VENC_DATACONSUME_VArg*)VArg;
	sBASE_VENC_DATACONSUME_Communicate* psCommu = psVArg->psCommu;
	eCOMM_POOL_Err ePoolRet = COMM_POOL_SUCCESS;
	LONG lEndFlag = 0;
	LONG lMOrder = 0;
	LONG lVOrder = 0;
	LONG lReadCount = 0;

	/* attach the pool */
	ePoolRet = eCOMM_POOL_Attach(*psVArg->plPoolId, &psVArg->lVConsumerKey, "virtual");
	if(ePoolRet != COMM_POOL_SUCCESS) {
		BVD_Debug("virtual attach pool error\n");
		return NULL;
	}

	/* main process handle */
	while(!lEndFlag) {
		/* wait until M consumer notified */
		pthread_mutex_lock(&psCommu->mutPayLoad);
		while(psCommu->eCommuStatus != BASE_VENC_DATACONSUME_CommuHaveData) {
			pthread_cond_wait(&psCommu->condPayLoad, &psCommu->mutPayLoad);
		}
		/* get data and release the lock */
		lMOrder = psCommu->lPayLoad;
		pthread_mutex_unlock(&psCommu->mutPayLoad);

		/* get read count */
		eCOMM_POOL_QueryConsumerOrder(*psVArg->plPoolId, psVArg->lVConsumerKey, &lVOrder);
		lReadCount = lMOrder - lVOrder;
		if(lReadCount < 0)
			lReadCount = lMOrder + COMM_POOL_MAXORDER - lVOrder;
		lReadCount -= psVArg->lVFrameCntTh;

		/* judge whether to quit */
		while((lReadCount-- >= 0) || (lMOrder == -1)) {
			ePoolRet = eCOMM_POOL_Read(*psVArg->plPoolId, NULL, NULL, psVArg->lVConsumerKey);
			if(ePoolRet != COMM_POOL_SUCCESS) {
				if(ePoolRet == COMM_POOL_NEEDTOQUIT) {
					/* indicate quit */
					BVD_Debug("V thread will quiting...\n");
					lEndFlag = 1;
					break;
				} else {
					/* indicate a error occur */
					BVD_Debug("error to read\n");
					return NULL;
				}
			}

			/* update lReadCount ? */
			
		}

		/* toggle the status */
		pthread_mutex_lock(&psCommu->mutPayLoad);
		if(psCommu->lPayLoad != -1)
			psCommu->eCommuStatus = BASE_VENC_DATACONSUME_CommuNoData;
		pthread_mutex_unlock(&psCommu->mutPayLoad);

	}

	/* detach the pool */
	ePoolRet = eCOMM_POOL_Dettach(*psVArg->plPoolId, psVArg->lVConsumerKey);
	if(ePoolRet != COMM_POOL_SUCCESS) {
		BVD_Debug("virtual dettach pool error\n");
		return NULL;
	}

	return NULL;
}

/*********************************************
* func : BASE_VENC_DATACONSUME_ReadCallback
* arg : void* pvArg
* ret : void
* note : M thread read callback function, notice the first byte is channel info, should skip first
*********************************************/
void BASE_VENC_DATACONSUME_ReadCallback(void* pvArg) {
	sCOMM_POOL_CBFuncArg* psCBArg = (sCOMM_POOL_CBFuncArg*) pvArg;
	sBASE_VENC_DATACONSUME_CallBackArg* psDataConsumeArg = (sBASE_VENC_DATACONSUME_CallBackArg*) (psCBArg->pvArg);
	sCOMM_MW_VFILE_Info* psVfile = psDataConsumeArg->psVfile;
	eCOMM_MW_VFILE_Ret eVfileRet = COMM_MW_VFILE_SUCC;
	sBASE_MW_SD_STAT* psSDStat = &gsSDStat;
	sBASE_API_LEDCTL_Info* psLedCtlInfo = &gsLedCtlInfo;
	eBASE_MW_SD_ERROR eSdRet = BASE_MW_SD_SUCCESS;
	eBASE_MW_FILESORT_RET eFileSortRet = BASE_MW_FILESORT_SUCCESS;
	eBASE_API_LEDCTL_LedNewStatus eStatus = BASE_API_LEDCTL_NORMAL;
	CHAR cFileName1[128];
	psDataConsumeArg->lCBRet = BASE_VENC_DATACONSUME_CALLBACK_SUCCESS;
	
	BVD_FuncIn();

	{
		/* first of all, check the valid */
		assert( *psCBArg->pucHeader == 0
			|| *psCBArg->pucHeader == 1
			|| *psCBArg->pucHeader == 2
		);
		
		psCBArg->pucHeader ++;
		psCBArg->lLen --;
		{
			// SD is not init, need to update the SD space parammeter
#ifdef SD_CHECK_DEBUG				
			pthread_mutex_lock(&psSDStat->sMutStatus);
			
			if (BASE_MW_SD_SUCCESS != psSDStat->lSdState ) 
			{
				eSdRet = eBASE_SD_MW_GETSPACE(psSDStat, TFCARD, ".h264", 1);
				if(eSdRet != BASE_MW_SD_SUCCESS) {
					psSDStat->lSdState = BASE_MW_SD_INVALID;
					psSDStat->eVideoStat = BASE_MW_VIDEO_NOREC;						
					BVD_Debug("get space error <%d>\n", eSdRet);
					pthread_mutex_unlock(&psSDStat->sMutStatus);	

					// printf("=== SD failed! ===\n");
					eStatus = BASE_API_LEDCTL_SDFAIL;
					eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);	
					psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_TFCARD;
					return;
				} else {
					psSDStat->lSdState = BASE_MW_SD_SUCCESS;
					psSDStat->eVideoStat= BASE_MW_VIDEO_REC;						
				}
			} 
			
			//Init  the file list of current audio directory
			if (BASE_MW_SD_SUCCESS == psSDStat->lSdState && BASE_MW_FILESORT_Valid != sFileSortInfo.lState) {
				sFileSortInfo.lFileMaxLen = TFCARD_MAX_FILE_NUM;
				eFileSortRet = eBASE_MW_FILESORT_Init(&sFileSortInfo, sFileSortInfo.lFileMaxLen, psDataConsumeArg->cDirName);
				if (eFileSortRet == BASE_MW_FILESORT_SUCCESS) {
					//printf("--- Main Card:eBASE_MW_FILESORT_Init() OK!---\n");
				} else {
					//printf("--- Main Card:eBASE_MW_FILESORT_Init() Fail!---\n");
					pthread_mutex_unlock(&psSDStat->sMutStatus);
					psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_FILESORT;
					return;
				}		
			}
			pthread_mutex_unlock(&psSDStat->sMutStatus);
#endif			
		}	
		
		if(psDataConsumeArg->eFileType == BASE_VENC_DATACONSUME_FILENEW) {
			
			memset(psDataConsumeArg->pcFilePos, 0, BASE_VENC_DATACONSUME_MAXFILENAME);
			eCOMM_UTIL_GetFileName(psDataConsumeArg->pcFilePos, BASE_VENC_DATACONSUME_MAXFILENAME, ".h264");
			
			/* check whether enough space */
#ifdef SD_CHECK_DEBUG				
			pthread_mutex_lock(&psSDStat->sMutStatus);
			//if (BASE_MW_SD_SUCCESS != psSDStat->lSdState ) 
			{
				eSdRet = eBASE_SD_MW_GETSPACE(psSDStat, TFCARD, ".h264", 1);
				if(eSdRet != BASE_MW_SD_SUCCESS) {
					psSDStat->lSdState = BASE_MW_SD_INVALID;
					psSDStat->eVideoStat= BASE_MW_VIDEO_NOREC;						
					BVD_Debug("get space error <%d>\n", eSdRet);
					pthread_mutex_unlock(&psSDStat->sMutStatus);
					
					eStatus = BASE_API_LEDCTL_SDFAIL;
					eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);		
					psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_TFCARD;
					return;
				} else {
					psSDStat->lSdState = BASE_MW_SD_SUCCESS;
					psSDStat->eVideoStat= BASE_MW_VIDEO_REC;					
				}
			}
			//printf("--- Update the Space:FreeKB = [%ld]---\n",psSDStat->lFreeKB);
			
			eFileSortRet = eBASE_MW_FILESORT_CheckSpace(
				&sFileSortInfo, psSDStat->lTotalKB, BASE_MW_RECCARD_RATIO, BASE_MW_SD_FILE_SIZEKB);
			if(eFileSortRet != BASE_MW_FILESORT_SUCCESS) {
				//printf("--- eBASE_MW_FILESORT_CheckSpace() Failed -- \n");
				pthread_mutex_unlock(&psSDStat->sMutStatus);
				psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_FILESORT;
				return;
			}			

			/*
			while(((psSDStat->lFreeKB - BASE_MW_SD_FILE_TEST) <= BASE_MW_SD_FILE_SIZEKB) 
					|| (sFileSortInfo.lFileLen >= TFCARD_MAX_FILE_NUM)) 
			*/
			while ((BASE_MW_FILESORT_IsNoenough == sFileSortInfo.lIsEnough) || (sFileSortInfo.lFileLen >= TFCARD_MAX_FILE_NUM))
			{
				/* remove the oldest file */
				if (sFileSortInfo.lFileLen == 0) {
					pthread_mutex_unlock(&psSDStat->sMutStatus);
					psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_FILESORT;
					return;				
				}				
				memset(cFileName1,0,sizeof(cFileName1));
				eFileSortRet = eBASE_MW_FILESORT_Read(&sFileSortInfo,cFileName1);
				if(eFileSortRet != BASE_MW_FILESORT_SUCCESS) {
					//printf("--- eBASE_MW_FILESORT_Read() %s Failed -- \n",cFileName1);
					pthread_mutex_unlock(&psSDStat->sMutStatus);
					psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_FILESORT;
					return;
				}
				//printf("Current the FileNum : %ld!\n", sFileSortInfo.lFileLen);
				eFileSortRet = remove(cFileName1);
			  	if(eFileSortRet)
			  	{
					BVD_Debug("Remove the FileName : %s failed!\n", cFileName1);
					//perror("Error to remove file");
					eFileSortRet = unlink(cFileName1);
					if(eFileSortRet) {
						BVD_Debug("Unlink the FileName : %s failed!\n", cFileName1);
				  		//perror("Error to remove file");	
						if (BASE_MW_SD_SUCCESS == psSDStat->lSdState) {
							printf("Can't find the FileName : %s \n", cFileName1);
							continue;							
						} else {
							pthread_mutex_unlock(&psSDStat->sMutStatus);
							psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_TFCARD;
							
							return;						
						}
					}
			  	}
				//printf("Remove the FileName : %s successful!\n", cFileName1);
				
				/* check the space again */
				eSdRet = eBASE_SD_MW_CHECKSTATUS(psSDStat);
				if(eSdRet != BASE_MW_SD_SUCCESS) {
					psSDStat->lSdState = BASE_MW_SD_INVALID;
					psSDStat->eVideoStat= BASE_MW_VIDEO_NOREC;						
					BVD_Debug("get space error <%d>\n", eSdRet);
					pthread_mutex_unlock(&psSDStat->sMutStatus);
					
					eStatus = BASE_API_LEDCTL_SDFAIL;
					eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);		
					psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_TFCARD;
					
					return;
				} else {
					psSDStat->lSdState = BASE_MW_SD_SUCCESS;
					psSDStat->eVideoStat= BASE_MW_VIDEO_REC;					
				}			

				/* check the free space */
				eFileSortRet = eBASE_MW_FILESORT_CheckSpace(
					&sFileSortInfo, psSDStat->lTotalKB, BASE_MW_RECCARD_RATIO, BASE_MW_SD_FILE_SIZEKB);			
				if(eFileSortRet != BASE_MW_FILESORT_SUCCESS) {
					//printf("--- eBASE_MW_FILESORT_CheckSpace() Failed -- \n");
					pthread_mutex_unlock(&psSDStat->sMutStatus);
					psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_FILESORT;
					
					return;
				}				
			}
			//psSDStat->lFreeKB -= BASE_MW_SD_FILE_SIZEKB;
			pthread_mutex_unlock(&psSDStat->sMutStatus);
#endif

			eVfileRet = eCOMM_MW_VFILE_Open(psVfile, psDataConsumeArg->pcPathPos);
			if(eVfileRet != COMM_MW_VFILE_SUCC) {
				BVD_Debug("vfile error with <%d>\n", eVfileRet);
				psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_VFILE;
				return ;
			}
			//printf("Open the FileName : %s successful!\n", psDataConsumeArg->pcPathPos);
			
#ifdef SD_CHECK_DEBUG				
			eFileSortRet = eBASE_MW_FILESORT_Write(&sFileSortInfo,psDataConsumeArg->pcPathPos);
			if (eFileSortRet != BASE_MW_FILESORT_SUCCESS) {
				//printf("--- eBASE_MW_FILESORT_Write() %s Failed -- \n",psDataConsumeArg->pcPathPos);
				psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_FILESORT;
				
				return;
			} 		
			//printf("Add the FileName : %s successful!\n", psDataConsumeArg->pcPathPos);
			//printf("Current the FileNum : %ld!\n", sFileSortInfo.lFileLen);
#endif
			/* toggle the file type */
			psDataConsumeArg->eFileType = BASE_VENC_DATACONSUME_FILECONTINUE;
		}

		/* write the data to the file */
		eVfileRet = eCOMM_MW_VFILE_Write(psVfile, psCBArg->pucHeader, psCBArg->lLen);
		if(eVfileRet != COMM_MW_VFILE_SUCC) {
			BVD_Debug("vfile error with <%d>\n", eVfileRet);
			psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_VFILE;
			if (0 == psDataConsumeArg->lCurFrame) {
				eFileSortRet = remove(psDataConsumeArg->pcPathPos);
			  	if(eFileSortRet)
			  	{
					BVD_Debug("Remove the empty file : %s failed!\n", psDataConsumeArg->pcPathPos);
					perror("Error to Remove the empty file");
			  	}
				//printf("Remove the empty file : %s successful!\n", psDataConsumeArg->pcPathPos);				
			}
			psDataConsumeArg->eFileType = BASE_VENC_DATACONSUME_FILENEW;
			psDataConsumeArg->lCurFrame = 0;			
			eStatus = BASE_API_LEDCTL_SDFAIL;
			eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);
			return ;
		}

		/* judge whether need new file */
		if(++psDataConsumeArg->lCurFrame >= psDataConsumeArg->lFrameTh) {
			eVfileRet = eCOMM_MW_VFILE_Close(psVfile);
			if(eVfileRet != COMM_MW_VFILE_SUCC) {
				BVD_Debug("vfile error with <%d>\n", eVfileRet);
				psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_VFILE;
				return ;
			}
			
			psDataConsumeArg->eFileType = BASE_VENC_DATACONSUME_FILENEW;
			psDataConsumeArg->lCurFrame = 0;
		}
	}

	BVD_FuncOut();

	return;
}

#define IFRAME_FIRST
#define DEBUG_SAVE_FILE
/*********************************************
* func : BASE_VENC_DATACONSUME_ReadCallback_ISliceVersion
* arg : void* pvArg
* ret : void
* note : in this version, every files begin with I Slice
*********************************************/
void BASE_VENC_DATACONSUME_ReadCallback_ISliceVersion(void* pvArg) {
	sCOMM_POOL_CBFuncArg* psCBArg = (sCOMM_POOL_CBFuncArg*) pvArg;
	sBASE_VENC_DATACONSUME_CallBackArg* psDataConsumeArg = (sBASE_VENC_DATACONSUME_CallBackArg*) (psCBArg->pvArg);
	eBASE_VENC_DATACONSUME_Ret eDataConsumeRet = BASE_VENC_DATACONSUME_SUCCESS;
#ifdef DEBUG_SAVE_FILE
	sBASE_MW_SD_STAT* psSDStat = &gsSDStat;
	sBASE_API_LEDCTL_Info* psLedCtlInfo = &gsLedCtlInfo;
	eBASE_MW_SD_ERROR eSdRet = BASE_MW_SD_SUCCESS;
	eBASE_MW_FILESORT_RET eFileSortRet = BASE_MW_FILESORT_SUCCESS;
	eBASE_API_LEDCTL_LedNewStatus eStatus = BASE_API_LEDCTL_NORMAL;
	CHAR cFileName1[128];
#endif

	BVD_FuncIn();

	{
		/* first of all, check the valid */
		assert( *psCBArg->pucHeader == 0
			|| *psCBArg->pucHeader == 1
			|| *psCBArg->pucHeader == 2
		);
		
		psCBArg->pucHeader ++;
		psCBArg->lLen --;

		{
#ifdef DEBUG_SAVE_FILE				
			pthread_mutex_lock(&psSDStat->sMutStatus);
			
			if (BASE_MW_SD_SUCCESS != psSDStat->lSdState ) 
			{
				eSdRet = eBASE_SD_MW_GETSPACE(psSDStat, TFCARD, ".h264", 1);
				if(eSdRet != BASE_MW_SD_SUCCESS) {
					psSDStat->lSdState = BASE_MW_SD_INVALID;
					psSDStat->eVideoStat = BASE_MW_VIDEO_NOREC;						
					BVD_Debug("get space error <%d>\n", eSdRet);
					pthread_mutex_unlock(&psSDStat->sMutStatus);	

					// printf("=== SD failed! ===\n");
					eStatus = BASE_API_LEDCTL_SDFAIL;
#ifdef IFRAME_FIRST
					psDataConsumeArg->lCurFrame = 0;
					eCOMM_MW_VFILE_Reset(psDataConsumeArg->psVfile);
#endif /* IFRAME_FIRST */
					eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);	
					psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_TFCARD;
					return;
				} else {
					psSDStat->lSdState = BASE_MW_SD_SUCCESS;
					psSDStat->eVideoStat= BASE_MW_VIDEO_REC;						
				}
			} 
			
			if (BASE_MW_SD_SUCCESS == psSDStat->lSdState && BASE_MW_FILESORT_Valid != sFileSortInfo.lState) {
				sFileSortInfo.lFileMaxLen = TFCARD_MAX_FILE_NUM;
				eFileSortRet = eBASE_MW_FILESORT_Init(&sFileSortInfo, sFileSortInfo.lFileMaxLen, psDataConsumeArg->cDirName);
				if (eFileSortRet == BASE_MW_FILESORT_SUCCESS) {
					//printf("--- Main Card:eBASE_MW_FILESORT_Init() OK!---\n");
				} else {
					//printf("--- Main Card:eBASE_MW_FILESORT_Init() Fail!---\n");
					pthread_mutex_unlock(&psSDStat->sMutStatus);
					psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_FILESORT;
					return;
				}		
			}
			pthread_mutex_unlock(&psSDStat->sMutStatus);
#endif			
		}
		
		psDataConsumeArg->lCBRet = BASE_VENC_DATACONSUME_CALLBACK_SUCCESS;
		if(psDataConsumeArg->eFileType == BASE_VENC_DATACONSUME_FILENEW) {
			/* check whether is I Slice */
			if (! lBASE_MW_COMM_IsISlice(psCBArg->pucHeader, psCBArg->lLen)) {
				/* new file should begin with I Slice */
				BVD_Debug("waiting for I Slice for new file\n");
				return;
			}

#ifdef DEBUG_SAVE_FILE			
			pthread_mutex_lock(&psSDStat->sMutStatus);
			{
				eSdRet = eBASE_SD_MW_GETSPACE(psSDStat, TFCARD, ".h264", 1);
				if(eSdRet != BASE_MW_SD_SUCCESS) {
					psSDStat->lSdState = BASE_MW_SD_INVALID;
					psSDStat->eVideoStat= BASE_MW_VIDEO_NOREC;						
					BVD_Debug("get space error <%d>\n", eSdRet);
					pthread_mutex_unlock(&psSDStat->sMutStatus);

					eStatus = BASE_API_LEDCTL_SDFAIL;
#ifdef IFRAME_FIRST
					psDataConsumeArg->lCurFrame = 0;
					eCOMM_MW_VFILE_Reset(psDataConsumeArg->psVfile);
#endif /* IFRAME_FIRST */
					eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);		
					psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_TFCARD;
					return;
				} else {
					psSDStat->lSdState = BASE_MW_SD_SUCCESS;
					psSDStat->eVideoStat= BASE_MW_VIDEO_REC;					
				}
			}
			
			eFileSortRet = eBASE_MW_FILESORT_CheckSpace(
				&sFileSortInfo, psSDStat->lTotalKB, BASE_MW_RECCARD_RATIO, BASE_MW_SD_FILE_SIZEKB);			
			if(eFileSortRet != BASE_MW_FILESORT_SUCCESS) {
				//printf("--- eBASE_MW_FILESORT_CheckSpace() Failed -- \n");
				pthread_mutex_unlock(&psSDStat->sMutStatus);
				psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_FILESORT;
				return;
			}			

			while ((BASE_MW_FILESORT_IsNoenough == sFileSortInfo.lIsEnough) || (sFileSortInfo.lFileLen >= TFCARD_MAX_FILE_NUM))
			{
				/* remove the oldest file */
				if (sFileSortInfo.lFileLen == 0) {
					pthread_mutex_unlock(&psSDStat->sMutStatus);
					psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_FILESORT;
					return;				
				}				
				memset(cFileName1,0,sizeof(cFileName1));
				eFileSortRet = eBASE_MW_FILESORT_Read(&sFileSortInfo,cFileName1);
				if(eFileSortRet != BASE_MW_FILESORT_SUCCESS) {
					//printf("--- eBASE_MW_FILESORT_Read() %s Failed -- \n",cFileName1);
					pthread_mutex_unlock(&psSDStat->sMutStatus);
					psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_FILESORT;
					return;
				}
				//printf("Current the FileNum : %ld!\n", sFileSortInfo.lFileLen);
				eFileSortRet = remove(cFileName1);
			  	if(eFileSortRet)
			  	{
					BVD_Debug("Remove the FileName : %s failed!\n", cFileName1);
					//perror("Error to remove file");
					eFileSortRet = unlink(cFileName1);
					if(eFileSortRet) {
						BVD_Debug("Unlink the FileName : %s failed!\n", cFileName1);
				  		//perror("Error to remove file");	
						if (BASE_MW_SD_SUCCESS == psSDStat->lSdState) {
							//printf("Can't find the FileName : %s \n", cFileName1);
							continue;							
						} else {
							pthread_mutex_unlock(&psSDStat->sMutStatus);
							psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_TFCARD;
							return;						
						}
					}
			  	}
				//printf("Remove the FileName : %s successful!\n", cFileName1);
				
				/* check the space again */
				eSdRet = eBASE_SD_MW_CHECKSTATUS(psSDStat);
				if(eSdRet != BASE_MW_SD_SUCCESS) {
					psSDStat->lSdState = BASE_MW_SD_INVALID;
					psSDStat->eVideoStat= BASE_MW_VIDEO_NOREC;						
					BVD_Debug("get space error <%d>\n", eSdRet);
					pthread_mutex_unlock(&psSDStat->sMutStatus);
					
					eStatus = BASE_API_LEDCTL_SDFAIL;
					eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);		
					psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_TFCARD;
					return;
				} else {
					psSDStat->lSdState = BASE_MW_SD_SUCCESS;
					psSDStat->eVideoStat= BASE_MW_VIDEO_REC;					
				}			

				/* check the free space */
				eFileSortRet = eBASE_MW_FILESORT_CheckSpace(
					&sFileSortInfo, psSDStat->lTotalKB, BASE_MW_RECCARD_RATIO, BASE_MW_SD_FILE_SIZEKB);
				if(eFileSortRet != BASE_MW_FILESORT_SUCCESS) {
					//printf("--- eBASE_MW_FILESORT_CheckSpace() Failed -- \n");
					pthread_mutex_unlock(&psSDStat->sMutStatus);
					psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_FILESORT;
					return;
				}				
			}
			pthread_mutex_unlock(&psSDStat->sMutStatus);
#endif
			
			/* only the first time or error recover should enter this branch */
			eDataConsumeRet = eBASE_VENC_DATACONSUME_OpenNewFile(psDataConsumeArg);
			if (eDataConsumeRet != BASE_VENC_DATACONSUME_SUCCESS) {
				BVD_Debug("error with <%d>\n", eDataConsumeRet);
				return;
			}
#ifdef DEBUG_SAVE_FILE				
			eFileSortRet = eBASE_MW_FILESORT_Write(&sFileSortInfo,psDataConsumeArg->pcPathPos);
			if (eFileSortRet != BASE_MW_FILESORT_SUCCESS) {
				//printf("--- eBASE_MW_FILESORT_Write() %s Failed -- \n",psDataConsumeArg->pcPathPos);
				psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_FILESORT;
				
				return;
			} 		
			//printf("Add the FileName : %s successful!\n", psDataConsumeArg->pcPathPos);
			//printf("Current the FileNum : %ld!\n", sFileSortInfo.lFileLen);
#endif				
		}

		/* judge whether need new file */
		if((psDataConsumeArg->lCurFrame++ < psDataConsumeArg->lFrameTh)
			|| (! lBASE_MW_COMM_IsISlice(psCBArg->pucHeader, psCBArg->lLen))) {
			/* write current frame to file */
			eDataConsumeRet = eBASE_VENC_DATACONSUME_WriteCurrentFrame(psCBArg);
			if (eDataConsumeRet != BASE_VENC_DATACONSUME_SUCCESS) {
				BVD_Debug("write current frame error <%d>\n", eDataConsumeRet);
				return;
			}

			/* successful write current frame */
			// BVD_Debug("successful write current frame\n");
			return;
		}

		/* need write to new file */
		assert(psDataConsumeArg->lCurFrame >= psDataConsumeArg->lFrameTh);
		assert(lBASE_MW_COMM_IsISlice(psCBArg->pucHeader, psCBArg->lLen));
		eDataConsumeRet = eBASE_VENC_DATACONSUME_CloseFile(psDataConsumeArg);
		if (eDataConsumeRet != BASE_VENC_DATACONSUME_SUCCESS) {
			BVD_Debug("close file error <%d>\n", eDataConsumeRet);
			return;
		}

#ifdef DEBUG_SAVE_FILE				
		pthread_mutex_lock(&psSDStat->sMutStatus);
		{
			eSdRet = eBASE_SD_MW_GETSPACE(psSDStat, TFCARD, ".h264", 1);
			if(eSdRet != BASE_MW_SD_SUCCESS) {
				psSDStat->lSdState = BASE_MW_SD_INVALID;
				psSDStat->eVideoStat= BASE_MW_VIDEO_NOREC;
				BVD_Debug("get space error <%d>\n", eSdRet);
				pthread_mutex_unlock(&psSDStat->sMutStatus);
				
				eStatus = BASE_API_LEDCTL_SDFAIL;
#ifdef IFRAME_FIRST
				psDataConsumeArg->lCurFrame = 0;
				eCOMM_MW_VFILE_Reset(psDataConsumeArg->psVfile);
#endif /* IFRAME_FIRST */
				eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);		
				psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_TFCARD;
				return;
			} else {
				psSDStat->lSdState = BASE_MW_SD_SUCCESS;
				psSDStat->eVideoStat= BASE_MW_VIDEO_REC;
			}
		}
		//printf("--- Update the Space:FreeKB = [%ld]---\n",psSDStat->lFreeKB);
		
		eFileSortRet = eBASE_MW_FILESORT_CheckSpace(
			&sFileSortInfo, psSDStat->lTotalKB, BASE_MW_RECCARD_RATIO, BASE_MW_SD_FILE_SIZEKB);
		if(eFileSortRet != BASE_MW_FILESORT_SUCCESS) {
			//printf("--- eBASE_MW_FILESORT_CheckSpace() Failed -- \n");
			pthread_mutex_unlock(&psSDStat->sMutStatus);
			psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_FILESORT;
			return;
		}			

		while ((BASE_MW_FILESORT_IsNoenough == sFileSortInfo.lIsEnough) || (sFileSortInfo.lFileLen >= TFCARD_MAX_FILE_NUM))
		{
			/* remove the oldest file */
			if (sFileSortInfo.lFileLen == 0) {
				pthread_mutex_unlock(&psSDStat->sMutStatus);
				psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_FILESORT;
				return;
			}
			memset(cFileName1,0,sizeof(cFileName1));
			eFileSortRet = eBASE_MW_FILESORT_Read(&sFileSortInfo,cFileName1);
			if(eFileSortRet != BASE_MW_FILESORT_SUCCESS) {
				//printf("--- eBASE_MW_FILESORT_Read() %s Failed -- \n",cFileName1);
				pthread_mutex_unlock(&psSDStat->sMutStatus);
				psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_FILESORT;
				return;
			}
			//printf("Current the FileNum : %ld!\n", sFileSortInfo.lFileLen);
			eFileSortRet = remove(cFileName1);
		  	if(eFileSortRet)
		  	{
				BVD_Debug("Remove the FileName : %s failed!\n", cFileName1);
				//perror("Error to remove file");
				eFileSortRet = unlink(cFileName1);
				if(eFileSortRet) {
					BVD_Debug("Unlink the FileName : %s failed!\n", cFileName1);
			  		//perror("Error to remove file");	
					if (BASE_MW_SD_SUCCESS == psSDStat->lSdState) {
						printf("Can't find the FileName : %s \n", cFileName1);
						continue;
					} else {
						pthread_mutex_unlock(&psSDStat->sMutStatus);
						psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_TFCARD;
						
						return;
					}
				}
		  	}
			//printf("Remove the FileName : %s successful!\n", cFileName1);
			
			/* check the space again */
			eSdRet = eBASE_SD_MW_CHECKSTATUS(psSDStat);
			if(eSdRet != BASE_MW_SD_SUCCESS) {
				psSDStat->lSdState = BASE_MW_SD_INVALID;
				psSDStat->eVideoStat= BASE_MW_VIDEO_NOREC;
				BVD_Debug("get space error <%d>\n", eSdRet);
				pthread_mutex_unlock(&psSDStat->sMutStatus);
				
				eStatus = BASE_API_LEDCTL_SDFAIL;
				eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);		
				psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_TFCARD;
				return;
			} else {
				psSDStat->lSdState = BASE_MW_SD_SUCCESS;
				psSDStat->eVideoStat= BASE_MW_VIDEO_REC;
			}			

			/* check the free space */
			eFileSortRet = eBASE_MW_FILESORT_CheckSpace(
				&sFileSortInfo, psSDStat->lTotalKB, BASE_MW_RECCARD_RATIO, BASE_MW_SD_FILE_SIZEKB);
			if(eFileSortRet != BASE_MW_FILESORT_SUCCESS) {
				//printf("--- eBASE_MW_FILESORT_CheckSpace() Failed -- \n");
				pthread_mutex_unlock(&psSDStat->sMutStatus);
				psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_FILESORT;
				return;
			}
		}
		pthread_mutex_unlock(&psSDStat->sMutStatus);
#endif

		/* open new file */
		eDataConsumeRet = eBASE_VENC_DATACONSUME_OpenNewFile(psDataConsumeArg);
		if (eDataConsumeRet != BASE_VENC_DATACONSUME_SUCCESS) {
			BVD_Debug("error with <%d>\n", eDataConsumeRet);
			return;
		}

#ifdef DEBUG_SAVE_FILE				
		eFileSortRet = eBASE_MW_FILESORT_Write(&sFileSortInfo,psDataConsumeArg->pcPathPos);
		if (eFileSortRet != BASE_MW_FILESORT_SUCCESS) {
			printf("--- eBASE_MW_FILESORT_Write() %s Failed -- \n",psDataConsumeArg->pcPathPos);
			psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_FILESORT;
			
			return;
		} 		
		//printf("Add the FileName : %s successful!\n", psDataConsumeArg->pcPathPos);
		//printf("Current the FileNum : %ld!\n", sFileSortInfo.lFileLen);
#endif			
		
		/* write current frame to file */
		eDataConsumeRet = eBASE_VENC_DATACONSUME_WriteCurrentFrame(psCBArg);
		if (eDataConsumeRet != BASE_VENC_DATACONSUME_SUCCESS) {
			BVD_Debug("write current frame error <%d>\n", eDataConsumeRet);
			return;
		}
		psDataConsumeArg->lCurFrame ++;
	}

	BVD_FuncOut();

	return;
}

/*********************************************
* func : eBASE_VENC_DATACONSUME_UpdateTimeThreshold(sBASE_VENC_DATACONSUME_Info* psInfo, LONG lNewMTimeTh, LONG lNewVTimeTh)
* arg : sBASE_VENC_DATACONSUME_Info* psInfo, LONG lNewMTimeTh, LONG lNewVTimeTh
* ret : eBASE_VENC_DATACONSUME_Ret
* note : update M time threshold and V time threshold
*********************************************/
eBASE_VENC_DATACONSUME_Ret eBASE_VENC_DATACONSUME_UpdateTimeThreshold(sBASE_VENC_DATACONSUME_Info* psInfo, LONG lNewMTimeTh, LONG lNewVTimeTh) {
	eBASE_VENC_DATACONSUME_Ret eRet = BASE_VENC_DATACONSUME_SUCCESS;

	BVD_FuncIn();

	{
		if (lNewMTimeTh != BASE_VENC_DATACONSUME_DEFAULTTIME) {
			BVD_Debug("update M time th : < %ld > -> < %ld >\n", psInfo->lTimeMTh, lNewMTimeTh);
			psInfo->lTimeMTh = lNewMTimeTh;
			psInfo->psMArg->lMFrameCntTh = BASE_VENC_DATACONSUME_GET_FRAME_FROM_SEC(lNewMTimeTh);
		}

		if (lNewVTimeTh != BASE_VENC_DATACONSUME_DEFAULTTIME) {
			BVD_Debug("update V time th : < %ld > -> < %ld >\n", psInfo->lTimeVTh, lNewVTimeTh);
			psInfo->lTimeVTh = lNewVTimeTh;
			psInfo->psVArg->lVFrameCntTh = BASE_VENC_DATACONSUME_GET_FRAME_FROM_SEC(lNewVTimeTh);
		}
	}

	BVD_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_VENC_DATACONSUME_OpenNewFile(sBASE_VENC_DATACONSUME_CallBackArg* psDataConsumeArg)
* arg : sBASE_VENC_DATACONSUME_CallBackArg* psDataConsumeArg
* ret : eBASE_VENC_DATACONSUME_Ret
* note : open new file
*********************************************/
eBASE_VENC_DATACONSUME_Ret eBASE_VENC_DATACONSUME_OpenNewFile(sBASE_VENC_DATACONSUME_CallBackArg* psDataConsumeArg) {
	eBASE_VENC_DATACONSUME_Ret eRet = BASE_VENC_DATACONSUME_SUCCESS;
	sCOMM_MW_VFILE_Info* psVfile = psDataConsumeArg->psVfile;
	eCOMM_MW_VFILE_Ret eVfileRet = COMM_MW_VFILE_SUCC;

	BVD_FuncIn();

	{
		memset(psDataConsumeArg->pcFilePos, 0, BASE_VENC_DATACONSUME_MAXFILENAME);
		eCOMM_UTIL_GetFileName(psDataConsumeArg->pcFilePos, BASE_VENC_DATACONSUME_MAXFILENAME, ".h264");

		eVfileRet = eCOMM_MW_VFILE_Open(psVfile, psDataConsumeArg->pcPathPos);
		if(eVfileRet != COMM_MW_VFILE_SUCC) {
			BVD_Debug("vfile error with <%d>\n", eVfileRet);
			psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_VFILE;
			return BASE_VENC_DATACONSUME_FILEOPEN;
		}
		printf("Open the FileName : %s successful!\n", psDataConsumeArg->pcPathPos);

		/* toggle the file type */
		psDataConsumeArg->eFileType = BASE_VENC_DATACONSUME_FILECONTINUE;
#ifdef RECONFIGTIME
		eCOMM_UTIL_SaveCurCalendarTime2file(SAVEFILE_TIMESTAMP);
#endif
	}

	BVD_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_VENC_DATACONSUME_WriteCurrentFrame(sCOMM_POOL_CBFuncArg* psCBArg)
* arg : sCOMM_POOL_CBFuncArg* psCBArg
* ret : eBASE_VENC_DATACONSUME_Ret
* note : write current frame
*********************************************/
eBASE_VENC_DATACONSUME_Ret eBASE_VENC_DATACONSUME_WriteCurrentFrame(sCOMM_POOL_CBFuncArg* psCBArg) {
	eBASE_VENC_DATACONSUME_Ret eRet = BASE_VENC_DATACONSUME_SUCCESS;
	sBASE_VENC_DATACONSUME_CallBackArg* psDataConsumeArg = (sBASE_VENC_DATACONSUME_CallBackArg*) psCBArg->pvArg;
	sCOMM_MW_VFILE_Info* psVfile = psDataConsumeArg->psVfile;
	eCOMM_MW_VFILE_Ret eVfileRet = COMM_MW_VFILE_SUCC;
	sBASE_API_LEDCTL_Info* psLedCtlInfo = &gsLedCtlInfo;

	BVD_FuncIn();

	{
		eVfileRet = eCOMM_MW_VFILE_Write(psVfile, psCBArg->pucHeader, psCBArg->lLen);
		if(eVfileRet != COMM_MW_VFILE_SUCC) {
			BVD_Debug("vfile error with <%d>\n", eVfileRet);
			psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_VFILE;
			psDataConsumeArg->eFileType = BASE_VENC_DATACONSUME_FILENEW;
			psDataConsumeArg->lCurFrame = 0;
			eCOMM_MW_VFILE_Close(psVfile);
#ifdef IFRAME_FIRST
			eCOMM_MW_VFILE_Reset(psVfile);
#endif /* IFRAME_FIRST */
			eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg, BASE_API_LEDCTL_SDFAIL);
			return BASE_VENC_DATACONSUME_FILEWRITE;
		}
	}

	BVD_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_VENC_DATACONSUME_CloseFile(sBASE_VENC_DATACONSUME_CallBackArg* psDataConsumeArg)
* arg : sBASE_VENC_DATACONSUME_CallBackArg* psDataConsumeArg
* ret : eBASE_VENC_DATACONSUME_Ret
* note : close current file
*********************************************/
eBASE_VENC_DATACONSUME_Ret eBASE_VENC_DATACONSUME_CloseFile(sBASE_VENC_DATACONSUME_CallBackArg* psDataConsumeArg) {
	eBASE_VENC_DATACONSUME_Ret eRet = BASE_VENC_DATACONSUME_SUCCESS;
	sCOMM_MW_VFILE_Info* psVfile = psDataConsumeArg->psVfile;
	eCOMM_MW_VFILE_Ret eVfileRet = COMM_MW_VFILE_SUCC;

	BVD_FuncIn();

	{
		eVfileRet = eCOMM_MW_VFILE_Close(psVfile);
		if(eVfileRet != COMM_MW_VFILE_SUCC) {
			BVD_Debug("vfile error with <%d>\n", eVfileRet);
			psDataConsumeArg->eFileType = BASE_VENC_DATACONSUME_FILENEW;
			psDataConsumeArg->lCurFrame = 0;
#ifdef IFRAME_FIRST
			eCOMM_MW_VFILE_Reset(psVfile);
#endif /* IFRAME_FIRST */
			psDataConsumeArg->lCBRet |= BASE_VENC_DATACONSUME_CALLBACK_VFILE;
			return BASE_VENC_DATACONSUME_FILECLOSE;
		}
		
		psDataConsumeArg->eFileType = BASE_VENC_DATACONSUME_FILENEW;
		psDataConsumeArg->lCurFrame = 0;
	}

	BVD_FuncOut();

	return eRet;
}

#ifdef __cplusplus
}
#endif
