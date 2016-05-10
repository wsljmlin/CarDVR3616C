#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include <fcntl.h>
#include <sys/ioctl.h>
#include "comm_dd_adc.h"
#include "hi_adc.h"

/***************************************************
	macro / enum
***************************************************/
//#define BASE_API_ADC_DEBUG
#ifdef BASE_API_ADC_DEBUG
#define BAA_Debug(fmt, arg...) fprintf(stdout, "[ BDA ] : %s() <%d> "fmt, __func__, __LINE__, ##arg)
#else
#define BAA_Debug(fmt, arg...)
#endif

#define BAA_Info(fmt, arg...) fprintf(stdout, "[ BDA ] : %s() <%d> "fmt, __func__, __LINE__, ##arg)

#define BAA_FuncIn() BAA_Debug("in\n")
#define BAA_FuncOut() BAA_Debug("out\n")
#define BAA_iVal(iVal) BAA_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define BAA_lVal(lVal) BAA_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define BAA_PtVal(ptVal) BAA_Debug("pt %s @ %p\n", #ptVal, ptVal)

#define BASE_API_ADC_DEV "/dev/hi_adc"


/***************************************************
	variable
***************************************************/

/***************************************************
	prototype
***************************************************/

/***************************************************
	function
***************************************************/

/*********************************************
* func : eCOMM_DD_ADC_Config(sCOMM_DD_ADC_Info* psAdcInfo, eCOMM_DD_ADC_CFG_TYPE eType, LONG lVal)
* arg : sCOMM_DD_ADC_Info* psAdcInfo, eCOMM_DD_ADC_CFG_TYPE eType, LONG lVal
* ret : eCOMM_DD_ADC_Ret
* note :
*********************************************/
eCOMM_DD_ADC_Ret eCOMM_DD_ADC_Config(sCOMM_DD_ADC_Info* psAdcInfo, eCOMM_DD_ADC_CFG_TYPE eType, LONG lVal) {
	eCOMM_DD_ADC_Ret eRet = eCOMM_DD_ADC_SUCCESS;

	BAA_FuncIn();

	{
		if(-1 == psAdcInfo->lFd) {
			BAA_Info("invalid file description\n");
			eRet = eCOMM_DD_ADC_FAIL;
		}

		switch(eType) {
			case eCOMM_DD_ADC_CFG_SAMPLE:
				psAdcInfo->lSamples = lVal;
				eRet = ioctl(psAdcInfo->lFd, HI_ADC_IOC_SET_SAMPLES, &(psAdcInfo->lSamples));
				break;

			case eCOMM_DD_ADC_CFG_THROD:
				psAdcInfo->lThrod = lVal;
				eRet = ioctl(psAdcInfo->lFd, HI_ADC_IOC_SET_THROD, &(psAdcInfo->lThrod));
				break;

			case eCOMM_DD_ADC_CFG_CH:
				psAdcInfo->lCh = lVal;
				eRet = ioctl(psAdcInfo->lFd, HI_ADC_IOC_SET_CH, &(psAdcInfo->lCh));
				break;
		}
	}

	BAA_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_ADC_Config(sCOMM_DD_ADC_Info* psAdcInfo, eCOMM_DD_ADC_CFG_TYPE eType, LONG lVal)
* arg : sCOMM_DD_ADC_Info* psAdcInfo, eCOMM_DD_ADC_CFG_TYPE eType, LONG lVal
* ret : eCOMM_DD_ADC_Ret
* note :
*********************************************/
eCOMM_DD_ADC_Ret eCOMM_DD_ADC_GetCfg(sCOMM_DD_ADC_Info* psAdcInfo, eCOMM_DD_ADC_CFG_TYPE eType, LONG* plVal) {
	eCOMM_DD_ADC_Ret eRet = eCOMM_DD_ADC_SUCCESS;

	BAA_FuncIn();

	{
		if(-1 == psAdcInfo->lFd) {
			BAA_Info("invalid file description\n");
			eRet = eCOMM_DD_ADC_FAIL;
		}

		switch(eType) {
			case eCOMM_DD_ADC_CFG_SAMPLE:
				eRet = ioctl(psAdcInfo->lFd, HI_ADC_IOC_GET_SAMPLES, plVal);
				break;

			case eCOMM_DD_ADC_CFG_THROD:
				eRet = ioctl(psAdcInfo->lFd, HI_ADC_IOC_GET_THROD, plVal);
				break;

			case eCOMM_DD_ADC_CFG_CH:
				eRet = ioctl(psAdcInfo->lFd, HI_ADC_IOC_GET_CH, plVal);
				break;
		}
	}

	BAA_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_ADC_Init(sCOMM_DD_ADC_Info* psAdcInfo)
* arg : sCOMM_DD_ADC_Info* psAdcInfo
* ret : eCOMM_DD_ADC_Ret
* note :
*********************************************/
eCOMM_DD_ADC_Ret eCOMM_DD_ADC_Init(sCOMM_DD_ADC_Info* psAdcInfo) {
	eCOMM_DD_ADC_Ret eRet = eCOMM_DD_ADC_SUCCESS;

	BAA_FuncIn();

	{
		memset(psAdcInfo, 0, sizeof(sCOMM_DD_ADC_Info));
		psAdcInfo->lFd = -1;

		/* open adc dev */
		psAdcInfo->lFd = open(BASE_API_ADC_DEV, O_RDWR);
		if(-1 == psAdcInfo->lFd) {			
			eRet = eCOMM_DD_ADC_FAIL;
			BAA_Info("Can not open adc device!\n");
			return eRet;		
		}
	}

	BAA_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_ADC_DeInit(sCOMM_DD_ADC_Info* psAdcInfo)
* arg : sCOMM_DD_ADC_Info* psAdcInfo
* ret : eCOMM_DD_ADC_Ret
* note :
*********************************************/
eCOMM_DD_ADC_Ret eCOMM_DD_ADC_DeInit(sCOMM_DD_ADC_Info* psAdcInfo) {
	eCOMM_DD_ADC_Ret eRet = eCOMM_DD_ADC_SUCCESS;

	BAA_FuncIn();

	{
		/* close device */
		close(psAdcInfo->lFd);
		
		psAdcInfo->lFd = -1;
	}

	BAA_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_ADC_GetRst(sCOMM_DD_ADC_Info* psAdcInfo)
* arg : sCOMM_DD_ADC_Info* psAdcInfo
* ret : eCOMM_DD_ADC_Ret
* note :
*********************************************/
eCOMM_DD_ADC_Ret eCOMM_DD_ADC_GetRst(sCOMM_DD_ADC_Info* psInfo) {
	eCOMM_DD_ADC_Ret eRet = eCOMM_DD_ADC_SUCCESS;

	BAA_FuncIn();
	{
		psInfo->lRst = 0;
		read(psInfo->lFd, &(psInfo->lRst), 4);
	}
	BAA_FuncOut();

	return eRet;
}

#ifdef __cplusplus
}
#endif
