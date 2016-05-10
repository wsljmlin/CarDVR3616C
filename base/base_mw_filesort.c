#ifdef __cplusplus
extern "C"{
#endif

/***************************************************
	include
***************************************************/
#define _BSD_SOURCE
#define _XOPEN_SOURCE 500
#include <ftw.h>  
#include "comm_queue.h"
#include "base_mw_filesort.h"
#include "assert.h"
#include <dirent.h>
/***************************************************
	macro / enum
***************************************************/
//#define BASE_MW_FILESORT_DEBUG
#ifdef BASE_MW_FILESORT_DEBUG
#define BMF_Debug(fmt, arg...) fprintf(stdout, "[ BMF ] : %s() <%d> "fmt, __func__, __LINE__, ##arg)
#else
#define BMF_Debug(fmt, arg...)
#endif

#define BMF_FuncIn() BMF_Debug("in\n")
#define BMF_FuncOut() BMF_Debug("out\n")

#define BMF_iVal(ival) BMF_Debug("%s <%d>\n", #ival, ival);
#define BMF_lVal(lval) BMF_Debug("%s <%ld>\n", #lval, lval);
#define BMF_ptVal(ptval) BMF_Debug("%s <%p>\n", #ptval, ptval);

#define FILEPOSITION_MAX 128

#define BUGFIX_LOOPREC
/***************************************************
	struct
***************************************************/

/***************************************************
	variable
***************************************************/
CHAR cFileVideoPosition[FILEPOSITION_MAX];
CHAR cFileSubcardPosition[FILEPOSITION_MAX];
CHAR cFileSnapPosition[FILEPOSITION_MAX];
LONG lUsedSize = 0;
LONG lTotalFileNum = 0;
/***************************************************
	prototype
***************************************************/
int iBASE_MW_FILESORT_SortByTime(const struct dirent ** pvFileSrc, const struct dirent ** pvFileTgt);
int iBASE_MW_FILESORT_GetOccupySize(const char *cpDirpath, const struct stat *spFileStat, int lTypeFlag, struct FTW *spFtwBuf); 
eBASE_MW_FILESORT_RET eBASE_MW_FILESORT_GetUsedSpace(sBASE_MW_FILESORT_Info* psInfo);
	

/***************************************************
	function
***************************************************/

/***************************************************************************
  * func : iBASE_MW_FILESORT_SortByTime(const struct dirent ** pvFileSrc, const struct dirent ** pvFileTgt)
  * arg : const struct dirent ** pvFileSrc, const struct dirent ** pvFileTgt
  * ret : int
  * note : sort files by time
***************************************************************************/
int iBASE_MW_FILESORT_SortByTime(const struct dirent ** pvFileSrc, const struct dirent ** pvFileTgt) {
	const struct dirent ** sFileSrc = NULL;
	const struct dirent ** sFileTgt = NULL;
	
	BMF_FuncIn();

	{
		
		struct stat sSrcFileStat;
    	struct stat sTgtFileStat;

		sFileSrc =  pvFileSrc;
		sFileTgt =  pvFileTgt;

		if(access((*sFileSrc)->d_name, F_OK | R_OK | W_OK))
		{
			perror("FleSort Check first file:");
			/* need to handle  */
			BMF_FuncOut();
			return -1;						
		}	

		if(access((*sFileTgt)->d_name, F_OK | R_OK | W_OK))
		{
			perror("FleSort Check second file:");
			/* need to handle  */
			BMF_FuncOut();
			return -1;						
		}	

		if(stat((*sFileSrc)->d_name, &sSrcFileStat) == -1) {
			perror("stat : ");
			printf("Stat function return error handle file <%s>!\n", (*sFileSrc)->d_name);
			/* need to handle  */
			BMF_FuncOut();
			return -1;
		}
		if(stat((*sFileTgt)->d_name, &sTgtFileStat) == -1) {
			perror("stat : ");
			printf("Stat function return error handle file <%s>!\n", (*sFileSrc)->d_name);
			/* need to handle */
			BMF_FuncOut();
			return -1;
		}
		
		if(sSrcFileStat.st_mtime > sTgtFileStat.st_mtime) {
			BMF_FuncOut();
			return 1;
		} else if (sSrcFileStat.st_mtime < sTgtFileStat.st_mtime) {
			BMF_FuncOut();
			return -1;
		} else {
			BMF_FuncOut();
			return 0;
		}

	}

	BMF_FuncOut();
}

/***************************************************************************
  * func : eBASE_MW_FILESORT_GetOccupySize(const char *cpDirpath, const struct stat *spsFileStat,long lTypeFlag, struct FTW *spFtwBuf)
  * arg : const char *cpDirpath, const struct stat *spsFileStat,long lTypeFlag, struct FTW *spFtwBuf
  * ret : int
  * note :
***************************************************************************/
int iBASE_MW_FILESORT_GetOccupySize(const char *cpDirpath, const struct stat *spFileStat, int lTypeFlag, struct FTW *spFtwBuf) {
	eBASE_MW_FILESORT_RET eRet = BASE_MW_FILESORT_SUCCESS;
	
	BMF_FuncIn();
	
	{
		LONG lCurFileSize = (int)(spFileStat->st_size>>10);
		lUsedSize += lCurFileSize; 
		++lTotalFileNum;
		BMF_Debug("=====================================\n");
		BMF_Debug("Cur_FileNum:%d Cur_FilePath:%s\n",lTotalFileNum,cpDirpath); 
		BMF_Debug("Cur_FileSize:%d KB\n",lCurFileSize); 
		BMF_Debug("=====================================\n");
		
	}

	BMF_FuncOut();

	return eRet;
}

/***************************************************************************
  * func : eBASE_MW_FILESORT_GetUsedSpace(sBASE_MW_FILESORT_Info* psInfo)
  * arg : sBASE_MW_FILESORT_Info* psInfo
  * ret : eBASE_MW_FILESORT_RET
  * note :
***************************************************************************/
eBASE_MW_FILESORT_RET eBASE_MW_FILESORT_GetUsedSpace(sBASE_MW_FILESORT_Info* psInfo) {
	eBASE_MW_FILESORT_RET eRet = BASE_MW_FILESORT_SUCCESS;
	sBASE_MW_DIR_STAT* psDIRStat = &gsDIRStat;
	//CHAR cCurDirPath[128];
	LONG lCurTotalFileNum;
	
	BMF_FuncIn();

	{
		BMF_Debug("mutex lock enter !\n");

		pthread_mutex_lock(&psDIRStat->sMutStatus);
		//getcwd(cCurDirPath,sizeof(cCurDirPath));
		chdir(psInfo->pcFilePath);
		//getcwd(cCurDirPath,sizeof(cCurDirPath));
		lUsedSize = 0;
		lTotalFileNum = 0;
		
		eRet = nftw(psInfo->pcFilePath,iBASE_MW_FILESORT_GetOccupySize,10,FTW_PHYS);
		if(eRet < 0){   
			printf("-nftw:[wrong:%ld]ntfw search %ld files\n",(LONG)eRet,lTotalFileNum); 
			chdir(psDIRStat->cOriginalDir);
			//getcwd(cCurDirPath,sizeof(cCurDirPath));		
			pthread_mutex_unlock(&psDIRStat->sMutStatus);
			BMF_Debug("mutex lock exit !\n");
			
			return BASE_MW_NFTW_FAIL;   
		}	
		
		chdir(psDIRStat->cOriginalDir);
		//getcwd(cCurDirPath,sizeof(cCurDirPath));	
		lCurTotalFileNum = lTotalFileNum;
		psInfo->lUsedSize = lUsedSize;
		pthread_mutex_unlock(&psDIRStat->sMutStatus);			
		BMF_Debug("mutex lock exit !\n");	
  	 	printf("pathname:<%s>;total file numbers:<%ld>;total size:<%ld> KB\n", psInfo->pcFilePath,lCurTotalFileNum,psInfo->lUsedSize);	
		
				
	}

	BMF_FuncOut();

	return eRet;
}

/***************************************************************************
  * func : eBASE_MW_FILESORT_CheckSpace(sBASE_MW_FILESORT_Info* psInfo, LONG lTotalSpaceSize, double dRatio)
  * arg : sBASE_MW_FILESORT_Info* psInfo,LONG lTotalSpaceSize, double dRatio
  * ret : eBASE_MW_FILESORT_RET
  * note :
***************************************************************************/

eBASE_MW_FILESORT_RET eBASE_MW_FILESORT_CheckSpace(sBASE_MW_FILESORT_Info* psInfo, LONG lTotalSpaceSize, double dRatio, LONG lMaxFileSize) {
	eBASE_MW_FILESORT_RET eRet = BASE_MW_FILESORT_SUCCESS;
#ifdef BUGFIX_LOOPREC
#else
	LONG lUsableSize = (LONG)(lTotalSpaceSize * dRatio);
	LONG lUsedSize;
#endif /* BUGFIX_LOOPREC */

	BMF_FuncIn();
	
	{
#ifdef BUGFIX_LOOPREC
		struct statfs sStatFS={0,};
		LONG lTotalKB= 0, lFreeKB = 0, lUsedKB=0, lUsableSize=0;
		
		if(statfs("/opt/mainCard", &sStatFS) < 0){
			return BASE_MW_NFTW_FAIL; 
		}
		lFreeKB = (((LONG)sStatFS.f_frsize>>10) * ((LONG)sStatFS.f_bfree));

		lUsableSize = lFreeKB * dRatio;
		psInfo->lUsableSize = lUsableSize;
		
		if( (lUsableSize >= lMaxFileSize) ){
#else
		psInfo->lUsableSize = lUsableSize;
		eRet = eBASE_MW_FILESORT_GetUsedSpace(psInfo);
		if (BASE_MW_FILESORT_SUCCESS != eRet) {
			printf("GetUsedSpace filed!\n");
			
			return eRet;
		}

		lUsedSize = psInfo->lUsedSize;
		if ((lUsableSize- lUsedSize) >= lMaxFileSize) {
#endif /* BUGFIX_LOOPREC */
			psInfo->lIsEnough = BASE_MW_FILESORT_IsEnough;
		}  else {
			psInfo->lIsEnough = BASE_MW_FILESORT_IsNoenough;
		}
#ifdef BUGFIX_LOOPREC
		printf("Cur_UsableSize:%ld KB;Cur_State:%ld\n",lUsableSize, psInfo->lIsEnough);
#else
		printf("Cur_UsableSize:%ld KB;Cur_UsedSize:%ld KB,Cur_State:%ld\n",lUsableSize, lUsedSize,psInfo->lIsEnough);		
#endif /* BUGFIX_LOOPREC */
	}

	BMF_FuncOut();

	return eRet;
}


/***************************************************************************
  * func : eBASE_MW_FILESORT_Init(sBASE_MW_FILESORT_Info* psInfo, const LONG lMaxLen, const CHAR* pcFilePath)
  * arg :sBASE_MW_FILESORT_Info* psInfo, const LONG lMaxLen, const CHAR* pcFilePath
  * ret : eBASE_MW_FILESORT_RET
  * note : sort files by time
***************************************************************************/
eBASE_MW_FILESORT_RET eBASE_MW_FILESORT_Init(sBASE_MW_FILESORT_Info* psInfo, const LONG lMaxLen, const CHAR* pcFilePath)
{

	eBASE_MW_FILESORT_RET eRet = BASE_MW_FILESORT_SUCCESS;
	struct dirent **ppsDirentry = NULL;
	sBASE_MW_DIR_STAT* psDIRStat = &gsDIRStat;
	LONG lCnt = 0;
	CHAR *pStr = NULL;
	CHAR cFileName[256];
	//CHAR buf[128];
	LONG lFileScanLen;
	
	BMF_FuncIn();

	{
		psInfo->lState = BASE_MW_FILESORT_Invalid;
		/* init the file list */
		psInfo->lFileMaxLen = lMaxLen;
		psInfo->pcFilePath = strdup(pcFilePath);
		psInfo->lFileLen = 0;
		psInfo->lIsEnough = BASE_MW_FILESORT_IsNoenough;
		MALLOC(sizeof(sCOMM_QUEUE));
		psInfo->psFileList = (sCOMM_QUEUE*)malloc(sizeof(sCOMM_QUEUE));
		vCOMM_LQ_Init(psInfo->psFileList, psInfo->lFileMaxLen);
			
		/* scan the path and save to the file list */	
		if (0 == strcmp(psInfo->pcFilePath,BASE_MW_SD_MAINCARDPATH)) {
			memset(cFileVideoPosition, 0, FILEPOSITION_MAX);
			strcpy(cFileVideoPosition, psInfo->pcFilePath);			
		} else if (0 == strcmp(psInfo->pcFilePath,BASE_MW_SD_SUBCARDPATH)) {
			memset(cFileSubcardPosition, 0, FILEPOSITION_MAX);
			strcpy(cFileSubcardPosition, psInfo->pcFilePath);		
		} else if (0 == strcmp(psInfo->pcFilePath,BASE_MW_SD_SNAPPATH)) {
			memset(cFileSnapPosition, 0, FILEPOSITION_MAX);
			strcpy(cFileSnapPosition, psInfo->pcFilePath);		
		}	
		
		
		BMF_Debug("mutex lock enter !\n");
		pthread_mutex_lock(&psDIRStat->sMutStatus);		
		//getcwd(buf,sizeof(buf));
		chdir(psInfo->pcFilePath);
		//getcwd(buf,sizeof(buf));

		lFileScanLen = 0;
		lFileScanLen = scandir(psInfo->pcFilePath, &ppsDirentry, 0, iBASE_MW_FILESORT_SortByTime);
		chdir(psDIRStat->cOriginalDir);
		//getcwd(buf,sizeof(buf));		
		pthread_mutex_unlock(&psDIRStat->sMutStatus);	
		BMF_Debug("mutex lock exit !\n");
		
		if (lFileScanLen < 0) {
			BMF_Debug("Scandir function return error\n");
			/* need to handle ?? */
			//eBASE_MW_FILESORT_DeInit(psInfo);
			vCOMM_LQ_DeInit(psInfo->psFileList);
			if (NULL != psInfo->pcFilePath) 
				free(psInfo->pcFilePath);				
			eRet = BASE_MW_SCANDIR_FAIL;
			
			return eRet;

		}
		else {
			while (lCnt < lFileScanLen) {
				pStr = strrchr(ppsDirentry[lCnt]->d_name, '.');
				if (NULL != pStr) {
					if(0 == strcmp(pStr,".h264") || 0 == strcmp(pStr,".adpcm") || 0 == strcmp(pStr,".jpg")) {
						memset(cFileName,0,sizeof(cFileName));
						sprintf(cFileName,"%s/%s",psInfo->pcFilePath,ppsDirentry[lCnt]->d_name);
						vCOMM_LQ_Write(psInfo->psFileList,cFileName,AUTO_DETECT_STRLEN);	
						psInfo->lFileLen ++;
					}				
				}
				
				if (NULL != ppsDirentry[lCnt]) {
					free(ppsDirentry[lCnt]);
				}
				lCnt++;
			}
			
			free(ppsDirentry);
		}	

		psInfo->lState = BASE_MW_FILESORT_Valid;

	}

	BMF_FuncOut();

	return eRet;
}

/***************************************************************************
  * func : eBASE_MW_FILESORT_Write(sBASE_MW_FILESORT_Info* psInfo, const CHAR* pcFileName )
  * arg : sBASE_MW_FILESORT_Info* psInfo, const CHAR* pcFileName 
  * ret : eBASE_MW_FILESORT_RET
  * note : add a filename to the file list
***************************************************************************/
eBASE_MW_FILESORT_RET eBASE_MW_FILESORT_Write(sBASE_MW_FILESORT_Info* psInfo, const CHAR* pcFileName ) {

	eBASE_MW_FILESORT_RET eRet = BASE_MW_FILESORT_SUCCESS;

	BMF_FuncIn();

	{
		vCOMM_LQ_Write(psInfo->psFileList, (CHAR*)pcFileName,AUTO_DETECT_STRLEN);
		psInfo->lFileLen++;

	}

	BMF_FuncOut();

	return eRet;
}

/***************************************************************************
  * func : eBASE_MW_FILESORT_Read(sBASE_MW_FILESORT_Info* psInfo, CHAR* pcFileName )
  * arg : sBASE_MW_FILESORT_Info* psInfo, CHAR* pcFileName
  * ret : eBASE_MW_FILESORT_RET
  * note : read and remove the filename from the file list
***************************************************************************/
eBASE_MW_FILESORT_RET eBASE_MW_FILESORT_Read(sBASE_MW_FILESORT_Info* psInfo, CHAR* pcFileName ) {

	eBASE_MW_FILESORT_RET eRet = BASE_MW_FILESORT_SUCCESS;

	BMF_FuncIn();

	{	
		vCOMM_LQ_Read(psInfo->psFileList,&pcFileName);
		psInfo->lFileLen--;

	}

	BMF_FuncOut();

	return eRet;
}

/***************************************************************************
  * func : eBASE_MW_FILESORT_DeInit(sBASE_MW_FILESORT_Info* psInfo)
  * arg : sBASE_MW_FILESORT_Info* psInfo
  * ret : eBASE_MW_FILESORT_RET
  * note : deinit the file list
***************************************************************************/
eBASE_MW_FILESORT_RET eBASE_MW_FILESORT_DeInit(sBASE_MW_FILESORT_Info* psInfo) {

	eBASE_MW_FILESORT_RET eRet = BASE_MW_FILESORT_SUCCESS;

	BMF_FuncIn();

	{
		if (psInfo->lState != BASE_MW_FILESORT_Invalid) {
			vCOMM_LQ_DeInit(psInfo->psFileList);
			if (NULL != psInfo->pcFilePath) 
				free(psInfo->pcFilePath);			
		}

		psInfo->lState = BASE_MW_FILESORT_Invalid;
		psInfo->lIsEnough = BASE_MW_FILESORT_IsNoenough;

	}

	BMF_FuncOut();

	return eRet;
}

#ifdef __cplusplus
}
#endif
