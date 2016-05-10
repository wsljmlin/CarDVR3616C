#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************
	include
*********************************************************/
#define _GNU_SOURCE
#include "base_mw_subcard_rec.h"
#include <fcntl.h>
#include <assert.h>

/*********************************************************
	macro / enum
*********************************************************/
//#define BASE_MW_SUBCARD_REC_DEBUG
#ifdef BASE_MW_SUBCARD_REC_DEBUG
#define BMSR_Debug(fmt, arg...) fprintf(stdout, "[ BMSR ] : %s() %d "fmt, __func__, __LINE__, ##arg)
#else
#define BMSR_Debug(fmt, arg...)
#endif

#define BMSR_FuncIn() BMSR_Debug("in\n")
#define BMSR_FuncOut() BMSR_Debug("out\n")

#define BMSR_iVal(v) BMSR_Debug("%s : %d\n", #v, v)
#define BMSR_lVal(v) BMSR_Debug("%s : %ld\n", #v, v)
#define BMSR_ptVal(v) BMSR_Debug("%s : %p\n", #v, v)

#define BASE_MW_SUBCARD_GetFrameCntFromTime(sec) (30 * (sec))
#define BASE_MW_SUBCARD_MaxFileName 64
#define BASE_MW_SUBCARD_NALTYPEMASK 0x1f
#define BASE_MW_SUBCARD_NALTYPE_PSLICE 1
#define BASE_MW_SUBCARD_REC_ERRORCNTMAX 20

typedef enum {
	BASE_MW_SUBCARD_REC_ISNOTEND = 0x0,
	BASE_MW_SUBCARD_REC_ISEND = 0x1,
	BASE_MW_SUBCARD_REC_ENDMASK = 0x1,
} eBASE_MW_SUBCARD_REC_Endflag;

typedef enum {
	BASE_MW_SUBCARD_REC_CBSUCCESS = 0x00,
	BASE_MW_SUBCARD_REC_CBFAIL = 0x10,
	BASE_MW_SUBCARD_REC_CBGETFRAMETYPE = 0x20,
	BASE_MW_SUBCARD_REC_CBOPENFILE = 0x40,
	BASE_MW_SUBCARD_REC_CBWRITEFILE = 0x80,
	BASE_MW_SUBCARD_REC_CBVFILE = 0x100,
	BASE_MW_SUBCARD_REC_CBTFCARD = 0x200,
	BASE_MW_SUBCARD_REC_CBFILESORT = 0x400,
	BASE_MW_SUBCARD_REC_CBRETMASK = 0xff0,
} eBASE_MW_SUBCARD_REC_CALLBACKRET;

/*********************************************************
	struct
*********************************************************/

/*********************************************************
	variable
*********************************************************/
#ifdef SUBCARD_CHECK_DEBUG
sBASE_MW_FILESORT_Info sFileSort;
#else
LONG glFirSubCardInit = 0;
LONG glFolderIndex = 0;
sCOMM_SPACESAFER_Info gsTwoFolder[TWO_FOLDER] = 
{
	[0] = {
		.eStatus = COMM_SPACESAFER_STATUS_INVALID,
	},
	[1] = {
		.eStatus = COMM_SPACESAFER_STATUS_INVALID,
	}
};
CHAR gcMountInfo[TWO_FOLDER][32] = {"/opt/subCard",
									"/opt/subCard01"};
CHAR gcTargetFileInfo[TWO_FOLDER][32] = {"/opt/subCard/",
										"/opt/subCard01/"};
#endif /* SUBCARD_CHECK_DEBUG */
/*********************************************************
	prototype
*********************************************************/

/*********************************************************
	function
*********************************************************/
void vBASE_MW_SUBCARD_REC_Read(void* pvArg);
eBASE_MW_SUBCARD_REC_Ret eBASE_MW_SUBCARD_REC_GetFrameType(eBASE_MW_SUBCARD_REC_Frametype* peType, UCHAR* pucHeader, LONG lLen);


/**************************************************************
 * func : eBASE_MW_SUBCARD_REC_Init(sBASE_MW_SUBCARD_REC_Info* psInfo, LONG lPoolId, LONG lTimeTh, CHAR* pcPathName)
 * arg : sBASE_MW_SUBCARD_REC_Info* psInfo, LONG lPoolId, LONG lTimeTh, CHAR* pcPathName
 * ret : eBASE_MW_SUBCARD_REC_Ret
 * note : init the struct of subcard rec
 *************************************************************/
eBASE_MW_SUBCARD_REC_Ret eBASE_MW_SUBCARD_REC_Init(sBASE_MW_SUBCARD_REC_Info* psInfo, LONG lPoolId, LONG lTimeTh, CHAR* pcPathName) {
	eBASE_MW_SUBCARD_REC_Ret eRet = BASE_MW_SUBCARD_REC_SUCCESS;
	eCOMM_MW_VFILE_Ret eVFileRet = COMM_MW_VFILE_SUCC;
	sBASE_MW_SUBCARD_REC_ReadInfo* psReadInfo = NULL;

	BMSR_FuncIn();

	{
		psInfo->lPoolId = lPoolId;
		psInfo->lTimeThreshold = lTimeTh;
		psInfo->pcDirPath = NULL;
		
		/* init path */
		memset(psInfo->cRecPath, 0, BASE_MW_SUBCARD_MAXFILENAME);
		if(pcPathName) {
			strcpy(psInfo->cRecPath, pcPathName);
			psInfo->pcDirPath = strdup(pcPathName);

			/* if not have this path, then build it first */
			if(access(psInfo->cRecPath, F_OK) != 0) {
				BMSR_Debug("the path %s is not exist, we will make it first\n", psInfo->cRecPath);
				mkdir(psInfo->cRecPath, 
					S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
			}
		} else {
			/* the default path is current path */
			strcpy(psInfo->cRecPath, "./");
			psInfo->pcDirPath = strdup("./");
		}

		/* init callback argument */
		MALLOC(sizeof(sBASE_MW_SUBCARD_REC_ReadInfo));
		psInfo->psReadInfo = malloc(sizeof(sBASE_MW_SUBCARD_REC_ReadInfo));
		if(!psInfo->psReadInfo) {
			BMSR_Debug("error with malloc readinfo\n");
			return BASE_MW_SUBCARD_REC_MALLOC;
		}

		psReadInfo = psInfo->psReadInfo;
		memset(psReadInfo, 0, sizeof(sBASE_MW_SUBCARD_REC_ReadInfo));
		psReadInfo->lFrameThreshold = BASE_MW_SUBCARD_GetFrameCntFromTime(psInfo->lTimeThreshold);
		psReadInfo->lFrameCurCnt = 0;
		psReadInfo->eFrameType = BASE_MW_SUBCARD_REC_ISISLICE;
		psReadInfo->eOverLayType = BASE_MW_SUBCARD_OVERLAY_FORCE;
		psReadInfo->lRet = 0;
		psReadInfo->pcPathName = psInfo->cRecPath;
		psReadInfo->pcFileName = psInfo->cRecPath + strlen(psInfo->cRecPath);

		/* init v file */
		MALLOC(sizeof(sCOMM_MW_VFILE_Info));
		psReadInfo->psVfile = malloc(sizeof(sCOMM_MW_VFILE_Info));
		if(!psReadInfo->psVfile) {
			BMSR_Debug("error with malloc psVfile\n");
			return BASE_MW_SUBCARD_REC_MALLOC;
		}

		eVFileRet = eCOMM_MW_VFILE_Init(psReadInfo->psVfile, psReadInfo->pcPathName, 
			COMM_MW_VFILE_SETFLAG_CLEAR_BEFORE_CLOSE | COMM_MW_VFILE_SETFLAG_WITH_SYNC);
		if(eVFileRet != COMM_MW_VFILE_SUCC) {
			BMSR_Debug("init vfile error with <%d>\n", eVFileRet);
			return BASE_MW_SUBCARD_REC_VFILE;
		}
		
	}

	BMSR_FuncOut();

	return eRet;
}

/**************************************************************
 * func : eBASE_MW_SUBCARD_REC_Deinit(sBASE_MW_SUBCARD_REC_Info* psInfo)
 * arg : sBASE_MW_SUBCARD_REC_Info* psInfo
 * ret : eBASE_MW_SUBCARD_REC_Ret
 * note : deinit the struct of subcard rec
 *************************************************************/
eBASE_MW_SUBCARD_REC_Ret eBASE_MW_SUBCARD_REC_Deinit(sBASE_MW_SUBCARD_REC_Info* psInfo) {
	eBASE_MW_SUBCARD_REC_Ret eRet = BASE_MW_SUBCARD_REC_SUCCESS;
	eCOMM_MW_VFILE_Ret eVfileRet = COMM_MW_VFILE_SUCC;
	sBASE_MW_SUBCARD_REC_ReadInfo* psRdInfo = psInfo->psReadInfo;
	sCOMM_MW_VFILE_Info* psVfile = psRdInfo->psVfile;

	BMSR_FuncIn();

	{
		/* deinit vfile */
		eVfileRet = eCOMM_MW_VFILE_Deinit(psVfile);
		assert(eVfileRet == COMM_MW_VFILE_SUCC);

		/* free resource */
		free(psVfile);
		free(psRdInfo);
		if (NULL != psInfo->pcDirPath) {
			free(psInfo->pcDirPath);
			psInfo->pcDirPath = NULL;
		}
	}

	BMSR_FuncOut();

	return eRet;
}


/**************************************************************
 * func : eBASE_MW_SUBCARD_REC_Consume(sBASE_MW_SUBCARD_REC_Info* psInfo)
 * arg : sBASE_MW_SUBCARD_REC_Info* psInfo
 * ret : eBASE_MW_SUBCARD_REC_Ret
 * note : begin to consumer
 *************************************************************/
eBASE_MW_SUBCARD_REC_Ret eBASE_MW_SUBCARD_REC_Consume(sBASE_MW_SUBCARD_REC_Info* psInfo) {
	eBASE_MW_SUBCARD_REC_Ret eRet = BASE_MW_SUBCARD_REC_SUCCESS;
	eCOMM_POOL_Err ePoolRet = COMM_POOL_SUCCESS;
	eBASE_MW_SUBCARD_REC_Endflag eRecEndFlag = BASE_MW_SUBCARD_REC_ISNOTEND;
	eBASE_MW_SUBCARD_REC_CALLBACKRET eRecRet = BASE_MW_SUBCARD_REC_CBSUCCESS;
	sBASE_MW_SUBCARD_REC_ReadInfo* psRdInfo = psInfo->psReadInfo;
	sBASE_API_LEDCTL_Info* psLedCtlInfo = &gsLedCtlInfo;
	eBASE_API_LEDCTL_LedNewStatus eStatus = BASE_API_LEDCTL_NORMAL;
	
	BMSR_FuncIn();

	{
		/* clear the return val and reset current count */
		psRdInfo->lRet = 0;
		psRdInfo->lFrameCurCnt = 0;
		
		/* attach to the pool */
		ePoolRet = eCOMM_POOL_Attach(psInfo->lPoolId, &psInfo->lConsumerKey, "subcard");
		if(ePoolRet != COMM_POOL_SUCCESS) {
			return BASE_MW_SUBCARD_REC_POOL;
		}

		BMSR_Debug("subcard ready to read\n");
#ifdef SUBCARD_CHECK_DEBUG
		{
			eBASE_MW_FILESORT_RET eFileSortRet = BASE_MW_FILESORT_SUCCESS;
			sBASE_MW_SD_STAT* psSDStat = &gsNANDStat;
			eBASE_MW_SD_ERROR eSdRet = BASE_MW_SD_SUCCESS;
			// SD is not init, need to update the SD space parammeter
			BMSR_Debug("mutex lock enter !\n");
			pthread_mutex_lock(&psSDStat->sMutStatus);
			
			if (BASE_MW_SD_SUCCESS != psSDStat->lSdState ) 
			{
				/* wait the nand card at most 20s */
				int iThreshold = 0;
				do {
					printf("[ NAND FLASH ] : not ready, wait for %d sec ..\n", iThreshold);
					eSdRet = eBASE_SD_MW_GETSPACE(psSDStat, NANDFLASH, ".h264", 1);
					pthread_mutex_unlock(&psSDStat->sMutStatus);
					usleep(1000 * 1000);
					pthread_mutex_lock(&psSDStat->sMutStatus);
				} while(iThreshold++ < BASE_MW_SUBCARD_REC_ERRORCNTMAX
					&& eSdRet != BASE_MW_SD_SUCCESS);

				if(eSdRet != BASE_MW_SD_SUCCESS) {
					printf("[ NAND FLASH ] : there probably no nand flash at all\n");
					psSDStat->lSdState = BASE_MW_SD_INVALID;
					psSDStat->eVideoStat = BASE_MW_VIDEO_NOREC; 					
					BMSR_Debug("get space error <%d>\n", eSdRet);
					//pthread_mutex_unlock(&psSDStat->sMutStatus);	
					
					eStatus = BASE_API_LEDCTL_SDFAIL;
					eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);	
				} else {
					printf("[ NAND FLASH ] : find the nand flash !\n");
					psSDStat->lSdState = BASE_MW_SD_SUCCESS;
					//psSDStat->eVideoStat= BASE_MW_SUBCARD_REC;	
				}
			} 
			
			//Init	the file list of current audio directory
			if (BASE_MW_SD_SUCCESS == psSDStat->lSdState && BASE_MW_FILESORT_Valid != sFileSort.lState) {
				sFileSort.lFileMaxLen = NAND_MAX_FILE_NUM;
				eFileSortRet = eBASE_MW_FILESORT_Init(&sFileSort, sFileSort.lFileMaxLen, BASE_MW_SD_SUBCARDPATH);
				if (eFileSortRet == BASE_MW_FILESORT_SUCCESS) {
					printf("--- Subcard:eBASE_MW_FILESORT_Init() OK!---\n");
				} else {
					printf("--- Subcard:eBASE_MW_FILESORT_Init() Fail!---\n");
				}		
			}
			
			pthread_mutex_unlock(&psSDStat->sMutStatus);
		}
#else
		{
			LONG lCnt;
			CHAR cPath[COMM_SPACESAFER_MAX] = {0};			
			eCOMM_SPACESAFER_Ret eSubCardRet = COMM_SPACESAFER_SUCCESS;
			for(lCnt = 0;lCnt < TWO_FOLDER;lCnt++) {
				/* the nand card is init already */
				if(COMM_SPACESAFER_STATUS_VALID == gsTwoFolder[lCnt].eStatus) {
					continue;
				}
				
				if (0 == glFirSubCardInit) {
					/* wait the nand card at most 20s for first init */
					int iThreshold = 0;
					do {
						printf("[%s] : not ready, wait for %d sec ..\n",gcTargetFileInfo[lCnt], iThreshold);
						eSubCardRet = eCOMM_SPACESAFER_Init(&gsTwoFolder[lCnt], 
													gcTargetFileInfo[lCnt],
													gcMountInfo[lCnt], 2*COMM_NAND_FILE_SIZEKB , 
													psRdInfo->eOverLayType, 1.0);
						usleep(1000 * 1000);
					} while(iThreshold++ < BASE_MW_SUBCARD_REC_ERRORCNTMAX
						&& eSubCardRet != COMM_SPACESAFER_SUCCESS);	
				} else {
					/* no wait the nand card */
					eSubCardRet = eCOMM_SPACESAFER_Init(&gsTwoFolder[lCnt], 
													gcTargetFileInfo[lCnt],
													gcMountInfo[lCnt], 2*COMM_NAND_FILE_SIZEKB , 
													psRdInfo->eOverLayType, 1.0);				
				}	

				/* the nand card init fail */
				if(eSubCardRet != COMM_SPACESAFER_SUCCESS) {
					printf("[%s] : there probably no nand flash at all\n",gcTargetFileInfo[lCnt]);
					
					eStatus = BASE_API_LEDCTL_SDFAIL;
					eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);	
				} else {
					printf("[%s] : find the nand flash !\n",gcTargetFileInfo[lCnt]);
				}							
			}	
			glFirSubCardInit = 1;
			
			/* check if the two folders is empty */
			eSubCardRet = eCOMM_SPACESAFER_Compare(&gsTwoFolder[0],
				&gsTwoFolder[1], COMM_SPACESAFER_LASTEST, cPath, &glFolderIndex);	
			if(eSubCardRet == COMM_SPACESAFER_SUCCESS) {
				/* if not have this path, then build it first */
				if (strlen(cPath) > 0) {
					memset(psInfo->cRecPath, 0, sizeof(psInfo->cRecPath));
					strcpy(psInfo->cRecPath, cPath);
					if (NULL != psInfo->pcDirPath) {
						free(psInfo->pcDirPath);
						psInfo->pcDirPath = NULL;
						psInfo->pcDirPath = strdup(cPath);
					}
					if(access(psInfo->cRecPath, F_OK) != 0) {
						BMSR_Debug("the path %s is not exist, we will make it first\n", psInfo->cRecPath);
						mkdir(psInfo->cRecPath, 
							S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
					}				
				} else {
					/* the default path is current path */
					strcpy(psInfo->cRecPath, "./");
					psInfo->pcDirPath = strdup("./");			
				}		
				psRdInfo->pcPathName = psInfo->cRecPath;
				psRdInfo->pcFileName = psInfo->cRecPath + strlen(psInfo->cRecPath);			
				//ASSERT(eCOMM_SPACESAFER_QuerySpace(&gsTwoFolder[lFolderIndex]));	
			} 				
		}
#endif /* SUBCARD_CHECK_DEBUG */

		while(eRecEndFlag == BASE_MW_SUBCARD_REC_ISNOTEND) {
			ePoolRet = eCOMM_POOL_Read(psInfo->lPoolId, 
				vBASE_MW_SUBCARD_REC_Read, (void*) psRdInfo, psInfo->lConsumerKey);		
			if(ePoolRet != COMM_POOL_SUCCESS) {
				BMSR_Debug("read error\n");
				if(ePoolRet == COMM_POOL_NEEDTOQUIT) {
					/* indicate read all the data */
					BMSR_Debug("indicate read all the data");
					break;
				} else {
					eCOMM_POOL_Dettach(psInfo->lPoolId, psInfo->lConsumerKey);
					return BASE_MW_SUBCARD_REC_POOL;
				}
			}	
			eRecEndFlag = psRdInfo->lRet & BASE_MW_SUBCARD_REC_ENDMASK;
			eRecRet = psRdInfo->lRet & BASE_MW_SUBCARD_REC_CBRETMASK;
				
			if(eRecRet != BASE_MW_SUBCARD_REC_CBSUCCESS) {
				BMSR_Debug("Callback func error return with <%d>\n", eRecRet);
				
#ifdef SUBCARD_CHECK_DEBUG
				if (BASE_MW_SUBCARD_REC_CBTFCARD == eRecRet || 
								BASE_MW_SUBCARD_REC_CBFILESORT == eRecRet) 
				{
					pthread_mutex_lock(&psSDStat->sMutStatus);
					psSDStat->lSdState = BASE_MW_SD_INVALID;
					pthread_mutex_unlock(&psSDStat->sMutStatus);		
					
					printf("subCard vfile error: deinit the filesort resource!\n");
					eFileSortRet = eBASE_MW_FILESORT_DeInit(&sFileSort);
					if (eFileSortRet == BASE_MW_FILESORT_SUCCESS) {
						printf("--- Subcard:eBASE_MW_FILESORT_DeInit() OK!---\n");
					} else {
						printf("--- Subcard:eBASE_MW_FILESORT_DeInit() Fail!---\n");
					}					
				}
#else
				{
					LONG lCnt;
					for(lCnt = 0;lCnt < TWO_FOLDER;lCnt++) {
						eCOMM_SPACESAFER_DeInit(&gsTwoFolder[lCnt]);
					}	
				}
#endif /* SUBCARD_CHECK_DEBUG */

				eCOMM_POOL_Dettach(psInfo->lPoolId, psInfo->lConsumerKey);
				return BASE_MW_SUBCARD_REC_CALLBACK;
			}
		}		

		BMSR_Debug("subcard read over\n");

		/* detach from the pool */
		ePoolRet = eCOMM_POOL_Dettach(psInfo->lPoolId, psInfo->lConsumerKey);
		if(ePoolRet != COMM_POOL_SUCCESS) {
			return BASE_MW_SUBCARD_REC_POOL;
		}
	}

	BMSR_FuncOut();

	return eRet;
}

/**************************************************************
 * func : pvBASE_MW_SUBCARD_REC_Read(void* pvArg)
 * arg : void* pvArg
 * ret : void*
 * note : the callback function of subcard pool read, notice the first byte is channel info, should skip first
 *************************************************************/
void vBASE_MW_SUBCARD_REC_Read(void* pvArg) {
	eBASE_MW_SUBCARD_REC_Ret eRet = BASE_MW_SUBCARD_REC_SUCCESS;
	eCOMM_MW_VFILE_Ret eVfileRet = COMM_MW_VFILE_SUCC;
	sCOMM_POOL_CBFuncArg* psCBArg = (sCOMM_POOL_CBFuncArg*) pvArg;
	sBASE_MW_SUBCARD_REC_ReadInfo* psReadInfo = (sBASE_MW_SUBCARD_REC_ReadInfo*) psCBArg->pvArg;
	eBASE_MW_SUBCARD_REC_Frametype* peFrameType = &(psReadInfo->eFrameType);
	sCOMM_MW_VFILE_Info* psVfile = psReadInfo->psVfile;
	sBASE_API_LEDCTL_Info* psLedCtlInfo = &gsLedCtlInfo;
	eBASE_API_LEDCTL_LedNewStatus eStatus = BASE_API_LEDCTL_NORMAL;
	
	BMSR_FuncIn();

	{
		/* first of all, check the valid */
#ifdef USE_720P
		assert(*psCBArg->pucHeader == 1);
#else
		assert(*psCBArg->pucHeader == 0);
#endif
		psCBArg->pucHeader ++;
		psCBArg->lLen --;
		
		/* prepare return value */
		psReadInfo->lRet &= (~BASE_MW_SUBCARD_REC_CBRETMASK);
		
		{
#ifdef SUBCARD_CHECK_DEBUG
			CHAR cFileName1[128];
			sBASE_MW_SD_STAT* psSDStat = &gsNANDStat;
			eBASE_MW_SD_ERROR eSdRet = BASE_MW_SD_SUCCESS;
			eBASE_MW_FILESORT_RET eFileSortRet = BASE_MW_FILESORT_SUCCESS;
			// SD is not init, need to update the SD space parammeter
			BMSR_Debug("mutex lock enter !\n");
			pthread_mutex_lock(&psSDStat->sMutStatus);
			
			if (BASE_MW_SD_SUCCESS != psSDStat->lSdState ) 
			{
				eSdRet = eBASE_SD_MW_GETSPACE(psSDStat, NANDFLASH, ".h264", 1);
				if(eSdRet != BASE_MW_SD_SUCCESS) {
					psSDStat->lSdState = BASE_MW_SD_INVALID;
					psSDStat->eVideoStat = BASE_MW_VIDEO_NOREC;						
					//printf("get space error <%d>\n", eSdRet);
					pthread_mutex_unlock(&psSDStat->sMutStatus);	
					
					eStatus = BASE_API_LEDCTL_SDFAIL;
					eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);	
					psReadInfo->lRet |= BASE_MW_SUBCARD_REC_CBTFCARD;
					
					return;
				} else {
					psSDStat->lSdState = BASE_MW_SD_SUCCESS;
					//psSDStat->eVideoStat= BASE_MW_SUBCARD_REC;	
				}
			} 
			
			//Init  the file list of current audio directory
			if (BASE_MW_SD_SUCCESS == psSDStat->lSdState && BASE_MW_FILESORT_Valid != sFileSort.lState) {
				sFileSort.lFileMaxLen = NAND_MAX_FILE_NUM;
				CHAR cDestDir[128];
				strcpy(cDestDir, psReadInfo->pcPathName);
				eFileSortRet = eBASE_MW_FILESORT_Init(&sFileSort, sFileSort.lFileMaxLen, dirname(cDestDir));
				if (eFileSortRet == BASE_MW_FILESORT_SUCCESS) {
					//printf("--- Subcard:eBASE_MW_FILESORT_Init() OK!---\n");
				} else {
					//printf("--- Subcard:eBASE_MW_FILESORT_Init() Fail!---\n");
					pthread_mutex_unlock(&psSDStat->sMutStatus);
					psReadInfo->lRet |= BASE_MW_SUBCARD_REC_CBFILESORT;
					
					return;
				}		
			}
			
			pthread_mutex_unlock(&psSDStat->sMutStatus);
#else			
#endif /* SUBCARD_CHECK_DEBUG */
		}		
		
		eRet = eBASE_MW_SUBCARD_REC_GetFrameType(peFrameType, psCBArg->pucHeader, psCBArg->lLen);
		if(eRet != BASE_MW_SUBCARD_REC_SUCCESS) {
			psReadInfo->lRet |= BASE_MW_SUBCARD_REC_CBGETFRAMETYPE;
			return;
		}

		/* get the first I slice */
		if(!psReadInfo->lFrameCurCnt) {
			if(*peFrameType != BASE_MW_SUBCARD_REC_ISISLICE) {
				return;
			} else {
#ifdef SUBCARD_CHECK_DEBUG
				/* check whether enough space */
				pthread_mutex_lock(&psSDStat->sMutStatus);

				{
					eSdRet = eBASE_SD_MW_GETSPACE(psSDStat, NANDFLASH, ".h264", 1);
					if(eSdRet != BASE_MW_SD_SUCCESS) {
						psSDStat->lSdState = BASE_MW_SD_INVALID;
						psSDStat->eVideoStat= BASE_MW_VIDEO_NOREC;						
						//printf("get space error <%d>\n", eSdRet);
						pthread_mutex_unlock(&psSDStat->sMutStatus);
						
						eStatus = BASE_API_LEDCTL_SDFAIL;
						eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);	
						psReadInfo->lRet |= BASE_MW_SUBCARD_REC_CBTFCARD;
						
						return;
					} else {
						psSDStat->lSdState = BASE_MW_SD_SUCCESS;				
					}
				}
				//printf("--- Update the Space:FreeKB = [%ld]---\n",psSDStat->lFreeKB);
				
				while((psSDStat->lFreeKB <= BASE_MW_NAND_FILE_SIZEKB 
					&& psReadInfo->eOverLayType == BASE_MW_SUBCARD_OVERLAY_FORCE) 
						|| (sFileSort.lFileLen >= NAND_MAX_FILE_NUM)) 
				{
					/* remove the oldest file */
					if (sFileSort.lFileLen == 0) {
						pthread_mutex_unlock(&psSDStat->sMutStatus);
						printf("--- FileSort is empty -- \n");
						psReadInfo->lRet |= BASE_MW_SUBCARD_REC_CBFILESORT;
						
						return;				
					}
					memset(cFileName1,0,sizeof(cFileName1));
					eFileSortRet = eBASE_MW_FILESORT_Read(&sFileSort,cFileName1);
					if(eFileSortRet != BASE_MW_FILESORT_SUCCESS) {
						//printf("--- eBASE_MW_FILESORT_Read() %s Failed -- \n",cFileName1);
						pthread_mutex_unlock(&psSDStat->sMutStatus);
						psReadInfo->lRet |= BASE_MW_SUBCARD_REC_CBFILESORT;
						
						return;
					}
					//printf("Current the FileNum : %ld!\n", sFileSort.lFileLen);
					
					if(access(cFileName1,F_OK | R_OK | W_OK))
					{
						perror("Check file:");
						if (BASE_MW_SD_SUCCESS == psSDStat->lSdState) {
							//printf("Can't find the FileName : %s \n", cFileName1);
							continue;								
						} else {
							pthread_mutex_unlock(&psSDStat->sMutStatus);
							psReadInfo->lRet |= BASE_MW_SUBCARD_REC_CBTFCARD;
							
							return;							
						}						
					}	
					eFileSortRet = remove(cFileName1);
				  	if(eFileSortRet)
				  	{
						BMSR_Debug("Remove the FileName : %s failed!\n", cFileName1);
						//perror("Error to remove file");
						if (BASE_MW_SD_SUCCESS == psSDStat->lSdState) {
							//printf("Can't find the FileName : %s \n", cFileName1);
							continue;								
						} else {
							pthread_mutex_unlock(&psSDStat->sMutStatus);
							psReadInfo->lRet |= BASE_MW_SUBCARD_REC_CBTFCARD;
							
							return;							
						}
				  	}
					//printf("Remove the FileName : %s successful!\n", cFileName1);
					
					/* check the space again */
					eSdRet = eBASE_SD_MW_CHECKSTATUS(psSDStat);
					if(eSdRet != BASE_MW_SD_SUCCESS) {
						psSDStat->lSdState = BASE_MW_SD_INVALID;
						psSDStat->eVideoStat= BASE_MW_VIDEO_NOREC;						
						//printf("get space error <%d>\n", eSdRet);
						pthread_mutex_unlock(&psSDStat->sMutStatus);
						
						eStatus = BASE_API_LEDCTL_SDFAIL;
						eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);		
						psReadInfo->lRet |= BASE_MW_SUBCARD_REC_CBTFCARD;
						
						return;
					} else {
						psSDStat->lSdState = BASE_MW_SD_SUCCESS;				
					}				
				}
				
				pthread_mutex_unlock(&psSDStat->sMutStatus);	
#else			
				/* check which folders have space */
				CHAR cPath[COMM_SPACESAFER_MAX] = {0};	
				eCOMM_SPACESAFER_Ret eSubCardRet = COMM_SPACESAFER_SUCCESS;
				/* check current folders whether have space */
				eSubCardRet = eCOMM_SPACESAFER_CheckSpace(
					&gsTwoFolder[glFolderIndex],2*COMM_NAND_FILE_SIZEKB);
				if (eSubCardRet != COMM_SPACESAFER_SUCCESS) {
					//printf("[%s] have not enough space\n",gsTwoFolder[glFolderIndex].cTargetPath);
					eSubCardRet = eCOMM_SPACESAFER_Compare(&gsTwoFolder[0],
						&gsTwoFolder[1], COMM_SPACESAFER_OLDEST, cPath, &glFolderIndex);	
					if(eSubCardRet == COMM_SPACESAFER_SUCCESS) {
						/* if not have this path, then build it first */
						if (strlen(cPath) > 0) {
							psReadInfo->pcPathName = cPath;
							psReadInfo->pcFileName = cPath + strlen(cPath);			
						} else {
							/* the default path is current path */	
							strcpy(cPath, "./");
							psReadInfo->pcPathName = cPath;
							psReadInfo->pcFileName = cPath + strlen(cPath);							
						}					
					} else {
						//printf("[%s and %s] are not normal\n", gsTwoFolder[0].cTargetPath,gsTwoFolder[1].cTargetPath);
						eStatus = BASE_API_LEDCTL_SDFAIL;
						eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);		
						psReadInfo->lRet |= BASE_MW_SUBCARD_REC_CBTFCARD;
						
						return;				
					}
				}				
				
				eSubCardRet = eCOMM_SPACESAFER_QuerySpace(&gsTwoFolder[glFolderIndex]);
				if(COMM_SPACESAFER_SUCCESS != eSubCardRet) {
					//printf("[%s] is not normal\n", gsTwoFolder[glFolderIndex].cTargetPath);
					eStatus = BASE_API_LEDCTL_SDFAIL;
					eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);		
					psReadInfo->lRet |= BASE_MW_SUBCARD_REC_CBTFCARD;
					
					return;
				}
#endif /* SUBCARD_CHECK_DEBUG */
				/* now, we find first I Slice, just open the file and begin to rec */
				memset(psReadInfo->pcFileName, 0, strlen(psReadInfo->pcFileName) + 1);
				eCOMM_UTIL_GetFileName(psReadInfo->pcFileName, BASE_MW_SUBCARD_MaxFileName, ".h264");
				
				eVfileRet = eCOMM_MW_VFILE_Open(psVfile, psReadInfo->pcPathName);
				if(eVfileRet != COMM_MW_VFILE_SUCC) {
					psReadInfo->lRet |= BASE_MW_SUBCARD_REC_CBVFILE;
					return;
				}

#ifdef SUBCARD_CHECK_DEBUG
				eFileSortRet = eBASE_MW_FILESORT_Write(&sFileSort,psReadInfo->pcPathName);
				if (eFileSortRet != BASE_MW_FILESORT_SUCCESS) {
					//printf("--- eBASE_MW_FILESORT_Write() %s Failed -- \n",psReadInfo->pcPathName);
					psReadInfo->lRet |= BASE_MW_SUBCARD_REC_CBFILESORT;
					
					return;
				} 		
				printf("Add the FileName : %s successful!\n", psReadInfo->pcPathName);
#else
				eSubCardRet = eCOMM_SPACESAFER_RegisterFile(&gsTwoFolder[glFolderIndex], psReadInfo->pcPathName);
				if(COMM_SPACESAFER_SUCCESS != eSubCardRet) {
					//printf("register file error!\n");
					psReadInfo->lRet |= BASE_MW_SUBCARD_REC_CBFILESORT;
					
					return;					
				}
#endif /* SUBCARD_CHECK_DEBUG */
			}
		}

		/* now we can handle the frame */
		if((psReadInfo->lFrameCurCnt++ < psReadInfo->lFrameThreshold) || (*peFrameType != BASE_MW_SUBCARD_REC_ISISLICE)) {
			eVfileRet = eCOMM_MW_VFILE_Write(psVfile, psCBArg->pucHeader, psCBArg->lLen);
			if(eVfileRet != COMM_MW_VFILE_SUCC) {
				psReadInfo->lRet |= BASE_MW_SUBCARD_REC_CBVFILE;
				eStatus = BASE_API_LEDCTL_SDFAIL;
				eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);				
				return;
			}
			
		} else {
			/* should terminate this record */
			eVfileRet = eCOMM_MW_VFILE_Close(psVfile);
			if(eVfileRet != COMM_MW_VFILE_SUCC) {
				psReadInfo->lRet |= BASE_MW_SUBCARD_REC_CBVFILE;
				return;
			}
			
			psReadInfo->lRet |= BASE_MW_SUBCARD_REC_ISEND;
			return;
		}
	}

	BMSR_FuncOut();

	return;
}

/**************************************************************
 * func : eBASE_MW_SUBCARD_REC_GetFrameType(eBASE_MW_SUBCARD_REC_Frametype* peType, UCHAR* pucHeader, LONG lLen)
 * arg : eBASE_MW_SUBCARD_REC_Frametype* peType, UCHAR* pucHeader, LONG lLen
 * ret : eBASE_MW_SUBCARD_REC_Ret
 * note : get frame type, first skip prefix, then judge whether p
 *************************************************************/
eBASE_MW_SUBCARD_REC_Ret eBASE_MW_SUBCARD_REC_GetFrameType(eBASE_MW_SUBCARD_REC_Frametype* peType, UCHAR* pucHeader, LONG lLen) {
	eBASE_MW_SUBCARD_REC_Ret eRet = BASE_MW_SUBCARD_REC_SUCCESS;
	*peType = BASE_MW_SUBCARD_REC_NOTISLICE;
	UCHAR* pucTmp = pucHeader;
	LONG lPrefixCnt = 0;

	BMSR_FuncIn();

	{
		/* skip prefix 00 */
		while(*pucTmp == 0) {
			pucTmp ++;
			lPrefixCnt ++;
		}

		/* check valid */
		assert(*pucTmp == 1);
		assert(lPrefixCnt == 3 || lPrefixCnt ==2);

		/* skip 01 */
		pucTmp ++;

		/* judge */
		if((*pucTmp & BASE_MW_SUBCARD_NALTYPEMASK) != BASE_MW_SUBCARD_NALTYPE_PSLICE) {
			BMSR_Debug("found I Slice\n");
			*peType = BASE_MW_SUBCARD_REC_ISISLICE;
		}
	}

	BMSR_FuncOut();

	return eRet;
}

/**************************************************************
 * func : eBASE_MW_SUBCARD_REC_UpdateRecDuration(sBASE_MW_SUBCARD_REC_Info* psInfo, LONG lNewTimeTh)
 * arg : sBASE_MW_SUBCARD_REC_Info* psInfo, LONG lNewTimeTh
 * ret : eBASE_MW_SUBCARD_REC_Ret
 * note : update the record duration
 *************************************************************/
eBASE_MW_SUBCARD_REC_Ret eBASE_MW_SUBCARD_REC_UpdateRecDuration(sBASE_MW_SUBCARD_REC_Info* psInfo, LONG lNewTimeTh) {
	eBASE_MW_SUBCARD_REC_Ret eRet = BASE_MW_SUBCARD_REC_SUCCESS;

	BMSR_FuncIn();

	{
		BMSR_Debug("update time th : < %ld > -> < %ld >\n", psInfo->lTimeThreshold, lNewTimeTh);
		psInfo->lTimeThreshold = lNewTimeTh;
		psInfo->psReadInfo->lFrameThreshold = BASE_MW_SUBCARD_GetFrameCntFromTime(lNewTimeTh);
	}
	
	BMSR_FuncOut();

	return eRet;
}

/**************************************************************
 * func : eBASE_MW_SUBCARD_REC_UpdateOverlayType(sBASE_MW_SUBCARD_REC_Info* psInfo, eBASE_MW_SUBCARD_OVERLAY_Type eNewOverLayType)
 * arg : sBASE_MW_SUBCARD_REC_Info* psInfo, eBASE_MW_SUBCARD_OVERLAY_Type eNewOverLayType
 * ret : eBASE_MW_SUBCARD_REC_Ret
 * note : update the overlay type
 *************************************************************/
eBASE_MW_SUBCARD_REC_Ret eBASE_MW_SUBCARD_REC_UpdateOverlayType(sBASE_MW_SUBCARD_REC_Info* psInfo, eBASE_MW_SUBCARD_OVERLAY_Type eNewOverLayType) {
	eBASE_MW_SUBCARD_REC_Ret eRet = BASE_MW_SUBCARD_REC_SUCCESS;

	BMSR_FuncIn();

	{
		BMSR_Debug("update overlay type : < %d > -> < %d >\n", psInfo->psReadInfo->eOverLayType, eNewOverLayType);
		psInfo->psReadInfo->eOverLayType = eNewOverLayType;
	}
	
	BMSR_FuncOut();

	return eRet;
}

/**************************************************************
 * func : eBASE_MW_SUBCARD_REC_GetOverlayType(sBASE_MW_SUBCARD_REC_Info* psInfo, eBASE_MW_SUBCARD_OVERLAY_Type* peOverlayType)
 * arg : sBASE_MW_SUBCARD_REC_Info* psInfo, eBASE_MW_SUBCARD_OVERLAY_Type* peOverlayType
 * ret : eBASE_MW_SUBCARD_REC_Ret
 * note : update the overlay type
 *************************************************************/
eBASE_MW_SUBCARD_REC_Ret eBASE_MW_SUBCARD_REC_GetOverlayType(sBASE_MW_SUBCARD_REC_Info* psInfo, eBASE_MW_SUBCARD_OVERLAY_Type* peOverlayType) {
	eBASE_MW_SUBCARD_REC_Ret eRet = BASE_MW_SUBCARD_REC_SUCCESS;

	BMSR_FuncIn();

	{
		BMSR_Debug("overlay type : < %d >\n", psInfo->psReadInfo->eOverLayType);
		*peOverlayType = psInfo->psReadInfo->eOverLayType;
	}
	
	BMSR_FuncOut();

	return eRet;
}

#ifdef __cplusplus
}
#endif

