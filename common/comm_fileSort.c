#ifdef __cplusplus
extern "C"{
#endif

/***************************************************
	include
***************************************************/
#include "comm_fileSort.h"
#include "assert.h"

/***************************************************
	macro / enum
***************************************************/
//#define COMM_FILESORT_DEBUG
#ifdef COMM_FILESORT_DEBUG
#define CF_Debug(fmt, arg...) fprintf(stdout, "[ CFSORT ] : %s() <%d> "fmt, __func__, __LINE__, ##arg)
#else
#define CF_Debug(fmt, arg...)
#endif

#define CF_FuncIn() //CF_Debug("in\n")
#define CF_FuncOut() //CF_Debug("out\n")

#define CF_iVal(ival) CF_Debug("%s <%d>\n", #ival, ival);
#define CF_lVal(lval) CF_Debug("%s <%ld>\n", #lval, lval);
#define CF_ptVal(ptval) CF_Debug("%s <%p>\n", #ptval, ptval);

/***************************************************
	struct
***************************************************/

/***************************************************
	variable
***************************************************/
/* A mutex protecting filesort */
pthread_mutex_t pFileSort_mutex	= PTHREAD_MUTEX_INITIALIZER;

/***************************************************
	prototype
***************************************************/
void vFileSortShowEntry(void* pvEntry);
LONG lFileSortIsKeyOfValue(void* pvName, void* pvEntry);
int iCOMM_FILESORT_SortByTime(const struct dirent ** pvFileSrc, const struct dirent ** pvFileTgt);

/***************************************************
	function
***************************************************/
void vFileSortShowEntry(void* pvEntry) {
	sCOMM_FILESORT_UserEntry* psEntry = (sCOMM_FILESORT_UserEntry*) pvEntry;
	//CF_Debug("FileName:%-8s\n",psEntry->cFileName);
}

LONG lFileSortIsKeyOfValue(void* pvName, void* pvEntry) {
	LONG lIsKeyOfValue = 0;
	CHAR* pcName = (CHAR*) pvName;
	sCOMM_FILESORT_UserEntry* psEntry = (sCOMM_FILESORT_UserEntry*) pvEntry;

	if(!strcmp(pcName, psEntry->cFileName)) {
		return 1;
	}

	return lIsKeyOfValue;
}

/***************************************************************************
  * func : lBASE_MW_FILESORT_SortByTime(const struct dirent ** pvFileSrc, const struct dirent ** pvFileTgt)
  * arg : const struct dirent ** pvFileSrc, const struct dirent ** pvFileTgt
  * ret : LONG
  * note : sort files by time
***************************************************************************/
int iCOMM_FILESORT_SortByTime(const struct dirent ** pvFileSrc, const struct dirent ** pvFileTgt) {
	const struct dirent ** sFileSrc = NULL;
	const struct dirent ** sFileTgt = NULL;
	
	CF_FuncIn();

	{
		
		struct stat sSrcFileStat;
    		struct stat sTgtFileStat;

		sFileSrc =  pvFileSrc;
		sFileTgt =  pvFileTgt;

		if(stat((*sFileSrc)->d_name, &sSrcFileStat) == -1) {
			perror("stat : ");
			printf("Stat function return error handle file <%s>!\n", (*sFileSrc)->d_name);
			/* need to handle  */
			CF_FuncOut();
			return -1;
		}
		if(stat((*sFileTgt)->d_name, &sTgtFileStat) == -1) {
			perror("stat : ");
			printf("Stat function return error handle file <%s>!\n", (*sFileSrc)->d_name);
			/* need to handle */
			CF_FuncOut();
			return -1;
		}
		
		if(sSrcFileStat.st_mtime > sTgtFileStat.st_mtime) {
			CF_FuncOut();
			return 1;
		} else if (sSrcFileStat.st_mtime < sTgtFileStat.st_mtime) {
			CF_FuncOut();
			return -1;
		} else {
			CF_FuncOut();
			return 0;
		}

	}

	CF_FuncOut();
}


/***************************************************************************
  * func : eCOMM_FILESORT_Ret eCOMM_FILESORT_Init(sCOMM_FILESORT_Info* psInfo) 
  * arg : (sCOMM_FILESORT_Info* psInfo)
  * ret : eCOMM_FILESORT_Ret
  * note : init the list resource
***************************************************************************/
eCOMM_FILESORT_Ret eCOMM_FILESORT_Init(sCOMM_FILESORT_Info* psInfo) {
	eCOMM_FILESORT_Ret eRet = COMM_FILESORT_SUCCESS;

	CF_FuncIn();

	{
		psInfo->lState = COMM_FILESORT_Invalid;
		
		/* init the file list */
		psInfo->psFileNameList = (sCOMM_LIST_Info*)malloc(sizeof(sCOMM_LIST_Info));
		eCOMM_LIST_Init(psInfo->psFileNameList, lFileSortIsKeyOfValue, vFileSortShowEntry, sizeof(sCOMM_FILESORT_UserEntry));

	}

	CF_FuncOut();

	return eRet;	
}

/***************************************************************************
  * func : eCOMM_FILESORT_Creat eCOMM_FILESORT_Init(sCOMM_FILESORT_Info* psInfo, const CHAR* pcFilePath) 
  * arg : sCOMM_FILESORT_Info* psInfo, const CHAR* pcFilePath
  * ret : eCOMM_FILESORT_Ret
  * note : init the file list
***************************************************************************/
eCOMM_FILESORT_Ret eCOMM_FILESORT_Creat(sCOMM_FILESORT_Info* psInfo, const CHAR* pcFilePath) {
	eCOMM_FILESORT_Ret eRet = COMM_FILESORT_SUCCESS;
	struct dirent **ppsDirentry = NULL;
	sCOMM_FILESORT_UserEntry sFileNameEntry;
	LONG lCnt = 0;
	CHAR *pStr = NULL;
	CHAR cCurrentDir[128];
	//CHAR cTmpDir[128];
	LONG lFileScanLen;
	
	CF_FuncIn();

	{
		psInfo->lState = COMM_FILESORT_Invalid;
		/* init the file list */
		psInfo->pcFilePath = strdup(pcFilePath);
		psInfo->lFileLen = 0;
		
		CF_Debug("mutex lock enter !\n");
		pthread_mutex_lock(&pFileSort_mutex);	
		
		getcwd(cCurrentDir,sizeof(cCurrentDir));
		//CF_Debug("Current Dir:%s\n",cCurrentDir);
		chdir(psInfo->pcFilePath);
		//getcwd(cTmpDir,sizeof(cTmpDir));
		//CF_Debug("Tmp Dir:%s\n",cTmpDir);

		lFileScanLen = 0;
		lFileScanLen = scandir(psInfo->pcFilePath, &ppsDirentry, 0, iCOMM_FILESORT_SortByTime);
		
		chdir(cCurrentDir);
		//getcwd(cTmpDir,sizeof(cTmpDir));	
		//CF_Debug("Tmp Dir:%s\n",cTmpDir);
		
		pthread_mutex_unlock(&pFileSort_mutex);	
		CF_Debug("mutex lock exit !\n");
		
		if (lFileScanLen < 0) {
			CF_Debug("Scandir function return error\n");
			/* need to handle ?? */
			eCOMM_LIST_Reset(psInfo->psFileNameList);
			if (NULL != psInfo->pcFilePath) 
				free(psInfo->pcFilePath);				
			eRet = COMM_FILESORT_CREATE;
			
			return eRet;

		} else {
			while (lCnt < lFileScanLen) {
				pStr = strrchr(ppsDirentry[lCnt]->d_name, '.');
				if (NULL != pStr) {
					if(0 == strcmp(pStr,".ts") || 0 == strcmp(pStr,".h264") || 0 == strcmp(pStr,".adpcm") || 0 == strcmp(pStr,".jpg")) {
						memset(&sFileNameEntry, 0, sizeof(sFileNameEntry));
						sprintf(sFileNameEntry.cFileName, "%s%s",psInfo->pcFilePath, ppsDirentry[lCnt]->d_name);
						eCOMM_LIST_Create(psInfo->psFileNameList, (void*)&sFileNameEntry);
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

		psInfo->lState = COMM_FILESORT_Valid;
		CF_Debug("psInfo->lFileLen=%ld\n",psInfo->lFileLen);

	}

	CF_FuncOut();

	return eRet;
}

/***************************************************************************
  * func : eCOMM_FILESORT_Delete(sCOMM_FILESORT_Info* psInfo, CHAR* pcFileName)
  * arg : sCOMM_FILESORT_Info* psInfo, CHAR* pcFileName
  * ret : eCOMM_FILESORT_Ret
  * note : delete the entry from the file list
***************************************************************************/
eCOMM_FILESORT_Ret eCOMM_FILESORT_Delete(sCOMM_FILESORT_Info* psInfo, CHAR* pcFileName) {
	eCOMM_FILESORT_Ret eRet = COMM_FILESORT_SUCCESS;
	sCOMM_FILESORT_UserEntry sFileNameEntry;
	
	CF_FuncIn();

	{
		strcpy(sFileNameEntry.cFileName, pcFileName);
		eRet = eCOMM_LIST_Delete(psInfo->psFileNameList, (void*)&sFileNameEntry);
		if(eRet == COMM_FILESORT_SUCCESS) {	
			psInfo->lFileLen --;
			CF_Debug("delete the list name: %s\n", sFileNameEntry.cFileName);
		} else {
			CF_Debug("can not delete the list name: %s\n", sFileNameEntry.cFileName);	
			return COMM_FILESORT_DELETE;		
		}		

	}

	CF_FuncOut();

	return eRet;
}

/***************************************************************************
  * func : eCOMM_FILESORT_GetCount(sCOMM_FILESORT_Info* psInfo, LONG* plCount)
  * arg : sCOMM_FILESORT_Info* psInfo, LONG* plCount
  * ret : eCOMM_FILESORT_Ret
  * note : get the length of the file list 
***************************************************************************/
eCOMM_FILESORT_Ret eCOMM_FILESORT_GetCount(sCOMM_FILESORT_Info* psInfo, LONG* plCount) {
	eCOMM_FILESORT_Ret eRet = COMM_FILESORT_SUCCESS;

	CF_FuncIn();

	{
		eCOMM_LIST_GetCount(psInfo->psFileNameList, plCount);
		CF_Debug("list count=%ld,file len=%ld\n",*plCount,psInfo->lFileLen);
	}

	CF_FuncOut();

	return eRet;
}

/***************************************************************************
  * func : eCOMM_FILESORT_Reset(sCOMM_FILESORT_Info* psInfo)
  * arg : sCOMM_FILESORT_Info* psInfo
  * ret : eCOMM_FILESORT_Ret
  * note : reset the file list
***************************************************************************/
eCOMM_FILESORT_Ret eCOMM_FILESORT_Reset(sCOMM_FILESORT_Info* psInfo) {
	eCOMM_FILESORT_Ret eRet = COMM_FILESORT_SUCCESS;

	CF_FuncIn();

	{
		eCOMM_LIST_Reset(psInfo->psFileNameList);
	}

	CF_FuncOut();

	return eRet;
}

/***************************************************************************
  * func : eCOMM_FILESORT_Write(sCOMM_FILESORT_Info* psInfo, const CHAR* pcFileName )
  * arg : sCOMM_FILESORT_Info* psInfo, const CHAR* pcFileName 
  * ret : eCOMM_FILESORT_Ret
  * note : add a filename to the file list
***************************************************************************/
eCOMM_FILESORT_Ret eCOMM_FILESORT_Write(sCOMM_FILESORT_Info* psInfo, const CHAR* pcFileName ) {
	eCOMM_FILESORT_Ret eRet = COMM_FILESORT_SUCCESS;
	sCOMM_FILESORT_UserEntry sFileNameEntry;

	CF_FuncIn();

	{
		memset(&sFileNameEntry, 0, sizeof(sFileNameEntry));
		strcpy(sFileNameEntry.cFileName, pcFileName);
		eRet = eCOMM_LIST_Create(psInfo->psFileNameList, (void*)&sFileNameEntry);
		if(eRet == COMM_FILESORT_SUCCESS) {	
			psInfo->lFileLen ++;
			eCOMM_LIST_ShowEntry(psInfo->psFileNameList);
			//printf("add new file:%s\n", sFileNameEntry.cFileName);
			CF_Debug("add new file:%s\n", sFileNameEntry.cFileName);
			printf("\n");
		} else {
			CF_Debug("can not add new file: %s\n", sFileNameEntry.cFileName);	
			return COMM_FILESORT_WRITE;		
		}
		
	}

	CF_FuncOut();

	return eRet;
}

/***************************************************************************
  * func : eCOMM_FILESORT_Read(sCOMM_FILESORT_Info* psInfo, CHAR* pcListKey, CHAR* pcFileName)
  * arg : sCOMM_FILESORT_Info* psInfo, CHAR* pcListKey, CHAR* pcFileName
  * ret : eCOMM_FILESORT_Ret
  * note : read the filename from the file list
***************************************************************************/
eCOMM_FILESORT_Ret eCOMM_FILESORT_Read(sCOMM_FILESORT_Info* psInfo, eCOMM_LIST_RetrieveKey eKey, CHAR* pcFileName) {
	eCOMM_FILESORT_Ret eRet = COMM_FILESORT_SUCCESS;

	CF_FuncIn();

	{	
		sCOMM_FILESORT_UserEntry sEntryFind;
		memset(&sEntryFind, 0, sizeof(sCOMM_FILESORT_UserEntry));
		switch (eKey) {
			case COMM_LIST_RETRIEVEKEY_FIRST:
				eCOMM_LIST_ShowEntry(psInfo->psFileNameList);
				break;
			case COMM_LIST_RETRIEVEKEY_LAST:
				break;
			default:
				eRet = COMM_LIST_RETRIEVEKEY;
		}		
		
		eRet = eCOMM_LIST_RetrieveSpecial(psInfo->psFileNameList, eKey, (void*)&sEntryFind);	
		if(eRet == COMM_FILESORT_SUCCESS) {
			//CF_Debug("entry name: %s\n", sEntryFind.cFileName);	
			strcpy(pcFileName, sEntryFind.cFileName);
			//CF_Debug("find entry : name %s\n", pcFileName);	
		} else {		
			CF_Debug("can not find entry key\n");	
			return COMM_FILESORT_READ;
		}

	}

	CF_FuncOut();

	return eRet;
}

/***************************************************************************
  * func : eCOMM_FILESORT_DeInit(sCOMM_FILESORT_Info* psInfo)
  * arg : sCOMM_FILESORT_Info* psInfo
  * ret : eCOMM_FILESORT_Ret
  * note : deinit the file list
***************************************************************************/
eCOMM_FILESORT_Ret eCOMM_FILESORT_DeInit(sCOMM_FILESORT_Info* psInfo) {
	eCOMM_FILESORT_Ret eRet = COMM_FILESORT_SUCCESS;

	CF_FuncIn();

	{
		if((NULL == psInfo)) {
			return eRet;	
		}
		
		if ( (psInfo->lState != COMM_FILESORT_Invalid)) {
			eCOMM_LIST_Deinit(psInfo->psFileNameList);
			
			if (psInfo->pcFilePath) {
				free(psInfo->pcFilePath);	
				psInfo->pcFilePath = NULL;
			}
			
			if(psInfo->psFileNameList) {
				free(psInfo->psFileNameList);
				psInfo->psFileNameList = NULL;
			}		

			psInfo->lState = COMM_FILESORT_Invalid;
		}		
	}
	
	CF_FuncOut();

	return eRet;		
}

#ifdef __cplusplus
}
#endif
