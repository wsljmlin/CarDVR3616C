#include "base_interface.h"
#include "base_comm.h"
#include "base_mw_region.h"
#include "base_mw_sd.h"
#include "base_venc_handle.h"
#include "base_api_ledctl.h"
#include "base_ircut_ctl.h"
#include "base_mw_wifi.h"

#ifdef __cplusplus
extern "C"{
#endif

extern void* vBASE_VENC_Ctrl_Thread(void* pvArg);
extern sBASE_IRCUT_CTL_Info gsIrCutInfo;;

//#define BASE_AVM_DEBUG
#ifdef BASE_AVM_DEBUG
#define BA_Debug(fmt, arg...) fprintf(stdout, "[ BA ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define BA_Debug(fmt, arg...)
#endif

#define BA_FuncIn() BA_Debug("in\n")
#define BA_FuncOut() BA_Debug("out\n")
#define BA_iVal(iVal) BA_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define BA_lVal(lVal) BA_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define BA_PtVal(ptVal) BA_Debug("pt %s @ %p\n", #ptVal, ptVal)

/*------------------------------------------------------------------------------*/
/*!
	@brief		VENC,AUDIO common init
*/
/* -----------------------------------------------------------------------------*/
void lBASE_VENC_InitGlobal()
{
	LONG lCnt;	
	CHAR pcCfVencCtrlNameTmp[32];
	eBASE_MW_SD_ERROR eRet;
	sBASE_MW_SD_STAT* psSDStat = &gsSDStat;
#ifdef SUBCARD_CHECK_DEBUG
	sBASE_MW_SD_STAT* psNANDStat = &gsNANDStat;
#endif /* SUBCARD_CHECK_DEBUG */
	sBASE_MW_DIR_STAT* psDIRStat = &gsDIRStat;
	sBASE_API_LEDCTL_Info* psLedCtlInfo = &gsLedCtlInfo;
	//eBASE_API_LEDCTL_LedNewStatus eStatus = BASE_API_LEDCTL_NORMAL;
	
	/* init cf struct */	
	vCOMM_CF_InitCF(&stCfBaseRet, BASE_CFBASETIMEOUT, "avmToVenc");

	/* rtp para init */
	memset(gsParaRtp,0,sizeof(gsParaRtp));
	for(lCnt = BASE_VIDEO_CH0;lCnt < BASE_VIDEO_AUDIO_CH_NUM;lCnt ++)
	{
		/* rtp para init  */
		gsParaRtp[lCnt].eRtpCmd = BASE_RTP_INVALID;
		gsParaRtp[lCnt].psess = NULL;
		gsParaRtp[lCnt].loc_port = BASE_LOCALPORTBASE + lCnt * 2;
		gsParaRtp[lCnt].rmt_port = 0;
		strcpy(gsParaRtp[lCnt].cClientIp,"0");		
	}
	
	/* Global attr of file,venc ,vo,video buffer.etc. init*/
	memset(gsRtpAttr,0,sizeof(gsRtpAttr));
	memset(&gsFileAttr,0,sizeof(gsFileAttr));
	memset(gsVencAttr,0,sizeof(gsVencAttr));
	memset(lCurrentChangeParamsArray,0,sizeof(lCurrentChangeParamsArray));
	memset(stCfVencCtrlCh,0,sizeof(stCfVencCtrlCh));
	memset(&gsHighThreadArgv,0,sizeof(gsHighThreadArgv));
	memset(&gsIrCutInfo,0,sizeof(gsIrCutInfo));
	
	for (lCnt = BASE_VIDEO_CH0; lCnt < BASE_VIDEO_CH2 + 1; lCnt++)
	{
		/* conditionflag init between VencCtrl and venc getstream */
		sprintf(pcCfVencCtrlNameTmp,"%s%ld","VencToVencGetstreamCh",lCnt);
		vCOMM_CF_InitCF(&stCfVencCtrlCh[lCnt], BASE_CFBASETIMEOUT, pcCfVencCtrlNameTmp);

	}

	/* iplist init */
	vCOMM_IPList_Init(&gIPList);

	/* init all poll */
	eCOMM_POOL_InitAll();	

	/*timer*/
	eCOMM_TIMER_InitAll();
		
	/* init LED parammeter */
	eRet = eBASE_API_LEDCTL_Init(psLedCtlInfo);
	if (BASE_API_LEDCTL_SUCCESS != eRet) {
		printf("eBASE_API_LEDCTL_Init function fail\n");
		eBASE_API_LEDCTL_Deinit(psLedCtlInfo);
	}

	/* ir-cut init */
	eBASE_IRCUT_CTL_Init(&gsIrCutInfo);
	
	/* get SD  space parammeter */
	psSDStat->lSdState = BASE_MW_SD_INVALID;
	psSDStat->eAudioStat = BASE_MW_AUDIO_NOREC;
	psSDStat->eVideoStat = BASE_MW_VIDEO_NOREC;
	psSDStat->eSnapStat = BASE_MW_SNAP_NOREC;
	pthread_mutex_init(&psSDStat->sMutStatus, NULL);
	
	eRet = eBASE_SD_MW_GETSPACE(psSDStat, TFCARD, ".h264", 1);
	if(eRet != BASE_MW_SD_SUCCESS) {
		printf("=== get space error <%d> ===\n", eRet);
		//eStatus = BASE_API_LEDCTL_SDFAIL;		
	} else {
		psSDStat->lSdState = BASE_MW_SD_SUCCESS;
	}	
	printf("--- Get SD space Parameter ---\n");

#ifdef SUBCARD_CHECK_DEBUG
	/* init NAND space parammeter */
	psNANDStat->lSdState = BASE_MW_SD_INVALID;
	psNANDStat->eAudioStat = BASE_MW_AUDIO_NOREC;
	psNANDStat->eVideoStat = BASE_MW_VIDEO_NOREC;
	psNANDStat->eSnapStat = BASE_MW_SNAP_NOREC;
	pthread_mutex_init(&psNANDStat->sMutStatus, NULL);
	
	eRet = eBASE_SD_MW_GETSPACE(psNANDStat, NANDFLASH, ".h264", 1);
	if(eRet != BASE_MW_SD_SUCCESS) {
		printf("=== get space error <%d> ===\n", eRet);
		//eStatus = BASE_API_LEDCTL_SDFAIL;			
	} else {
		psNANDStat->lSdState = BASE_MW_SD_SUCCESS;
	}	
	printf("--- Get NAND space Parameter ---\n");
#endif /* SUBCARD_CHECK_DEBUG */

	printf("--- Init DIR Parameter ---\n");
	/* init DIR parammeter */
	getcwd(psDIRStat->cOriginalDir,sizeof(psDIRStat->cOriginalDir));
	pthread_mutex_init(&psDIRStat->sMutStatus, NULL);

	/* init wifi */
#ifdef WIFI_ENABLE	
	eBASE_MW_WIFI_Init(&gsWifiInfo);
#endif

}


/*------------------------------------------------------------------------------*/
/*!
	@brief		VENC,AUDIO common deinit
*/
/* -----------------------------------------------------------------------------*/
void lBASE_VENC_DeInitGlobal()
{
	BA_FuncIn();
	
	LONG lCnt;
	sBASE_MW_SD_STAT* psSDStat = &gsSDStat;
#ifdef SUBCARD_CHECK_DEBUG
	sBASE_MW_SD_STAT* psNANDStat = &gsNANDStat;
#endif /* SUBCARD_CHECK_DEBUG */
	sBASE_MW_DIR_STAT* psDIRStat = &gsDIRStat;
	sBASE_API_LEDCTL_Info* psLedCtlInfo = &gsLedCtlInfo;
	
	for (lCnt = BASE_VIDEO_CH0; lCnt < BASE_VIDEO_CH2 + 1; lCnt++)
	{
		/* deinit conditionflag init between VencCtrl,venc getstream */
		vCOMM_CF_DeInitCF(&(stCfVencCtrlCh[lCnt]));
	}

	/* deinit ip list  */	
	vCOMM_IPList_DeInit(&gIPList);

	/* deinit cf struct */	
	vCOMM_CF_DeInitCF(&stCfBaseRet);

	/* deinit pool */
	eCOMM_POOL_DeinitAll();

	/* deinit SD  space parammeter */
	psSDStat->lSdState = BASE_MW_SD_INVALID;
	psSDStat->eAudioStat = BASE_MW_AUDIO_NOREC;
	psSDStat->eVideoStat = BASE_MW_VIDEO_NOREC;
	psSDStat->eSnapStat = BASE_MW_SNAP_NOREC;	
	pthread_mutex_destroy(&psSDStat->sMutStatus);

#ifdef SUBCARD_CHECK_DEBUG
	/* deinit NAND  space parammeter */	
	psNANDStat->lSdState = BASE_MW_SD_INVALID;
	psNANDStat->eAudioStat = BASE_MW_AUDIO_NOREC;
	psNANDStat->eVideoStat = BASE_MW_VIDEO_NOREC;
	psNANDStat->eSnapStat = BASE_MW_SNAP_NOREC;
	pthread_mutex_destroy(&psNANDStat->sMutStatus);
#endif /* SUBCARD_CHECK_DEBUG */

	/*deinit Dir parammeter */
	pthread_mutex_destroy(&psDIRStat->sMutStatus);

	eBASE_API_LEDCTL_Deinit(psLedCtlInfo);
	BA_Debug("LED deinit\n");

	/* ir-cut deinit */
	eBASE_IRCUT_CTL_DeInit(&gsIrCutInfo);
	BA_Debug("IR-CUT deinit\n");
	
	/* deinit wifi */
#ifdef WIFI_ENABLE		
	eBASE_MW_WIFI_Deinit(&gsWifiInfo);
	BA_Debug("WIFI deinit\n");
#endif	
	
	BA_FuncOut();
}
/*------------------------------------------------------------------------------*/
/*!
	@brief		AV messege management, communicate with Ctl process
*/
/* -----------------------------------------------------------------------------*/
void vBASE_AVMsgManagement()
{
	pthread_t venc_pid;
	LONG lRet=0;
	LONG lEndFlag=0;
	LONG lCh=-1;		// channel

	/* feedback parameter */
	CHAR cPipeFeedbackParas[COMM_MAXPARAFEEDBACKCHAR];
	CHAR* pcPipeFeedbackParas=cPipeFeedbackParas;

	/* for recieve from pipe */	
	CHAR cCmds[COMM_CMDSPACKETMAXCHAR];	
	CHAR* pcCmds=cCmds;	
	
	/* request id */	
	LONG lReqInfo=-1;	
	
	/* feedback */
	LONG lVEncFeedback=-1;

	/* for split */
	CHAR *pcDelim = ",";
	CHAR cParamsArray[(COMM_CMD_ARRAYSIZE+2)*3][COMM_IP_ADDRESS_LENGTH];
	CHAR **ppcParamsArray = (CHAR **)cParamsArray;
	LONG lparamsArraySize = 0;
	LONG lVal;
	LONG lCmd;
	LONG lOpt;

	/* parse the command and set CF */
	LONG i,j;
	LONG lConditionFlagValue = 0;

	/* clear communictaion buffer */
	memset(glCommuBuf,0,sizeof(glCommuBuf));

	/* parameter */
	sCOMM_PARA* psParas=gsParas;
	plCurrentChangeParamsArray	= lCurrentChangeParamsArray;
	sBASE_API_LEDCTL_Info* psLedCtlInfo = &gsLedCtlInfo;

	/* for debug */
	 debug_show("AV message management start\n");

	/* init venc and audio thread commn resoures */
	lBASE_VENC_InitGlobal();

	/* create led control thread */
	lRet = eBASE_API_LEDCTL_StartThread(psLedCtlInfo);
	if (BASE_API_LEDCTL_SUCCESS != lRet) {
		printf("eBASE_API_LEDCTL_StartThread function fail\n");
		exit(-1);
	}
	
	/* create venc thread */
	lRet=pthread_create(&venc_pid, NULL, vBASE_VENC_Ctrl_Thread, (void*)(&lEndFlag));
	if(lRet!=0){
		printf("[ thread ] : venc create failure!\n");
		exit(-1);
	}

	while(1){
		/* zero the cpara */
		memset(cCmds, 0,sizeof(cCmds));	
		memset(lCurrentChangeParamsArray, -1, sizeof(lCurrentChangeParamsArray));
		
		/* wait for request and get the parameter item */	
		vCOMM_Pipe_Read(&gsPipeCommand, pcCmds);
		if(!strlen(pcCmds)){
			debug_info("[vBASE_AVMsgManagement] vCOMM_Pipe_Read return ERROR \n");
			//continue;
		}

		BA_Debug("get command %s\n", pcCmds);
		
		/* split */
		debug_info("\n\n before split: %s\n\n", pcCmds);
		vCOMM_CMD_Split(pcCmds, pcDelim, ppcParamsArray, &lparamsArraySize);
		/* ppcParamsArray content: req,head,lReqInfo, CMD,Opt,Value,CMD,Opt,Value,..., req,tail,lReqInfo */
		lReqInfo = atol(ppcParamsArray[2]);
		lCh=COMM_GETREQINFO(lReqInfo, COMM_REQCH);
		debug_info("the channel is [ %ld ]\n", lCh);

		/* store uid to communictaion buffer */
		glCommuBuf[BASE_COMMU_UID]=COMM_GETREQINFO(lReqInfo, COMM_REQUI);

		
		/* lFeedback at once when receive the pipe command */
		memset(cPipeFeedbackParas, 0,sizeof(cPipeFeedbackParas));
		sprintf(cPipeFeedbackParas, "0&%ld", lReqInfo);
		vCOMM_Pipe_Write(&gsPipeFeedback, cPipeFeedbackParas);

		/* prepare for the feedback */
		memset(cPipeFeedbackParas,0, sizeof(cPipeFeedbackParas));

		/* prepare the feedback head */
		snprintf(cPipeFeedbackParas, COMM_MAXPARAFEEDBACKCHAR, "1&%ld", lReqInfo);

		/* pcParamsArray content: req,head,lReqInfo, CMD,Opt,Value,CMD,Opt,Value,..., req,tail,lReqInfo */
		
		/* control process will ensure lparamsArraySize > 6 */
		lCmd = atol(ppcParamsArray[3]);
		/* if isp or eth cmd, should modified by channel */
		switch ( lCmd ){
			case COMM_CMD_VENC0:
			case COMM_CMD_ISP0:
				lCmd += lCh * COMM_CMDCHSTEP;
				break;
			default:
				break;
		}
		lConditionFlagValue = lCmd;
		
		for(i = 3, j=0; i < (lparamsArraySize - 3); i += 3, j += 2){ 
			lOpt = atol(ppcParamsArray[i+1]);
			lVal = atol(ppcParamsArray[i+2]);
			
			plCurrentChangeParamsArray[j] = (lCmd<<COMM_CMDMAXSUBSHIFT) + lOpt;
			plCurrentChangeParamsArray[j+1] = lVal;
			snprintf(cPipeFeedbackParas, COMM_MAXPARAFEEDBACKCHAR, "%s&%ld %ld", pcPipeFeedbackParas, ((lCmd<<COMM_CMDMAXSUBSHIFT) + lOpt), lVal);
		}

		/* send condition flag value to VEnc thread */
		// VEnc thread judge which part parameters can be used, and then set lConditionFlagValue to 0
		BA_Debug(" Send condition flag value to VEnc <%ld>\n", lConditionFlagValue);
		vCOMM_CF_SetIdCF(&stCfBaseRet, lConditionFlagValue,COMM_COND_BLOCK);
		/* wait for corresponding response */
		BA_Debug(" wait for VEnc corresponding response\n");
		vCOMM_CF_RdCF(&stCfBaseRet, &lVEncFeedback);
		BA_Debug(" get VEnc corresponding response lVEncFeedback=%ld.........\n", lVEncFeedback);

		if(COMM_COND_CFSYSRETSUC == lVEncFeedback){
			// update parameters to global parameters
			for(i = 0; i < ( (lparamsArraySize -6)/3); i++){
				debug_info(" update parameters to global parameters .........\n");
				vCOMM_PARA_SetPara(psParas+plCurrentChangeParamsArray[2*i], plCurrentChangeParamsArray[2*i], plCurrentChangeParamsArray[2*i+1]);
			}
			/* reset params array */
			memset(lCurrentChangeParamsArray, -1, sizeof(lCurrentChangeParamsArray));

			debug_info(" reset CF and lCurrentChangeParamsArray .........\n");
		}else if( lVEncFeedback==COMM_COND_CFSYSRETFAIL ){
			/* lFeedback on error */
			memset(cPipeFeedbackParas,0, sizeof(cPipeFeedbackParas));
			sprintf(cPipeFeedbackParas, "-1&%ld", lReqInfo);
			vCOMM_Pipe_Write(&gsPipeFeedback, cPipeFeedbackParas);

			/* do not need update parameters list, reset array */
			memset(lCurrentChangeParamsArray, -1, sizeof(lCurrentChangeParamsArray));
			continue;
		}else if( lVEncFeedback==COMM_COND_CFSYSQUIT ){
			/* lFeedback system quit */
			memset(cPipeFeedbackParas,0, sizeof(cPipeFeedbackParas));
			sprintf(cPipeFeedbackParas, "-2&%ld", lReqInfo);
			vCOMM_Pipe_Write(&gsPipeFeedback, cPipeFeedbackParas);

			/* do not need update parameters list, reset array */
			memset(lCurrentChangeParamsArray, -1, sizeof(lCurrentChangeParamsArray));

			/* system quit, should break of the while loop */
			break;
		}

		/* lFeedback on success, write the update parameters back */
		/* add tail */
		if(snprintf(cPipeFeedbackParas, COMM_MAXPARAFEEDBACKCHAR, "%s&", pcPipeFeedbackParas ) >=COMM_MAXPARAFEEDBACKCHAR){
			printf("para to large!\n");
		}

		/* write back to the pipe */
		debug_info(" write process over information to the pipe .........\n");
		vCOMM_Pipe_Write(&gsPipeFeedback, cPipeFeedbackParas);
		
	}
	
	/* wait for venc thread stop */
	pthread_join(venc_pid, (void**) NULL);
	eBASE_API_LEDCTL_StopThread(psLedCtlInfo);
	
	lBASE_VENC_DeInitGlobal();
	
	debug_show("AV message management stop\n");
}


#ifdef __cplusplus
}
#endif

