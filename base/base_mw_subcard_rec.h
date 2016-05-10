#ifndef __BASE_MW_SUBCARD_REC_H__
#define __BASE_MW_SUBCARD_REC_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************
	include
*********************************************************/
#include "base_comm.h"
#include "base_api_ledctl.h"
#include <libgen.h>

#ifdef SUBCARD_CHECK_DEBUG
#include "base_mw_sd.h"
#include "base_mw_filesort.h"
#else
#include "comm_spaceSafer.h"
#endif /* SUBCARD_CHECK_DEBUG */
/*********************************************************
	macro / enum
*********************************************************/
#define TWO_FOLDER (2)
#define BASE_MW_SUBCARD_MAXFILENAME 256

typedef enum {
	BASE_MW_SUBCARD_REC_SUCCESS,
	BASE_MW_SUBCARD_REC_FAIL,
	BASE_MW_SUBCARD_REC_POOL,
	BASE_MW_SUBCARD_REC_CALLBACK,
	BASE_MW_SUBCARD_REC_VFILE,
	BASE_MW_SUBCARD_REC_MALLOC,
} eBASE_MW_SUBCARD_REC_Ret;

typedef enum {
	BASE_MW_SUBCARD_REC_NOTISLICE,
	BASE_MW_SUBCARD_REC_ISISLICE,
} eBASE_MW_SUBCARD_REC_Frametype;

typedef enum {
	BASE_MW_SUBCARD_OVERLAY_FORCE,
	BASE_MW_SUBCARD_OVERLAY_NOFORCE,
} eBASE_MW_SUBCARD_OVERLAY_Type;

/*********************************************************
	struct
*********************************************************/
typedef struct {
	LONG lFrameThreshold;
	LONG lFrameCurCnt;
	eBASE_MW_SUBCARD_REC_Frametype eFrameType;
	LONG lRet;
	CHAR* pcPathName;
	CHAR* pcFileName;
	sCOMM_MW_VFILE_Info* psVfile;
	eBASE_MW_SUBCARD_OVERLAY_Type eOverLayType;
} sBASE_MW_SUBCARD_REC_ReadInfo;

typedef struct {
	LONG lPoolId;
	LONG lTimeThreshold;
	LONG lConsumerKey;
	sBASE_MW_SUBCARD_REC_ReadInfo* psReadInfo;
	CHAR cRecPath[BASE_MW_SUBCARD_MAXFILENAME];
	CHAR* pcDirPath;
} sBASE_MW_SUBCARD_REC_Info;

/*********************************************************
	variable
*********************************************************/
/*********************************************************
	prototype
*********************************************************/
eBASE_MW_SUBCARD_REC_Ret eBASE_MW_SUBCARD_REC_Init(sBASE_MW_SUBCARD_REC_Info* psInfo, LONG lPoolId, LONG lTimeTh, CHAR* pcPathName);
eBASE_MW_SUBCARD_REC_Ret eBASE_MW_SUBCARD_REC_Consume(sBASE_MW_SUBCARD_REC_Info* psInfo);
eBASE_MW_SUBCARD_REC_Ret eBASE_MW_SUBCARD_REC_Deinit(sBASE_MW_SUBCARD_REC_Info* psInfo);
eBASE_MW_SUBCARD_REC_Ret eBASE_MW_SUBCARD_REC_UpdateRecDuration(sBASE_MW_SUBCARD_REC_Info* psInfo, LONG lNewTimeTh);
eBASE_MW_SUBCARD_REC_Ret eBASE_MW_SUBCARD_REC_UpdateOverlayType(sBASE_MW_SUBCARD_REC_Info* psInfo, eBASE_MW_SUBCARD_OVERLAY_Type eNewOverLayType);
eBASE_MW_SUBCARD_REC_Ret eBASE_MW_SUBCARD_REC_GetOverlayType(sBASE_MW_SUBCARD_REC_Info* psInfo, eBASE_MW_SUBCARD_OVERLAY_Type* peOverlayType);

#ifdef __cplusplus
}
#endif

#endif