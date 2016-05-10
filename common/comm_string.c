#ifdef __cplusplus
extern "C" {
#endif


/****************************************************
	include
****************************************************/
#include "comm_string.h"

/****************************************************
	macro / define
****************************************************/
// #define COMM_MW_STRING_Debug
#ifdef COMM_MW_STRING_Debug
#define CMS_Debug(fmt, args...) fprintf(stdout, "[ CMS ] : %s <%d> "fmt, __func__, __LINE__, ##args)
#define CMS_FuncIn() fprintf(stdout, "[ CMS ] : %s <%d> in\n", __func__, __LINE__)
#define CMS_FuncOut() fprintf(stdout, "[ CMS ] : %s <%d> out\n", __func__, __LINE__)
#define CMS_iVal(iVal) fprintf(stdout, "[ CMS ] : %s <%d> %s <%d> @ <%p>\n", __func__, __LINE__, #iVal, iVal, &iVal)
#define CMS_lVal(lVal) fprintf(stdout, "[ CMS ] : %s <%d> %s <%ld> @ <%p>\n", __func__, __LINE__, #lVal, lVal, &lVal)
#define CMS_PtVal(PtVal) fprintf(stdout, "[ CMS ] : %s <%d> %s <%p> @ <%p>\n", __func__, __LINE__, #PtVal, PtVal, &PtVal)
#else
#define CMS_Debug(fmt, args...)
#define CMS_FuncIn()
#define CMS_FuncOut()
#define CMS_iVal(iVal)
#define CMS_lVal(lVal)
#define CMS_PtVal(PtVal)
#endif

/****************************************************
	enum / struct
****************************************************/

/****************************************************
	variable
****************************************************/

/****************************************************
	func
****************************************************/

/***************************************************
 * func : COMM_STRING_FindSubString(CHAR* pcSrc, CHAR* pcTargetSubString)
 * arg : CHAR* pcSrc, CHAR* pcTargetSubString
 * ret : CHAR*
 * note : find a substring of src string
 ***************************************************/
CHAR* COMM_STRING_FindSubString(CHAR* pcSrc, CHAR* pcTargetSubString) {
	CHAR* pcRet = NULL;
	CHAR* pcTmp = pcSrc;
	CHAR* pcNext = NULL;
	CHAR pcFirst = * pcTargetSubString;
	LONG lSubStringLen = strlen(pcTargetSubString);

	pcNext = index(pcTmp, pcFirst);
	while(pcNext) {
		if(!strncmp(pcNext, pcTargetSubString, lSubStringLen)) {
			pcRet = pcNext;
			break;
		}

		/* update */
		pcTmp = pcNext;
		pcNext = index(pcTmp, pcFirst);
	}

	return pcRet;
}

/***************************************************
 * func : COMM_SRING_ReplaceSubString(CHAR* pcSrc, LONG lReplaceCnt, CHAR* pcReplaceString)
 * arg : CHAR* pcSrc, LONG lReplaceCnt, CHAR* pcReplaceString
 * ret : eCOMM_STRING_Ret
 * note : replace a sub string
 ***************************************************/
eCOMM_STRING_Ret COMM_STRING_ReplaceSubString(CHAR* pcSrc, LONG lReplaceLen, CHAR* pcReplaceString) {
	eCOMM_STRING_Ret eRet = COMM_STRING_SUCCESS;
	CHAR* pcRemainRestore = NULL;
	CHAR* pcTmp = NULL;

	/* init */
	pcRemainRestore = strdup(pcSrc + lReplaceLen);
	memset(pcSrc, 0, strlen(pcSrc) + 1);

	/* construct the final string */
	pcTmp = pcSrc;
	strcpy(pcTmp, pcReplaceString);
	pcTmp += strlen(pcTmp);
	strcpy(pcTmp, pcRemainRestore);

	free(pcRemainRestore);

	return eRet;
}

/***************************************************
 * func : COMM_STRING_GetString(CHAR* pcSrc, CHAR* pcPrefix, CHAR* pcSuffix, CHAR* pcOutString, LONG* plOutLen)
 * arg : CHAR* pcSrc, CHAR* pcPrefix, CHAR* pcSuffix, CHAR* pcOutString, LONG* plOutLen
 * ret : eCOMM_STRING_Ret
 * note : find a string with prefix and suffix
 ***************************************************/
eCOMM_STRING_Ret COMM_STRING_GetString(CHAR* pcSrc, CHAR* pcPrefix, CHAR* pcSuffix, CHAR* pcOutString, LONG* plOutLen) {
	eCOMM_STRING_Ret eRet = COMM_STRING_SUCCESS;
	CHAR* pcBegin;
	CHAR* pcEnd;
	LONG lStrLen;

	if(strncmp(pcSrc, pcPrefix, strlen(pcPrefix))) {
		/* indicate pcSrc not satisfied prefix */
		return COMM_STRING_PREFIXERROR;
	}

	pcBegin = pcSrc + strlen(pcPrefix);
	pcEnd = COMM_STRING_FindSubString(pcBegin, pcSuffix);
	if(!pcEnd) {
		/* indicate not satisfied suffix */
		return COMM_STRING_SUFFIXERROR;
	}

	lStrLen = pcEnd - pcBegin;
	strncpy(pcOutString, pcBegin, lStrLen);
	pcOutString[lStrLen] = '\0';

	*plOutLen = lStrLen + strlen(pcPrefix) + strlen(pcSuffix);

	return eRet;
}


#ifdef __cplusplus
}
#endif
