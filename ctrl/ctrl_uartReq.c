#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "ctrl_uartReq.h"
#include "ctrl_uartReq_parseCommand.h"
#include "ctrl_interface.h"

#include <assert.h>

/***************************************************
	macro / enum
***************************************************/
//#define CTRL_UARTREQ_DEBUG
#ifdef CTRL_UARTREQ_DEBUG
#define CU_Debug(fmt, arg...) fprintf(stdout, "[ CU ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CU_Debug(fmt, arg...)
#endif

#define CU_FuncIn() CU_Debug("in\n")
#define CU_FuncOut() CU_Debug("out\n")
#define CU_iVal(iVal) CU_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CU_lVal(lVal) CU_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CU_PtVal(ptVal) CU_Debug("pt %s @ %p\n", #ptVal, ptVal)

#define CTRL_UARTREQ_COMMANDMAX 128
#define CTRL_UARTREQ_REQUESTMAX 1024
#define CTRL_UARTREQ_FEEDBACKMAX 128

/***************************************************
	variable
***************************************************/

/***************************************************
	prototype
***************************************************/
void* pvCTRL_UARTREQ_Thread(void* pvArg);
void vCTRL_UARTREQ_Thread_Cleanup(void* pvArg);
eCTRL_UARTREQ_Ret eCTRL_UARTREQ_Feedback(sCTRL_UARTREQ_ThreadArg* psThreadArg, sCTRL_UARTREQ_PARSECOMMAND_Info* psCmdInfo, LONG lReqCnt);

/***************************************************
	function
***************************************************/

/*********************************************
* func : eCTRL_UARTREQ_Init(sCTRL_UARTREQ_Info* psInfo, sCOMM_QUEUE* psCommandQueue, sCOMM_QUEUE* psFeedbackQueue, sCOMM_QUEUE* psRequestQueue)
* arg : sCTRL_UARTREQ_Info* psInfo, sCOMM_QUEUE* psCommandQueue, sCOMM_QUEUE* psFeedbackQueue, sCOMM_QUEUE* psRequestQueue
* ret : eCTRL_UARTREQ_Ret
* note :
*********************************************/
eCTRL_UARTREQ_Ret eCTRL_UARTREQ_Init(sCTRL_UARTREQ_Info* psInfo, sCOMM_QUEUE* psCommandQueue, sCOMM_QUEUE* psFeedbackQueue, sCOMM_QUEUE* psRequestQueue) {
	eCTRL_UARTREQ_Ret eRet = CTRL_UARTREQ_SUCCESS;

	CU_FuncIn();

	{
		/* init the two queue */
		psInfo->psCommandQueue = psCommandQueue;
		psInfo->psFeedbackQueue = psFeedbackQueue;
		psInfo->psRequestQueue = psRequestQueue;

		/* init status and tid */
		psInfo->eStatus = CTRL_UARTREQ_Status_NotEnd;
		psInfo->tid = -1;

		/* init resource */
		MALLOC(sizeof(sCTRL_UARTREQ_ThreadArg));
		psInfo->psThreadArg = malloc(sizeof(sCTRL_UARTREQ_ThreadArg));
		if(!psInfo->psThreadArg) {
			CU_Debug("malloc error\n");
			return CTRL_UARTREQ_MALLOC;
		}
		MALLOC(CTRL_UARTREQ_COMMANDMAX * sizeof(UCHAR));
		psInfo->pucPayLoad = malloc(CTRL_UARTREQ_COMMANDMAX * sizeof(UCHAR));
		if(!psInfo->pucPayLoad) {
			CU_Debug("malloc error\n");
			return CTRL_UARTREQ_MALLOC;
		}
	}

	CU_FuncOut();

	return eRet;
}

/*********************************************
* func : eCTRL_UARTREQ_Deinit(sCTRL_UARTREQ_Info* psInfo)
* arg : sCTRL_UARTREQ_Info* psInfo
* ret : eCTRL_UARTREQ_Ret
* note :
*********************************************/
eCTRL_UARTREQ_Ret eCTRL_UARTREQ_Deinit(sCTRL_UARTREQ_Info* psInfo) {
	eCTRL_UARTREQ_Ret eRet = CTRL_UARTREQ_SUCCESS;

	CU_FuncIn();

	{
		psInfo->eStatus = CTRL_UARTREQ_Status_NotEnd;
		psInfo->psCommandQueue = NULL;
		psInfo->psRequestQueue = NULL;
		psInfo->psFeedbackQueue = NULL;
		psInfo->tid = -1;

		/* release resource */
		if(psInfo->psThreadArg) {
			free(psInfo->psThreadArg);
			psInfo->psThreadArg = NULL;
		}

		if(psInfo->pucPayLoad) {
			free(psInfo->pucPayLoad);
			psInfo->pucPayLoad = NULL;
		}

	}

	CU_FuncOut();

	return eRet;
}

/*********************************************
* func : eCTRL_UARTREQ_StartThread(sCTRL_UARTREQ_Info* psInfo)
* arg : sCTRL_UARTREQ_Info* psInfo
* ret : eCTRL_UARTREQ_Ret
* note :
*********************************************/
eCTRL_UARTREQ_Ret eCTRL_UARTREQ_StartThread(sCTRL_UARTREQ_Info* psInfo) {
	eCTRL_UARTREQ_Ret eRet = CTRL_UARTREQ_SUCCESS;
	sCTRL_UARTREQ_ThreadArg* psThreadArg = psInfo->psThreadArg;
	LONG lRet = 0;

	CU_FuncIn();

	{
		/* init argument */
		psThreadArg->peStatus = &psInfo->eStatus;
		psThreadArg->psCommandQueue = psInfo->psCommandQueue;
		psThreadArg->psFeedbackQueue = psInfo->psFeedbackQueue;
		psThreadArg->psRequestQueue = psInfo->psRequestQueue;
		psThreadArg->pucCommand = psInfo->pucPayLoad;

		/* start thread */
		psInfo->eStatus = CTRL_UARTREQ_Status_NotEnd;
		lRet = pthread_create(&psInfo->tid, NULL, pvCTRL_UARTREQ_Thread, (void*)psThreadArg);
		if(lRet != 0) {
			CU_Debug("create thread error\n");
			return CTRL_UARTREQ_THREAD;
		}

	}

	CU_FuncOut();

	return eRet;
}

/*********************************************
* func : eCTRL_UARTREQ_StopThread(sCTRL_UARTREQ_Info* psInfo)
* arg : sCTRL_UARTREQ_Info* psInfo
* ret : eCTRL_UARTREQ_Ret
* note :
*********************************************/
eCTRL_UARTREQ_Ret eCTRL_UARTREQ_StopThread(sCTRL_UARTREQ_Info* psInfo) {
	eCTRL_UARTREQ_Ret eRet = CTRL_UARTREQ_SUCCESS;
	void* pvThreadRet;

	CU_FuncIn();

	{
		/* toggle the status */
		psInfo->eStatus = CTRL_UARTREQ_Status_IsEnd;

		/* cancel the thread */
		pthread_cancel(psInfo->tid);

		/* join the thread */
		pthread_join(psInfo->tid, &pvThreadRet);
		if(pvThreadRet == PTHREAD_CANCELED) {
			CU_Debug("thread have been canceled\n");
		}

	}

	CU_FuncOut();

	return eRet;
}

/*********************************************
* func : pvCTRL_UARTREQ_Thread(void* pvArg)
* arg : void* pvArg
* ret : void*
* note : main thread of uart
*********************************************/
void* pvCTRL_UARTREQ_Thread(void* pvArg) {
	sCTRL_UARTREQ_ThreadArg* psThreadArg = (sCTRL_UARTREQ_ThreadArg*)pvArg;
	sCTRL_UARTREQ_PARSECOMMAND_Info sParseCommandInfo;
	eCTRL_UARTREQ_PARSECOMMAND_Ret eParseCommand = CTRL_UARTREQ_PARSECOMMAND_SUCCESS;
	sCTRL_UARTREQ_SAVEREQ_Info* psSaveReq = &psThreadArg->sSaveReq;
	eCTRL_UARTREQ_Ret eUartRet = CTRL_UARTREQ_SUCCESS;
	eCTRL_UARTREQ_PARSECOMMAND_Status* peParseCmdStatus = & sParseCommandInfo.sFuncArg.eStatus;
	LONG lReqCnt = 0;

	CU_FuncIn();

	{
		/* register cleanup function */
		pthread_cleanup_push(vCTRL_UARTREQ_Thread_Cleanup, pvArg);

		/* register feedback */
		vCOMM_CF_RegCF(&stCfCtlRetUart);

		/* malloc request resource */
		MALLOC(CTRL_UARTREQ_REQUESTMAX * sizeof(UCHAR));
		psThreadArg->pucRequest = malloc(CTRL_UARTREQ_REQUESTMAX * sizeof(UCHAR));
		if(!psThreadArg->pucRequest) {
			CU_Debug("thread malloc error\n");
			return NULL;
		}
		
		MALLOC(CTRL_UARTREQ_FEEDBACKMAX * sizeof(UCHAR));
		psThreadArg->pucFeedBack = malloc(CTRL_UARTREQ_FEEDBACKMAX * sizeof(UCHAR));
		if(!psThreadArg->pucFeedBack) {
			CU_Debug("thread malloc error\n");
			return NULL;
		}

		/* init request saver */
		eCTRL_UARTREQ_SAVEREQ_Init(psSaveReq, psThreadArg->pucRequest, CTRL_UARTREQ_REQUESTMAX);

		/* init parse command info */
		*peParseCmdStatus = CTRL_UARTREQ_PARSECOMMAND_STATUS_NotConfig;
		
		while(*psThreadArg->peStatus != CTRL_UARTREQ_Status_IsEnd) {
			/* reset the request count */
			lReqCnt = 0;
			
			/* read command from command queue */
			memset(psThreadArg->pucCommand, 0, CTRL_UARTREQ_COMMANDMAX);
			vCOMM_LQ_Read(psThreadArg->psCommandQueue, (CHAR**)&psThreadArg->pucCommand);
			CU_Debug("get command : %s\n", psThreadArg->pucCommand);

			/* parse the command */
			eParseCommand = eCTRL_UARTREQ_PARSECOMMAND_HandleCommand(&sParseCommandInfo,
				psThreadArg->pucCommand, psThreadArg->pucFeedBack, psSaveReq);
			if(eParseCommand != CTRL_UARTREQ_PARSECOMMAND_SUCCESS) {
				CU_Debug("parse command error with <%d>\n", eParseCommand);
				/* feedback bad command */
				vCOMM_LQ_Write(psThreadArg->psFeedbackQueue, (CHAR*)psThreadArg->pucFeedBack, AUTO_DETECT_STRLEN);
				continue;
			}

			/* write the request to list */
			CHAR* pcReqEntryPt;
			while (eCTRL_UARTREQ_SAVEREQ_ReadReq(psSaveReq, &pcReqEntryPt) != CTRL_UARTREQ_SAVEREQ_READEND) {
				vCOMM_LQ_Write(psThreadArg->psRequestQueue, pcReqEntryPt, AUTO_DETECT_STRLEN);
				CU_Debug("%s\n", pcReqEntryPt);
				lReqCnt ++;
			}

			/* feedback */
			eUartRet = eCTRL_UARTREQ_Feedback(psThreadArg, &sParseCommandInfo, lReqCnt);
			if (eUartRet != CTRL_UARTREQ_SUCCESS) {
				CU_Debug("feedback error with <%d>\n", eUartRet);
			} else {
				if (!(*peParseCmdStatus & CTRL_UARTREQ_PARSECOMMAND_STATUS_Configed)) {
					CU_Debug("the system will be configed !\n");
					*peParseCmdStatus |= CTRL_UARTREQ_PARSECOMMAND_STATUS_Configed;
				}
			}

		}

		/* execute cleanup function */
		pthread_cleanup_pop(1);
		
	}

	CU_FuncOut();

	return NULL;
}

/*********************************************
* func : vCTRL_UARTREQ_Thread_Cleanup(void* pvArg)
* arg : void* pvArg
* ret : void
* note : thread clean up function
*********************************************/
void vCTRL_UARTREQ_Thread_Cleanup(void* pvArg) {
	sCTRL_UARTREQ_ThreadArg* psThreadArg = (sCTRL_UARTREQ_ThreadArg*)pvArg;

	CU_FuncIn();

	{
		if(psThreadArg->pucRequest) {
			free(psThreadArg->pucRequest);
			psThreadArg->pucRequest = NULL;
		}

		if(psThreadArg->pucFeedBack) {
			free(psThreadArg->pucFeedBack);
			psThreadArg->pucFeedBack = NULL;
		}

		/* deregist cf */
		vCOMM_CF_DeregCF(&stCfCtlRetUart);
	}

	CU_FuncOut();	
}

/*********************************************
* func : eCTRL_UARTREQ_Feedback(sCTRL_UARTREQ_ThreadArg* psThreadArg, sCTRL_UARTREQ_PARSECOMMAND_Info* psCmdInfo, LONG lReqCnt)
* arg : sCTRL_UARTREQ_ThreadArg* psThreadArg, sCTRL_UARTREQ_PARSECOMMAND_Info* psCmdInfo, LONG lReqCnt
* ret : eCTRL_UARTREQ_Ret
* note : feedback to get command
*********************************************/
eCTRL_UARTREQ_Ret eCTRL_UARTREQ_Feedback(sCTRL_UARTREQ_ThreadArg* psThreadArg, sCTRL_UARTREQ_PARSECOMMAND_Info* psCmdInfo, LONG lReqCnt) {
	eCTRL_UARTREQ_Ret eRet = CTRL_UARTREQ_SUCCESS;
	LONG lCfId;
	LONG lCfRet;
	LONG lReqCntTmp = 0;
	sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psCmdArg = &psCmdInfo->sFuncArg;
	LONG lIsFault = 0;

	CU_FuncIn();

	{
		
		/* wait all the feedback from base process */
		while (lReqCntTmp != lReqCnt) {
			vCOMM_CF_GetIdCF(&stCfCtlRetUart, &lCfId, COMM_COND_BLOCK);
			vCOMM_CF_RdCF(&stCfCtlRetUart, &lCfRet);

			switch(lCfRet) {
				case COMM_CF_FEEDBACKFINISHFAIL:
					CU_Debug("feedback fail\n");
					lIsFault = 1;
					lReqCntTmp++;
					break;
					
				case COMM_CF_FEEDBACKATONCEFAIL:
					CU_Debug("feedback at once fail\n");
					break;
					
				case COMM_CF_FEEDBACKATONCESUC:
					CU_Debug("feedback at once\n");
					break;
					
				case COMM_CF_FEEDBACKFINISHSUC:
					CU_Debug("feedback success\n");
					lReqCntTmp++;
					break;
					
				case COMM_CF_FEEDBACKOTHERGUI:
					CU_Debug("feedback from other gui\n");
					break;
					
				case COMM_CF_FEEDBACKFINISHSYSQUIT:
					CU_Debug("feedback system quit\n");
					lReqCntTmp++;
					break;
					
				default:
					break;
			}
		}

		/* fault case should change the feedback */
		if (lIsFault) {
			/* indicate at least recieve a error feedback */
			psCmdArg->eRet = CTRL_UARTREQ_PARSECOMMAND_Function_FeedbackError;
			vGetFeedback(psCmdArg, (CHAR*)psThreadArg->pucFeedBack);
			eRet = CTRL_UARTREQ_FEEDBACK;
		}

		/* feedback to getCommand */
		vCOMM_LQ_Write(psThreadArg->psFeedbackQueue, (CHAR*)psThreadArg->pucFeedBack, AUTO_DETECT_STRLEN);
	}

	CU_FuncOut();

	return eRet;
}


#ifdef __cplusplus
}
#endif
