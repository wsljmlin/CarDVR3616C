#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "comm_util.h"
#include <assert.h>

/***************************************************
	macro / enum
***************************************************/
//#define COMM_UTIL_DEBUG
#ifdef COMM_UTIL_DEBUG
#define CU_Debug(fmt, arg...) fprintf(stdout, "[ CU ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CU_Debug(fmt, arg...)
#endif

#define CU_FuncIn() CU_Debug("in\n")
#define CU_FuncOut() CU_Debug("out\n")
#define CU_iVal(iVal) CU_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CU_lVal(lVal) CU_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CU_PtVal(ptVal) CU_Debug("pt %s @ %p\n", #ptVal, ptVal)


/***************************************************
	variable
***************************************************/

/***************************************************
	prototype
***************************************************/

/***************************************************
	function
***************************************************/

/*********************************************
 * func : eCOMM_UTIL_GetFileName(CHAR* pcFileName, LONG lMaxFileName, CHAR* pcSuffix)
 * arg : CHAR* pcFileName, LONG lMaxFileName, CHAR* pcSuffix
 * ret : eCOMM_UTIL_Ret
 * note : get file name YYYYMMDDhhmmss.xxx
 *********************************************/
eCOMM_UTIL_Ret eCOMM_UTIL_GetFileName(CHAR* pcFileName, LONG lMaxFileName, CHAR* pcSuffix) {
	eCOMM_UTIL_Ret eRet = COMM_UTIL_SUCCESS;
	/* YYYYMMDDhhmmss.xxx*/
	const LONG lMAXFILENAME = 32;
	time_t sCurTime;
	struct tm* psCurParseTime;

	CU_FuncIn();

	{
		if(lMaxFileName < lMAXFILENAME) {
			CU_Debug("too small file name buffer <%ld>, we need at least %ld\n", lMaxFileName, lMAXFILENAME);
			return COMM_UTIL_FILENAME;
		}
		
		/* get current time */
		sCurTime = time(NULL);

		/* construct file name */
		psCurParseTime = localtime(&sCurTime);
		strftime(pcFileName, lMaxFileName, "%Y%m%d%H%M%S", psCurParseTime);
		strcat(pcFileName, pcSuffix);
		
		CU_Debug("filename : %s\n", pcFileName);

	}

	CU_FuncOut();

	return eRet;
}

/**************************************************************
 * func : eCOMM_UTIL_GetFrameType(LONG* lIsISlice, UCHAR* pucHeader)
 * arg : LONG* lIsISlice, UCHAR* pucHeader
 * ret : eCOMM_UTIL_Ret
 * note : get frame type, first skip prefix, then judge whether p
 *************************************************************/
eCOMM_UTIL_Ret eCOMM_UTIL_GetFrameType(LONG* lIsISlice, UCHAR* pucHeader) {
	eCOMM_UTIL_Ret eRet = COMM_UTIL_SUCCESS;
	*lIsISlice = 0;
	UCHAR* pucTmp = pucHeader;
	LONG lPrefixCnt = 0;

	CU_FuncIn();

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
		if((*pucTmp & 0x1f) != 1) {
			CU_Debug("found I Slice\n");
			*lIsISlice = 1;
		}
	}

	CU_FuncIn();

	return eRet;
}

/**************************************************************
 * func : eCOMM_UTIL_SaveCurCalendarTime2file(const CHAR *pcFileName)
 * arg : const CHAR *pcFileName
 * ret : eCOMM_UTIL_Ret
 * note : save current calendar time to file
 *************************************************************/
 eCOMM_UTIL_Ret eCOMM_UTIL_SaveCurCalendarTime2file(const CHAR *pcFileName) {
 	eCOMM_UTIL_Ret eRet = COMM_UTIL_SUCCESS;
	
	time_t sReceiveCfgTime;
	/* save time val */
	FILE* fp = NULL;
	CHAR pcbuf[64] = "";
	
	assert(pcFileName != NULL);
	/* get recive config command time */
	sReceiveCfgTime = time(NULL);
		
	fp = fopen(pcFileName, "w");
	if(!fp) {
		printf("create file %s failed!\n", pcFileName);
	} else {
		sprintf(pcbuf, "%s=%u","sec", (unsigned int)sReceiveCfgTime);
		//printf("-----time string:%s\n", pcbuf);
		fwrite(pcbuf,  strlen(pcbuf), 1, fp);
		fclose(fp);
	}

	return eRet;
 }
/**************************************************************
 * func : eCOMM_UTIL_SaveCurCalendarTime2file(const CHAR *pcFileName, FILE* fp, time_t *pTime)
 * arg : const CHAR *pcFileName, FILE* fp, time_t *pTime
 * ret : eCOMM_UTIL_Ret
 * note : load calendtar time from file(file handler or filename)
 *************************************************************/
eCOMM_UTIL_Ret eCOMM_UTIL_LoadTimeCalendarfile(const CHAR *pcFileName, FILE* fp, time_t *pTime) {
 	eCOMM_UTIL_Ret eRet = COMM_UTIL_SUCCESS;
	LONG lTime = 0;
	
 	assert(pTime != NULL);
	
	FILE* fplocal = NULL;
	if(NULL == fp) {
		assert(pcFileName != NULL);
		fplocal = fopen(pcFileName, "r");
		if(!fplocal) {
			printf("open file %s failed!\n", pcFileName);
			return COMM_UTIL_FAIL;
		}
		fscanf(fplocal, "sec=%ld",&lTime);
		//printf("----------read sec=%d\n usec=%d \n", isec, iusec);
		fclose(fplocal);
		
	} else {
		fscanf(fp, "sec=%ld",& lTime);
	}
	*pTime = (time_t)lTime;

	return eRet;
 }

/**************************************************************
 * func : eCOMM_UTIL_SaveCurCalendarTime2file(const CHAR *pcFileName)
 * arg : const CHAR *pcFileName
 * ret : eCOMM_UTIL_Ret
 * note : save current time timeval format time to file
 *************************************************************/
 eCOMM_UTIL_Ret eCOMM_UTIL_SaveCurTimeVal2file(const CHAR *pcFileName) {
 	eCOMM_UTIL_Ret eRet = COMM_UTIL_SUCCESS;
	
	struct timeval sReceiveCfgTime;
	/* save time val */
	FILE* fp = NULL;
	CHAR pcbuf[64] = "";
	
	assert(pcFileName != NULL);
	/* get recive config command time */
	gettimeofday(&sReceiveCfgTime, NULL);
		
	fp = fopen(pcFileName, "w");
	if(!fp) {
		printf("create file %s failed!\n", pcFileName);
	} else {
		sprintf(pcbuf, "%s=%u\n%s=%u","sec", (unsigned int)sReceiveCfgTime.tv_sec, "usec", (unsigned int)sReceiveCfgTime.tv_usec);
		//printf("-----time string:%s\n", pcbuf);
		fwrite(pcbuf,  strlen(pcbuf), 1, fp);
		fclose(fp);
	}

	return eRet;
 }
/**************************************************************
 * func : eCOMM_UTIL_SaveCurCalendarTime2file(const CHAR *pcFileName, FILE* fp, time_t *pTime)
 * arg : const CHAR *pcFileName, FILE* fp, time_t *pTime
 * ret : eCOMM_UTIL_Ret
 * note : load  timeval format time from file(file handler or filename)
 *************************************************************/
eCOMM_UTIL_Ret eCOMM_UTIL_LoadTimeValfile(const CHAR *pcFileName, FILE* fp, struct timeval *pTime) {
 	eCOMM_UTIL_Ret eRet = COMM_UTIL_SUCCESS;
	
 	assert(pTime != NULL);
	FILE* fplocal = NULL;

	if(NULL == fp) {
		assert(pcFileName != NULL);
		fplocal = fopen(pcFileName, "r");
		if(!fplocal) {
			printf("open file %s failed!\n", pcFileName);
			return COMM_UTIL_FAIL;
		}
		fscanf(fplocal, "sec=%d\nusec=%d",(int *)&pTime->tv_sec, (int *)&pTime->tv_usec);
		//printf("----------read sec=%d\n usec=%d \n", isec, iusec);
		fclose(fplocal);
	} else {
		fscanf(fp, "sec=%d\nusec=%d",(int *)&pTime->tv_sec, (int *)&pTime->tv_usec);
	}
	return eRet;
 }

#ifdef __cplusplus
}
#endif
