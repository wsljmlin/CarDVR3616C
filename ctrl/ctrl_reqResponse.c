#include "../common/comm_interface.h"

#ifdef __cplusplus
extern "C"{
#endif

/* define */
//#define CTRL_REQRESPONSE_DEBUG
#ifdef CTRL_REQRESPONSE_DEBUG
#define CR_Debug(fmt, arg...) fprintf(stdout, "[ CR ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CR_Debug(fmt, arg...)
#endif

#define CR_FuncIn() CR_Debug("in\n")
#define CR_FuncOut() CR_Debug("out\n")
#define CR_iVal(iVal) CR_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CR_lVal(lVal) CR_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CR_PtVal(ptVal) CR_Debug("pt %s @ %p\n", #ptVal, ptVal)

/* extern two pipes */
extern sCOMM_PIPE_COMMU gsPipeCommand;
extern sCOMM_PIPE_COMMU gsPipeFeedback;

/* extern parameter list */
extern  sCOMM_PARA gsParas[];

/* extern condition flag */
extern sCOMM_CONDFLAG stCfCtlRetTerminal;
extern sCOMM_CONDFLAG stCfCtlRetGui;
extern sCOMM_CONDFLAG stCfCtlRetUart;

typedef enum {
	CTRL_REQRESPONSE_FEEDBACKNOTFIRST,
	CTRL_REQRESPONSE_FEEDBACKFIRST
} eCTRL_REQRESPONSE_FEEDBACKTYPE;

/*------------------------------------------------------------------------------*/
/*!
    @brief			feedback according to reqInfo
    
    @param			LONG lReqInfo				[in]					the request info
    @param			LONG lType					[in]					feedback type
    @param			LONG lRet						[in]					feedback content
    
*/
/* -----------------------------------------------------------------------------*/
void vCTRL_Feedback(LONG lReqInfo, LONG lType, LONG lRet);


/*------------------------------------------------------------------------------*/
/*!
    @brief			parse the command, communicate with base process
    
    @param			CHAR* pcReq				[in]						the request

    @retval			COMM_RET_SUCCESS							success
					COMM_RET_SYSQUIT							system quit
*/
/* -----------------------------------------------------------------------------*/
LONG lCTRL_RequestResponse(CHAR* pcReq){

	/* return */
	LONG lRet=COMM_RET_SUCCESS;
	
	/* lFeedback from base process */
	CHAR cFb[COMM_MAXPARAFEEDBACKCHAR];
	CHAR* pc;
	CHAR* pcNext;

	/* parameter to udpate */
	sCOMM_PARA* pstParas=gsParas;
	sCOMM_PARA stPara;

	/* request infomation */
	LONG lReqInfo=0;
	LONG lReqRet=0;

	debug_show("\n\nrequestResponse start\n");
	/* write to pipe */
	vCOMM_Pipe_Write(&gsPipeCommand, pcReq);

	/* wait and handle the feedback */
	while(1){
		/* clear the buffer */
		memset(cFb, 0,sizeof(cFb));
		
		/* handle according to lFeedback */
		vCOMM_Pipe_Read(&gsPipeFeedback, cFb);
		if( !strlen(cFb)){
			debug_info("[lCTRL_RequestResponse] ERROR when lCOMM_Pipe_Read \n");
			/* consider feedback */
			vCTRL_Feedback(lReqInfo, CTRL_REQRESPONSE_FEEDBACKNOTFIRST, COMM_CF_FEEDBACKFINISHSUC);
			break;
		}
		
		/* get ret infomation */
		pc=cFb;
		pcNext=index(pc, '&');
		*pcNext='\0';
		lReqRet=atol(pc);
		pc=pcNext+1;

		/* get reqId */
		pcNext=index(pc, '&');
		if(pcNext!=NULL){
			*pcNext='\0';
		}
		lReqInfo=atol(pc);
		if(pcNext!=NULL){
			pc=pcNext+1;
		}

		/* handle according to ret and reqId */
		if(lReqRet==0){
			/* feedback at once */
			CR_Debug("get back at once\n");

			/* consider feedback */
			vCTRL_Feedback(lReqInfo, CTRL_REQRESPONSE_FEEDBACKFIRST, COMM_CF_FEEDBACKATONCESUC);

		} else { 				
			if (lReqRet==-1) {
				/* system set failure */
				CR_Debug("feed back error\n");

				/* consider feedback */
				vCTRL_Feedback(lReqInfo, CTRL_REQRESPONSE_FEEDBACKNOTFIRST, COMM_CF_FEEDBACKFINISHFAIL);

			} else if (lReqRet==-2){
				/* system quit */
				CR_Debug("feed back system quit\n");

				/* feedback */
				vCTRL_Feedback(lReqInfo, CTRL_REQRESPONSE_FEEDBACKNOTFIRST, COMM_CF_FEEDBACKFINISHSYSQUIT);
				
				lRet=COMM_RET_SYSQUIT;

			} else {
				/* system feedback ok */
				CR_Debug("feed back ok\n");
				
				/* feed back ok, should return the update parameters */
				/* we should go through the parameters one by one */
				while((pcNext=index(pc, '&'))!=NULL){
					*pcNext='\0';

					/* get the update parameter */
					vCOMM_PARA_CharToPara(&stPara, pc);
					lCOMM_PARA_UpdateParas(&pstParas, &stPara);

					if(*++pcNext=='\0'){
						/* indicate finished */
						break;
					}

					/* update pc */
					pc=pcNext;
				}

				/* consider feedback */
				vCTRL_Feedback(lReqInfo, CTRL_REQRESPONSE_FEEDBACKNOTFIRST, COMM_CF_FEEDBACKFINISHSUC);

			}

			/* lFeedback finish */
			break;
		}
	}

	debug_show("requestResponse stop\n");
	return lRet;
}

void vCTRL_Feedback(LONG lReqInfo, LONG lType, LONG lRet){
	LONG lUid;

	/* get uid */
	lUid=COMM_GETREQINFO(lReqInfo, COMM_REQUI);

	/* begin to feedback according to type */
	if(CTRL_REQRESPONSE_FEEDBACKFIRST==lType){
		/* feedback at once */
		if(lUid == COMM_UI_TERMINAL){
			/* terminal request */
			vCOMM_CF_SetIdCF(&stCfCtlRetTerminal, lReqInfo,COMM_COND_BLOCK);
			vCOMM_CF_WtCF (&stCfCtlRetTerminal, lRet);
		} else if (lUid == COMM_UI_UART) {
			/* uart request */
			vCOMM_CF_SetIdCF(&stCfCtlRetUart, lReqInfo, COMM_COND_BLOCK);
			vCOMM_CF_WtCF(&stCfCtlRetUart, lRet);
		} else if(lUid < COMM_UI_MAXGUICNT){
			/* gui request */
			vCOMM_CF_SetIdCF(&stCfCtlRetGui, lReqInfo,COMM_COND_BLOCK);
			vCOMM_CF_WtCF (&stCfCtlRetGui, lRet);
		} 
	}else{
		/* other case */
		if(lUid == COMM_UI_TERMINAL){
			/* terminal request */
			vCOMM_CF_SetIdCF(&stCfCtlRetTerminal, lReqInfo,COMM_COND_BLOCK);
			vCOMM_CF_WtCF (&stCfCtlRetTerminal, lRet);

			vCOMM_CF_SetIdCF(&stCfCtlRetGui, lReqInfo,COMM_COND_BLOCK);
			vCOMM_CF_WtCF (&stCfCtlRetGui, lRet);

		} else if(lUid == COMM_UI_UART) {
			/* uart request */
			vCOMM_CF_SetIdCF(&stCfCtlRetUart, lReqInfo, COMM_COND_BLOCK);
			vCOMM_CF_WtCF (&stCfCtlRetUart, lRet);

			vCOMM_CF_SetIdCF(&stCfCtlRetGui, lReqInfo,COMM_COND_BLOCK);
			vCOMM_CF_WtCF (&stCfCtlRetGui, lRet);
		} else if(lUid < COMM_UI_MAXGUICNT){
			/* gui request */
			vCOMM_CF_SetIdCF(&stCfCtlRetGui, lReqInfo,COMM_COND_BLOCK);
			vCOMM_CF_WtCF (&stCfCtlRetGui, lRet);
		} 
	}
}


#ifdef __cplusplus
}
#endif


