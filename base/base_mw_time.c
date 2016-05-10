#ifdef __cplusplus
extern "C" {
#endif

/********************************************************
	include
********************************************************/
#include "base_mw_time.h"
#include "comm_util.h"
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE  500 
#endif
#include <time.h>

/********************************************************
	def / macro
********************************************************/
// #define BASE_MW_TIME_DEBUG
#ifdef BASE_MW_TIME_DEBUG
#define BMT_Debug(fmt, arg...) fprintf(stdout, "[ BWT ] : %s() <%d> "fmt, __func__, __LINE__, ##arg)
#define BMT_FuncIn() fprintf(stdout, "[ BWT ] : %s() <%d> in\n", __func__, __LINE__)
#define BMT_FuncOut() fprintf(stdout, "[ BWT ] : %s() <%d> out\n", __func__, __LINE__)
#define BMT_iVal(iVal) fprintf(stdout, "[ BWT ] : %s() <%d> %s <%d> @ %p\n", __func__, __LINE__, #iVal, iVal, &iVal)
#define BMT_lVal(lVal) fprintf(stdout, "[ BWT ] : %s() <%d> %s <%ld> @ %p\n", __func__, __LINE__, #lVal, lVal, &lVal)
#define BMT_PtVal(PtVal) fprintf(stdout, "[ BWT ] : %s() <%d> pt %s <%p>\n", __func__, __LINE__, #PtVal, PtVal);
#else
#define BMT_Debug(fmt, arg...)
#define BMT_FuncIn()
#define BMT_FuncOut()
#define BMT_iVal(iVal)
#define BMT_lVal(lVal)
#define BMT_PtVal(PtVal)
#endif

/********************************************************
	struct
********************************************************/

/********************************************************
	variable
********************************************************/

/********************************************************
	prototype
********************************************************/
eBASE_MW_TIME_Ret eBASE_MW_TIME_GetSysCmd(const sBASE_MW_TIME_Info* psTimeInfo, CHAR* pcCmd);
eBASE_MW_TIME_Ret eBASE_MW_TIME_CheckTime(const sBASE_MW_TIME_Info* psTimeInfo);

/********************************************************
	function
********************************************************/

/*******************************************************
 * function : eBASE_MW_TIME_GetTime(sBASE_MW_TIME_Info* psTimeInfo, CHAR* pcTime)
 * argu : sBASE_MW_TIME_Info* psTimeInfo, CHAR* pcTime
 * ret : eBASE_MW_TIME_Ret
 * note : get time from char, YYYY:MM:DD:HH:MM:SS
 ******************************************************/
eBASE_MW_TIME_Ret eBASE_MW_TIME_GetTime(sBASE_MW_TIME_Info* psTimeInfo, CHAR* pcTime) {
	LONG* plStructHeader = NULL;
	LONG lTypeTmp = 0;
	CHAR* pcSectionTmp = NULL;
	CHAR cDelim[] = ":";
	
	BMT_FuncIn();

	{
		memset(psTimeInfo, 0, sizeof(sBASE_MW_TIME_Info));
		plStructHeader = (LONG*) psTimeInfo;

		pcSectionTmp = strtok(pcTime, cDelim);
		if(pcSectionTmp) {
			do {

				plStructHeader[lTypeTmp++] = atol(pcSectionTmp);
				pcSectionTmp = strtok(NULL, cDelim);

			} while(pcSectionTmp);
		}
	}

	BMT_FuncOut();
	
	return BASE_MW_TIME_SUCCESS;
}

/*******************************************************
 * function : eBASE_MW_TIME_SetTime(const sBASE_MW_TIME_Info* psTimeInfo)
 * argu : const sBASE_MW_TIME_Info* psTimeInfo
 * ret : eBASE_MW_TIME_Ret
 * note : set time 
 ******************************************************/
eBASE_MW_TIME_Ret eBASE_MW_TIME_SetTime(const sBASE_MW_TIME_Info* psTimeInfo) {
	eBASE_MW_TIME_Ret eRet = BASE_MW_TIME_SUCCESS;
	CHAR cCmd[128];
	
	BMT_FuncIn();

	{
		eRet = eBASE_MW_TIME_CheckTime(psTimeInfo);
		if(eRet != BASE_MW_TIME_SUCCESS) {
			return eRet;
		}
		
		eBASE_MW_TIME_GetSysCmd(psTimeInfo, cCmd);
		system(cCmd);
	}

	BMT_FuncOut();
	
	return eRet;
}

/*******************************************************
 * function : eBASE_MW_TIME_GetSysCmd(const sBASE_MW_TIME_Info* psTimeInfo, CHAR* pcCmd)
 * argu : const sBASE_MW_TIME_Info* psTimeInfo, CHAR* pcCmd
 * ret : eBASE_MW_TIME_Ret
 * note : get system command
 ******************************************************/
eBASE_MW_TIME_Ret eBASE_MW_TIME_GetSysCmd(const sBASE_MW_TIME_Info* psTimeInfo, CHAR* pcCmd) {

	BMT_FuncIn();

	{
		sprintf(pcCmd, "date -s %04ld.%02ld.%02ld-%02ld:%02ld:%02ld > /dev/null",
			psTimeInfo->lYear,
			psTimeInfo->lMonth,
			psTimeInfo->lDay,
			psTimeInfo->lHour,
			psTimeInfo->lMinute,
			psTimeInfo->lSecond);
		BMT_Debug("command <%s>\n", pcCmd);
	}

	BMT_FuncOut();

	return BASE_MW_TIME_SUCCESS;
}

/*******************************************************
 * function : eBASE_MW_TIME_CheckTime(const sBASE_MW_TIME_Info* psTimeInfo)
 * argu : const sBASE_MW_TIME_Info* psTimeInfo
 * ret : eBASE_MW_TIME_Ret
 * note : check the valid of time info
 ******************************************************/
eBASE_MW_TIME_Ret eBASE_MW_TIME_CheckTime(const sBASE_MW_TIME_Info* psTimeInfo) {
	eBASE_MW_TIME_Ret eRet = BASE_MW_TIME_FORMATERROR;

	if(
		(psTimeInfo->lDay >= 0 && psTimeInfo->lDay <= 31)
		&& (psTimeInfo->lHour >= 0 && psTimeInfo->lHour <= 24)
		&& (psTimeInfo->lMinute >= 0 && psTimeInfo->lMinute <= 60)
		&& (psTimeInfo->lMonth >= 0 && psTimeInfo->lMonth <= 12)
		&& (psTimeInfo->lSecond >= 0 && psTimeInfo->lSecond <= 60)
		&& (psTimeInfo->lYear >= 2014)
	) {
		eRet = BASE_MW_TIME_SUCCESS;
	}

	return eRet;
}

eBASE_MW_TIME_Ret eBASE_MW_TIME_ConfigTimeFromCmd(sBASE_MW_TIME_Info* psTimeInfo) {
	eBASE_MW_TIME_Ret eRet = BASE_MW_TIME_SUCCESS;
	struct timeval sTime;
	struct timeval scurTime, ssettime; 
	int idiffsec, idiffusec;
	FILE* fp = NULL;
	/* open time config file when first config */
	fp = fopen(BOOT_TIMESTAMP, "r");
	if(!fp) {
		/* use default time difference */
		BMT_Debug("create file %s failed!\n", BOOT_TIMESTAMP);
		idiffusec = 300000;
		idiffsec = 4;
	} else {
		struct timeval scurTime0;
		eCOMM_UTIL_LoadTimeValfile(NULL, fp, &sTime);
		
		BMT_Debug("----------read sec=%d\n usec=%d \n", sTime.tv_sec, sTime.tv_usec);
		fclose(fp);
		
		/* calculate differ time */
		gettimeofday(&scurTime0, NULL);
		idiffusec = ((scurTime0.tv_usec -sTime.tv_usec) > 0) ? 
			(scurTime0.tv_usec - sTime.tv_usec) : (scurTime0.tv_usec - sTime.tv_usec + 1000000);
		idiffsec = ((scurTime0.tv_usec - sTime.tv_usec) > 0) ? 
			(scurTime0.tv_sec - sTime.tv_sec) : (scurTime0.tv_sec - sTime.tv_sec - 1);
	}
	/* set system time */
	eRet = eBASE_MW_TIME_SetTime(psTimeInfo);	
	if(eRet != BASE_MW_TIME_SUCCESS) {
		printf("set time error! err <%ld> \n", (LONG)eRet);
		return BASE_MW_TIME_FAIL;
	}
	gettimeofday(&scurTime, NULL);
	//system("date");
	ssettime.tv_sec = scurTime.tv_sec + idiffsec + (scurTime.tv_usec + idiffusec) / 1000000;
	ssettime.tv_usec =   (scurTime.tv_usec + idiffusec) % 1000000;
	BMT_Debug("----------current sec=%d\n usec=%d \n", scurTime.tv_sec, scurTime.tv_usec);
	BMT_Debug("----------before second setttime sec=%d\n usec=%d \n", ssettime.tv_sec, ssettime.tv_usec);
	settimeofday(&ssettime, NULL);
	//system("date");

	return eRet;
}

/* compare time received from command and savefile time */
LONG lBASE_MW_TIME_CompareTime(sBASE_MW_TIME_Info* psTime, const CHAR* pcFileName)
{
	LONG ldifftime = 0;

	CHAR cTimeStr[64];
	struct tm tm;
	time_t sTime;
	time_t sTime2;

	memset(cTimeStr, 0, sizeof(cTimeStr));
	sprintf(cTimeStr, "%02ld/%02ld/%02ld %02ld:%02ld:%02ld", psTime->lMonth, psTime->lDay, (psTime->lYear)%100,
												psTime->lHour, psTime->lMinute, psTime->lSecond);
	//char *strptime(const char *s, const char *format, struct tm *tm);
	debug_info("-------time=%s----\n", cTimeStr);
	if(NULL != strptime(cTimeStr, "%x %X", &tm) ) {
		debug_info("y:%d m: %d d:%d h :%d  m: %d s:%d\n", tm.tm_year, tm.tm_mon, tm.tm_mday,
									tm.tm_hour, tm.tm_min, tm.tm_sec);
		sTime = mktime(&tm);
		/* load time file */
		eCOMM_UTIL_LoadTimeCalendarfile(pcFileName, NULL, &sTime2);
		ldifftime = difftime(sTime, sTime2);
		debug_info("differ time = %ld\n", ldifftime);

	}
	return ldifftime;
}

LONG lBASE_MW_TIME_CompareNowTime( const CHAR* pcFileName)
{
	LONG ldifftime = 0;
	time_t sTime;
	time_t sTime2;

	sTime = time(NULL);

	eCOMM_UTIL_LoadTimeCalendarfile(pcFileName, NULL, &sTime2);
	ldifftime = difftime(sTime, sTime2);
	debug_info("differ time = %ld\n", ldifftime);

	return ldifftime;
}



#ifdef __cplusplus
}
#endif
