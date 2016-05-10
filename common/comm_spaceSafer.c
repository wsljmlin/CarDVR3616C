#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "comm_spaceSafer.h"

/***************************************************
	macro / enum
***************************************************/
//#define COMM_SPACESAFER_DEBUG
#ifdef COMM_SPACESAFER_DEBUG
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
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_GetCurrentFileSize(sCOMM_SPACESAFER_Info* psInfo, LONG* plFileSize);
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_DeleteOlderFile(sCOMM_SPACESAFER_Info* psInfo, CHAR* pcOlderFileName);
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_Create(sCOMM_SPACESAFER_Info* psInfo);
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_Reset(sCOMM_SPACESAFER_Info* psInfo);
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_Restart(sCOMM_SPACESAFER_Info* psInfo);
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_UnRegisterFile(sCOMM_SPACESAFER_Info* psInfo, CHAR* pcFileName);
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_QueryCheck(sCOMM_SPACESAFER_Info* psInfo,
																sCOMM_SPACESAFER_SpaceStatus* psSpaceStatus);
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_QueryHealthy(sCOMM_SPACESAFER_Info* psInfo);
/***************************************************
	function
***************************************************/

/*********************************************
* func : eCOMM_SPACESAFER_GetCurrentFileSize(sCOMM_SPACESAFER_Info* psInfo, LONG* plFileSize)
* arg  : sCOMM_SPACESAFER_Info* psInfo, LONG* plFileSize
* ret  : eCOMM_SPACESAFER_Ret
* note :
*********************************************/
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_GetCurrentFileSize(sCOMM_SPACESAFER_Info* psInfo, LONG* plFileSize) {
	eCOMM_SPACESAFER_Ret eRet = COMM_SPACESAFER_SUCCESS;
	sCOMM_FILESORT_Info* psFIleSortInfo = psInfo->psFileSortInfo;
	CHAR cCurrentFileName[COMM_SPACESAFER_MAX];
		
	CS_FuncIn();

	{
		/* init buffer */
		memset(cCurrentFileName, 0, sizeof(cCurrentFileName));
		/* get current file name */
		eRet = eCOMM_FILESORT_Read(psFIleSortInfo, COMM_LIST_RETRIEVEKEY_LAST, cCurrentFileName);
		if (eRet) {
			CS_Debug("find the current file failed\n");
			return eRet;
		}

		eRet = eCOMM_FILESYSTEM_GetFileSize(cCurrentFileName, plFileSize);
		if (eRet) {
			CS_Debug("get the current file size failed\n");
			return eRet;
		}		
	}

	CS_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_SPACESAFER_DeleteOlderFile(sCOMM_SPACESAFER_Info* psInfo, CHAR* pcOlderFileName)
* arg  : sCOMM_SPACESAFER_Info* psInfo, CHAR* pcOlderFileName
* ret  : eCOMM_SPACESAFER_Ret
* note :
*********************************************/
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_DeleteOlderFile(sCOMM_SPACESAFER_Info* psInfo, CHAR* pcOlderFileName) {
	eCOMM_SPACESAFER_Ret eRet = COMM_SPACESAFER_SUCCESS;
	sCOMM_FILESORT_Info* psFIleSortInfo = psInfo->psFileSortInfo;

		
	CS_FuncIn();

	{
		eRet = eCOMM_FILESORT_Read(psFIleSortInfo, COMM_LIST_RETRIEVEKEY_FIRST, pcOlderFileName);
		if (eRet) {
			CS_Debug("find the older file failed\n");
			return eRet;
		}
		
		eRet = eCOMM_FILESYSTEM_DeleteFile(pcOlderFileName);
		if (eRet) {
			CS_Debug("delete the older file failed\n");
			return eRet;
		}
		//CS_Debug("delete file name :%s\n",pcOlderFileName);
		printf("\n");		
	}

	CS_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_SPACESAFER_UnRegisterFile(sCOMM_SPACESAFER_Info* psInfo, CHAR* pcFileName)
* arg  : sCOMM_SPACESAFER_Info* psInfo, CHAR* pcFileName
* ret  : eCOMM_SPACESAFER_Ret
* note :
*********************************************/
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_UnRegisterFile(sCOMM_SPACESAFER_Info* psInfo, CHAR* pcFileName) {
	eCOMM_SPACESAFER_Ret eRet = COMM_SPACESAFER_SUCCESS;
	sCOMM_FILESORT_Info* psFIleSortInfo = psInfo->psFileSortInfo;
	
	CS_FuncIn();

	{
		eRet = eCOMM_FILESORT_Delete(psFIleSortInfo, pcFileName);
		if (COMM_SPACESAFER_SUCCESS != eRet) {
			CS_Debug("unregister file failed\n");
		}		
	}

	CS_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_SPACESAFER_Create(sCOMM_SPACESAFER_Info* psInfo)
* arg  : sCOMM_SPACESAFER_Info* psInfo
* ret  : eCOMM_SPACESAFER_Ret
* note :
*********************************************/
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_Create(sCOMM_SPACESAFER_Info* psInfo) {
	eCOMM_SPACESAFER_Ret eRet = COMM_SPACESAFER_SUCCESS;
	sCOMM_FILESORT_Info* psFileSortInfo = psInfo->psFileSortInfo;
	sCOMM_FILESYSTEM_Info* psFileSystemInfo = psInfo->psFileSystemInfo;
	
	CS_FuncIn();

	{
		eRet = eCOMM_FILESYSTEM_GetPathInfo(psInfo->cTargetPath, psFileSystemInfo);
		if (COMM_SPACESAFER_SUCCESS != eRet) {
			CS_Debug("get path info error\n");
			return COMM_SPACESAFER_CREATE;			
		}
		strcpy(psInfo->cMakeUp, psFileSystemInfo->cMakeUp);
		
		eRet = eCOMM_FILESORT_Creat(psFileSortInfo, psInfo->cTargetPath);
		if (COMM_SPACESAFER_SUCCESS != eRet) {
			CS_Debug("path list creat error\n");
			return COMM_SPACESAFER_CREATE;			
		}

		psInfo->eStatus = COMM_SPACESAFER_STATUS_VALID;
	}

	CS_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_SPACESAFER_Reset(sCOMM_SPACESAFER_Info* psInfo)
* arg  : sCOMM_SPACESAFER_Info* psInfo
* ret  : eCOMM_SPACESAFER_Ret
* note :
*********************************************/
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_Reset(sCOMM_SPACESAFER_Info* psInfo) {
	eCOMM_SPACESAFER_Ret eRet = COMM_SPACESAFER_SUCCESS;
	sCOMM_FILESORT_Info* psFIleSortInfo = psInfo->psFileSortInfo;
	
	CS_FuncIn();

	{
		eRet = eCOMM_FILESORT_Reset(psFIleSortInfo);
		if (COMM_SPACESAFER_SUCCESS != eRet) {
			CS_Debug("space reset failed\n");
		}		

		memset(psInfo->cMakeUp, 0, sizeof(psInfo->cMakeUp));

		psInfo->eStatus = COMM_SPACESAFER_STATUS_INVALID;
	}

	CS_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_SPACESAFER_Restart(sCOMM_SPACESAFER_Info* psInfo)
* arg  : sCOMM_SPACESAFER_Info* psInfo
* ret  : eCOMM_SPACESAFER_Ret
* note :
*********************************************/
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_Restart(sCOMM_SPACESAFER_Info* psInfo) {
	eCOMM_SPACESAFER_Ret eRet = COMM_SPACESAFER_SUCCESS;

	CS_FuncIn();

	{
		eRet = eCOMM_SPACESAFER_Reset(psInfo);
		if (eRet) {
			CS_Debug("space reset failed\n");
			return eRet;
		}
		
		eRet  = eCOMM_SPACESAFER_Create(psInfo);
		if (eRet) {
			CS_Debug("space create failed\n");	
			return eRet;
		}
	}

	CS_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_SPACESAFER_DeInit(sCOMM_SPACESAFER_Info* psInfo)
* arg  : sCOMM_SPACESAFER_Info* psInfo
* ret  : eCOMM_SPACESAFER_Ret
* note :
*********************************************/
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_DeInit(sCOMM_SPACESAFER_Info* psInfo) {
	eCOMM_SPACESAFER_Ret eRet = COMM_SPACESAFER_SUCCESS;

	CS_FuncIn();

	{
		if (NULL == psInfo) {
			return eRet;
		} else {
			eCOMM_FILESORT_DeInit(psInfo->psFileSortInfo);
			/* free resource */
			if(psInfo->psFileSortInfo) {
				free(psInfo->psFileSortInfo);
				psInfo->psFileSortInfo = NULL;
			}
			
			if(psInfo->psFileSystemInfo) {
				free(psInfo->psFileSystemInfo);		
				psInfo->psFileSystemInfo = NULL;
			}	

			psInfo->eStatus = COMM_SPACESAFER_STATUS_INVALID;		
		}
	}

	CS_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_SPACESAFER_Init(sCOMM_SPACESAFER_Info* psInfo, CHAR* pcTargetPath, CHAR* pcMountPath, LONG lFileSizeTHreshold, LONG lIsForceOverlay)
* arg  : sCOMM_SPACESAFER_Info* psInfo, CHAR* pcTargetPath, CHAR* pcMountPath, LONG lFileSizeTHreshold, LONG lIsForceOverlay
* ret  : eCOMM_SPACESAFER_Ret
* note :
*********************************************/
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_Init(sCOMM_SPACESAFER_Info* psInfo, 
														CHAR * pcTargetPath, 
														CHAR* pcMountPath, 
														LONG lFileSizeTHreshold, 
														LONG lIsForceOverlay,
														double dRatio) 
{
	eCOMM_SPACESAFER_Ret eRet = COMM_SPACESAFER_SUCCESS;

	CS_FuncIn();

	{
		/* init parameter */
		memset(psInfo, 0, sizeof(sCOMM_SPACESAFER_Info));
		psInfo->eStatus = COMM_SPACESAFER_STATUS_INVALID;
		psInfo->lFileSizeThreshold = lFileSizeTHreshold;
		psInfo->lIsForceOverlay = lIsForceOverlay;
		
		strcpy(psInfo->cTargetPath, pcTargetPath);
		strcpy(psInfo->cMountPath, pcMountPath);
		getcwd(psInfo->cCurrentPath,sizeof(psInfo->cCurrentPath));
		//CS_Debug("Current Dir:%s\n",psInfo->cCurrentPath);	
		CS_Debug("TargetPath:%s\n",psInfo->cTargetPath);	
		CS_Debug("MountPath:%s\n",psInfo->cMountPath);

		psInfo->psFileSortInfo = malloc(sizeof(sCOMM_FILESORT_Info));
		if(!psInfo->psFileSortInfo) {
			CS_Debug("malloc error\n");
			return COMM_SPACESAFER_MALLOC;
		}	

		psInfo->psFileSystemInfo = malloc(sizeof(sCOMM_FILESYSTEM_Info));
		if(!psInfo->psFileSystemInfo) {
			CS_Debug("malloc error\n");
			return COMM_SPACESAFER_MALLOC;
		}	
		psInfo->psFileSystemInfo->dRatio = dRatio;

		eCOMM_FILESORT_Init(psInfo->psFileSortInfo);	
		eRet = eCOMM_SPACESAFER_Create(psInfo);
		if (COMM_SPACESAFER_SUCCESS != eRet) {
			CS_Debug("eCOMM_SPACESAFER_Create error\n");
			return COMM_SPACESAFER_CREATE;			
		}
		
#ifdef COMM_SPACESAFER_DEBUG
		LONG lFileLen = 0;	
		sCOMM_FILESORT_Info* psFileSortInfo = psInfo->psFileSortInfo;
		eCOMM_FILESORT_GetCount(psFileSortInfo,&lFileLen);
		CS_Debug("lFileLen=%ld\n",lFileLen);	
#endif /* COMM_SPACESAFER_DEBUG */	
	}

	CS_FuncOut();

	return eRet;
}

eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_QueryCheck(sCOMM_SPACESAFER_Info* psInfo,
																sCOMM_SPACESAFER_SpaceStatus* psSpaceStatus)
{
	eCOMM_SPACESAFER_Ret eRet = COMM_SPACESAFER_SUCCESS;
	sCOMM_FILESYSTEM_Info* psFileSystemInfo = psInfo->psFileSystemInfo;
	LONG lFileSizeThreshold = psInfo->lFileSizeThreshold;
	
	CS_FuncIn();

	{
		if (psInfo->eStatus == COMM_SPACESAFER_STATUS_INVALID) {
			eRet = eCOMM_SPACESAFER_Restart(psInfo);
			if (eRet) {
				CS_Debug("space restart failed\n");	
				return eRet;
			}		
			
			printf("************<eCOMM_SPACESAFER_Restart>**********\n");			
		}		
		
		eRet = eCOMM_FILESYSTEM_JudgeMakeUp(psInfo->cMakeUp);
		if (eRet)  {
			eRet = eCOMM_SPACESAFER_Restart(psInfo);
			if (eRet) {
				CS_Debug("space restart failed\n");
				return eRet;
			}		
		}	

		eRet = eCOMM_FILESYSTEM_GetFreeSpaceSize(psInfo->cTargetPath,&(psFileSystemInfo->lAvailableSize), &(psFileSystemInfo->lFreeSize));
		if (eRet) {	
			return eRet;
		}	
			
		if (psFileSystemInfo->lFreeSize <= lFileSizeThreshold)
		{
			*psSpaceStatus = COMM_SPACESAFER_NOTHAVE_SPACE;
		} else {
			*psSpaceStatus = COMM_SPACESAFER_HAVE_SPACE;
		}
	}

	CS_FuncOut();

	return eRet;
}

eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_CheckSpace(sCOMM_SPACESAFER_Info* psInfo,LONG lFileSizeTHreshold)
{
	eCOMM_SPACESAFER_Ret eRet = COMM_SPACESAFER_SUCCESS;
	sCOMM_FILESYSTEM_Info* psFileSystemInfo = psInfo->psFileSystemInfo;
	//LONG lFileSizeThreshold = psInfo->lFileSizeThreshold;
	
	CS_FuncIn();

	{
		if (psInfo->eStatus == COMM_SPACESAFER_STATUS_INVALID) {
			eRet = eCOMM_SPACESAFER_Restart(psInfo);
			if (eRet) {
				CS_Debug("space restart failed\n");	
				return eRet;
			}
			CS_Debug("<eCOMM_SPACESAFER_Restart>\n");
		}		
		
		eRet = eCOMM_FILESYSTEM_JudgeMakeUp(psInfo->cMakeUp);
		if (eRet)  {
			eRet = eCOMM_SPACESAFER_Restart(psInfo);
			if (eRet) {
				CS_Debug("space restart failed\n");
				return eRet;
			}		
		}	

		eRet = eCOMM_FILESYSTEM_GetFreeSpaceSize(psInfo->cTargetPath,&(psFileSystemInfo->lAvailableSize), &(psFileSystemInfo->lFreeSize));
		if (eRet) {	
			return eRet;
		}	
			
		if (psFileSystemInfo->lFreeSize < lFileSizeTHreshold)
		{
			eRet = COMM_SPACESAFER_NOSPACE;
		} 
	}

	CS_FuncOut();

	return eRet;
}
/*********************************************
* func : eCOMM_SPACESAFER_QuerySpace(sCOMM_SPACESAFER_Info* psInfo)
* arg  : sCOMM_SPACESAFER_Info* psInfo
* ret  : eCOMM_SPACESAFER_Ret
* note :
*********************************************/
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_QuerySpace(sCOMM_SPACESAFER_Info* psInfo) {
	eCOMM_SPACESAFER_Ret eRet = COMM_SPACESAFER_SUCCESS;
	sCOMM_FILESORT_Info* psFileSortInfo = psInfo->psFileSortInfo;
	sCOMM_FILESYSTEM_Info* psFileSystemInfo = psInfo->psFileSystemInfo;
	LONG lFileSizeThreshold = psInfo->lFileSizeThreshold;
	LONG lIsForceOverlay = psInfo->lIsForceOverlay;
	CHAR cOldFileName[COMM_SPACESAFER_MAX];
	LONG lFileListLen = 0;
	
	CS_FuncIn();

	{
		if (psInfo->eStatus == COMM_SPACESAFER_STATUS_INVALID) {
			eRet = eCOMM_SPACESAFER_Restart(psInfo);
			if (eRet) {
				CS_Debug("space restart failed\n");	
				return eRet;
			}		
			
			printf("************<eCOMM_SPACESAFER_Restart>**********\n");			
		}		
		
		eRet = eCOMM_FILESYSTEM_JudgeMakeUp(psInfo->cMakeUp);
		if (eRet)  {
			eRet = eCOMM_SPACESAFER_Restart(psInfo);
			if (eRet) {
				CS_Debug("space restart failed\n");	
				return eRet;
			}		
		}	

		eRet = eCOMM_FILESYSTEM_GetFreeSpaceSize(psInfo->cTargetPath,&(psFileSystemInfo->lAvailableSize), &(psFileSystemInfo->lFreeSize));
		if (eRet) {	
			return eRet;
		}	
			
		while(psFileSystemInfo->lFreeSize <= lFileSizeThreshold)
		{
			if (lIsForceOverlay == eCOMM_SPACESAFER_Overlay) {
				/* judge whether the file list is empty */
				eCOMM_FILESORT_GetCount(psFileSortInfo, &lFileListLen);
				if (0 == lFileListLen) {
					printf("************<file list is empty>**********\n");
					eRet = eCOMM_SPACESAFER_Restart(psInfo);
					if (eRet) {
						CS_Debug("space restart failed\n");	
						return eRet;
					}					
					//return COMM_SPACESAFER_NOFILE;
				}
				
				/* remove the oldest file */
				memset(cOldFileName, 0, sizeof(cOldFileName));
				eRet = eCOMM_SPACESAFER_DeleteOlderFile(psInfo, cOldFileName);
				if (eRet) {
					eRet = eCOMM_FILESYSTEM_JudgeMakeUp(psInfo->cMakeUp);
					if (eRet) {
						printf("************<TF card has changed>**********\n");
						return eRet;
					}
					printf("************<the older file may be changed>**********\n");
					eRet = eCOMM_SPACESAFER_UnRegisterFile(psInfo, cOldFileName);
					if (eRet) {	
						return eRet;				
					}	
					printf("************<unregister the older file(may be delete)>**********\n");
				} else {
					eRet = eCOMM_SPACESAFER_UnRegisterFile(psInfo, cOldFileName);
					if (eRet) {	
						return eRet;				
					}				
				}
					
				eRet = eCOMM_FILESYSTEM_GetFreeSpaceSize(psInfo->cTargetPath,&(psFileSystemInfo->lAvailableSize), &(psFileSystemInfo->lFreeSize));
				if (eRet) {	
					return eRet;
				}
			} else {				
				printf("have no space to store\n");				
				return COMM_SPACESAFER_NOSPACE;			
			}
		}		
	}

	CS_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_SPACESAFER_QueryHealthy(sCOMM_SPACESAFER_Info* psInfo)
* arg  : sCOMM_SPACESAFER_Info* psInfo
* ret  : eCOMM_SPACESAFER_Ret
* note :
*********************************************/
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_QueryHealthy(sCOMM_SPACESAFER_Info* psInfo) {
	eCOMM_SPACESAFER_Ret eRet = COMM_SPACESAFER_SUCCESS;
	LONG lFileSize = 0;
	
	CS_FuncIn();

	{
		psInfo->eStatus = COMM_SPACESAFER_STATUS_INVALID;
		eRet = eCOMM_FILESYSTEM_JudgeMakeUp(psInfo->cMakeUp);
		if (eRet) {
			return eRet;
		}
		
		eRet = eCOMM_SPACESAFER_GetCurrentFileSize(psInfo, &lFileSize);
		if (eRet) {
			return eRet;
		}	
		
		//assert(lFileSize < (psInfo->lFileSizeThreshold));
		psInfo->eStatus = COMM_SPACESAFER_STATUS_VALID;
	}

	CS_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_SPACESAFER_RegisterFile(sCOMM_SPACESAFER_Info* psInfo, CHAR* pcFileName)
* arg  : sCOMM_SPACESAFER_Info* psInfo, CHAR* pcFileName
* ret  : eCOMM_SPACESAFER_Ret
* note :
*********************************************/
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_RegisterFile(sCOMM_SPACESAFER_Info* psInfo, CHAR* pcFileName) {
	eCOMM_SPACESAFER_Ret eRet = COMM_SPACESAFER_SUCCESS;
	sCOMM_FILESORT_Info* psFIleSortInfo = psInfo->psFileSortInfo;
	
	CS_FuncIn();

	{
		eRet = eCOMM_SPACESAFER_QueryHealthy(psInfo);
		if (COMM_SPACESAFER_SUCCESS != eRet) {
			CS_Debug("last file is too large\n");
		}
		
		eRet = eCOMM_FILESORT_Write(psFIleSortInfo, pcFileName);
		if (COMM_SPACESAFER_SUCCESS != eRet) {
			CS_Debug("register file failed\n");
		}			
	}

	CS_FuncOut();

	return eRet;
}

eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_Compare(sCOMM_SPACESAFER_Info* psFirtInfo,
															sCOMM_SPACESAFER_Info* psSecInfo,
															sCOMM_SPACESAFER_Time sTimeStatus,
															CHAR *pcPathName,
															LONG *plIndex)
{
	eCOMM_SPACESAFER_Ret eRet = COMM_SPACESAFER_SUCCESS;
	sCOMM_LIST_Info *psFirstList = NULL;
	sCOMM_LIST_Info *psSecList = NULL;
	LONG lFirstLen = 0;
	LONG lSecLen = 0;
	
	CS_FuncIn();

	{	
		if (psFirtInfo->eStatus == COMM_SPACESAFER_STATUS_INVALID) {
			//eRet = eCOMM_SPACESAFER_Restart(psFirtInfo);
			if (COMM_SPACESAFER_SUCCESS != (eCOMM_SPACESAFER_Restart(psFirtInfo))) {
				CS_Debug("[%s] space restart failed!\n",psFirtInfo->cTargetPath);	
				//return eRet;
			}				
		}

		if (psSecInfo->eStatus == COMM_SPACESAFER_STATUS_INVALID) {
			//eRet = eCOMM_SPACESAFER_Restart(psSecInfo);
			if (COMM_SPACESAFER_SUCCESS != (eCOMM_SPACESAFER_Restart(psSecInfo))) {
				CS_Debug("[%s] space restart failed!\n",psSecInfo->cTargetPath);	
				//return eRet;
			}				
		}
		
		if((COMM_SPACESAFER_STATUS_VALID == psFirtInfo->eStatus) && 
				(COMM_SPACESAFER_STATUS_VALID != psSecInfo->eStatus)) 
		{
			*plIndex = 0;
			strncpy(pcPathName,psFirtInfo->cTargetPath,COMM_SPACESAFER_MAX);
			CS_Debug("%s is normal\n",psFirtInfo->cTargetPath);
			CS_Debug("%s is not normal\n",psSecInfo->cTargetPath);	
			
			return eRet;
		} else if ((COMM_SPACESAFER_STATUS_VALID != psFirtInfo->eStatus) && 
				(COMM_SPACESAFER_STATUS_VALID == psSecInfo->eStatus))
		{
			*plIndex = 1;
			strncpy(pcPathName,psSecInfo->cTargetPath,COMM_SPACESAFER_MAX);
			CS_Debug("%s is not normal\n",psFirtInfo->cTargetPath);
			CS_Debug("%s is normal\n",psSecInfo->cTargetPath);
			
			return eRet;
		} else if ((COMM_SPACESAFER_STATUS_VALID != psFirtInfo->eStatus) && 
				(COMM_SPACESAFER_STATUS_VALID != psSecInfo->eStatus)) {
			CS_Debug("%s is not normal\n",psFirtInfo->cTargetPath);
			CS_Debug("%s is not normal\n",psSecInfo->cTargetPath);	

			return COMM_SPACESAFER_COPMPARE;				
		}
		/* get file num fo folder */
		psFirstList = psFirtInfo->psFileSortInfo->psFileNameList;
		psSecList = psSecInfo->psFileSortInfo->psFileNameList;		
		eCOMM_LIST_GetCount(psFirstList,&lFirstLen);
		eCOMM_LIST_GetCount(psSecList,&lSecLen);	
		CS_Debug("lFirstLen=%ld,lSecLen=%ld\n",lFirstLen,lSecLen);
		
		/* check whether the two folders is empty */
		if ((0 == lFirstLen) && (0 == lSecLen)) {
			sCOMM_SPACESAFER_SpaceStatus sSpaceStatus = COMM_SPACESAFER_INVALID_SPACE;
			eCOMM_SPACESAFER_QueryCheck(psFirtInfo,&sSpaceStatus);
			if (COMM_SPACESAFER_HAVE_SPACE == sSpaceStatus) {
				*plIndex = 0;
				strncpy(pcPathName,psFirtInfo->cTargetPath,COMM_SPACESAFER_MAX);
				CS_Debug("%s is null and have space\n",psFirtInfo->cTargetPath);
				CS_Debug("%s is null\n",psSecInfo->cTargetPath);
			} else if (COMM_SPACESAFER_NOTHAVE_SPACE == sSpaceStatus){ 
				eCOMM_SPACESAFER_QueryCheck(psSecInfo,&sSpaceStatus);
				if (COMM_SPACESAFER_HAVE_SPACE == sSpaceStatus) {
					*plIndex = 1;
					strncpy(pcPathName,psSecInfo->cTargetPath,COMM_SPACESAFER_MAX);
					CS_Debug("%s is null and have no space\n",psFirtInfo->cTargetPath);
					CS_Debug("%s is null and have space\n",psSecInfo->cTargetPath);
				} else {
					CS_Debug("%s is null and have no space\n",psFirtInfo->cTargetPath);
					CS_Debug("%s is null and have no space\n",psSecInfo->cTargetPath);	

					return COMM_SPACESAFER_COPMPARE;
				}
			}
		} else if ((0 != lFirstLen) && (0 == lSecLen)) {
			sCOMM_SPACESAFER_SpaceStatus sSpaceStatus = COMM_SPACESAFER_INVALID_SPACE;
			eCOMM_SPACESAFER_QueryCheck(psFirtInfo,&sSpaceStatus);
			if (COMM_SPACESAFER_HAVE_SPACE == sSpaceStatus) {
				*plIndex = 0;
				strncpy(pcPathName,psFirtInfo->cTargetPath,COMM_SPACESAFER_MAX);
				CS_Debug("%s is not null and have space\n",psFirtInfo->cTargetPath);
				CS_Debug("%s is null\n",psSecInfo->cTargetPath);				
			} else {
				eCOMM_SPACESAFER_QueryCheck(psSecInfo,&sSpaceStatus);
				if (COMM_SPACESAFER_HAVE_SPACE == sSpaceStatus) {
					*plIndex = 1;
					strncpy(pcPathName,psSecInfo->cTargetPath,COMM_SPACESAFER_MAX);
					CS_Debug("%s is not null and have not space\n",psFirtInfo->cTargetPath);
					CS_Debug("%s is null and have space\n",psSecInfo->cTargetPath);	

					//return eRet;
				} else {
					*plIndex = 0;
					strncpy(pcPathName,psFirtInfo->cTargetPath,COMM_SPACESAFER_MAX);
					CS_Debug("%s is not null,we will cover the old file\n",psFirtInfo->cTargetPath);
					CS_Debug("%s is null and have no space\n",psSecInfo->cTargetPath);	
				}
			}
		} else if ((0 == lFirstLen) && (0 != lSecLen)) {
			sCOMM_SPACESAFER_SpaceStatus sSpaceStatus = COMM_SPACESAFER_INVALID_SPACE;
			eCOMM_SPACESAFER_QueryCheck(psSecInfo,&sSpaceStatus);
			if (COMM_SPACESAFER_HAVE_SPACE == sSpaceStatus) {
				*plIndex = 1;
				strncpy(pcPathName,psSecInfo->cTargetPath,COMM_SPACESAFER_MAX);
				CS_Debug("%s is null\n",psFirtInfo->cTargetPath);
				CS_Debug("%s is not null and have space\n",psSecInfo->cTargetPath);					
			} else {
				eCOMM_SPACESAFER_QueryCheck(psFirtInfo,&sSpaceStatus);
				if (COMM_SPACESAFER_HAVE_SPACE == sSpaceStatus) {
					*plIndex = 0;
					strncpy(pcPathName,psFirtInfo->cTargetPath,COMM_SPACESAFER_MAX);
					CS_Debug("%s is null and have space\n",psFirtInfo->cTargetPath);
					CS_Debug("%s is not null and have no space\n",psSecInfo->cTargetPath);	
				} else {
					*plIndex = 1;
					strncpy(pcPathName,psSecInfo->cTargetPath,COMM_SPACESAFER_MAX);
					CS_Debug("%s is null and have no space\n",psFirtInfo->cTargetPath);
					CS_Debug("%s is not null,we will cover the old file\n",psSecInfo->cTargetPath);	
				}
			}
		} else {
			CHAR cFirName[COMM_SPACESAFER_MAX] = {0};
			CHAR cSecName[COMM_SPACESAFER_MAX] = {0};	
			struct stat sFirtStat;
	    		struct stat sSecStat;	
		
			if (COMM_SPACESAFER_OLDEST == sTimeStatus) {
				/* get the latest file name */
				eCOMM_LIST_RetrieveSpecial(psFirstList,COMM_LIST_RETRIEVEKEY_FIRST,cFirName);
				eCOMM_LIST_RetrieveSpecial(psSecList,COMM_LIST_RETRIEVEKEY_FIRST,cSecName);

				if(stat(cFirName, &sFirtStat) == -1) {
					//perror("stat : ");
					CS_Debug("Stat function return error handle file <%s>!\n", cFirName);
				}			
				if(stat(cSecName, &sSecStat) == -1) {
					//perror("stat : ");
					CS_Debug("Stat function return error handle file <%s>!\n", cSecName);
				}	

				CS_Debug("cFirName:%s<%ld>\n",cFirName,sFirtStat.st_mtime);
				CS_Debug("cSecName:%s<%ld>\n",cSecName,sSecStat.st_mtime);	
				
				if(sFirtStat.st_mtime < sSecStat.st_mtime) {
					*plIndex = 0;
					strncpy(pcPathName,psFirtInfo->cTargetPath,COMM_SPACESAFER_MAX);
					CS_Debug("%s < %s\n",cFirName,cSecName);
				} else if (sFirtStat.st_mtime > sSecStat.st_mtime) {
					*plIndex = 1;
					strncpy(pcPathName,psSecInfo->cTargetPath,COMM_SPACESAFER_MAX);
					CS_Debug("%s > %s\n",cFirName,cSecName);
				}					
			} else if (COMM_SPACESAFER_LASTEST == sTimeStatus) {
				/* get the latest file name */
				eCOMM_LIST_RetrieveSpecial(psFirstList,COMM_LIST_RETRIEVEKEY_LAST,cFirName);
				eCOMM_LIST_RetrieveSpecial(psSecList,COMM_LIST_RETRIEVEKEY_LAST,cSecName);		
				
				if(stat(cFirName, &sFirtStat) == -1) {
					//perror("stat : ");
					CS_Debug("Stat function return error handle file <%s>!\n", cFirName);
				}			
				if(stat(cSecName, &sSecStat) == -1) {
					//perror("stat : ");
					CS_Debug("Stat function return error handle file <%s>!\n", cSecName);
				}	
				CS_Debug("cFirName:%s<%ld>\n",cFirName,sFirtStat.st_mtime);
				CS_Debug("cSecName:%s<%ld>\n",cSecName,sSecStat.st_mtime);				

				if(sFirtStat.st_mtime > sSecStat.st_mtime) {
					*plIndex = 0;
					strncpy(pcPathName,psFirtInfo->cTargetPath,COMM_SPACESAFER_MAX);
					CS_Debug("%s > %s\n",cFirName,cSecName);
				} else if (sFirtStat.st_mtime < sSecStat.st_mtime) {
					*plIndex = 1;
					strncpy(pcPathName,psSecInfo->cTargetPath,COMM_SPACESAFER_MAX);
					CS_Debug("%s < %s\n",cFirName,cSecName);
				}		
			}		
		}
	
	}

	CS_FuncOut();

	return eRet;
}

#ifdef __cplusplus
}
#endif
