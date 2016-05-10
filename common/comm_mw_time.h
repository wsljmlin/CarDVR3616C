#ifndef __COMM_MW_TIME_H__
#define __COMM_MW_TIME_H__

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
	COMM_MW_TIME_SUCCESS,
	COMM_MW_TIME_FAIL,
} eCOMM_MW_TIME_Ret;

#define COMM_MW_TIME_NAME_MAX 64

/*********************************************************
	struct
*********************************************************/
typedef struct {
	time_t sCurrentVisitTime;
	time_t sSaveVisitTime;
	LONG lDeltTimeSec;
	CHAR cName[COMM_MW_TIME_NAME_MAX];
} sCOMM_MW_TIME_Info;

/*********************************************************
	variable
*********************************************************/

/*********************************************************
	prototype
*********************************************************/
eCOMM_MW_TIME_Ret eCOMM_MW_TIME_Init(sCOMM_MW_TIME_Info* psInfo, CHAR* pcName);
eCOMM_MW_TIME_Ret eCOMM_MW_TIME_Reset(sCOMM_MW_TIME_Info* psInfo);
eCOMM_MW_TIME_Ret eCOMM_MW_TIME_Update(sCOMM_MW_TIME_Info* psInfo);
eCOMM_MW_TIME_Ret eCOMM_MW_TIME_Save(sCOMM_MW_TIME_Info* psInfo);
LONG lCOMM_MW_TIME_GetDeltTime(sCOMM_MW_TIME_Info* psInfo);

#ifdef __cplusplus
}
#endif

#endif
