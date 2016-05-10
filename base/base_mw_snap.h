#ifndef __BASE_MW_SNAP_H__
#define __BASE_MW_SNAP_H__

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
	include
****************************************************************/
#include "base_comm.h"
#include "base_mw_filesort.h"
#include "base_api_ledctl.h"

/****************************************************************
	macro / enum
****************************************************************/
typedef enum {
	BASE_MW_SNAP_SUCCESS,
	BASE_MW_SNAP_ERROR,
	BASE_MW_SNAP_STARTERROR,
	BASE_MW_SNAP_STOPERROR,
	BASE_MW_SNAP_PROCESSERROR,
} eBASE_MW_SNAP_ERROR;

typedef enum {
	BASE_MW_SNAP_START,
	BASE_MW_SNAP_STOP,
	BASE_MW_SNAP_NOTUSE,
}eBASE_MW_SNAP_STARTSGIN;

/****************************************************************
	struct
****************************************************************/
typedef struct {
	VI_CHN ViChn_SnapExt;
	VENC_CHN VencChn_Snap;
	eBASE_CH_STATE eState;
	eBASE_VENC_PIC_SIZE eSize;
	sBASE_MW_FILESORT_Info sFileSortInfo;				//add by yudong
} sBASE_VIDEO_SNAP;

/****************************************************************
	variable
****************************************************************/
sBASE_VIDEO_SNAP gsSnap;
const CHAR RECSNAPPATH[64] ;
/****************************************************************
	prototype
****************************************************************/
LONG lBASE_VENC_SnapHandle(sBASE_VIDEO_SNAP* psSnap);
LONG lBASE_VENC_SnapHandle_Stop(sBASE_VIDEO_SNAP* psSnap);

#ifdef __cplusplus
}
#endif

#endif