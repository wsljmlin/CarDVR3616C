#ifndef __COMM_STRING_H__
#define __COMM_STRING_H__

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************
	include
****************************************************/
#include "comm_common.h"

/****************************************************
	macro / define
****************************************************/

/****************************************************
	enum / struct
****************************************************/
typedef enum {
	COMM_STRING_SUCCESS,
	COMM_STRING_PREFIXERROR,
	COMM_STRING_SUFFIXERROR,
	COMM_STRING_FAIL,
} eCOMM_STRING_Ret;

/****************************************************
	variable
****************************************************/

/****************************************************
	prototype
****************************************************/
CHAR* COMM_STRING_FindSubString(CHAR* pcSrc, CHAR* pcTargetSubString);
eCOMM_STRING_Ret COMM_STRING_ReplaceSubString(CHAR* pcSrc, LONG lReplaceLen, CHAR* pcReplaceString);
eCOMM_STRING_Ret COMM_STRING_GetString(CHAR* pcSrc, CHAR* pcPrefix, CHAR* pcSuffix, CHAR* pcOutString, LONG* plOutLen);

#ifdef __cplusplus
}
#endif

#endif
