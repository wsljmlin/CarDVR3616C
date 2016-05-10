#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include <fcntl.h>
#include <sys/ioctl.h>
#include "comm_dd_ircut.h"
#include "hi_gpio.h"

/***************************************************
	macro / enum
***************************************************/
//#define BASE_API_IRCUT_DEBUG
#ifdef BASE_API_IRCUT_DEBUG
#define BAI_Debug(fmt, arg...) fprintf(stdout, "[ BAI ] : %s() <%d> "fmt, __func__, __LINE__, ##arg)
#else
#define BAI_Debug(fmt, arg...)
#endif

#define BAI_Info(fmt, arg...) fprintf(stdout, "[ BAI ] : %s() <%d> "fmt, __func__, __LINE__, ##arg)

#define BAI_FuncIn() BAI_Debug("in\n")
#define BAI_FuncOut() BAI_Debug("out\n")
#define BAI_iVal(iVal) BAI_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define BAI_lVal(lVal) BAI_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define BAI_PtVal(ptVal) BAI_Debug("pt %s @ %p\n", #ptVal, ptVal)

#define BASE_API_GPIO_DEV "/dev/IRCUT"


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
* func : eCOMM_DD_IRCUT_Init(sCOMM_DD_IRCUT_Info* psInfo, LONG lFd)
* arg : sCOMM_DD_IRCUT_Info* psInfo, LONG lFd
* ret : eCOMM_DD_IRCUT_Ret
* note :
*********************************************/
eCOMM_DD_IRCUT_Ret eCOMM_DD_IRCUT_Init(sCOMM_DD_IRCUT_Info* psInfo, LONG lFd) {
	eCOMM_DD_IRCUT_Ret eRet = eCOMM_DD_IRCUT_SUCCESS;

	BAI_FuncIn();
	
	{
		memset(psInfo, 0, sizeof(sCOMM_DD_IRCUT_Info));
		psInfo->lFd = -1;
#if 1
		/* open ircut dev */
		psInfo->lFd = open(BASE_API_GPIO_DEV, O_RDWR);
		if(-1 == psInfo->lFd) {			
			eRet = eCOMM_DD_IRCUT_FAIL;
			BAI_Info("Can not open gpio device!\n");
			return eRet;		
		} 
#else
		psInfo = lFd;
#endif
	}

	BAI_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_IRCUT_Set(sCOMM_DD_IRCUT_Info* psInfo, eCOMM_DD_IRCUT_TYPE lType)
* arg : sCOMM_DD_IRCUT_Info* psInfo, eCOMM_DD_IRCUT_TYPE lType
* ret : eCOMM_DD_IRCUT_Ret
* note :
*********************************************/
eCOMM_DD_IRCUT_Ret eCOMM_DD_IRCUT_Set(sCOMM_DD_IRCUT_Info* psInfo, eCOMM_DD_IRCUT_TYPE lType) {
	eCOMM_DD_IRCUT_Ret eRet = eCOMM_DD_IRCUT_SUCCESS;

	BAI_FuncIn();

	{
		if(-1 == psInfo->lFd) {
			BAI_Info("invalid file description\n");
			eRet = eCOMM_DD_IRCUT_FAIL;
		}
		
		switch(lType) {
			case eCOMM_DD_IRCUT_TYPE_DAY:
				eRet = ioctl(psInfo->lFd, HI_IRCUT_REG_SET, NULL);
				psInfo->eIrcutType = eCOMM_DD_IRCUT_TYPE_DAY;
				break;

			case eCOMM_DD_IRCUT_TYPE_NIGHT:
				eRet = ioctl(psInfo->lFd, HI_IRCUT_REG_CLEAR, NULL);
				psInfo->eIrcutType = eCOMM_DD_IRCUT_TYPE_NIGHT;
				break;

			default:
				break;
		}
	}

	BAI_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_IRCUT_DeInit(sCOMM_DD_IRCUT_Info* psInfo)
* arg : sCOMM_DD_IRCUT_Info* psInfo
* ret : eCOMM_DD_IRCUT_Ret
* note :
*********************************************/
eCOMM_DD_IRCUT_Ret eCOMM_DD_IRCUT_DeInit(sCOMM_DD_IRCUT_Info* psInfo) {
	eCOMM_DD_IRCUT_Ret eRet = eCOMM_DD_IRCUT_SUCCESS;

	BAI_FuncIn();

	{
#if 1
		close(psInfo->lFd);
#endif
		psInfo->lFd = -1;
	}

	BAI_FuncOut();

	return eRet;
}

#ifdef __cplusplus
}
#endif
