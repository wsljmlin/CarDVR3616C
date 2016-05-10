#ifndef __BASE_MW_TIME_H__
#define __BASE_MW_TIME_H__

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************
	include
********************************************************/
#include "comm_common.h"

/********************************************************
	macro / enum
********************************************************/
typedef enum {
	BASE_MW_TIME_SUCCESS,
	BASE_MW_TIME_FORMATERROR,
	BASE_MW_TIME_FAIL,
} eBASE_MW_TIME_Ret;

/********************************************************
	struct
********************************************************/
typedef struct {
	LONG lYear;
	LONG lMonth;
	LONG lDay;
	LONG lHour;
	LONG lMinute;
	LONG lSecond;
} sBASE_MW_TIME_Info;

/********************************************************
	variable
********************************************************/
sBASE_MW_TIME_Info gsTime;

/********************************************************
	prototype
********************************************************/
eBASE_MW_TIME_Ret eBASE_MW_TIME_GetTime(sBASE_MW_TIME_Info* psTimeInfo, CHAR* pcTime);
eBASE_MW_TIME_Ret eBASE_MW_TIME_SetTime(const sBASE_MW_TIME_Info* psTimeInfo);
eBASE_MW_TIME_Ret eBASE_MW_TIME_ConfigTimeFromCmd(sBASE_MW_TIME_Info* psTimeInfo) ;
LONG lBASE_MW_TIME_CompareTime(sBASE_MW_TIME_Info* psTime, const CHAR* pcFileName);
LONG lBASE_MW_TIME_CompareNowTime( const CHAR* pcFileName);

#ifdef __cplusplus
}
#endif

#endif
