#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "comm_dd_button.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

/***************************************************
	macro / enum
***************************************************/
//#define COMM_DD_BUTTON_DEBUG
#ifdef COMM_DD_BUTTON_DEBUG
#define CDB_Debug(fmt, arg...) fprintf(stdout, "[ CDB ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CDB_Debug(fmt, arg...)
#endif

#define CDB_FuncIn() CDB_Debug("in\n")
#define CDB_FuncOut() CDB_Debug("out\n")
#define CDB_iVal(iVal) CDB_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CDB_lVal(lVal) CDB_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CDB_PtVal(ptVal) CDB_Debug("pt %s @ %p\n", #ptVal, ptVal)


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
* func : eCOMM_DD_BUTTON_Init(sCOMM_DD_BUTTON_Info* psInfo, CHAR* pcDev)
* arg  : sCOMM_DD_BUTTON_Info* psInfo, CHAR* pcDev
* ret  : eCOMM_DD_BUTTON_Ret
* note :
*********************************************/
eCOMM_DD_BUTTON_Ret eCOMM_DD_BUTTON_Init(sCOMM_DD_BUTTON_Info* psInfo, CHAR* pcDev) {
	eCOMM_DD_BUTTON_Ret eRet = COMM_DD_BUTTON_SUCCESS;

	CDB_FuncIn();

	{
		strcpy(psInfo->pcDev,pcDev);
	}

	CDB_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_BUTTON_Open(sCOMM_DD_BUTTON_Info* psInfo)
* arg  : sCOMM_DD_BUTTON_Info* psInfo
* ret  : eCOMM_DD_BUTTON_Ret
* note :
*********************************************/
eCOMM_DD_BUTTON_Ret eCOMM_DD_BUTTON_Open(sCOMM_DD_BUTTON_Info* psInfo) {
	eCOMM_DD_BUTTON_Ret eRet = COMM_DD_BUTTON_SUCCESS;

	CDB_FuncIn();

	{
		psInfo->lFd = open(psInfo->pcDev,O_RDWR); 
		if(psInfo->lFd  < 0){ 
			CDB_Debug("Open the button device failed\n");

			return COMM_DD_BUTTON_OPEN; 
		}

	}

	CDB_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_BUTTON_Read(sCOMM_DD_BUTTON_Info* psInfo)
* arg  : sCOMM_DD_BUTTON_Info* psInfo
* ret  : eCOMM_DD_BUTTON_Ret
* note :
*********************************************/
eCOMM_DD_BUTTON_Ret eCOMM_DD_BUTTON_Read(sCOMM_DD_BUTTON_Info* psInfo) {
	eCOMM_DD_BUTTON_Ret eRet = COMM_DD_BUTTON_SUCCESS;

	CDB_FuncIn();

	{
		UCHAR ucBuf[COMM_DD_BUTTON_READBUFMAX];
		memset(ucBuf,0,sizeof(ucBuf));
		read(psInfo->lFd, ucBuf, COMM_DD_BUTTON_READBUFMAX);

	}

	CDB_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_BUTTON_Write(sCOMM_DD_BUTTON_Info* psInfo)
* arg  : sCOMM_DD_BUTTON_Info* psInfo
* ret  : eCOMM_DD_BUTTON_Ret
* note :
*********************************************/
eCOMM_DD_BUTTON_Ret eCOMM_DD_BUTTON_Write(sCOMM_DD_BUTTON_Info* psInfo) {
	eCOMM_DD_BUTTON_Ret eRet = COMM_DD_BUTTON_SUCCESS;

	CDB_FuncIn();

	{
		UCHAR ucBuf[COMM_DD_BUTTON_READBUFMAX];
		memset(ucBuf,0,sizeof(ucBuf));
		write(psInfo->lFd, ucBuf, COMM_DD_BUTTON_READBUFMAX);

	}

	CDB_FuncOut();

	return eRet;	
}

/*********************************************
* func : eCOMM_DD_BUTTON_Close(sCOMM_DD_BUTTON_Info* psInfo)
* arg  : sCOMM_DD_BUTTON_Info* psInfo
* ret  : eCOMM_DD_BUTTON_Ret
* note :
*********************************************/
eCOMM_DD_BUTTON_Ret eCOMM_DD_BUTTON_Close(sCOMM_DD_BUTTON_Info* psInfo) {
	eCOMM_DD_BUTTON_Ret eRet = COMM_DD_BUTTON_SUCCESS;

	CDB_FuncIn();

	{
		eRet = close(psInfo->lFd); 
		if(eRet < 0){ 
			CDB_Debug("close button device failed\n"); 

			return COMM_DD_BUTTON_CLOSE;
		}
	}

	CDB_FuncOut();

	return eRet;
}

#ifdef __cplusplus
}
#endif
