#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************
	include
**********************************************************************************/
#define _GNU_SOURCE
#include "base_mw_sd.h"
#include <fcntl.h>

/*********************************************************************************
	macro
**********************************************************************************/

// #define SD_DEBUG
#ifdef SD_DEBUG
#define debug_log(fmt, args...) fprintf(stdout, "[ SD MW ] : "fmt, ##args)
#define debug_log_funcIn() fprintf(stdout, "in %s\n", __func__)
#define debug_log_funcOut() fprintf(stdout, "out %s\n", __func__)
#define debug_log_showErr(err) fprintf(stderr, "%s <%d> : Error <%d>\n", __func__, __LINE__, err)
#else
#define debug_log(fmt, args...)
#define debug_log_funcIn()
#define debug_log_funcOut()
#define debug_log_showErr(err)
#endif

#define BMS_iVal(ival) debug_log("%s <%d>\n", #ival, ival)
#define BMS_lVal(lval) debug_log("%s <%ld>\n", #lval, lval)
#define BMS_ptVal(ptval) debug_log("%s <%p>\n", #ptval, ptval)

/*********************************************************************************
	variable
**********************************************************************************/

/*********************************************************************************
	prototype
**********************************************************************************/
eBASE_MW_SD_ERROR eBASE_MW_SD_CHECKEXIST(sBASE_MW_SD_STAT *psSDStat);
eBASE_MW_SD_ERROR eBASE_SD_MW_MKWORKDIR(sBASE_MW_SD_STAT *psSDStat);

/*********************************************************************************
	function
**********************************************************************************/

/*******************************************************************************
* @brief		: eBASE_MW_SD_CHECK(sBASE_MW_SD_STAT *psSDStat)
* @param	: sBASE_MW_SD_STAT *psSDStat
* @retval		: eBASE_MW_SD_ERROR
* @note		: check whether sd exist,if exit create work dirctory
*******************************************************************************/

eBASE_MW_SD_ERROR eBASE_MW_SD_CHECK(sBASE_MW_SD_STAT *psSDStat)
{
	eBASE_MW_SD_ERROR eRet = BASE_MW_SD_SUCCESS;
	debug_log_funcIn();

	/*****************************************/
	{
		/* sd check */	
		eRet = eBASE_MW_SD_CHECKEXIST(psSDStat);
		if(eRet != BASE_MW_SD_SUCCESS) {
			debug_log_funcOut();
			return eRet;
		}
		
		/* mkdir workdir */
		eRet = eBASE_SD_MW_MKWORKDIR(psSDStat);
		if(eRet != BASE_MW_SD_SUCCESS) {
			debug_log_funcOut();
			return eRet;
		}
	}
	/*****************************************/

	debug_log_funcOut();
	return eRet;
}

/*******************************************************************************
* @brief		: eBASE_MW_SD_CHECKEXIST(sBASE_MW_SD_STAT *psSDStat)
* @param	: sBASE_MW_SD_STAT *psSDStat
* @retval		: eBASE_MW_SD_ERROR
* @note		: check whether sd exist
*******************************************************************************/
eBASE_MW_SD_ERROR eBASE_MW_SD_CHECKEXIST(sBASE_MW_SD_STAT *psSDStat)
{
	eBASE_MW_SD_ERROR eRet = BASE_MW_SD_SUCCESS;
	debug_log_funcIn();

#if 1	
	FILE *pMntPath;
	CHAR cMnt[256], cDev[256], cFs[256];
	LONG lFindPath;

	/* get mount point path */
	pMntPath = fopen("/proc/mounts", "r");
	if (pMntPath == NULL) {
		printf("mount infomation open failed!\n");
		eRet = BASE_MW_SD_MOUNTFAIL;
		debug_log_funcOut();
		return eRet;
	}
	lFindPath = 0;
	while (fgets(cMnt, 256, pMntPath)) {
		sscanf(cMnt, "%[^ ] %[^ ] %*s\n", cDev, cFs);
		//printf("cDev:%s;cFs:%s\n",cDev,cFs);
		if (0 == strcmp(psSDStat->cDevName,NANDFLASH)) {
			if (strstr(cDev, "mtdblo") != NULL) {
				printf("Mount Info:%s\n",cDev);
				lFindPath = 1;
				break;
			}				
		} else if (0 == strcmp(psSDStat->cDevName,TFCARD)) {
			if (strstr(cDev, "mmcblk") != NULL) {
				printf("Mount Info:%s\n",cDev);
				lFindPath = 1;
				break;
			}			
		}

	}
	fclose(pMntPath);	
	
	/* check whether sd card path exist */
	if(0 == lFindPath)
	{
		eRet = BASE_MW_SD_MOUNTFAIL;
		psSDStat->eSDMount = BASE_MW_SD_NOTMOUNT;
		strcpy(psSDStat->cMountPoint,"");
		strcpy(psSDStat->cDevName,"");
		//debug_log(" SD mount error!\n");
		debug_log_funcOut();
		return eRet;
	}
	psSDStat->eSDMount = BASE_MW_SD_MOUNT;
	if (0 == strcmp(psSDStat->cDevName,NANDFLASH)) {
		strcpy(psSDStat->cMountPoint,BASE_MW_NAND_PATH);
	} else if (0 == strcmp(psSDStat->cDevName,TFCARD)) {
		strcpy(psSDStat->cMountPoint,BASE_MW_SD_PATH);
	}
	//printf("Mount point:%s\n",psSDStat->cMountPoint);		
#endif	

	debug_log_funcOut();
	return eRet;
}
/*******************************************************************************
* @brief		: eBASE_MW_SD_ERROR eBASE_SD_MW_MKWORKDIR(sBASE_MW_SD_STAT *psSDStat)
* @param	: sBASE_MW_SD_STAT *psSDStat
* @retval		: eBASE_MW_SD_ERROR
* @note		: create sd work directory
*******************************************************************************/
eBASE_MW_SD_ERROR eBASE_SD_MW_MKWORKDIR(sBASE_MW_SD_STAT *psSDStat)
{
	eBASE_MW_SD_ERROR eRet = BASE_MW_SD_SUCCESS;
	
	debug_log_funcIn();

	/*****************************************/
	{
		if(BASE_MW_SD_MOUNT != psSDStat->eSDMount)
		{
			eRet = BASE_MW_SD_MOUNTFAIL;
			return eRet;
		}

		if (0 == strcmp(psSDStat->cMountPoint,BASE_MW_SD_PATH)) {
			if(access(BASE_MW_SD_MAINCARDPATH,F_OK))
			{
				eRet = system("mkdir -p /opt/mainCard/rec/");
			}	
			if(access(BASE_MW_SD_SNAPPATH,F_OK))
			{
				eRet = system("mkdir -p /opt/mainCard/snapshot/");
			}			
		} else if (0 == strcmp(psSDStat->cMountPoint,BASE_MW_NAND_PATH)) {
			if(access(BASE_MW_SD_SUBCARDPATH,F_OK))
			{
				eRet = system("mkdir -p /opt/subCard/");
			}		
		}
	}
	/*****************************************/

	debug_log_funcOut();
	
	return eRet;
}

/*******************************************************************************
* @brief		: eBASE_MW_SD_ERROR eBASE_SD_MW_MKWORKDIR(sBASE_MW_SD_STAT *psSDStat)
* @param	: sBASE_MW_SD_STAT *psSDStat
* @retval		: eBASE_MW_SD_ERROR
* @note		: create sd work directory
*******************************************************************************/
eBASE_MW_SD_ERROR eBASE_SD_MW_CHECKSTATUS(sBASE_MW_SD_STAT *psSDStat)
{
	eBASE_MW_SD_ERROR eRet = BASE_MW_SD_SUCCESS;
	struct statfs sStatFS;
	
	debug_log_funcIn();

	/*****************************************/
	{
		if(BASE_MW_SD_MOUNT != psSDStat->eSDMount)
		{
			eRet = BASE_MW_SD_MOUNTFAIL;
			return eRet;
		}
		
		if(statfs(psSDStat->cMountPoint, &sStatFS) == -1)
		{
			//debug_log("statfs failed for path->[%s]\n", psSDStat->cMountPoint);
			printf("statfs failed for path->[%s]\n", psSDStat->cMountPoint);
			eRet = BASE_MW_SD_GETSTATERR;
			return eRet;
		}

		psSDStat->lTotalKB = (((LONG)sStatFS.f_frsize)>>10) * ((LONG)sStatFS.f_blocks);
		psSDStat->lFreeKB = (((LONG)sStatFS.f_frsize>>10) * ((LONG)sStatFS.f_bfree));
		psSDStat->lUsedKB = (LONG)(psSDStat->lTotalKB - psSDStat->lFreeKB);

		//printf("Device[%s]:Total %ld Kb,used %ld Kb,avable %ld MB\n",psSDStat->cDevName,
		//	psSDStat->lTotalKB,psSDStat->lUsedKB,BASE_MW_SD_KB2MB(psSDStat->lFreeKB));
	}
	/*****************************************/

	debug_log_funcOut();
	
	return eRet;
}

/*******************************************************************************
* @brief		: eBASE_SD_MW_REMOVEFILE(CHAR* pcPath,CHAR* pcPostfix,LONG lFileNum)
* @param	: CHAR* pcPath,CHAR* pcPostfix,LONG lFileNum
* @retval		: eBASE_MW_SD_ERROR
* @note		: rmove file
*******************************************************************************/
eBASE_MW_SD_ERROR eBASE_SD_MW_REMOVEFILE(CHAR* pcPath,CHAR* pcPostfix,LONG lFileNum)
{
	eBASE_MW_SD_ERROR eRet = BASE_MW_SD_SUCCESS;
#if 1
	CHAR cOldFile[64];
	//DIR Dirp;
	DIR* pDirp = NULL;
	struct dirent sDirent;
	struct dirent* pDirent = &sDirent;
	LONG lFileCnt = 0;
	
	/* check whether the dirctory have file */
	pDirp = opendir(pcPath);
	while(pDirp)
	{
		/* the first and second file is "." and "..",
		  the third file is the true file */
		pDirent = readdir(pDirp);
		if(NULL != pDirent)
		{
			lFileCnt++;
			/* every time only read one file */
			if(lFileCnt>2)
			{
				sprintf(cOldFile,"%s%s",pcPath,pDirent->d_name);
				debug_log("oldest file:%s\n",cOldFile);
				break;
			}
		}
		else
		{
			break;
		}
	}
	closedir(pDirp);
	
	if(2 == lFileCnt)
	{
		eRet = BASE_MW_SD_HAVENOFILE;
		debug_log("have no file\n");
		return eRet;
	}

	eRet = remove(cOldFile);
	if(eRet)
	{	
		debug_log("remove file %s fail,eRet=%d!\n",cOldFile,eRet);
		perror("Error to remove file");
		eRet = BASE_MW_SD_REMOVEFILE;
	}
#endif
	return eRet;
}

/*******************************************************************************
* @brief		: eBASE_SD_MW_GETSPACE(sBASE_MW_SD_STAT *psSDStat, CHAR* pcPath, CHAR* pcPostfix, LONG lFileNum)
* @param		: 
* @retval		: eBASE_MW_SD_ERROR
* @note		: get space
*******************************************************************************/
eBASE_MW_SD_ERROR eBASE_SD_MW_GETSPACE(sBASE_MW_SD_STAT *psSDStat, CHAR* pcPath, CHAR* pcPostfix, LONG lFileNum) {
	eBASE_MW_SD_ERROR eRet = BASE_MW_SD_SUCCESS;

	debug_log_funcIn();

	{
		strcpy(psSDStat->cDevName,pcPath);
		eRet = eBASE_MW_SD_CHECK(psSDStat);
		if(eRet != BASE_MW_SD_SUCCESS) {
			debug_log_funcOut();
			return eRet;
		}

		eRet = eBASE_SD_MW_CHECKSTATUS(psSDStat);
		if(eRet != BASE_MW_SD_SUCCESS) {
			debug_log_funcOut();
			return eRet;
		}
		
		BMS_lVal(psSDStat->lFreeKB - BASE_MW_SD_FILE_TEST);
	}

	debug_log_funcOut();

	return eRet;
}

/*******************************************************************************
* @brief		: eBASE_SD_MW_CheckSpace(CHAR* pcPath, LONG lSpaceThreshold)
* @param	: CHAR* pcPath, LONG lSpaceThreshold
* @retval		: eBASE_MW_SD_ERROR
* @note		: check space
*******************************************************************************/
eBASE_MW_SD_ERROR eBASE_SD_MW_CheckSpace(CHAR* pcPath, LONG lSpaceThreshold) {
	eBASE_MW_SD_ERROR eRet = BASE_MW_SD_SUCCESS;
	sBASE_MW_SD_STAT sSDStat;

	debug_log_funcIn();

	{
		/* judge the path whether is valid */
		if ((0 != strcmp(pcPath, NANDFLASH)) && (0 != strcmp(pcPath, TFCARD))) {
			eRet = BASE_MW_SD_HAVENOPATH;

			debug_log_funcOut();
			return eRet; 			
		}  

		/* get the space */
		eRet = eBASE_SD_MW_GETSPACE(&sSDStat, pcPath, ".h264", 1);
		if(eRet != BASE_MW_SD_SUCCESS) {
			debug_log("get space error <%d>\n", eRet);
			
			debug_log_funcOut();
			return eRet;					
		} 

		/* judge the free space whether is enough */
		if(sSDStat.lFreeKB <= lSpaceThreshold) {
			eRet = BASE_MW_SD_SPACENOTENOUGH;

			debug_log_funcOut();
			return eRet;
		}		
	}

	debug_log_funcOut();

	return eRet;	
			
}

#ifdef __cplusplus
}
#endif

