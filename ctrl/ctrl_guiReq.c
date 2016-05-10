#include "ctrl_interface.h"

#ifdef __cplusplus
extern "C"{
#endif

/* extern parameter list */
extern  sCOMM_PARA gsParas[];
extern LONG glEndFlag;

/* define */
//#define CTRL_GUIREQ_DEBUG
#ifdef CTRL_GUIREQ_DEBUG
#define CG_Debug(fmt, arg...) fprintf(stdout, "[ CG ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CG_Debug(fmt, arg...)
#endif

#define CG_FuncIn() CG_Debug("in\n")
#define CG_FuncOut() CG_Debug("out\n")
#define CG_iVal(iVal) CG_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CG_lVal(lVal) CG_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CG_PtVal(ptVal) CG_Debug("pt %s @ %p\n", #ptVal, ptVal)

/* define */
#define CTRL_SERVERPORT 3516
#define CTRL_LOCALADDRESS "127.0.0.1"
#define CTRL_GUI_INVALIDVAL -1
#define CTRL_GUI_INVALIDFD -1
#define CTRL_GUI_MAXNETBUFFER ((4+8)*COMM_MAXPARACNT)

typedef enum {
	CTRL_GUI_USE,
	CTRL_GUI_NOTUSE
} eCTRL_GUI_STATE;

typedef enum {
	eCTRL_GUI_ALLCLOSE,
	eCTRL_GUI_ALLNOTCLOSE
} eCTRL_GUI_ALLSTATE;

typedef enum {
	CTRL_GUI_SINGLE,
	CTRL_GUI_ALL,
} eCTRL_GUI_TYPE;

typedef enum {
	CTRL_GUI_FEEDBACKNONEED,			// not need feedback
	CTRL_GUI_FEEDBACKATONCE,			// feedback at once
	CTRL_GUI_FEEDBACKSUCCESS,		// feedback success
	CTRL_GUI_FEEDBACKOTHER,			// feedback from other client
	CTRL_GUI_FEEDBACKSYSQUIT,		// feedback system quit
	CTRL_GUI_FEEDBACKVDA,			//feedback vda detect
	CTRL_GUI_FEEDBACKVDAREACHMAX, //vda region connection reach the max
	CTRL_GUI_FEEDBACKSDREMOVE,
} eCTRL_GUI_FEEDBACKTYPE;

/* extern */
extern sCTRL_GUI_CLIENT gsGuiClient[];
extern void vCTRL_CmdPreHandle(CHAR* pcCmd);


/*------------------------------------------------------------------------------*/
/*!
    @brief			the thread of gui request
    
    @param			void* arg					[in]						NULL

    @retval			NULL
*/
/* -----------------------------------------------------------------------------*/
void* pvCTRL_Gui_Req(void* arg);

/*------------------------------------------------------------------------------*/
/*!
    @brief			connect to the server to release accept block
    
    @param			NULL					[in]						NULL

    @retval			NULL
*/
/* -----------------------------------------------------------------------------*/
void vCTRL_Gui_ReqDown();

/*------------------------------------------------------------------------------*/
/*!
    @brief			the gui thread, each communicate with gui

    @param			void* arg					[in]						sCTRL_GUI_CLIENT*
    
    @retval			NULL
*/
/* -----------------------------------------------------------------------------*/
void* pvCTRL_Gui_Client(void* arg);

/*------------------------------------------------------------------------------*/
/*!
    @brief			check whether all the client is close now
    
    @retval			LONG						eCTRL_GUI_ALLCLOSE : all the clients are close
    												eCTRL_GUI_ALLNOTCLOSE : at least one client not close
*/
/* -----------------------------------------------------------------------------*/
LONG lCTRL_Gui_Client_ChkClose();

/*------------------------------------------------------------------------------*/
/*!
    @brief			notified the gui client, already reach max count

    @param			LONG lAcceptFd					[in]						the accept fd
    
*/
/* -----------------------------------------------------------------------------*/
void vCTRL_Gui_ReachMax(LONG lAcceptFd);

/*------------------------------------------------------------------------------*/
/*!
    @brief			the gui control thread, feedback all the gui client

    @param			void* arg					[in]						NULL
    
    @retval			NULL
*/
/* -----------------------------------------------------------------------------*/
void* pvCTRL_Gui_Control(void* arg);

/*------------------------------------------------------------------------------*/
/*!
    @brief			the gui feedback function

    @param			LONG lCfId					[in]						condition flag id
    @param			LONG lCfRet					[in]						condition flag return
    @param			LONG lType					[in]						CTRL_GUI_SINGLE or CTRL_GUI_ALL
    
    @retval			NULL
*/
/* -----------------------------------------------------------------------------*/
void vCTRL_Gui_Feedback(LONG lCfId, LONG lCfRet, LONG lType);

/*------------------------------------------------------------------------------*/
/*!
    @brief			send parameter list back to client

    @param			sCTRL_GUI_CLIENT* psClient					[in]						the client to be send
    @param			sCOMM_PARA** ppParas							[in]						parameters list
    @param			LONG lType										[in]						CTRL_GUI_FEEDBACKATONCE
    																								CTRL_GUI_FEEDBACKSUCCESS
    																								CTRL_GUI_FEEDBACKOTHER
    																								CTRL_GUI_FEEDBACKSYSQUIT
    																								CTRL_GUI_FEEDBACKNONEED
*/
/* -----------------------------------------------------------------------------*/
void vCTRL_Gui_SendToClient(sCTRL_GUI_CLIENT* psClient, sCOMM_PARA** ppParas, LONG lType);


/*------------------------------------------------------------------------------*/
/*!
    @brief			init the sCTRL_GUI_CLIENT struct
 
*/
/* -----------------------------------------------------------------------------*/
void vCTRL_Gui_Init();


/*------------------------------------------------------------------------------*/
/*!
    @brief			close the sCTRL_GUI_CLIENT struct
 
*/
/* -----------------------------------------------------------------------------*/
void vCTRL_Gui_Close();


/*------------------------------------------------------------------------------*/
/*!
    @brief			get a usable gui
    @retval			NULL								all the gui is in use
 					sCTRL_GUI_CLIENT*			pointer point to the available gui client
*/
/* -----------------------------------------------------------------------------*/
sCTRL_GUI_CLIENT* psCTRL_Gui_GetClient();



/* implement */
void* pvCTRL_Gui_Req(void* arg){

	/* return value */
	LONG lRet;
	
	/* thread id */
	pthread_t pid;
	pthread_t pid_ctl;
	pthread_attr_t sAttr;

	/* struct of sCOMM_NET */
	sCOMM_NET sServer;

	/* accept fd */
	LONG lAcceptFd;

	/* socket address of server and client */
	struct sockaddr_in sCliAddr;
	LONG lSinSize;

	/* ip string */
	CHAR* pcIp;

	/* the argument */
	sCTRL_GUI_CLIENT* psClient;

	debug_show("gui request start\n");

	/* init the global parameters */
	vCTRL_Gui_Init();

	/* establish the server socket */
	vCOMM_Net_Init(&sServer, CTRL_SERVERPORT, COMM_NET_SERVER, COMM_UI_MAXGUICNT);

	/* begin the control thread */
	lRet=pthread_create(&pid_ctl, NULL, pvCTRL_Gui_Control, NULL);
	if(lRet){
		debug_info("[ Thread ] : gui control thread create error!\n");
		exit(-1);
	}

	debug_info("i am waiting for Connect!\n");
	while(1){
		lSinSize = sizeof(struct sockaddr_in);

		/* wait until new gui request */
		if((lAcceptFd = accept(sServer.lSockFd, (struct sockaddr *)&sCliAddr, ( socklen_t * )&lSinSize)) == -1) {
			/* indicate control already close the socket */
			printf("error to accept\n");
			exit(-1);
		}

		/* get pcIp */
		pcIp = inet_ntoa( sCliAddr.sin_addr );
		if(!strcmp(pcIp, "127.0.0.1")){
			/* indicate exit */
			break;
		}

		/* get a gui Id */
		psClient=psCTRL_Gui_GetClient();
		if(!psClient){
			debug_info("gui client reach the max count!\n");
			/* notify the client */
			vCTRL_Gui_ReachMax(lAcceptFd);

			/* close the fd */
			close(lAcceptFd);
			continue;
		}

		/* prepare the argument for the client */
		memset(psClient->cIp, 0,sizeof(psClient->cIp));
		strcpy(psClient->cIp, pcIp);
		psClient->lSocket=lAcceptFd;
		psClient->lValid=CTRL_GUI_USE;

		/* init the thread attribute, let it detach */
		pthread_attr_init(&sAttr);
		pthread_attr_setdetachstate(&sAttr, PTHREAD_CREATE_DETACHED);
		
		/* create a thread, handle the gui request */
		lRet=pthread_create(&pid, &sAttr, pvCTRL_Gui_Client, (void*)psClient);
		if(lRet){
			debug_info("[ Thread ] : gui client thread create error!\n");
			exit(-1);
		}
	}

	/* wait until all the client is not use */
	pthread_join(pid_ctl, NULL);

	/* close the socket */
	vCOMM_Net_DeInit(&sServer);

	/* wait for all the clients is finished */
	while(lCTRL_Gui_Client_ChkClose()!=eCTRL_GUI_ALLCLOSE){
		debug_info("warning, client thread competetion with main thread\n");
		usleep(20);
	}

	debug_show("gui request stop\n");
	return (void*) NULL;
	
}
	

void* pvCTRL_Gui_Client(void* arg){
	/* return value */
	LONG lRet;
	volatile LONG lEndFlag=0;

	/* parameters list */
	sCOMM_PARA* psParas=gsParas;

	/* request info */
	LONG lReqInfo;

	/* for gui request */
	sCTRL_GUI_CLIENT* psClient;
	socklen_t sAddrLen;

	/* accept fd */
	LONG lAcceptFd;

	/* the recieved buffer */
	CHAR cCmdBuf[COMM_CMDSPACKETMAXCHAR];

	/* ip ,port and channel */
	CHAR* pcIp; 
	LONG lPort;
	LONG lCh=CTRL_GUI_INVALIDVAL;

	/* for command parse */
	CHAR* pcTmp;

	/* for feedback */
	LONG lCfId;
	LONG lCfRet;

	/* command length*/
	LONG lCmdLen = 0;

	sAddrLen = sizeof(struct sockaddr);

	/* get the argument */
	psClient=(sCTRL_GUI_CLIENT*)arg;
	lAcceptFd=psClient->lSocket;
	pcIp=psClient->cIp;

	debug_show("Gui Client [%u] start\n", psClient->eUi);

	/* condition flag */
	vCOMM_CF_InitCF(&(psClient->sCf), COMM_CFCTRLGUITIMEOUT, "ctlTogui");

	/* regist the cf */
	vCOMM_CF_RegCF(&(psClient->sCf));

	/* first of all, get the port */
	memset(cCmdBuf,0, sizeof(cCmdBuf));
	lRet = lCOMM_Net_Read(lAcceptFd, cCmdBuf, COMM_CMDSMAXCHAR);
	if ( COMM_RET_SUCCESS != lRet ) {
		debug_info("Error to receive !\n");

		/* deinit cf */
		vCOMM_CF_DeregCF(&(psClient->sCf));
		vCOMM_CF_DeInitCF(&(psClient->sCf));

		/* reset psClient */
		close(psClient->lSocket);
		psClient->lSocket=CTRL_GUI_INVALIDFD;
		psClient->lValid=CTRL_GUI_NOTUSE;
		
		pthread_exit (NULL);
	}

	/* return the parameters list */
	vCTRL_Gui_SendToClient(psClient, &psParas, CTRL_GUI_FEEDBACKNONEED);
	lPort=atol(cCmdBuf);
	psClient->lRtpPort=lPort;
	debug_info("GUI : port %ld\n", lPort);

	/* return the parameter list */

	while ( !lEndFlag ) {
		memset(cCmdBuf, 0,sizeof(cCmdBuf));
		lRet = lCOMM_Net_Read(lAcceptFd, cCmdBuf, COMM_CMDSMAXCHAR);
		if ( COMM_RET_SUCCESS != lRet ) {
			/* indicate terminate */
			if(CTRL_GUI_INVALIDVAL!=lCh && !glEndFlag){
				/* construct the cCmdBuf */
				memset(cCmdBuf, 0,sizeof(cCmdBuf));
				lCmdLen = sprintf(cCmdBuf, "%ld&%d,%d,%d,%d,%d,%d,%d,%d,%ld"
					, lCh, COMM_CMD_ETH, COMM_SUBCMD_ETHSTOPRESOLUTION, 1<<lCh,
					        COMM_CMD_ETH, COMM_SUBCMD_ETHVIDEOIPADDRESS,inet_addr(pcIp),
					        COMM_CMD_ETH, COMM_SUBCMD_ETHVIDEOPORT, lPort);

				lCmdLen += sprintf(cCmdBuf+lCmdLen, ",%d,%d,%d,%d,%d,%d,%d,%d,%d&\n"
					, COMM_CMD_ETH, COMM_SUBCMD_ETHAUDIOSTOP, 1,
					        COMM_CMD_ETH, COMM_SUBCMD_ETHAUDIOIPADDRESS,inet_addr(pcIp),
					        COMM_CMD_ETH, COMM_SUBCMD_ETHAUDIOPORT, COMM_AUDIO_RTP_PORT);

				printf("%s\n", cCmdBuf);
			} else {
				/* not recieve any date now, terminate immediately */
				break;
			}
			/* toggle the end flag, indicate terminate */
			lEndFlag=1;
		}

		/* prehandle the command, turn the string to long */
		vCTRL_CmdPreHandle(cCmdBuf);

		/* get channel */
		pcTmp=strtok(cCmdBuf, "&");
		lCh=atol(pcTmp);
		debug_info("GUI : channel %ld\n", lCh);

		/* update pcTmp and check the valid */
		pcTmp+=2;					// cmd,opt,val,cmd,opt,val...
		lRet=lCOMM_CMD_CheckCmds(pcTmp);
		if(lRet!=COMM_CMD_VALID){
			printf("GUI : error commands <%ld>\n", lRet);
			/* notify the client */

			continue;
		}

		/* get lock and allocate reqid */
		pthread_mutex_lock(&sMutReqId);
		if(++glReqId == COMM_CMDMAXREQID)
			glReqId=0;

		/* prepare reqInfo : include reqId, ch, ui */
		lReqInfo = (COMM_SETREQINFO(glReqId, COMM_REQID) | \
					COMM_SETREQINFO(lCh, COMM_REQCH) | \
					COMM_SETREQINFO(psClient->eUi, COMM_REQUI));
		
		/* modified the req tail and add tail and head to the req */	
		pcTmp[strlen(pcTmp)-1]='\0';
		vCOMM_CMD_AddCmdHeadTail( &pcTmp, lReqInfo );
		
		/* release the lock */
		pthread_mutex_unlock(&sMutReqId);

		/* send to request list */
		vCOMM_LQ_Write(&sReqList, pcTmp, AUTO_DETECT_STRLEN);

		printf("\nGUI <%u> Sending command...\n", psClient->eUi);

		/** wait until get the right reqInfo **/
		do{
			vCOMM_CF_GetIdCF(&(psClient->sCf), &lCfId, COMM_COND_BLOCK);
		} while ( lReqInfo!=lCfId);

		/* now, get the feedback and handle it */
		while(1) {

			/* read the feedback of the ret */
			vCOMM_CF_RdCF(&(psClient->sCf), &lCfRet);

			if(lCfRet==COMM_CF_FEEDBACKATONCESUC){
				/* first feedback, base process get the infomation */
				if(!lEndFlag){
					vCTRL_Gui_SendToClient(psClient, NULL, CTRL_GUI_FEEDBACKATONCE);
				}
				printf("GUI <%u> Base process handleing...\n", psClient->eUi);
			}else if(lCfRet==COMM_CF_FEEDBACKFINISHSUC){
				/* second feedback, base process finish handle the command */
				if(!lEndFlag){
					vCTRL_Gui_SendToClient(psClient, &psParas, CTRL_GUI_FEEDBACKSUCCESS);
				}
				printf("GUI <%u> Base process done!\n", psClient->eUi);
				break;
			}else{
				printf("GUI <%u> abnormal\n", psClient->eUi);
				/* abnormal branch */
				if(lCfRet==COMM_COND_CFSYSRETFAIL){
					/* time out */
					printf("GUI <%u> timeout to get feedback, the command is abnormal\n", psClient->eUi);
				} else if ( lCfRet==COMM_CF_FEEDBACKFINISHFAIL ){
					printf("GUI <%u> base handle abnormal\n", psClient->eUi);
				} else {
					printf("GUI <%u> unknowning bug ..", psClient->eUi);
				}
				if(!lEndFlag){
					vCTRL_Gui_SendToClient(psClient, NULL, lCfRet);
				}
				break;
			}

			/* again, get id */
			vCOMM_CF_GetIdCF(&(psClient->sCf), &lCfId, COMM_COND_BLOCK);
		}
		
	}

	/* close the fd */
	if(CTRL_GUI_INVALIDFD != psClient->lSocket){
		close(psClient->lSocket);
	}
	
	/* deregist the cf */
	vCOMM_CF_DeregCF(&(psClient->sCf));

	/* condition flag */
	vCOMM_CF_DeInitCF(&(psClient->sCf));

	/* reset the valid flag */
	psClient->lValid=CTRL_GUI_NOTUSE;

	debug_show("Gui Client [%u] stop\n", psClient->eUi);

	pthread_exit(NULL);
}

void vCTRL_Gui_Init(){
	LONG lUiId=COMM_UI_GUI1;
	sCTRL_GUI_CLIENT* psClient;

	/* init the gui struct */
	while( lUiId!=COMM_UI_MAXGUICNT ){
		/* get the current client */
		psClient=&gsGuiClient[lUiId];

		/* init the client */
		memset(psClient->cIp, 0,sizeof(psClient->cIp));
		psClient->eUi=lUiId;
		psClient->lCh=CTRL_GUI_INVALIDVAL;
		psClient->lRtpPort=CTRL_GUI_INVALIDVAL;
		psClient->lSocket=CTRL_GUI_INVALIDFD;
		psClient->lValid=CTRL_GUI_NOTUSE;
		
		lUiId++;
	}
}

sCTRL_GUI_CLIENT* psCTRL_Gui_GetClient(){
	LONG lGui=COMM_UI_GUI1;
	sCTRL_GUI_CLIENT* psClient=NULL;
	sCTRL_GUI_CLIENT* psClientRet=NULL;

	/* find a available gui */
	while( lGui!=COMM_UI_MAXGUICNT ){
		/* get the current client */
		psClient=&gsGuiClient[lGui++];

		/* found out the client not use */
		if(psClient->lValid==CTRL_GUI_NOTUSE){
			psClientRet=psClient;
			break;
		}
	}

	return psClientRet;
}

void vCTRL_Gui_Feedback(LONG lCfId, LONG lCfRet, LONG lType){
	LONG lUID=0;
	LONG lTargetUID=-1;
	sCOMM_CONDFLAG* psCf=NULL;
	sCTRL_GUI_CLIENT* psClient;
	sCOMM_PARA* psParas=gsParas;

	/* get target UID */
	lTargetUID=COMM_GETREQINFO(lCfId, COMM_REQUI);

	/* first, feedback the target */
	if(lTargetUID<COMM_UI_MAXGUICNT){
		psCf=&(gsGuiClient[lTargetUID].sCf);
		vCOMM_CF_SetIdCF(psCf, lCfId,COMM_COND_BLOCK);
		vCOMM_CF_WtCF (psCf, lCfRet);
	}

	/* second, if ALL, notified all the other client */	
	if ( CTRL_GUI_ALL==lType ){
		/* feedback for all the gui client except current one */
		while (lUID!=COMM_UI_MAXGUICNT){
			psClient=&(gsGuiClient[lUID]);

			if(psClient->lValid==CTRL_GUI_USE && psClient->eUi!=lTargetUID){
				vCTRL_Gui_SendToClient(psClient, &psParas, CTRL_GUI_FEEDBACKOTHER);
				printf("control feedback success send to Gui<%ld>\n", lUID);
			}

			lUID++;
		}
	} 
}

void* pvCTRL_Gui_Control(void* arg){

	/* for feedback */
	LONG lCfId;		// i.e. reqInfo, set by reqResponse
	LONG lCfRet;
	LONG lUID;
	
	CG_Debug("\n\ngui control thread start\n");

	/* regist the cf */
	vCOMM_CF_RegCF(&stCfCtlRetGui);

	while(1){

		vCOMM_CF_GetIdCF(&stCfCtlRetGui, &lCfId, COMM_COND_BLOCK);
		vCOMM_CF_RdCF(&stCfCtlRetGui, &lCfRet);

		CG_Debug("get information : %ld\n", lCfRet);

		if(lCfRet==COMM_CF_FEEDBACKFINISHSUC){
			/* if recieve the second feedback, then notified all the gui client */
			vCTRL_Gui_Feedback(lCfId, lCfRet, CTRL_GUI_ALL);
		} else if (lCfRet==COMM_CF_FEEDBACKFINISHSYSQUIT){
			/* if the feedback is system quit, notified all the client and gui */
			break;
		} else {
			/* else, check whether gui is the target, then feedback the target */
			lUID=COMM_GETREQINFO(lCfId, COMM_REQUI);
			if( lUID < COMM_UI_MAXGUICNT) {
				/* indicate gui client */
				vCTRL_Gui_Feedback(lCfId, lCfRet, CTRL_GUI_SINGLE);
			}
		}
	}

	/* close all the used gui client */
	vCTRL_Gui_Close();

	/* release the block */
	vCTRL_Gui_ReqDown();

	/* deregist the cf */
	vCOMM_CF_DeregCF(&stCfCtlRetGui);

	CG_Debug("gui control thread stop\n");

	/* return */
	return NULL;
}

void vCTRL_Gui_ReqDown(){

	/* the struct of socket */
	sCOMM_NET sClient;

	/* try to connect the server to release the accept block */
	vCOMM_Net_Init(&sClient, CTRL_SERVERPORT, COMM_NET_CLIENT, 0);

	/* prepare for server address */
	vCOMM_Net_Connect(&sClient, CTRL_LOCALADDRESS);

	/* close the socket */
	vCOMM_Net_DeInit(&sClient);

}

void vCTRL_Gui_Close(){

	sCTRL_GUI_CLIENT* psClient;
	LONG lUID;

	/* close all the used client */
	lUID=0;
	while (lUID!=COMM_UI_MAXGUICNT){
		psClient=&(gsGuiClient[lUID]);

		if(psClient->lValid==CTRL_GUI_USE){
			/* send to gui */
			vCTRL_Gui_SendToClient(psClient, NULL, CTRL_GUI_FEEDBACKSYSQUIT);

			/* close the client */
			close(psClient->lSocket);

			/* reset the struct */
			psClient->lSocket=CTRL_GUI_INVALIDFD;
			psClient->lValid=CTRL_GUI_NOTUSE;
		}

		lUID++;
	}

}

void vCTRL_Gui_SendToClient(sCTRL_GUI_CLIENT* psClient, sCOMM_PARA** ppParas, LONG lType){

	CHAR* pcTmp;

	/* the send buffer */
	CHAR cSendBuf[CTRL_GUI_MAXNETBUFFER];

	/* send the parameter list to all the used gui */
	pcTmp=cSendBuf;
	memset(cSendBuf, 0,sizeof(cSendBuf));
	
	/* construct send buffer */
	if(CTRL_GUI_FEEDBACKNONEED!=lType){
		pcTmp+=sprintf(pcTmp, "%ld&", lType);
	}
	if(ppParas!=NULL){
		vCOMM_PARA_ParasToChar(ppParas, pcTmp);
	}

	/* write back to the client */
	vCOMM_Net_Write(psClient->lSocket, cSendBuf, sizeof(cSendBuf));

}

void vCTRL_Gui_ReachMax(LONG lAcceptFd){
	CHAR cRet[]="-4&";

	/* write back to the client */
	vCOMM_Net_Write(lAcceptFd, cRet, sizeof(cRet));
}

LONG lCTRL_Gui_Client_ChkClose(){

	sCTRL_GUI_CLIENT* psClient;
	LONG lUID;
	LONG lRet=eCTRL_GUI_ALLCLOSE;

	/* close all the used client */
	lUID=0;
	while (lUID!=COMM_UI_MAXGUICNT){
		psClient=&(gsGuiClient[lUID]);

		if(psClient->lValid==CTRL_GUI_USE){
			/* indicate not all close */
			lRet=eCTRL_GUI_ALLNOTCLOSE;
			break;
		}

		lUID++;
	}

	return lRet;

}

eCOMM_RET_STAT eCTRL_PostFeedback_VdaHandle(eCOMM_UI eUi,eCOMM_CF_RET eCfcont) {

	eCOMM_RET_STAT eRet = COMM_RET_SUCCESS;

	if(COMM_UI_TERMINAL == eUi){
		/* termianl*/
		printf("terminal vda detect!\n");
	
	}else if(eUi > COMM_UI_MAXGUICNT){
		/* invalid ui*/
		printf("invalid client uid vda detect!\n");
	}else{
		/**/
		if(COMM_CF_VDARGNREACHMAX != eCfcont){
			vCTRL_Gui_SendToClient(&(gsGuiClient[eUi]),NULL,CTRL_GUI_FEEDBACKVDA);
		}else{
			vCTRL_Gui_SendToClient(&(gsGuiClient[eUi]),NULL,CTRL_GUI_FEEDBACKVDAREACHMAX);
		}
		
	}
	return eRet;
}

eCOMM_RET_STAT eCTRL_PostFeedback_RecHandle(eCOMM_UI eUi,eCOMM_CF_RET eCfcont) {
	
	eCOMM_RET_STAT eRet = COMM_RET_SUCCESS;
	if(COMM_UI_TERMINAL == eUi){
		/* termianl*/
		printf("terminal detect SD remove!\n");
	
	}else if(eUi > COMM_UI_MAXGUICNT){
		/* invalid ui*/
		printf("invalid client uid sd remove detect!\n");
	}else{
		/**/
		vCTRL_Gui_SendToClient(&(gsGuiClient[eUi]),NULL,CTRL_GUI_FEEDBACKSDREMOVE);		
	}
	return eRet;

}

#ifdef __cplusplus
}
#endif



