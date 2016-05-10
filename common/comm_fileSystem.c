#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "comm_fileSystem.h"

/***************************************************
	macro / enum
***************************************************/
//#define COMM_FILESYSTEM_DEBUG
#ifdef COMM_FILESYSTEM_DEBUG
#define CF_Debug(fmt, arg...) fprintf(stdout, "[ CFSYS ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CF_Debug(fmt, arg...)
#endif

#define CF_FuncIn() CF_Debug("in\n")
#define CF_FuncOut() CF_Debug("out\n")
#define CF_iVal(iVal) CF_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CF_lVal(lVal) CF_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CF_PtVal(ptVal) CF_Debug("pt %s @ %p\n", #ptVal, ptVal)


/***************************************************
	variable
***************************************************/
pthread_mutex_t sFileSystem_Mutex;
static LONG lUsedSize = 0;
static LONG lTotalFileNum = 0;
/***************************************************
	prototype
***************************************************/
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_CheckPath(CHAR* pcPathName);
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_MakeUp(CHAR* pcPathName, CHAR* pcMakeUp);
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_CHECKMOUNT(CHAR* pcPathName);

/***************************************************
	function
***************************************************/
/*********************************************
* func : eCOMM_FILESYSTEM_CheckPath(CHAR* pcPathName)
* arg  : CHAR* pcPathName
* ret   : eCOMM_FILESYSTEM_Ret
* note :
*********************************************/
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_CheckPath(CHAR* pcPathName) {
	eCOMM_FILESYSTEM_Ret eRet = COMM_FILESYSTEM_SUCCESS;

	CF_FuncIn();

	{
		if(access(pcPathName,F_OK)) {
			eRet = COMM_FILESYSTEM_PATHNOEXIT;
			CF_Debug("file path is not exit:%s failed\n",pcPathName);
			return eRet;
		}

		eRet = eCOMM_FILESYSTEM_CHECKMOUNT(pcPathName);
		if (COMM_FILESYSTEM_SUCCESS != eRet) {
			CF_Debug("mount point is not exit:%s failed\n",pcPathName);
			return eRet;
		}
	}

	CF_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_FILESYSTEM_MakeUp(CHAR* pcPathName, CHAR* pcMakeUp)
* arg  : CHAR* pcPathName, CHAR* pcMakeUp
* ret   : eCOMM_FILESYSTEM_Ret
* note :
*********************************************/
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_MakeUp(CHAR* pcPathName, CHAR* pcMakeUp) {
	eCOMM_FILESYSTEM_Ret eRet = COMM_FILESYSTEM_SUCCESS;
	CHAR cFileName[COMM_FILESYSTEM_MAKEUP_MAX];
	CHAR cCmd[COMM_FILESYSTEM_MAKEUP_MAX];
	
	CF_FuncIn();

	{
		memset(&cFileName, 0, sizeof(cFileName));
		sprintf(cFileName, "%s%s",pcPathName, TESTFILE);
		eCOMM_FILESYSTEM_DeleteFile(cFileName);

		memset(&cCmd, 0, sizeof(cCmd));
		sprintf(cCmd, "touch %s%s",pcPathName, TESTFILE);
		if (system(cCmd)) {
			eRet = COMM_FILESYSTEM_MAKEUP;
			CF_Debug("make up creat failed\n");

			return eRet;
		}
		
		memset(pcMakeUp, 0, strlen(pcMakeUp));
		sprintf(pcMakeUp, "%s%s",pcPathName, TESTFILE);
		CF_Debug("make up creat file:%s success\n",pcMakeUp);
	}

	CF_FuncOut();

	return eRet;
}

/*******************************************************************************
* @brief		: eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_CHECKMOUNT(CHAR* pcPathName)
* @param	: CHAR* pcPathName
* @retval		: eCOMM_FILESYSTEM_Ret
* @note		: check whether mount point exist
*******************************************************************************/
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_CHECKMOUNT(CHAR* pcPathName)
{
	eCOMM_FILESYSTEM_Ret eRet = COMM_FILESYSTEM_SUCCESS;
	CF_FuncIn();

	{
		FILE *pMntPath;
		CHAR cMnt[256], cDev[256], cFs[256];
		LONG lFindPath = 0;;

		/* get mount point path */
		pMntPath = fopen("/proc/mounts", "r");
		if (pMntPath == NULL) {
			printf("mount infomation open failed!\n");
			eRet = COMM_FILESYSTEM_MOUNT;
			CF_FuncOut();
			return eRet;
		}
		
		/* check whether mount path exist */
		while (fgets(cMnt, 256, pMntPath)) {
			sscanf(cMnt, "%[^ ] %[^ ] %*s\n", cDev, cFs);
			//printf("cDev:%s;cFs:%s\n",cDev,cFs);
			/*
			if (strstr(pcPathName, cFs) != NULL) {
				printf("Mount Info:%s\n",cDev);
				lFindPath = 1;
				break;
			}	
			*/
#ifndef NAND_FLASH
			if (strstr(cDev, "mtdblo") != NULL) {
				if (strstr(pcPathName, cFs) != NULL) {
					//printf("Mount Info:%s\n",cDev);
					lFindPath = 1;
					break;					
				}
			}
#else
			if (strstr(cDev, "mmcblk") != NULL) {
				if (strstr(pcPathName, cFs) != NULL) {
					//printf("Mount Info:%s\n",cDev);
					lFindPath = 1;
					break;					
				}
			}
#endif			
		}
		
		fclose(pMntPath);	
		
		if(0 == lFindPath) {
			eRet = COMM_FILESYSTEM_MOUNT;
			CF_Debug("can not find mount point!\n");
			CF_FuncOut();
			return eRet;
		}
	
	}

	CF_FuncOut();
		
	return eRet;
}

/*********************************************
* func : eCOMM_FILESYSTEM_Init()
* arg  :
* ret  : eCOMM_FILESYSTEM_Ret
* note :
*********************************************/
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_Init() {
	eCOMM_FILESYSTEM_Ret eRet = COMM_FILESYSTEM_SUCCESS;

	CF_FuncIn();

	{
		/* init mutex */
		pthread_mutex_init(&sFileSystem_Mutex, NULL);
	}

	CF_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_FILESYSTEM_DeInit()
* arg  :
* ret  : eCOMM_FILESYSTEM_Ret
* note :
*********************************************/
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_DeInit() {
	eCOMM_FILESYSTEM_Ret eRet = COMM_FILESYSTEM_SUCCESS;

	CF_FuncIn();

	{
		pthread_mutex_destroy(&sFileSystem_Mutex);
	}

	CF_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_FILESYSTEM_DeleteFile(CHAR* pcFileName)
* arg  : CHAR* pcFileName
* ret  : eCOMM_FILESYSTEM_Ret
* note :
*********************************************/
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_DeleteFile(CHAR* pcFileName) {
	eCOMM_FILESYSTEM_Ret eRet = COMM_FILESYSTEM_SUCCESS;

	CF_FuncIn();

	{
		/* judge:if */
		if (access(pcFileName, F_OK) == 0) {
			pthread_mutex_lock(&sFileSystem_Mutex);
			
			if(access(pcFileName, W_OK) == 0) {
				eRet = remove(pcFileName);
				if (eRet) {
					CF_Debug("remove the file:%s failed\n",pcFileName);
					perror("Error to remove file");
				} else {
					CF_Debug("remove the file:%s success\n",pcFileName);
				}

			} else {
				eRet = COMM_FILESYSTEM_DELETE;
				CF_Debug("the file:%s is not exit or readonly\n",pcFileName);
			}

			pthread_mutex_unlock(&sFileSystem_Mutex);
		} else {
			eRet = COMM_FILESYSTEM_DELETE;
			CF_Debug("the file:%s is not exit\n",pcFileName);
		}
	}

	CF_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_FILESYSTEM_GetFileSize(CHAR* pcFileName, LONG* plFileSize)
* arg  : CHAR* pcFileName, LONG* plFileSize
* ret  : eCOMM_FILESYSTEM_Ret
* note :
*********************************************/
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_GetFileSize(CHAR* pcFileName, LONG* plFileSize) {
	eCOMM_FILESYSTEM_Ret eRet = COMM_FILESYSTEM_SUCCESS;
	struct stat sFileStat;
	
	CF_FuncIn();

	{
		memset(&sFileStat, 0, sizeof(struct stat));
		if(stat(pcFileName, &sFileStat) == -1) {
			perror("stat : ");
			printf("Stat function return error handle file <%s>!\n", pcFileName);
			/* need to handle  */
			CF_FuncOut();
			return COMM_FILESYSTEM_GETFILESIZE;
		}	
		*plFileSize = (sFileStat.st_size >> 20);
		CF_Debug("file size:<%ld MB>\n",*plFileSize);
	}

	CF_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_FILESYSTEM_CheckSpace(CHAR* pcPathName, LONG lSpaceThreshold)
* arg  : CHAR* pcPathName, LONG lSpaceThreshold
* ret  : eCOMM_FILESYSTEM_Ret
* note :
*********************************************/
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_CheckSpace(CHAR* pcPathName, LONG lSpaceThreshold) {
	eCOMM_FILESYSTEM_Ret eRet = COMM_FILESYSTEM_SUCCESS;
	LONG lFreeSpace = 0;
	
	CF_FuncIn();

	{
		/* get free space */
		eRet = eCOMM_FILESYSTEM_GetAvailableSize(pcPathName, &lFreeSpace);
		if (eRet) {
			CF_Debug("get space available size failed\n");
			return eRet;
		}
		
		/* judge the free space whether is enough */
		if(lFreeSpace <= lSpaceThreshold) {
			eRet = COMM_FILESYSTEM_CHECKSPACE;
			CF_Debug("free space is not enough\n");
			
			CF_FuncOut();
			return eRet;
		}		
	}

	CF_FuncOut();

	return eRet;	
			
}

/***************************************************************************
  * func : iCOMM_FILESYSTEM__GetOccupySize(const char *cpDirpath, const struct stat *spsFileStat,long lTypeFlag, struct FTW *spFtwBuf)
  * arg : const char *cpDirpath, const struct stat *spsFileStat,long lTypeFlag, struct FTW *spFtwBuf
  * ret : int
  * note :
***************************************************************************/
int iCOMM_FILESYSTEM_GetOccupySize(const char *cpDirpath, const struct stat *spFileStat, int lTypeFlag, struct FTW *spFtwBuf) {
	int iRet = COMM_FILESYSTEM_SUCCESS;
	
	//CF_FuncIn();
	
	{
		LONG lCurFileSize = (int)(spFileStat->st_size>>10);
		lUsedSize += lCurFileSize; 
		++lTotalFileNum;
		//CF_Debug("=====================================\n");
		//CF_Debug("Cur_FileNum:%ld Cur_FilePath:%s\n",lTotalFileNum,cpDirpath); 
		//CF_Debug("Cur_FileSize:%ld KB\n",lCurFileSize); 
		//CF_Debug("=====================================\n");
		
	}

	//CF_FuncOut();

	return iRet;
}

/***************************************************************************
  * func : eBASE_MW_FILESORT_GetUsedSpace(sBASE_MW_FILESORT_Info* psInfo)
  * arg : sBASE_MW_FILESORT_Info* psInfo
  * ret : eBASE_MW_FILESORT_RET
  * note :
***************************************************************************/
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_GetUsedSpaceSize(CHAR* pcPathName, LONG* plUsedSpaceSize) {
	eCOMM_FILESYSTEM_Ret eRet = COMM_FILESYSTEM_SUCCESS;
	//sBASE_MW_DIR_STAT* psDIRStat = &gsDIRStat;
	CHAR cCurDirPath[128];
	CHAR cTmpDirPath[128];
	LONG lCurTotalFileNum;
	
	CF_FuncIn();

	{
		CF_Debug("mutex lock enter !\n");
		
		pthread_mutex_lock(&sFileSystem_Mutex);		
		getcwd(cCurDirPath,sizeof(cCurDirPath));
		chdir(pcPathName);
		getcwd(cTmpDirPath,sizeof(cTmpDirPath));
		lUsedSize = 0;
		lTotalFileNum = 0;
		
		eRet = nftw(pcPathName,iCOMM_FILESYSTEM_GetOccupySize,10,FTW_PHYS);
		if(eRet < 0){   
			printf("-nftw:[wrong:%ld]ntfw search %ld files\n",(LONG)eRet,lTotalFileNum); 
			chdir(cCurDirPath);
			getcwd(cTmpDirPath,sizeof(cTmpDirPath));		
			pthread_mutex_unlock(&sFileSystem_Mutex);
			CF_Debug("mutex lock exit !\n");
			
			return COMM_FILESYSTEM_GETSPACE;   
		}	
		
		chdir(cCurDirPath);
		getcwd(cTmpDirPath,sizeof(cTmpDirPath));	
		lCurTotalFileNum = lTotalFileNum;
		*plUsedSpaceSize = lUsedSize;
		pthread_mutex_unlock(&sFileSystem_Mutex);
		
		CF_Debug("mutex lock exit !\n");
		if (*plUsedSpaceSize > 1024) {
			CF_Debug("Current path:<%s>;Current file numbers:<%ld>;Used size:<%ld> Mb\n", pcPathName,lCurTotalFileNum,COMM_SYSTEM_KB2MB(*plUsedSpaceSize));
		} else {
			CF_Debug("Current path:<%s>;Current file numbers:<%ld>;Used size:<%ld> Kb\n", pcPathName,lCurTotalFileNum,(*plUsedSpaceSize));
		}
  			
		
				
	}

	CF_FuncOut();

	return eRet;
}
/*********************************************
* func : eCOMM_FILESYSTEM_GetFreeSpaceSize(CHAR* pcPathName, LONG* plTotalSpaceSize,LONG* plFreeSpaceSize)
* arg  : CHAR* pcPathName, LONG* plTotalSpaceSize,LONG* plFreeSpaceSize
* ret  : eCOMM_FILESYSTEM_Ret
* note :
*********************************************/
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_GetFreeSpaceSize(CHAR* pcPathName, LONG* plAvailableSize,LONG* plFreeSize) {
	eCOMM_FILESYSTEM_Ret eRet = COMM_FILESYSTEM_SUCCESS;
	CF_FuncIn();

	{
		LONG lUsedSpaceSize = 0;
		eRet = eCOMM_FILESYSTEM_GetUsedSpaceSize(pcPathName,&lUsedSpaceSize);
		if (COMM_FILESYSTEM_SUCCESS != eRet) {
			CF_Debug("get free space size failed\n");
			eRet = COMM_FILESYSTEM_GETSPACE;
			
			return eRet;
		}
		*plFreeSize = *plAvailableSize - lUsedSpaceSize;
	}

	CF_FuncOut();

	return eRet;	
}

/*********************************************
* func : eCOMM_FILESYSTEM_GetToalSpaceSize(CHAR* pcPathName,LONG* plTotalSpaceSize)
* arg  : CHAR* pcPathName,LONG* plTotalSpaceSize
* ret  : eCOMM_FILESYSTEM_Ret
* note :
*********************************************/
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_GetToalSpaceSize(CHAR* pcPathName,LONG* plTotalSpaceSize) {
	eCOMM_FILESYSTEM_Ret eRet = COMM_FILESYSTEM_SUCCESS;
	struct statfs sStatFS;
	
	CF_FuncIn();

	{
		pthread_mutex_lock(&sFileSystem_Mutex);

		eRet = eCOMM_FILESYSTEM_CheckPath(pcPathName);
		if (eRet) {
			pthread_mutex_unlock(&sFileSystem_Mutex);
			CF_Debug("get space available size failed\n");
			return eRet;
		}

		if(statfs(pcPathName, &sStatFS) == -1)
		{
			pthread_mutex_unlock(&sFileSystem_Mutex);
			CF_Debug("statfs failed for path->[%s]\n", pcPathName);
			eRet = COMM_FILESYSTEM_GETSPACE;
			
			return eRet;
		}
		LONG lTotalKB = (((LONG)sStatFS.f_frsize)>>10) * ((LONG)sStatFS.f_blocks);
		LONG lFreeKB = (((LONG)sStatFS.f_frsize>>10) * ((LONG)sStatFS.f_bfree));
		LONG lUsedKB = (LONG)(lTotalKB - lFreeKB);
		

		//*plTotalSpaceSize = COMM_SYSTEM_KB2MB(lTotalKB);
		*plTotalSpaceSize = lTotalKB;
			
		CF_Debug("Device[%s]:Total %ld Mb,used %ld Mb,avable %ld Mb\n",
					pcPathName, 
					COMM_SYSTEM_KB2MB(lTotalKB), 
					COMM_SYSTEM_KB2MB(lUsedKB), 
					COMM_SYSTEM_KB2MB(lFreeKB));	
		
		pthread_mutex_unlock(&sFileSystem_Mutex);

	}

	CF_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_FILESYSTEM_GetAvailableSize(CHAR* pcPathName, LONG* plAvailableSize)
* arg  : CHAR* pcPathName, LONG* plAvailableSize
* ret  : eCOMM_FILESYSTEM_Ret
* note :
*********************************************/
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_GetAvailableSize(CHAR* pcPathName, LONG* plAvailableSize) {
	eCOMM_FILESYSTEM_Ret eRet = COMM_FILESYSTEM_SUCCESS;
	struct statfs sStatFS;
	
	CF_FuncIn();

	{
		pthread_mutex_lock(&sFileSystem_Mutex);

		eRet = eCOMM_FILESYSTEM_CheckPath(pcPathName);
		if (eRet) {
			pthread_mutex_unlock(&sFileSystem_Mutex);
			CF_Debug("get space available size failed\n");
			return eRet;
		}

		if(statfs(pcPathName, &sStatFS) == -1)
		{
			pthread_mutex_unlock(&sFileSystem_Mutex);
			CF_Debug("statfs failed for path->[%s]\n", pcPathName);
			eRet = COMM_FILESYSTEM_GETSPACE;
			
			return eRet;
		}
		LONG lTotalKB = (((LONG)sStatFS.f_frsize)>>10) * ((LONG)sStatFS.f_blocks);
		LONG lFreeKB = (((LONG)sStatFS.f_frsize>>10) * ((LONG)sStatFS.f_bfree));
		LONG lUsedKB = (LONG)(lTotalKB - lFreeKB);

		*plAvailableSize = COMM_SYSTEM_KB2MB(lFreeKB);
			
		CF_Debug("Device[%s]:Total %ld Mb,used %ld Mb,avable %ld Mb\n",pcPathName, COMM_SYSTEM_KB2MB(lTotalKB), COMM_SYSTEM_KB2MB(lUsedKB), *plAvailableSize);	
		
		pthread_mutex_unlock(&sFileSystem_Mutex);

	}

	CF_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_FILESYSTEM_GetPathInfo(CHAR* pcPathName, sCOMM_FILESYSTEM_Info* psPathInfo)
* arg  : CHAR* pcPathName, sCOMM_FILESYSTEM_Info* psPathInfo
* ret  : eCOMM_FILESYSTEM_Ret
* note :
*********************************************/
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_GetPathInfo(CHAR* pcPathName, sCOMM_FILESYSTEM_Info* psPathInfo) {
	eCOMM_FILESYSTEM_Ret eRet = COMM_FILESYSTEM_SUCCESS;

	CF_FuncIn();

	{
		eRet = eCOMM_FILESYSTEM_CheckPath(pcPathName);
		if (COMM_FILESYSTEM_SUCCESS != eRet) {
			return eRet;
		}
		
		eRet = eCOMM_FILESYSTEM_MakeUp(pcPathName, psPathInfo->cMakeUp);
		if (COMM_FILESYSTEM_SUCCESS != eRet) {
			return eRet;
		}	
		
		eRet = eCOMM_FILESYSTEM_GetToalSpaceSize(pcPathName, &(psPathInfo->lTotalSize));
		if (COMM_FILESYSTEM_SUCCESS != eRet) {
			return eRet;
		}
		psPathInfo->lAvailableSize = (LONG)((psPathInfo->lTotalSize) * (psPathInfo->dRatio));
		CF_Debug("lAvailableSize:%ld Mb\n",COMM_SYSTEM_KB2MB(psPathInfo->lAvailableSize));
		
		CF_Debug("get path info success\n");
	}

	CF_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_FILESYSTEM_JudgeMakeUp(CHAR* pcOldMakeUp)
* arg  : CHAR* pcOldMakeUp
* ret  : eCOMM_FILESYSTEM_Ret
* note :
*********************************************/
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_JudgeMakeUp(CHAR* pcOldMakeUp) {
	eCOMM_FILESYSTEM_Ret eRet = COMM_FILESYSTEM_SUCCESS;

	CF_FuncIn();

	{
		if(access(pcOldMakeUp,F_OK | W_OK)) {
			eRet = COMM_FILESYSTEM_MAKEUP;
			printf("make up is not exit:%s failed\n",pcOldMakeUp);
		}
	}

	CF_FuncOut();

	return eRet;
}

#ifdef __cplusplus
}
#endif
