#ifndef __COMM_DD_ADC_H__
#define __COMM_DD_ADC_H__

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
 	eCOMM_DD_ADC_SUCCESS,
 	eCOMM_DD_ADC_FAIL,
} eCOMM_DD_ADC_Ret;

typedef enum {
	eCOMM_DD_ADC_CFG_SAMPLE,
	eCOMM_DD_ADC_CFG_THROD,
	eCOMM_DD_ADC_CFG_CH,
}eCOMM_DD_ADC_CFG_TYPE;

typedef enum {
	eCOMM_DD_ADC_CH0,
	eCOMM_DD_ADC_CH1,
}eCOMM_DD_ADC_CH;

typedef enum {
	eCOMM_DD_ADC_SAMPLE_1=1,
	eCOMM_DD_ADC_SAMPLE_2=2,
	eCOMM_DD_ADC_SAMPLE_4=4,
	eCOMM_DD_ADC_SAMPLE_8=8,
	eCOMM_DD_ADC_SAMPLE_16=16,
}eCOMM_DD_ADC_SAMPLE;

/*********************************************************
	struct
*********************************************************/
typedef struct {
	LONG lFd;
	LONG lSamples;
	LONG lThrod;
	LONG lCh;
	LONG lRst;
} sCOMM_DD_ADC_Info;


/*********************************************************
	variable
*********************************************************/

/*********************************************************
	prototype
*********************************************************/
eCOMM_DD_ADC_Ret eCOMM_DD_ADC_Config(sCOMM_DD_ADC_Info* psAdcInfo, eCOMM_DD_ADC_CFG_TYPE eType, LONG lVal);
eCOMM_DD_ADC_Ret eCOMM_DD_ADC_GetCfg(sCOMM_DD_ADC_Info* psAdcInfo, eCOMM_DD_ADC_CFG_TYPE eType, LONG* plVal);
eCOMM_DD_ADC_Ret eCOMM_DD_ADC_Init(sCOMM_DD_ADC_Info* psAdcInfo);
eCOMM_DD_ADC_Ret eCOMM_DD_ADC_DeInit(sCOMM_DD_ADC_Info* psAdcInfo);
eCOMM_DD_ADC_Ret eCOMM_DD_ADC_GetRst(sCOMM_DD_ADC_Info* psInfo) ;

#ifdef __cplusplus
}
#endif

#endif
