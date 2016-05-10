#ifndef __BASE_MW_SUBCARD_H__
#define __BASE_MW_SUBCARD_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************
	include
*********************************************************/
#include "base_comm.h"
#include "base_mw_subcard_rec.h"

/*********************************************************
	macro / enum
*********************************************************/
typedef enum {
	BASE_MW_SUBCARD_SUCCESS,
	BASE_MW_SUBCARD_FAIL,
	BASE_MW_SUBCARD_REC,
	BASE_MW_SUBCARD_CREATETHREAD,
	BASE_MW_SUBCARD_SYSTEMNOTSTART,
	BASE_MW_SUBCARD_NOTENOUGHSPACE,
} eBASE_MW_SUBCARD_Ret;

typedef enum {
	BASE_MW_SUBCARD_Invalid = 0x0,
	BASE_MW_SUBCARD_Valid = 0x1,
	BASE_MW_SUBCARD_Busy = 0x10,
	BASE_MW_SUBCARD_HaveWork = 0x20,
	BASE_MW_SUBCARD_VeryBusy = 0x40,
	BASE_MW_SUBCARD_NeedToQuit = 0x100,
} eBASE_MW_SUBCARD_Status;

/*********************************************************
	struct
*********************************************************/
typedef struct {
	sBASE_MW_SUBCARD_REC_Info sRecInfo;
	eBASE_MW_SUBCARD_Status eStatus;
	pthread_t tid;
	pthread_mutex_t mutStatus;
	pthread_cond_t condStatus;
} sBASE_MW_SUBCARD_Info;

/*********************************************************
	variable
*********************************************************/
sBASE_MW_SUBCARD_Info sSubCardInfo;

/*********************************************************
	prototype
*********************************************************/
eBASE_MW_SUBCARD_Ret eBASE_MW_SUBCARD_Init(sBASE_MW_SUBCARD_Info* psInfo, LONG lPoolId, LONG lTimeTh, CHAR* pcPath);
eBASE_MW_SUBCARD_Ret eBASE_MW_SUBCARD_StartSigProcess(sBASE_MW_SUBCARD_Info* psInfo);
eBASE_MW_SUBCARD_Ret eBASE_MW_SUBCARD_NewSignal(sBASE_MW_SUBCARD_Info* psInfo);
eBASE_MW_SUBCARD_Ret eBASE_MW_SUBCARD_Deinit(sBASE_MW_SUBCARD_Info* psInfo);
eBASE_MW_SUBCARD_Ret eBASE_MW_SUBCARD_UpdateTimeTh(sBASE_MW_SUBCARD_Info* psInfo, LONG lNewTimeTh);
eBASE_MW_SUBCARD_Ret eBASE_MW_SUBCARD_UpdateOverlayType(sBASE_MW_SUBCARD_Info* psInfo, eBASE_MW_SUBCARD_OVERLAY_Type eNewOverlayType);
eBASE_MW_SUBCARD_Ret eBASE_MW_SUBCARD_GetOverlayType(sBASE_MW_SUBCARD_Info* psInfo, eBASE_MW_SUBCARD_OVERLAY_Type* peOverlayType);

#ifdef __cplusplus
}
#endif

#endif
