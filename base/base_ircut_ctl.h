#ifndef __BASE_IRCUT_CTL_H__
#define __BASE_IRCUT_CTL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************
	include
*********************************************************/
#include "comm_dd_adc.h"
#include "comm_dd_ircut.h"
/*********************************************************
	macro / enum
*********************************************************/
typedef enum {
	eBASE_IRCUT_CTL_SUCCESS,
 	eBASE_IRCUT_CTL_FAIL,
} eBASE_IRCUT_CTL_Ret;

/*********************************************************
	struct
*********************************************************/
typedef struct {
	sCOMM_DD_ADC_Info sAdcInfo;
	sCOMM_DD_IRCUT_Info sIrcutInfo;
	LONG lTimeId;
} sBASE_IRCUT_CTL_Info;

/*********************************************************
	variable
*********************************************************/
/* ir-cut */
sBASE_IRCUT_CTL_Info gsIrCutInfo;
/*********************************************************
	prototype
*********************************************************/
eBASE_IRCUT_CTL_Ret eBASE_IRCUT_CTL_Init(sBASE_IRCUT_CTL_Info* psInfo);
eBASE_IRCUT_CTL_Ret eBASE_IRCUT_CTL_DeInit(sBASE_IRCUT_CTL_Info* psInfo);

#ifdef __cplusplus
}
#endif

#endif
