#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "base_ircut_ctl.h"
#include "comm_timer.h"

/***************************************************
	macro / enum
***************************************************/
//#define BASE_IRCUT_CTL_DEBUG
#ifdef BASE_IRCUT_CTL_DEBUG
#define BIC_Debug(fmt, arg...) fprintf(stdout, "[ BIC ] : %s() <%d> "fmt, __func__, __LINE__, ##arg)
#else
#define BIC_Debug(fmt, arg...)
#endif

#define BIC_Info(fmt, arg...) fprintf(stdout, "[ BIC ] : %s() <%d> "fmt, __func__, __LINE__, ##arg)

#define BIC_FuncIn() BIC_Debug("in\n")
#define BIC_FuncOut() BIC_Debug("out\n")
#define BIC_iVal(iVal) BIC_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define BIC_lVal(lVal) BIC_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define BIC_PtVal(ptVal) BIC_Debug("pt %s @ %p\n", #ptVal, ptVal)

#define BASE_IRCUT_CTL_TIME_INTERVAL  10//(60*2)
#define BASE_IRCUT_CTL_IRCUT_THROD 500


/***************************************************
	variable
***************************************************/

/***************************************************
	prototype
***************************************************/

/***************************************************
	function
***************************************************/


void vTimeDisplay(void) {
	struct tm *tm_loc;
	time_t tTime;
	
	/* init time */
	tTime = time(NULL);
	tm_loc = localtime(&tTime);

	printf("%d-%02d-%02d %02d:%02d:%02d\n",1900 + tm_loc->tm_year, tm_loc->tm_mon + 1, tm_loc->tm_mday, tm_loc->tm_hour,
		tm_loc->tm_min, tm_loc->tm_sec);
}

/*********************************************
* func : vBASE_IRCUT_CTL_CallBack(void* pVarg)
* arg : void* pVarg
* ret : 
* note :
*********************************************/
void vBASE_IRCUT_CTL_CallBack(void* pVarg) {

	BIC_FuncIn();

	{
		sBASE_IRCUT_CTL_Info* psInfo = (sBASE_IRCUT_CTL_Info*)pVarg;
		
		/* print time */
		vTimeDisplay();

		/* read adc convert result */
		eCOMM_DD_ADC_GetRst(&(psInfo->sAdcInfo));
		/* set ircut according result */
		eCOMM_DD_IRCUT_Set(&(psInfo->sIrcutInfo), ((psInfo->sAdcInfo.lRst == 1) ?\
			eCOMM_DD_IRCUT_TYPE_DAY : eCOMM_DD_IRCUT_TYPE_NIGHT));
	}

	BIC_FuncOut();

}

/*********************************************
* func : eBASE_IRCUT_CTL_ConfigADC(sCOMM_DD_ADC_Info* psInfo)
* arg : sCOMM_DD_ADC_Info* psInfo
* ret : eBASE_IRCUT_CTL_Ret
* note :
*********************************************/
eBASE_IRCUT_CTL_Ret eBASE_IRCUT_CTL_ConfigADC(sCOMM_DD_ADC_Info* psInfo) {
	eBASE_IRCUT_CTL_Ret eRet = eBASE_IRCUT_CTL_SUCCESS;
	
	BIC_FuncIn();

	{
		eRet =  eCOMM_DD_ADC_Config(psInfo, eCOMM_DD_ADC_CFG_CH, psInfo->lCh) ;
		if(eBASE_IRCUT_CTL_SUCCESS != eRet) {
			eRet = eBASE_IRCUT_CTL_FAIL;
			BIC_Info(" set adc ch error!\n");
			return eRet;
		}
		eRet =  eCOMM_DD_ADC_Config(psInfo, eCOMM_DD_ADC_CFG_SAMPLE,psInfo->lSamples) ;
		 if(eBASE_IRCUT_CTL_SUCCESS != eRet) {
			eRet = eBASE_IRCUT_CTL_FAIL;
			eCOMM_DD_ADC_GetCfg(psInfo, eCOMM_DD_ADC_CFG_SAMPLE, &(psInfo->lSamples)) ;
			BIC_Info(" set adc sample error, and it will use default value:%ld!\n", psInfo->lSamples);
		}
		eRet =  eCOMM_DD_ADC_Config(psInfo, eCOMM_DD_ADC_CFG_THROD,psInfo->lThrod) ;
		 if(eBASE_IRCUT_CTL_SUCCESS != eRet) {
			eRet = eBASE_IRCUT_CTL_FAIL;
			BIC_Info(" set adc ircut throd error!\n");
			return eRet;
		}
	}

	BIC_FuncOut();
	return eRet;

}

/*********************************************
* func : eBASE_IRCUT_CTL_Init(sBASE_IRCUT_CTL_Info* psInfo)
* arg : sBASE_IRCUT_CTL_Info* psInfo
* ret : eBASE_IRCUT_CTL_Ret
* note :
*********************************************/
eBASE_IRCUT_CTL_Ret eBASE_IRCUT_CTL_Init(sBASE_IRCUT_CTL_Info* psInfo) {
	eBASE_IRCUT_CTL_Ret eRet = eBASE_IRCUT_CTL_SUCCESS;

	BIC_FuncIn();

	if(0) {
		/* adc  init */
		eRet = eCOMM_DD_ADC_Init(&(psInfo->sAdcInfo));
		if(eCOMM_DD_ADC_SUCCESS != eRet) {
			eRet = eBASE_IRCUT_CTL_FAIL;
			BIC_Info(" adc init error!\n");
			return eRet;
		}
		/* ircut init */	
		eRet = eCOMM_DD_IRCUT_Init(&(psInfo->sIrcutInfo), 1);
		if(eCOMM_DD_IRCUT_SUCCESS != eRet) {
			eRet = eBASE_IRCUT_CTL_FAIL;
			BIC_Info(" ircut init error!\n");
			return eRet;
		}

		/* device config */
		psInfo->sAdcInfo.lCh = eCOMM_DD_ADC_CH0;
		psInfo->sAdcInfo.lSamples = eCOMM_DD_ADC_SAMPLE_4;
		psInfo->sAdcInfo.lThrod = BASE_IRCUT_CTL_IRCUT_THROD;

		eRet = eBASE_IRCUT_CTL_ConfigADC(&(psInfo->sAdcInfo));
		 if(eBASE_IRCUT_CTL_SUCCESS != eRet)  {
			eRet = eBASE_IRCUT_CTL_FAIL;
			BIC_Info(" config adc error!\n");
			return eRet;
		}

		 
		/* register callback handle function */
#if 0
		eRet = eCOMM_TIMER_Init(BASE_IRCUT_CTL_TIME_INTERVAL, vBASE_IRCUT_CTL_CallBack, (void*)psInfo, &(psInfo->lTimeId));
		if(eBASE_IRCUT_CTL_SUCCESS != eRet) {
			eRet = eBASE_IRCUT_CTL_FAIL;
			BIC_Info(" register ircut handle error!\n");
			return eRet;
		}
#else
	eCOMM_DD_IRCUT_Set(&(psInfo->sIrcutInfo), eCOMM_DD_IRCUT_TYPE_DAY );
	//eCOMM_DD_IRCUT_Set(&(psInfo->sIrcutInfo), eCOMM_DD_IRCUT_TYPE_NIGHT );


#endif
	}

	BIC_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_IRCUT_CTL_DeInit(sBASE_IRCUT_CTL_Info* psInfo)
* arg : sBASE_IRCUT_CTL_Info* psInfo
* ret : eBASE_IRCUT_CTL_Ret
* note :
*********************************************/
eBASE_IRCUT_CTL_Ret eBASE_IRCUT_CTL_DeInit(sBASE_IRCUT_CTL_Info* psInfo) {
	eBASE_IRCUT_CTL_Ret eRet = eBASE_IRCUT_CTL_SUCCESS;

	BIC_FuncIn();

	if(0) {
		/* unregister callback */
		eCOMM_TIMER_Deinit(psInfo->lTimeId);
		/* ircut deinit */
		eCOMM_DD_IRCUT_DeInit(&(psInfo->sIrcutInfo));
		/* adc deinit */
		eCOMM_DD_ADC_DeInit(&(psInfo->sAdcInfo));

	}

	BIC_FuncOut();

	return eRet;
}


#ifdef __cplusplus
}
#endif
