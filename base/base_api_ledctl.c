#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "base_api_ledctl.h"
#include "comm_timer.h"
#include <errno.h>

/***************************************************
	macro / enum
***************************************************/
//#define BASE_API_LEDCTL_DEBUG
#ifdef BASE_API_LEDCTL_DEBUG
#define BAL_Debug(fmt, arg...) fprintf(stdout, "[ BAL ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define BAL_Debug(fmt, arg...)
#endif

#define BAL_FuncIn() //BAL_Debug("in\n")
#define BAL_FuncOut() //BAL_Debug("out\n")
#define BAL_iVal(iVal) BAL_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define BAL_lVal(lVal) BAL_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define BAL_PtVal(ptVal) BAL_Debug("pt %s @ %p\n", #ptVal, ptVal)
#define BAL_Handle_Error(en, msg)	\
	     do { errno =en; perror(msg);} while (0)

/***************************************************
	variable
***************************************************/

/***************************************************
	prototype
***************************************************/
void eBASE_API_LEDCTL_GreenBlink(void* pvarg);
void eBASE_API_LEDCTL_GreenOn(void* pvarg);
void eBASE_API_LEDCTL_GreenOff(void* pvarg);
void eBASE_API_LEDCTL_RedOn(void* pvarg);
void eBASE_API_LEDCTL_RedOff(void* pvarg);
eBASE_API_LEDCTL_Ret  eBASE_API_LEDCTL_LedUpdate(sBASE_API_LEDCTL_Param* psLedParam);
void* pvBASE_API_LEDCTL_ThreadHandle(void* psArg);
void pvBASE_API_LEDCTL_Thread_Cleanup(void* pvArg);


/***************************************************
	function
***************************************************/

/*********************************************
* func : eBASE_API_LEDCTL_Init(sBASE_API_LEDCTL_Info* psInfo)
* arg : sBASE_API_LEDCTL_Info* psInfo
* ret : eBASE_API_LEDCTL_Ret
* note :
*********************************************/
eBASE_API_LEDCTL_Ret eBASE_API_LEDCTL_Init(sBASE_API_LEDCTL_Info* psInfo) {
	eBASE_API_LEDCTL_Ret eRet = BASE_API_LEDCTL_SUCCESS;
	eCOMM_MW_LED_Ret eLedRet = COMM_MW_LED_SUCCESS;
	sBASE_API_LED_CTL_Signal* psSignal;
	sBASE_API_LED_CTL_Signal* psRecSignal;
	sCOMM_MW_LED_Info* psLedInfo;
		
	BAL_FuncIn();

	{
		/* init */
		psInfo->psLedInfo = NULL;
		psInfo->psArg = NULL;
		
		/* malloc resource */
		MALLOC(sizeof(sCOMM_MW_LED_Info));
		psInfo->psLedInfo = malloc(sizeof(sCOMM_MW_LED_Info));
		if(!psInfo->psLedInfo) {
			BAL_Debug("malloc error\n");
			return BASE_API_LEDCTL_MALLOC;
		}

		psLedInfo = psInfo->psLedInfo;
		eLedRet = eCOMM_MW_LED_Init(psLedInfo);
		if (COMM_MW_LED_SUCCESS != eLedRet) {
			eRet = BASE_API_LEDCTL_LEDINIT;
			return eRet;
		}
		strcpy(psLedInfo->cDevName,LED_DEV);
		//printf("=== LedDev Name:<%s>===\n",psLedInfo->cDevName);
		MALLOC(sizeof(sBASE_API_LEDCTL_ThreadArg));
		psInfo->psArg = malloc(sizeof(sBASE_API_LEDCTL_ThreadArg));
		if(!psInfo->psArg) {
			BAL_Debug("malloc error\n");
			return BASE_API_LEDCTL_MALLOC;
		}
		
		MALLOC(sizeof(sBASE_API_LED_CTL_Signal));
		psInfo->psSignal = malloc(sizeof(sBASE_API_LED_CTL_Signal));
		if(!psInfo->psSignal) {
			BAL_Debug("malloc error\n");
			return BASE_API_LEDCTL_MALLOC;
		}

		MALLOC(sizeof(sBASE_API_LED_CTL_Signal));
		psInfo->psRecSignal = malloc(sizeof(sBASE_API_LED_CTL_Signal));
		if(!psInfo->psRecSignal) {
			BAL_Debug("malloc error\n");
			return BASE_API_LEDCTL_MALLOC;
		}
		
		psInfo->tid = -1;
		psInfo->eThreadStatus = BASE_API_LEDCTL_ISEND;

		/* init signal */
		psSignal = psInfo->psSignal;
		psSignal->eStatus = BASE_API_LEDCTL_NOTHAVESIG;
		pthread_cond_init(&psSignal->condStatus, NULL);
		pthread_mutex_init(&psSignal->mutStatus, NULL);

		/* init rec signal */
		psRecSignal = psInfo->psRecSignal;
		psRecSignal->eStatus = BASE_API_LEDCTL_NOTHAVESIG;
		pthread_cond_init(&psRecSignal->condStatus, NULL);
		pthread_mutex_init(&psRecSignal->mutStatus, NULL);		
		
	}

	BAL_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_API_LEDCTL_Deinit(sBASE_API_LEDCTL_Info* psInfo)
* arg : sBASE_API_LEDCTL_Info* psInfo
* ret : eBASE_API_LEDCTL_Ret
* note :
*********************************************/
eBASE_API_LEDCTL_Ret eBASE_API_LEDCTL_Deinit(sBASE_API_LEDCTL_Info* psInfo) {
	eBASE_API_LEDCTL_Ret eRet = BASE_API_LEDCTL_SUCCESS;
	eCOMM_MW_LED_Ret eLedRet = COMM_MW_LED_SUCCESS;
	sBASE_API_LED_CTL_Signal* psSignal;
	sBASE_API_LED_CTL_Signal* psRecSignal;
	sCOMM_MW_LED_Info* psLedInfo;

	BAL_FuncIn();

	{
		psLedInfo = psInfo->psLedInfo;
		eLedRet = eCOMM_MW_LED_Deinit(psLedInfo);
		if (COMM_MW_LED_SUCCESS != eLedRet) {
			eRet = BASE_API_LEDCTL_LEDINIT;
			return eRet;
		}
		if(psInfo->psLedInfo) {
			free(psInfo->psLedInfo);
			psInfo->psLedInfo = NULL;
		}

		if(psInfo->psArg) {
			free(psInfo->psArg);
			psInfo->psArg = NULL;
		}

		if(psInfo->psSignal) {
			/* deinit signal */
			psSignal = psInfo->psSignal;
			psSignal->eStatus = BASE_API_LEDCTL_NOTHAVESIG;
			pthread_mutex_destroy(&psSignal->mutStatus);
			pthread_cond_destroy(&psSignal->condStatus);
			
			free(psInfo->psSignal);
			psInfo->psSignal = NULL;
		}

		if(psInfo->psRecSignal) {
			/* deinit rec signal */
			psRecSignal = psInfo->psRecSignal;
			psRecSignal->eStatus = BASE_API_LEDCTL_NOTHAVESIG;
			pthread_mutex_destroy(&psRecSignal->mutStatus);
			pthread_cond_destroy(&psRecSignal->condStatus);
			
			free(psInfo->psRecSignal);
			psInfo->psRecSignal = NULL;
		}		

		psInfo->tid = -1;
		psInfo->eThreadStatus = BASE_API_LEDCTL_ISEND;

	}

	BAL_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_API_LEDCTL_StartThread(sBASE_API_LEDCTL_Info* psInfo)
* arg : sBASE_API_LEDCTL_Info* psInfo
* ret : eBASE_API_LEDCTL_Ret
* note :
*********************************************/
eBASE_API_LEDCTL_Ret eBASE_API_LEDCTL_StartThread(sBASE_API_LEDCTL_Info* psInfo) {
	eBASE_API_LEDCTL_Ret eRet = BASE_API_LEDCTL_SUCCESS;
	sBASE_API_LEDCTL_ThreadArg* psArg = psInfo->psArg;
	LONG lRet = 0;

	BAL_FuncIn();

	{
		/* init thread argument */
		psArg->peThreadStatus = BASE_API_LEDCTL_NOTEND;
		psArg->psLedInfo = psInfo->psLedInfo;
		psArg->psSignal = psInfo->psSignal;
		psArg->psRecSignal = psInfo->psRecSignal;
		/* start thread */
		lRet = pthread_create(&psInfo->tid, NULL, pvBASE_API_LEDCTL_ThreadHandle, (void*) psArg);
		if(lRet != 0) {
			BAL_Debug("create thread error\n");
			return BASE_API_LEDCTL_CREATETHREAD;
		}
		BAL_Debug("create thread OK\n");
	}

	BAL_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_API_LEDCTL_StopThread(sBASE_API_LEDCTL_Info* psInfo)
* arg : sBASE_API_LEDCTL_Info* psInfo
* ret : eBASE_API_LEDCTL_Ret
* note :
*********************************************/
eBASE_API_LEDCTL_Ret eBASE_API_LEDCTL_StopThread(sBASE_API_LEDCTL_Info* psInfo) {
	eBASE_API_LEDCTL_Ret eRet = BASE_API_LEDCTL_SUCCESS;
	sBASE_API_LEDCTL_ThreadArg* psArg = psInfo->psArg;
	void *res = NULL;
	
	BAL_FuncIn();

	{
		/* chage status */
		psArg->peThreadStatus = BASE_API_LEDCTL_ISEND;
		
		/* if current not busy, then cancel the thread */
		pthread_cancel(psInfo->tid);
		
		/* wait join the pthread */
		pthread_join(psInfo->tid, res);
		if (PTHREAD_CANCELED == res) {
			BAL_Debug("pthread was canceled!\n");
		}
		else {
			BAL_Debug("pthread wasn't canceled!\n");	
		}
	}

	BAL_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_API_LEDCTL_ChangeLed(sBASE_API_LEDCTL_Info* psInfo, eBASE_API_LEDCTL_LedNewStatus eStatus)
* arg : sBASE_API_LEDCTL_Info* psInfo, eBASE_API_LEDCTL_LedNewStatus eStatus
* ret : eBASE_API_LEDCTL_Ret
* note :
*********************************************/
eBASE_API_LEDCTL_Ret eBASE_API_LEDCTL_ChangeLed(sBASE_API_LEDCTL_ThreadArg* psInfo, eBASE_API_LEDCTL_LedNewStatus eStatus) {
	eBASE_API_LEDCTL_Ret eRet = BASE_API_LEDCTL_SUCCESS;
	sCOMM_MW_LED_Info* psLedInfo = psInfo->psLedInfo;
	LONG lNewStatus = 0;
	sBASE_API_LED_CTL_Signal* psSignal;

	BAL_FuncIn();

	{
		/* get lock */
		pthread_mutex_lock(&psLedInfo->mutStatus);

		/* change status */
		if((psLedInfo->lStatus & COMM_MW_LED_COLORMASK) != eStatus) {
			/* if the status is BASE_API_LEDCTL_NOFEEDBACK */
			if((psLedInfo->lStatus & COMM_MW_LED_COLORMASK) == BASE_API_LEDCTL_NOFEEDBACK) {
				/* release lock */
				pthread_mutex_unlock(&psLedInfo->mutStatus);	
				return eRet;
			}			

			/* change status now */
			psLedInfo->lStatus &= ~(COMM_MW_LED_COLORMASK);
			psLedInfo->lStatus |= eStatus;
			lNewStatus = 1;
		}
		
		/* release lock */
		pthread_mutex_unlock(&psLedInfo->mutStatus);

		/* new status should signal thread */
		if(lNewStatus) {
			psSignal = psInfo->psSignal;
			/* toggle the signal status */
			pthread_mutex_lock(&psSignal->mutStatus);
			psSignal->eStatus = BASE_API_LEDCTL_HAVESIG;
			pthread_mutex_unlock(&psSignal->mutStatus);

			/* notified the handle thread */
			BAL_Debug("sending signal\n");
			pthread_cond_signal(&psSignal->condStatus);
		}
	}

	BAL_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_API_LEDCTL_GreenBlink(void* pvarg)
* arg : pvarg
* ret : NULL
* note :
*********************************************/
void eBASE_API_LEDCTL_GreenBlink(void* pvarg){
	sCOMM_MW_LED_Info* psInfo = (sCOMM_MW_LED_Info*)pvarg;
	
	BAL_FuncIn();
	
	{
		//printf("GreenOn\n");
		eCOMM_MW_LED_GreenOn(psInfo);
		sleep(2);
		eCOMM_MW_LED_GreenOff(psInfo);
		//printf("GreenOff\n");
	}
	
	BAL_FuncOut();
}

/*********************************************
* func : eBASE_API_LEDCTL_GreenOn(void* pvarg)
* arg : pvarg
* ret : NULL
* note :
*********************************************/
void eBASE_API_LEDCTL_GreenOn(void* pvarg){
	sCOMM_MW_LED_Info* psInfo = (sCOMM_MW_LED_Info*)pvarg;
	
	BAL_FuncIn();
	
	{
		eCOMM_MW_LED_GreenOn(psInfo);
	}
	
	BAL_FuncOut();
}

/*********************************************
* func : eBASE_API_LEDCTL_RedOn(void* pvarg)
* arg : pvarg
* ret : NULL
* note :
*********************************************/
void eBASE_API_LEDCTL_RedOn(void* pvarg){
	sCOMM_MW_LED_Info* psInfo = (sCOMM_MW_LED_Info*)pvarg;
	
	BAL_FuncIn();
	
	{
		eCOMM_MW_LED_RedOn(psInfo);
	}
	
	BAL_FuncOut();
}

/*********************************************
* func : eBASE_API_LEDCTL_RedOff(void* pvarg)
* arg : pvarg
* ret : NULL
* note :
*********************************************/
void eBASE_API_LEDCTL_RedOff(void* pvarg){
	sCOMM_MW_LED_Info* psInfo = (sCOMM_MW_LED_Info*)pvarg;
	
	BAL_FuncIn();
	
	{
		eCOMM_MW_LED_RedOff(psInfo);
	}
	
	BAL_FuncOut();
}

/*********************************************
* func : eBASE_API_LEDCTL_GreenOff(void* pvarg)
* arg : pvarg
* ret : NULL
* note :
*********************************************/
void eBASE_API_LEDCTL_GreenOff(void* pvarg){
	sCOMM_MW_LED_Info* psInfo = (sCOMM_MW_LED_Info*)pvarg;
	
	BAL_FuncIn();
	
	{
		eCOMM_MW_LED_GreenOff(psInfo);
	}
	
	BAL_FuncOut();
}

/*******************************************************************************
* @brief		: eBASE_API_LEDCTL_LedUpdate(void* psArg)
* @param	: sCOMM_MW_LED_Info* psInfo
* @retval		: eBASE_API_LEDCTL_Ret
* @note		: 
*******************************************************************************/	

eBASE_API_LEDCTL_Ret  eBASE_API_LEDCTL_LedUpdate(sBASE_API_LEDCTL_Param* psLedParam)
{
	sCOMM_MW_LED_Info* psInfo = psLedParam->psLedInfo;
	sBASE_API_LED_CTL_Signal* psRecSignal = psLedParam->psRecSignal;
	sBASE_API_LEDCTL_Info* psLedCtlInfo = &gsLedCtlInfo;
	eBASE_API_LEDCTL_Ret eRet = BASE_API_LEDCTL_SUCCESS;
	eCOMM_TIMER_RET eTimerRet = COMM_TIMER_SUCCESS;
	eBASE_API_LEDCTL_LedNewStatus eStatus;
	struct timeval nowtime;
	struct timespec outtime;
	LONG lRet;

	BAL_FuncIn();
	/* init out time */
	memset(&nowtime, 0, sizeof(nowtime));
	memset(&outtime, 0, sizeof(outtime));
	
	BAL_Debug("update the led state!\n");
	pthread_mutex_lock(&psInfo->mutStatus);
	eStatus = psInfo->lStatus & COMM_MW_LED_COLORMASK;
	pthread_mutex_unlock(&psInfo->mutStatus);
	
	/* reset current led */
	eCOMM_MW_LED_GreenOff(psInfo);
	eCOMM_MW_LED_RedOff(psInfo);
	if (sLedTimer.lStatus == BASE_API_LEDCTL_HAVETIMER)
	{
		if (sLedTimer.lEvent == BASE_API_LEDCTL_REC) {
			eCOMM_TIMER_Deinit(sLedTimer.lFirFd);
			eCOMM_TIMER_Deinit(sLedTimer.lSecFd);
		} else if (sLedTimer.lEvent == BASE_API_LEDCTL_NOFEEDBACK) {
			eCOMM_TIMER_Deinit(sLedTimer.lFirFd);
			eCOMM_TIMER_Deinit(sLedTimer.lSecFd);	
			eCOMM_TIMER_Deinit(sLedTimer.lFirFd);
			eCOMM_TIMER_Deinit(sLedTimer.lFourthFd);
		}

		sLedTimer.lStatus = BASE_API_LEDCTL_NOTHAVETIMER;
		sLedTimer.lEvent = BASE_API_LEDCTL_INVALID;
	}
	/* toggle the current led case */
	switch(eStatus)
	{
		case BASE_API_LEDCTL_INVALID:
			eCOMM_MW_LED_GreenOff(psInfo);
			eCOMM_MW_LED_RedOff(psInfo);		
			break;
			
		case COMM_MW_LED_GREEN_On:
			eCOMM_MW_LED_GreenOn(psInfo);
			break;
			
		case COMM_MW_LED_Red_On:
			//printf("----- SD-fail event -----\n");
			eCOMM_MW_LED_RedOn(psInfo);				
			break;
			
		case BASE_API_LEDCTL_SNAP:
			//printf("----- snap event -----\n");
			eCOMM_MW_LED_RedOn(psInfo);
			eCOMM_MW_LED_GreenOn(psInfo);	
			usleep(100000);
			/* change status now */
			eStatus = BASE_API_LEDCTL_REC;
			eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);				
			break;
			
		case BASE_API_LEDCTL_REC:	
			//printf("----- rec event -----\n");
			eTimerRet=eCOMM_TIMER_Init(4, eBASE_API_LEDCTL_GreenOn, psInfo, &sLedTimer.lFirFd);
		    	if(COMM_TIMER_SUCCESS != eTimerRet)
		    	{
		        	BAL_Debug("timer init error!\n");
				eRet = BASE_API_LEDCTL_TIMER;
		        	return eRet;
		    	}	
			/* wait and get a new status */
			pthread_mutex_lock(&psRecSignal->mutStatus);
			//while (psRecSignal->eStatus != BASE_API_LEDCTL_HAVESIG) {
			while (1) {
				BAL_Debug("rec ready to wait\n");
				gettimeofday(&nowtime,NULL);
				outtime.tv_sec = nowtime.tv_sec + 2;
				outtime.tv_nsec = nowtime.tv_usec*1000;
				lRet = pthread_cond_timedwait(&psRecSignal->condStatus, &psRecSignal->mutStatus,&outtime);
				if (0 == lRet) {
					BAL_Debug("rec receiving a signal \n");
					eCOMM_TIMER_Deinit(sLedTimer.lFirFd);
					sLedTimer.lStatus = BASE_API_LEDCTL_NOTHAVETIMER;
					sLedTimer.lEvent = BASE_API_LEDCTL_INVALID;
					
					psRecSignal->eStatus &= ~(BASE_API_LEDCTL_HAVESIG);
					pthread_mutex_unlock(&psRecSignal->mutStatus);		
					BAL_FuncOut();
					return eRet;
				} else {
					BAL_Debug("rec waitting signal timeout \n");		
					psRecSignal->eStatus &= ~(BASE_API_LEDCTL_HAVESIG);
					pthread_mutex_unlock(&psRecSignal->mutStatus);	

					break;
				}
			}	
			
			eTimerRet=eCOMM_TIMER_Init(4, eBASE_API_LEDCTL_GreenOff, psInfo, &sLedTimer.lSecFd);
		    	if(COMM_TIMER_SUCCESS != eTimerRet)
		    	{
		        	BAL_Debug("timer init error!\n");
				eRet = BASE_API_LEDCTL_TIMER;
		        	return eRet;
		    	}		
			sLedTimer.lStatus = BASE_API_LEDCTL_HAVETIMER;
			sLedTimer.lEvent = BASE_API_LEDCTL_REC;
			break;

		case BASE_API_LEDCTL_NOFEEDBACK:
			//printf("----- no feedback event -----\n");
			
			/* Green Led On */
			eTimerRet=eCOMM_TIMER_Init(4, eBASE_API_LEDCTL_GreenOn, psInfo, &sLedTimer.lFirFd);
		    	if(COMM_TIMER_SUCCESS != eTimerRet)
		    	{
		        	BAL_Debug("timer init error!\n");
				eRet = BASE_API_LEDCTL_TIMER;
		        	return eRet;
		    	}	
				
			/* Red Led On */
			eTimerRet=eCOMM_TIMER_Init(4, eBASE_API_LEDCTL_RedOn, psInfo, &sLedTimer.lSecFd);
		    	if(COMM_TIMER_SUCCESS != eTimerRet)
		    	{
		        	BAL_Debug("timer init error!\n");
				eRet = BASE_API_LEDCTL_TIMER;
		        	return eRet;
		    	}	
				
			/* wait and get a new status */
			pthread_mutex_lock(&psRecSignal->mutStatus);
			//while (psRecSignal->eStatus != BASE_API_LEDCTL_HAVESIG) {
			while (1) {
				BAL_Debug("no feedback ready to wait\n");
				gettimeofday(&nowtime,NULL);
				outtime.tv_sec = nowtime.tv_sec + 2;
				outtime.tv_nsec = nowtime.tv_usec*1000;
				lRet = pthread_cond_timedwait(&psRecSignal->condStatus, &psRecSignal->mutStatus,&outtime);
				if (0 == lRet) {
					BAL_Debug("no feedback receiving a signal \n");
					eCOMM_TIMER_Deinit(sLedTimer.lFirFd);
					eCOMM_TIMER_Deinit(sLedTimer.lSecFd);
					sLedTimer.lStatus = BASE_API_LEDCTL_NOTHAVETIMER;
					sLedTimer.lEvent = BASE_API_LEDCTL_INVALID;
					
					psRecSignal->eStatus &= ~(BASE_API_LEDCTL_HAVESIG);
					pthread_mutex_unlock(&psRecSignal->mutStatus);		
					BAL_FuncOut();
					return eRet;
				} else {
					BAL_Debug("no feedback waitting signal timeout \n");		
					psRecSignal->eStatus &= ~(BASE_API_LEDCTL_HAVESIG);
					pthread_mutex_unlock(&psRecSignal->mutStatus);	

					break;
				}
			}	
			
			eTimerRet=eCOMM_TIMER_Init(4, eBASE_API_LEDCTL_GreenOff, psInfo, &sLedTimer.lThirdFd);
		    	if(COMM_TIMER_SUCCESS != eTimerRet)
		    	{
		        	BAL_Debug("timer init error!\n");
				eRet = BASE_API_LEDCTL_TIMER;
		        	return eRet;
		    	}		
			eTimerRet=eCOMM_TIMER_Init(4, eBASE_API_LEDCTL_RedOff, psInfo, &sLedTimer.lFourthFd);
		    	if(COMM_TIMER_SUCCESS != eTimerRet)
		    	{
		        	BAL_Debug("timer init error!\n");
				eRet = BASE_API_LEDCTL_TIMER;
		        	return eRet;
		    	}					
			sLedTimer.lStatus = BASE_API_LEDCTL_HAVETIMER;	
			sLedTimer.lEvent = BASE_API_LEDCTL_NOFEEDBACK;
			break;
			
		default :
			break;
	}	

	BAL_FuncOut();
	
	return eRet;
}

/*******************************************************************************
* @brief		: pvBASE_API_LEDCTL_ThreadHandle(void* psArg)
* @param	: void* psArg
* @retval		: NULL
* @note		: 
*******************************************************************************/
void* pvBASE_API_LEDCTL_ThreadHandle(void* psArg)
{
	eCOMM_MW_LED_Ret eRet = COMM_MW_LED_SUCCESS;
	sBASE_API_LEDCTL_ThreadArg* psLedCtl = (sBASE_API_LEDCTL_ThreadArg*)psArg;
	sBASE_API_LEDCTL_Param* psLedParam = malloc(sizeof(sBASE_API_LEDCTL_Param));
	psLedParam->psLedInfo = psLedCtl->psLedInfo;
	psLedParam->psRecSignal = psLedCtl->psRecSignal;
	
	sCOMM_MW_LED_Info* psLedInfo = psLedCtl->psLedInfo;
	sBASE_API_LED_CTL_Signal* psSignal = psLedCtl->psSignal;
	LONG lPthread_state;
	
	BAL_FuncIn();
	{
		/* register cleanup function */		
		pthread_cleanup_push(pvBASE_API_LEDCTL_Thread_Cleanup, psLedParam);
		
		eRet = eCOMM_MW_LED_Open(psLedInfo);
		if (COMM_MW_LED_SUCCESS != eRet) {
			BAL_Debug("open the led device error\n");
		}
		/* init led state*/
		eCOMM_MW_LED_GreenOn(psLedInfo);
		eCOMM_MW_LED_RedOff(psLedInfo);
		
		while(psLedCtl->peThreadStatus == BASE_API_LEDCTL_NOTEND) {
			/* wait and get a new status */
			pthread_mutex_lock(&psSignal->mutStatus);
			lPthread_state = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
			if (0 != lPthread_state)
				BAL_Handle_Error(lPthread_state,"pthread_setcancelstate");
			//BAL_Debug("set pthread cancelstate: enable\n");
			
			while(psSignal->eStatus != BASE_API_LEDCTL_HAVESIG) {
				BAL_Debug("ready to wait\n");
				pthread_cond_wait(&psSignal->condStatus, &psSignal->mutStatus);
				BAL_Debug("a signal is just arriving\n");
			}
			lPthread_state = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
			if (0 != lPthread_state)
				BAL_Handle_Error(lPthread_state,"pthread_setcancelstate");
			//BAL_Debug("set pthread cancelstate: disable\n");
			
			psSignal->eStatus &= ~(BASE_API_LEDCTL_HAVESIG);
			pthread_mutex_unlock(&psSignal->mutStatus);

			/* handle this status */
			eBASE_API_LEDCTL_LedUpdate(psLedParam);
		}

		/* deinit led state*/
		/* change status now */
		BAL_Debug("before close led\n");
		psLedInfo->lStatus &= ~(COMM_MW_LED_COLORMASK);
		psLedInfo->lStatus |= BASE_API_LEDCTL_INVALID;	
		eBASE_API_LEDCTL_LedUpdate(psLedParam);
		BAL_Debug("after close led\n");
		
		eRet = eCOMM_MW_LED_Close(psLedInfo);
		if (COMM_MW_LED_SUCCESS != eRet) {
			BAL_Debug("open the led device error\n");
		}

		if (NULL != psLedParam) {
			free(psLedParam);
		}

		pthread_cleanup_pop(1);
	}

	BAL_FuncOut();
	
	return NULL;
}

/*********************************************
* func : pvBASE_API_LEDCTL_Thread_Cleanup(void* pvArg)
* arg : void* pvArg
* ret : void
* note : thread clean up function
*********************************************/
void pvBASE_API_LEDCTL_Thread_Cleanup(void* pvArg) {
	sBASE_API_LEDCTL_Param* psLedParam = (sBASE_API_LEDCTL_Param*)pvArg;
	sCOMM_MW_LED_Info* psLedInfo = psLedParam->psLedInfo;

	BAL_FuncOut();

	{
		/* change status now */
		BAL_Debug("before close led\n");
		psLedInfo->lStatus &= ~(COMM_MW_LED_COLORMASK);
		psLedInfo->lStatus |= BASE_API_LEDCTL_INVALID;	
		eBASE_API_LEDCTL_LedUpdate(psLedParam);
		BAL_Debug("after close led\n");		

		eCOMM_MW_LED_Close(psLedInfo);
		
		free(psLedParam);
	}

	BAL_FuncOut();	
}

#ifdef __cplusplus
}
#endif
