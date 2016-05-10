#ifndef __COMM_UTIL_H__
#define __COMM_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************
	include
*********************************************************/
#include "comm_common.h"

/*********************************************************
	macro / enum
*********************************************************/
typedef enum {
	COMM_UTIL_SUCCESS,
	COMM_UTIL_FAIL,
	COMM_UTIL_FILENAME,
} eCOMM_UTIL_Ret;


/*********************************************************
	struct
*********************************************************/

/*********************************************************
	variable
*********************************************************/

/*********************************************************
	prototype
*********************************************************/
eCOMM_UTIL_Ret eCOMM_UTIL_GetFileName(CHAR* pcFileName, LONG lMaxFileName, CHAR* pcSuffix);
eCOMM_UTIL_Ret eCOMM_UTIL_GetFrameType(LONG* lIsISlice, UCHAR* pucHeader);
 eCOMM_UTIL_Ret eCOMM_UTIL_SaveCurTimeVal2file(const CHAR *pcFileName);
eCOMM_UTIL_Ret eCOMM_UTIL_LoadTimeValfile(const CHAR *pcFileName, FILE* fp, struct timeval *pTime) ;
eCOMM_UTIL_Ret eCOMM_UTIL_SaveCurCalendarTime2file(const CHAR *pcFileName) ;
eCOMM_UTIL_Ret eCOMM_UTIL_LoadTimeCalendarfile(const CHAR *pcFileName, FILE* fp, time_t *pTime);
#ifdef __cplusplus
}
#endif

#endif
