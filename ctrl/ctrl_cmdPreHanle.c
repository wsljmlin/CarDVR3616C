#ifdef __cplusplus
extern "C" {
#endif

/*************************************************
	include
*************************************************/
#include "comm_common.h"
#include "comm_mw_strpool.h"
#include "comm_string.h"
#include "comm_sharemem.h"

/*************************************************
	prototype
*************************************************/
void vCTRL_CmdPreHandle(CHAR* pcCmd);

/*************************************************
	extern
*************************************************/
extern pthread_mutex_t mutStringPoolWt;
extern sCOMM_SHAREMEM gsShareMem;
extern const LONG SM_OFFSET[];


/*************************************************
	function
*************************************************/
void vCTRL_CmdPreHandle(CHAR* pcCmd) {
	eCOMM_STRING_Ret eStringRet = COMM_STRING_SUCCESS;
	eCOMM_MW_StrPoolRet eStrPoolRet = COMM_MW_StrPoolSuccess;
	CHAR* pcSrcTmp = NULL;
	CHAR cStringInfo[128];
	CHAR cReplaceStr[16];
	LONG lLen;
	LONG lStrPoolIndex;

	pcSrcTmp = COMM_STRING_FindSubString(pcCmd, "string");
	while(pcSrcTmp) {
		/* get string */
		memset(cStringInfo, 0, sizeof(cStringInfo));
		eStringRet = COMM_STRING_GetString(pcSrcTmp, "string", "#", cStringInfo, &lLen);
		if(eStringRet != COMM_STRING_SUCCESS) {
			printf("[ PreHandle ] : string module Error with <%d>\n", eStringRet);
			exit(-1);
		}

		/* replace string */
		eStrPoolRet = COMM_MW_StrPool_WriteEntry(COMM_GET_STRPOOL(), cStringInfo, &lStrPoolIndex, &mutStringPoolWt);
		if(eStrPoolRet != COMM_MW_StrPoolSuccess) {
			printf("[ PreHandle ] : string pool module Error with <%d>\n", eStringRet);
			exit(-1);
		}
		memset(cReplaceStr, 0, sizeof(cReplaceStr));
		sprintf(cReplaceStr, "%ld", lStrPoolIndex);
		COMM_STRING_ReplaceSubString(pcSrcTmp, lLen, cReplaceStr);

		/* udpate */
		pcSrcTmp += lLen;
		pcSrcTmp = COMM_STRING_FindSubString(pcCmd, "string");
	}
}


#ifdef __cplusplus
}
#endif
