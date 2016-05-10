#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "ctrl_api_button.h"

/***************************************************
	macro / enum
***************************************************/
//#define CTRL_API_BUTTON_DEBUG
#ifdef CTRL_API_BUTTON_DEBUG
#define CAB_Debug(fmt, arg...) fprintf(stdout, "[ CAB ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CAB_Debug(fmt, arg...)
#endif

#define CAB_FuncIn() CAB_Debug("in\n")
#define CAB_FuncOut() CAB_Debug("out\n")
#define CAB_iVal(iVal) CAB_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CAB_lVal(lVal) CAB_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CAB_PtVal(ptVal) CAB_Debug("pt %s @ %p\n", #ptVal, ptVal)


/***************************************************
	variable
***************************************************/

/***************************************************
	prototype
***************************************************/
void* pvCTRL_BUTTON_Thread(void* pvArg);
/***************************************************
	function
***************************************************/

/*********************************************
* func : eCTRL_API_BUTTON_Init(sCTRL_API_BUTTON_Info* psInfo)
* arg  : sCTRL_API_BUTTON_Info* psInfo
* ret  : eCTRL_API_BUTTON_Ret
* note :
*********************************************/
eCTRL_API_BUTTON_Ret eCTRL_API_BUTTON_Init(sCTRL_API_BUTTON_Info* psInfo) {
	eCTRL_API_BUTTON_Ret eRet = CTRL_API_BUTTON_SUCCESS;

	CAB_FuncIn();

	{
		/* init status and tid */
		psInfo->eStatus = CTRL_API_BUTTON_Status_NotEnd;
		psInfo->tid = -1;

		/* init resource */
		psInfo->psThreadArg = malloc(sizeof(sCTRL_API_BUTTON_ThreadArg));
		if (!psInfo->psThreadArg) {
			CAB_Debug("malloc error\n");
			return CTRL_API_BUTTON_MALLOC;
		}
	}

	CAB_FuncOut();

	return eRet;
}

/*********************************************
* func : eCTRL_API_BUTTON_Deinit(sCTRL_API_BUTTON_Info* psInfo)
* arg  : sCTRL_API_BUTTON_Info* psInfo
* ret  : eCTRL_API_BUTTON_Ret
* note :
*********************************************/
eCTRL_API_BUTTON_Ret eCTRL_API_BUTTON_Deinit(sCTRL_API_BUTTON_Info* psInfo) {
	eCTRL_API_BUTTON_Ret eRet = CTRL_API_BUTTON_SUCCESS;

	CAB_FuncIn();

	{
		psInfo->eStatus = CTRL_API_BUTTON_Status_NotEnd;
		psInfo->tid = -1;

		/* release resource */
		if (psInfo->psThreadArg) {
			free(psInfo->psThreadArg);
			psInfo->psThreadArg = NULL;
		}

	}

	CAB_FuncOut();

	return eRet;
}

/*********************************************
* func : eCTRL_API_BUTTON_StartThread(sCTRL_API_BUTTON_Info* psInfo)
* arg  : sCTRL_API_BUTTON_Info* psInfo
* ret  : eCTRL_API_BUTTON_Ret
* note :
*********************************************/
eCTRL_API_BUTTON_Ret eCTRL_API_BUTTON_StartThread(sCTRL_API_BUTTON_Info* psInfo) {
	eCTRL_API_BUTTON_Ret eRet = CTRL_API_BUTTON_SUCCESS;
	sCTRL_API_BUTTON_ThreadArg* psThreadArg = psInfo->psThreadArg;

	CAB_FuncIn();

	{
		
		psThreadArg->peStatus = &psInfo->eStatus;
		psInfo->eStatus = CTRL_API_BUTTON_Status_NotEnd;
		eRet = pthread_create(&psInfo->tid, NULL, pvCTRL_BUTTON_Thread, (void*) psThreadArg);
		if(eRet != CTRL_API_BUTTON_SUCCESS) {
			CAB_Debug("create thread error\n");
			return CTRL_API_BUTTON_THREAD;
		}

	}

	CAB_FuncOut();

	return eRet;
}

/*********************************************
* func : eCTRL_API_BUTTON_StopThread(sCTRL_API_BUTTON_Info* psInfo)
* arg  : sCTRL_API_BUTTON_Info* psInfo
* ret  : eCTRL_API_BUTTON_Ret
* note :
*********************************************/
eCTRL_API_BUTTON_Ret eCTRL_API_BUTTON_StopThread(sCTRL_API_BUTTON_Info* psInfo) {
	eCTRL_API_BUTTON_Ret eRet = CTRL_API_BUTTON_SUCCESS;
	
	CAB_FuncIn();

	{
		/* toggle the status */
		psInfo->eStatus = CTRL_API_BUTTON_Status_IsEnd;
		
		/* join the thread */
		pthread_cancel(psInfo->tid);
		pthread_join(psInfo->tid, (void**) NULL);

	}

	CAB_FuncOut();

	return eRet;
}

/* implement */
void* pvCTRL_BUTTON_Thread(void* pvArg){
	sCTRL_API_BUTTON_ThreadArg *psThreadArg = (sCTRL_API_BUTTON_ThreadArg *)pvArg;

	CAB_FuncIn();

	{
		/* for command string */
		LONG lVal=0;
		sCOMM_CMD sCmd;
		CHAR cOneCmd[COMM_CMDSPACKETMAXCHAR];
		CHAR* pcOneCmd=cOneCmd;

		eCOMM_DD_BUTTON_Init(&psThreadArg->sButtonInfo, BUTTON_DEV);
		eCOMM_DD_BUTTON_Open(&psThreadArg->sButtonInfo);
		
		while (*psThreadArg->peStatus != CTRL_API_BUTTON_Status_IsEnd)  
		{   
			eCOMM_DD_BUTTON_Read(&psThreadArg->sButtonInfo);
			{
				CAB_Debug("Read button event\n");
				lVal=1;
				vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_RECORD, COMM_SUBCMD_RECSNAP, lVal); 
				vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
				vCOMM_CMD_AddReqInfo(pcOneCmd,COMM_UI_BUTTON);	

				/* send to request list */
				vCOMM_LQ_Write(&sReqList, pcOneCmd, AUTO_DETECT_STRLEN);
			}
		}  
		CAB_Debug("Exit button thread\n");
		eCOMM_DD_BUTTON_Close(&psThreadArg->sButtonInfo);
	}

	CAB_FuncOut();
	
	return NULL; 	
}

#ifdef __cplusplus
}
#endif
