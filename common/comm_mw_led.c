#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "comm_mw_led.h"
#include <sys/stat.h>
#include <fcntl.h>

/***************************************************
	macro / enum
***************************************************/
#define LED_TEST

//#define COMM_MW_LED_DEBUG
#ifdef COMM_MW_LED_DEBUG
#define CML_Debug(fmt, arg...) fprintf(stdout, "[ CML ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CML_Debug(fmt, arg...)
#endif

#define CML_FuncIn() //CML_Debug("in\n")
#define CML_FuncOut()// CML_Debug("out\n")
#define CML_iVal(iVal) CML_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CML_lVal(lVal) CML_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CML_PtVal(ptVal) CML_Debug("pt %s @ %p\n", #ptVal, ptVal)


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
* func : eCOMM_MW_LED_Init(sCOMM_MW_LED_Info* psInfo)
* arg : sCOMM_MW_LED_Info* psInfo
* ret : eCOMM_MW_LED_Ret
* note :
*********************************************/
eCOMM_MW_LED_Ret eCOMM_MW_LED_Init(sCOMM_MW_LED_Info* psInfo) {
	eCOMM_MW_LED_Ret eRet = COMM_MW_LED_SUCCESS;

	CML_FuncIn();

	{
		/* init parameter */
		psInfo->lFd = -1;
		psInfo->lStatus = COMM_MW_LED_VALID;
		memset(psInfo->cDevName, 0, MAX_BUF_LEN);
		
		/* init lock */
		pthread_mutex_init(&psInfo->mutStatus, NULL);

	}

	CML_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_MW_LED_Open(sCOMM_MW_LED_Info* psInfo)
* arg : sCOMM_MW_LED_Info* psInfo
* ret : eCOMM_MW_LED_Ret
* note :
*********************************************/
eCOMM_MW_LED_Ret eCOMM_MW_LED_Open(sCOMM_MW_LED_Info* psInfo) {
	eCOMM_MW_LED_Ret eRet = COMM_MW_LED_SUCCESS;

	CML_FuncIn();

	if(1){
		psInfo->lFd = open(psInfo->cDevName,O_RDWR);
		if(-1 == psInfo->lFd) {
			eRet = COMM_MW_LED_FAIL;
			CML_Debug("Can not open the file!\n")
			return eRet;
		}
		psInfo->lStatus = COMM_MW_LED_OPEN;
	}

	CML_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_MW_LED_Close(sCOMM_MW_LED_Info* psInfo)
* arg : sCOMM_MW_LED_Info* psInfo
* ret : eCOMM_MW_LED_Ret
* note :
*********************************************/
eCOMM_MW_LED_Ret eCOMM_MW_LED_Close(sCOMM_MW_LED_Info* psInfo) {
	eCOMM_MW_LED_Ret eRet = COMM_MW_LED_SUCCESS;

	CML_FuncIn();

	if(1){
		eRet = close(psInfo->lFd);
		if(-1 == eRet) {
			eRet = COMM_MW_LED_FAIL;
			CML_Debug("Can not close the file!\n")
			return eRet;
		}
		psInfo->lStatus = COMM_MW_LED_VALID;
	}

	CML_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_MW_LED_Deinit(sCOMM_MW_LED_Info* psInfo)
* arg : sCOMM_MW_LED_Info* psInfo
* ret : eCOMM_MW_LED_Ret
* note :
*********************************************/
eCOMM_MW_LED_Ret eCOMM_MW_LED_Deinit(sCOMM_MW_LED_Info* psInfo) {
	eCOMM_MW_LED_Ret eRet = COMM_MW_LED_SUCCESS;

	CML_FuncIn();

	{
		/* deinit parameter */
		psInfo->lFd = -1;
		psInfo->lStatus = COMM_MW_LED_VALID;
		
		/* deinit  lock */
		pthread_mutex_destroy(&psInfo->mutStatus);		

	}

	CML_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_MW_LED_RedOn(sCOMM_MW_LED_Info* psInfo)
* arg : sCOMM_MW_LED_Info* psInfo
* ret : eCOMM_MW_LED_Ret
* note :
*********************************************/
eCOMM_MW_LED_Ret eCOMM_MW_LED_RedOn(sCOMM_MW_LED_Info* psInfo) {
	eCOMM_MW_LED_Ret eRet = COMM_MW_LED_SUCCESS;

	CML_FuncIn();

	{
#ifdef LED_TEST		
		write(psInfo->lFd, NULL, LED_REDON);	
#else
		printf("Led-Red-On\n");
#endif
	}

	CML_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_MW_LED_RedOff(sCOMM_MW_LED_Info* psInfo)
* arg : sCOMM_MW_LED_Info* psInfo
* ret : eCOMM_MW_LED_Ret
* note :
*********************************************/
eCOMM_MW_LED_Ret eCOMM_MW_LED_RedOff(sCOMM_MW_LED_Info* psInfo) {
	eCOMM_MW_LED_Ret eRet = COMM_MW_LED_SUCCESS;

	CML_FuncIn();

	{
#ifdef LED_TEST		
		write(psInfo->lFd, NULL, LED_REDOFF);
#else
		printf("Led-Red-Off\n");
#endif
	}

	CML_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_MW_LED_GreenOn(sCOMM_MW_LED_Info* psInfo)
* arg : sCOMM_MW_LED_Info* psInfo
* ret : eCOMM_MW_LED_Ret
* note :
*********************************************/
eCOMM_MW_LED_Ret eCOMM_MW_LED_GreenOn(sCOMM_MW_LED_Info* psInfo) {
	eCOMM_MW_LED_Ret eRet = COMM_MW_LED_SUCCESS;

	CML_FuncIn();

	{
#ifdef LED_TEST		
		write(psInfo->lFd, NULL, LED_GREENON);
#else
		printf("Led-Green-On\n");
#endif
	}

	CML_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_MW_LED_GreenOff(sCOMM_MW_LED_Info* psInfo)
* arg : sCOMM_MW_LED_Info* psInfo
* ret : eCOMM_MW_LED_Ret
* note :
*********************************************/
eCOMM_MW_LED_Ret eCOMM_MW_LED_GreenOff(sCOMM_MW_LED_Info* psInfo) {
	eCOMM_MW_LED_Ret eRet = COMM_MW_LED_SUCCESS;

	CML_FuncIn();

	{
#ifdef LED_TEST		
		write(psInfo->lFd, NULL, LED_GREENOFF);
#else
		printf("Led-Green-Off\n");
#endif
	}

	CML_FuncOut();

	return eRet;
}

#ifdef __cplusplus
}
#endif
