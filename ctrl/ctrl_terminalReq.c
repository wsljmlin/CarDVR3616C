#define _GNU_SOURCE //only in order to delete warning of getline
#include "ctrl_interface.h"

#ifdef __cplusplus
extern "C"{
#endif


/* extern */
extern sCOMM_CONDFLAG stCfCtlRetTerminal;
extern LONG glEndFlag;
extern sCOMM_SHAREMEM gsShareMem;
extern const LONG SM_OFFSET[];

#define CTRL_CMD_INVALIDTYPE 0


/* macro */
typedef enum {
	CTRL_CMDNONEEDFEEDBACK=4,			// command no need feedback
	CTRL_CMDSUCCESS=5,						// command success
	CTRL_CMDNEEDPOSTHANDLE=6,			// command need post handle
	CTRL_CMDDEFAULTCH=0,					// command default channel
	CTRL_CMDCH0=0,							// command ch0
	CTRL_CMDCH1=1,							// command ch1
	CTRL_CMDCH2=2,							// command ch2
	CTRL_CMDNOTSUPPORT=-1,				// command not support
	CTRL_CMDNEEDPARA=-2,					// need at least one parameter
	CTRL_CMDPARANOTSUPPORT=-3,			// parameter not support
	CTRL_CMDPARAVALNOTSUPPORT=-4,		// parameter value not support
	CTRL_CMDPARAVALFMTERR=-5,			// parameter format not correct
	CTRL_CMDPARAVALNOCH=-6,				// parameter format not correct, no channel infomation
	CTRL_CMDPARAVALMULTIPLECH=-7,		// parameter format not correct, multiple channel set
	CTRL_CMDPARAVALMUSTPARALACK=-8,	// parameter format not correct, must parameters lack
	CTRL_CMDINFONOTFULL=-9,				// command information not full
} eCTRL_CMD_STATE;

typedef enum {
	CTRL_CMD_READFST,						// read first
	CTRL_CMD_READNOTFST					// read not first
} eCTRL_CMD_CFGFSTFLAG;

typedef struct {
	/* header */
	UCHAR* pucHeader;
	/* length */
	LONG lLength;
} sCTRL_CMD_REGION;

typedef enum {
	CTRL_CMD_REG,
	CTRL_CMD_MEM,
	CTRL_CMD_LIST,
	CTRL_CMD_LOADBMP,
	CTRL_CMD_FRAME,
	CTRL_CMD_REGWT,
} eCTRL_CMD_POSTTYPE;

typedef enum {
	CTRL_CMD_INVALIDCH=-1,
	CTRL_CMD_CH0,
	CTRL_CMD_CH1,
	CTRL_CMD_CH2,
} eCTRL_CMD_CH;

typedef enum {
	CTRL_CMD_REGION_COVER,
	CTRL_CMD_REGION_OVERLAY,
} eCTRL_CMD_REGION_CATEGORY;

typedef struct {
	/* post type */
	LONG lPostType;
	/* region */
	sCTRL_CMD_REGION sReg;
	sCTRL_CMD_REGION sMem;
	/* channel */
	eCTRL_CMD_CH eCh;
} sCTRL_CMD_POSTHANDLE;

typedef struct {
	LONG lMem;
	LONG lMemLen;
	LONG lReg;
	LONG lRegLen;
	LONG lMemWt;
	LONG lMemWtVal;
	LONG lRegWt;
	LONG lRegWtVal;
	LONG lRawViCh;
	LONG lRawViCnt;
} sCTRL_CMD_CHECKSYS;

typedef struct {
	LONG lVideoIp;
	LONG lVideoPort;
	LONG lVideoUse;
	LONG lAudioIp;
	LONG lAudioPort;
	LONG lAudioUse;
} sCTRL_CMD_CHECKETH;

typedef struct {
	LONG lHdmiWidth;
	LONG lHdmiHeight;
	LONG lPointX;
	LONG lPointY;
	LONG lVencPicSize;
	LONG lVoResolutio;
} sCTRL_CMD_CHECKHDMI;

typedef struct {
	LONG lVideoModReq;
	LONG lVideoMod;
} sCTRL_CMD_CHECKVENC;

typedef struct {
	LONG lCategory;
	LONG lCovResolution;
	LONG lCovStart;
	LONG lCovStop;
	LONG lCovChange;
	LONG lCovRecX;
	LONG lCovRecY;
	LONG lCovRecHeight;
	LONG lCovRecWidth;
	LONG lCovColor;
	LONG lOverlayResolution;
	LONG lOverlayStart;
	LONG lOverlayStop;
	LONG lOverlayChange;
	LONG lOverlayRecX;
	LONG lOverlayRecY;
	LONG lOverlayFAlpha;
	LONG lOverlayShowBmp;
	LONG lOverlayHeight;
	LONG lOverlayWidth;
} sCTRL_CMD_CHECKREG;

typedef struct {
	LONG lVdaResoution;
	LONG lVdaStart;
	LONG lVdaStop;
	LONG lVdaChange;
	LONG lVdaRecX;
	LONG lVdaRecY;
	LONG lVdaRecHeight;
	LONG lVdaRecWidth;
} sCTRL_CMD_CHECKVDA;

#define CTRL_SUPPORTCMDCNT 8					// current support command count
#define CTRL_CMDMAXCHAR 10					// the command [ not include parameter ] at most bytes
#define CTRL_CMDMAXALLCHAR 100				// the command [ include parameter ] at most bytes
#define CTRL_SUPPORTINFOCNT 1000			// the max infomation bytes
#define CTRL_REQMAXCHARALL 100				// the request at most bytes
#define CTRL_REQMAXCHAREACH 20				// the request at most bytes for each option

/* default dump length */
#define CTRL_DEFAULTBYTE_MEM 256
#define CTRL_DEFAULTBYTE_REG 1


static CHAR cCOMMAND[CTRL_SUPPORTCMDCNT][CTRL_CMDMAXCHAR]={
	{"system"},
	{"hdmi"},
	{"eth"},
	{"isp"},
	{"rec"},
	{"venc"},
	{"reg"},
	{"vda"}
};

static char cINFO[CTRL_SUPPORTINFOCNT];
CHAR* pcINFO=cINFO;
static sCTRL_CMD_POSTHANDLE sPostHandle;
sCTRL_CMD_POSTHANDLE* psPostHandle=&sPostHandle;

/*------------------------------------------------------------------------------*/
/*!
    @brief			the thread of terminal request
    
    @param			void* arg					[in]						NULL

    @retval			NULL
*/
/* -----------------------------------------------------------------------------*/
void* pvCTRL_TerminalReq(void* arg);

/*------------------------------------------------------------------------------*/
/*!
    @brief			show the help information
*/
/* -----------------------------------------------------------------------------*/
void vCTRL_TerminalReq_ShowHelp();

/*------------------------------------------------------------------------------*/
/*!
    @brief			show the system help information
*/
/* -----------------------------------------------------------------------------*/
void vCTRL_TerminalReq_ShowHelp_System();

/*------------------------------------------------------------------------------*/
/*!
    @brief			show the hdmi help information
*/
/* -----------------------------------------------------------------------------*/
void vCTRL_TerminalReq_ShowHelp_Hdmi();

/*------------------------------------------------------------------------------*/
/*!
    @brief			show the rec help information
*/
/* -----------------------------------------------------------------------------*/
void vCTRL_TerminalReq_ShowHelp_Rec();

/*------------------------------------------------------------------------------*/
/*!
    @brief			show the venc help information
*/
/* -----------------------------------------------------------------------------*/
void vCTRL_TerminalReq_ShowHelp_Venc();

/*------------------------------------------------------------------------------*/
/*!
    @brief			show the eth help information
*/
/* -----------------------------------------------------------------------------*/
void vCTRL_TerminalReq_ShowHelp_Eth();

/*------------------------------------------------------------------------------*/
/*!
    @brief			show the system isp information
*/
/* -----------------------------------------------------------------------------*/
void vCTRL_TerminalReq_ShowHelp_Isp();

/*------------------------------------------------------------------------------*/
/*!
    @brief			show the region cover information
*/
/* -----------------------------------------------------------------------------*/
void vCTRL_TerminalReq_ShowHelp_Reg();

/*------------------------------------------------------------------------------*/
/*!
    @brief			show the vda information
*/
/* -----------------------------------------------------------------------------*/
void vCTRL_TerminalReq_ShowHelp_Vda();

/*------------------------------------------------------------------------------*/
/*!
    @brief			convert the command into request
    
    @param			CHAR* lCmd				[in]						the command string
    @param			CHAR** ppcReq			[out]					the request: comamnd,option,val

    @retval			CTRL_CMDSUCCESS								command success
					CTRL_CMDNOTSUPPORT						command not support
					CTRL_CMDNEEDPARA							need at least one parameter
					CTRL_CMDPARANOTSUPPORT					parameter not support
					CTRL_CMDPARAVALNOTSUPPORT				parameter value not support
					CTRL_CMDPARAVALFMTERR					parameter format not correct
					CTRL_CMDPARAVALNOCH						parameter format not correct, no channel infomation
					CTRL_CMDPARAVALMULTIPLECH				parameter format not correct, multiple channel set
					CTRL_CMDPARAVALMUSTPARALACK			parameter format not correct, must parameters lack
*/
/* -----------------------------------------------------------------------------*/
LONG lCTRL_TerminalReq_Cmd2Req(char* lCmd, char** ppcReq);

/*------------------------------------------------------------------------------*/
/*!
    @brief			convert the command into request
    
    @param			CHAR* lCmd				[in]						the command string
    @param			CHAR* lCmd				[out]					the command string

    @retval			CTRL_CMDSUCCESS								command success
					CTRL_CMDNOTSUPPORT						command not support
					CTRL_CMDNEEDPARA							need at least one parameter
					CTRL_CMDPARANOTSUPPORT					parameter not support
					CTRL_CMDPARAVALNOTSUPPORT				parameter value not support
					CTRL_CMDPARAVALFMTERR					parameter format not correct
					CTRL_CMDPARAVALNOCH						parameter format not correct, no channel infomation
					CTRL_CMDPARAVALMULTIPLECH				parameter format not correct, multiple channel set
					CTRL_CMDPARAVALMUSTPARALACK			parameter format not correct, must parameters lack
*/
/* -----------------------------------------------------------------------------*/
LONG lCTRL_TerminalReq_GetDebugCmd(char* pcCmd);

/*------------------------------------------------------------------------------*/
/*!
    @brief			get the command name, output pointer point to the first option
    
    @param			CHAR* lCmd				[in]						the command string
    @param			CHAR** opt				[out]					point to the first option

    @retval			CTRL_CMDSUCCESS								command success
					CTRL_CMDNOTSUPPORT						command not support
					CTRL_CMDNEEDPARA							need at least one parameter
*/
/* -----------------------------------------------------------------------------*/
LONG lCTRL_TerminalReq_GetCmd(char* lCmd, char** opt);

/*------------------------------------------------------------------------------*/
/*!
    @brief			check the input of system command
    
    @param			sCTRL_CMD_CHECKSYS* psChkSys				[in]						the system struct to be check

    @retval			CTRL_CMDSUCCESS								command success
					CTRL_CMDPARANOTSUPPORT					parameter not support
					CTRL_CMDPARAVALNOTSUPPORT				parameter value not support
					CTRL_CMDPARAVALFMTERR					parameter format not correct
					CTRL_CMDPARAVALNOCH						parameter format not correct, no channel infomation
					CTRL_CMDPARAVALMULTIPLECH				parameter format not correct, multiple channel set
					CTRL_CMDPARAVALMUSTPARALACK			parameter format not correct, must parameters lack

*/
/* -----------------------------------------------------------------------------*/
eCTRL_CMD_STATE lCTRL_TerminalReq_CheckSys(sCTRL_CMD_CHECKSYS* psChkSys);

/*------------------------------------------------------------------------------*/
/*!
    @brief			check the input of eth command
    
    @param			sCTRL_CMD_CHECKETH* psChkEth				[in]						the eth struct to be check

    @retval			CTRL_CMDSUCCESS								command success
					CTRL_CMDPARANOTSUPPORT					parameter not support
					CTRL_CMDPARAVALNOTSUPPORT				parameter value not support
					CTRL_CMDPARAVALFMTERR					parameter format not correct
					CTRL_CMDPARAVALNOCH						parameter format not correct, no channel infomation
					CTRL_CMDPARAVALMULTIPLECH				parameter format not correct, multiple channel set
					CTRL_CMDPARAVALMUSTPARALACK			parameter format not correct, must parameters lack

*/
/* -----------------------------------------------------------------------------*/
eCTRL_CMD_STATE lCTRL_TerminalReq_CheckEth(sCTRL_CMD_CHECKETH* psChkEth);

/*------------------------------------------------------------------------------*/
/*!
    @brief			check the input of venc command
    
    @param			sCTRL_CMD_CHECKETH* psChkEth				[in]						the eth struct to be check

    @retval			CTRL_CMDSUCCESS								command success
					CTRL_CMDPARANOTSUPPORT					parameter not support
					CTRL_CMDPARAVALNOTSUPPORT				parameter value not support
					CTRL_CMDPARAVALFMTERR					parameter format not correct
					CTRL_CMDPARAVALNOCH						parameter format not correct, no channel infomation
					CTRL_CMDPARAVALMULTIPLECH				parameter format not correct, multiple channel set
					CTRL_CMDPARAVALMUSTPARALACK			parameter format not correct, must parameters lack

*/
/* -----------------------------------------------------------------------------*/
eCTRL_CMD_STATE lCTRL_TerminalReq_CheckVenc(sCTRL_CMD_CHECKVENC* psChkVenc);

/*------------------------------------------------------------------------------*/
/*!
    @brief			check the input of region command
    
    @param			sCTRL_CMD_CHECKETH* psChkEth				[in]						the eth struct to be check

    @retval			CTRL_CMDSUCCESS								command success
					CTRL_CMDPARANOTSUPPORT					parameter not support
					CTRL_CMDPARAVALNOTSUPPORT				parameter value not support
					CTRL_CMDPARAVALFMTERR					parameter format not correct
					CTRL_CMDPARAVALNOCH						parameter format not correct, no channel infomation
					CTRL_CMDPARAVALMULTIPLECH				parameter format not correct, multiple channel set
					CTRL_CMDPARAVALMUSTPARALACK			parameter format not correct, must parameters lack

*/
/* -----------------------------------------------------------------------------*/
eCTRL_CMD_STATE lCTRL_TerminalReq_CheckReg(sCTRL_CMD_CHECKREG* psChkReg);

/*------------------------------------------------------------------------------*/
/*!
    @brief			check the input of vda command
    
    @param			sCTRL_CMD_CHECKETH* psChkEth				[in]						the eth struct to be check

    @retval			CTRL_CMDSUCCESS								command success
					CTRL_CMDPARANOTSUPPORT					parameter not support
					CTRL_CMDPARAVALNOTSUPPORT				parameter value not support
					CTRL_CMDPARAVALFMTERR					parameter format not correct
					CTRL_CMDPARAVALNOCH						parameter format not correct, no channel infomation
					CTRL_CMDPARAVALMULTIPLECH				parameter format not correct, multiple channel set
					CTRL_CMDPARAVALMUSTPARALACK			parameter format not correct, must parameters lack

*/
/* -----------------------------------------------------------------------------*/
eCTRL_CMD_STATE lCTRL_TerminalReq_CheckVda(sCTRL_CMD_CHECKVDA* psChkVda);

/*------------------------------------------------------------------------------*/
/*!
    @brief			check the input of hdmi command
    
    @param			sCTRL_CMD_CHECKHDMI* psChkHdmi				[in]						the hdmi struct to be check

    @retval			CTRL_CMDSUCCESS								command success
					CTRL_CMDPARANOTSUPPORT					parameter not support
					CTRL_CMDPARAVALNOTSUPPORT				parameter value not support
					CTRL_CMDPARAVALFMTERR					parameter format not correct
					CTRL_CMDPARAVALNOCH						parameter format not correct, no channel infomation
					CTRL_CMDPARAVALMULTIPLECH				parameter format not correct, multiple channel set
					CTRL_CMDPARAVALMUSTPARALACK			parameter format not correct, must parameters lack

*/
/* -----------------------------------------------------------------------------*/
eCTRL_CMD_STATE lCTRL_TerminalReq_CheckHdmi(sCTRL_CMD_CHECKHDMI* psChkHdmi);
	
/*------------------------------------------------------------------------------*/
/*!
    @brief			parse the system sub command
    
    @param			CHAR* opt					[in]						the pointer point to the first option
    @param			CHAR** ppcReq			[out]					the request: comamnd,option,val

    @retval			CTRL_CMDSUCCESS								command success
					CTRL_CMDPARANOTSUPPORT					parameter not support
					CTRL_CMDPARAVALNOTSUPPORT				parameter value not support
					CTRL_CMDPARAVALFMTERR					parameter format not correct
					CTRL_CMDPARAVALNOCH						parameter format not correct, no channel infomation
					CTRL_CMDPARAVALMULTIPLECH				parameter format not correct, multiple channel set
					CTRL_CMDPARAVALMUSTPARALACK			parameter format not correct, must parameters lack
*/
/* -----------------------------------------------------------------------------*/
LONG lCTRL_TerminalReq_ParseSystemCmd(char* opt, char** ppcReq);

/*------------------------------------------------------------------------------*/
/*!
    @brief			parse the eth sub command
    
    @param			CHAR* opt					[in]						the pointer point to the first option
    @param			CHAR** ppcReq			[out]					the request: comamnd,option,val

    @retval			CTRL_CMDSUCCESS								command success
					CTRL_CMDPARANOTSUPPORT					parameter not support
					CTRL_CMDPARAVALNOTSUPPORT				parameter value not support
					CTRL_CMDPARAVALFMTERR					parameter format not correct
					CTRL_CMDPARAVALNOCH						parameter format not correct, no channel infomation
					CTRL_CMDPARAVALMULTIPLECH				parameter format not correct, multiple channel set
					CTRL_CMDPARAVALMUSTPARALACK			parameter format not correct, must parameters lack
*/
/* -----------------------------------------------------------------------------*/
LONG lCTRL_TerminalReq_ParseVencCmd(char* opt, char** ppcReq);

/*------------------------------------------------------------------------------*/
/*!
    @brief			parse the rtp sub command
    
    @param			CHAR* opt					[in]						the pointer point to the first option
    @param			CHAR** ppcReq			[out]					the request: comamnd,option,val

    @retval			CTRL_CMDSUCCESS								command success
					CTRL_CMDPARANOTSUPPORT					parameter not support
					CTRL_CMDPARAVALNOTSUPPORT				parameter value not support
					CTRL_CMDPARAVALFMTERR					parameter format not correct
					CTRL_CMDPARAVALNOCH						parameter format not correct, no channel infomation
					CTRL_CMDPARAVALMULTIPLECH				parameter format not correct, multiple channel set
					CTRL_CMDPARAVALMUSTPARALACK			parameter format not correct, must parameters lack
*/
/* -----------------------------------------------------------------------------*/
LONG lCTRL_TerminalReq_ParseEthCmd(char* opt, char** ppcReq);

/*------------------------------------------------------------------------------*/
/*!
    @brief			parse the isp sub command
    
    @param			CHAR* opt					[in]						the pointer point to the first option
    @param			CHAR** ppcReq			[out]					the request: comamnd,option,val

    @retval			CTRL_CMDSUCCESS								command success
					CTRL_CMDPARANOTSUPPORT					parameter not support
					CTRL_CMDPARAVALNOTSUPPORT				parameter value not support
					CTRL_CMDPARAVALFMTERR					parameter format not correct
					CTRL_CMDPARAVALNOCH						parameter format not correct, no channel infomation
					CTRL_CMDPARAVALMULTIPLECH				parameter format not correct, multiple channel set
					CTRL_CMDPARAVALMUSTPARALACK			parameter format not correct, must parameters lack
*/
/* -----------------------------------------------------------------------------*/
LONG lCTRL_TerminalReq_ParseIspCmd(char* opt, char** ppcReq);

/*------------------------------------------------------------------------------*/
/*!
    @brief			parse the isp sub command
    
    @param			CHAR* opt					[in]						the pointer point to the first option
    @param			CHAR** ppcReq			[out]					the request: comamnd,option,val

    @retval			CTRL_CMDSUCCESS								command success
					CTRL_CMDPARANOTSUPPORT					parameter not support
					CTRL_CMDPARAVALNOTSUPPORT				parameter value not support
					CTRL_CMDPARAVALFMTERR					parameter format not correct
					CTRL_CMDPARAVALNOCH						parameter format not correct, no channel infomation
					CTRL_CMDPARAVALMULTIPLECH				parameter format not correct, multiple channel set
					CTRL_CMDPARAVALMUSTPARALACK			parameter format not correct, must parameters lack
*/
/* -----------------------------------------------------------------------------*/
LONG lCTRL_TerminalReq_ParseRegCmd(char* opt, char** ppcReq);

/*------------------------------------------------------------------------------*/
/*!
    @brief			parse the rec sub command
    
    @param			CHAR* opt					[in]						the pointer point to the first option
    @param			CHAR** ppcReq			[out]					the request: comamnd,option,val

    @retval			CTRL_CMDSUCCESS								command success
					CTRL_CMDPARANOTSUPPORT					parameter not support
					CTRL_CMDPARAVALNOTSUPPORT				parameter value not support
					CTRL_CMDPARAVALFMTERR					parameter format not correct
					CTRL_CMDPARAVALNOCH						parameter format not correct, no channel infomation
					CTRL_CMDPARAVALMULTIPLECH				parameter format not correct, multiple channel set
					CTRL_CMDPARAVALMUSTPARALACK			parameter format not correct, must parameters lack
*/
/* -----------------------------------------------------------------------------*/
LONG lCTRL_TerminalReq_ParseRecCmd(char* opt, char** ppcReq);

/*------------------------------------------------------------------------------*/
/*!
    @brief			post handle
    
    @param			sCTRL_CMD_POSTHANDLE* psPostHandle				[in]						the pointer point to the posthandle struct

*/
/* -----------------------------------------------------------------------------*/
void vCTRL_TerminalReq_PostHandle(sCTRL_CMD_POSTHANDLE* psPostHandle); 

/*------------------------------------------------------------------------------*/
/*!
    @brief			reset post handle struct
    
    @param			sCTRL_CMD_POSTHANDLE* psPostHandle				[out]					the pointer point to the posthandle struct

*/
/* -----------------------------------------------------------------------------*/
void vCTRL_TerminalReq_PostHandle_Reset(sCTRL_CMD_POSTHANDLE* psPostHandle); 


/* implement */
void* pvCTRL_TerminalReq(void* arg){

	LONG lRet=0;

	/* terminal argument */
	sCTRL_TERMINAL_CLIENT* psTerminalClient=&gsTerminalClient;

	/* for config input */
	LONG lCfgFst=CTRL_CMD_READFST;
	FILE* psConfig;
	
	/* this is the command receive from terminal */
	CHAR cCmd[CTRL_CMDMAXALLCHAR];
	CHAR* pcCmd=cCmd;
	size_t len=sizeof(cCmd);
		
	/* cCmd will change to lReq */
	CHAR cReq[CTRL_REQMAXCHARALL];
	CHAR* pcReq=cReq;
	LONG lSpcCnt=0;
	CHAR c=' ';
	LONG lReqInfo=0;
	LONG lCh=-1;			// channel

	/* condition flag id and value */
	LONG lCfId;
	LONG lCfRet;

	/* end flag */
	volatile LONG lEndFlag=0;

	debug_show("terminal request start\n");
	
	/* welcome menu */
	printf("\n----------Welcome to IPC linux console system---------------\n\n");
	
	/* regist signal handle function <ctl+D> */

	/* regist the CF */
	vCOMM_CF_RegCF(&stCfCtlRetTerminal);

	/* init mutex */
	pthread_mutex_init(&mutStringPoolWt, NULL);

	/* in order to make > below, wait a little for other thread init */
	sleep(1);
	
	while(!lEndFlag){
		/* clear the information buffer */
		memset(cINFO, 0,sizeof(cINFO));
		
		/* prepare the buffer */
		memset(pcCmd, 0, CTRL_CMDMAXALLCHAR);

		/* get command from terminal, or from config file */
		if(CTRL_CMD_NONEEDREAD==psTerminalClient->lCfgType){
			/* wait for user input */
			printf("cmd > ");
			
			/* from terminal */
			getline(&pcCmd, &len, stdin);

		} else {
			if(CTRL_CMD_READFST==lCfgFst){
				/* first time, open the file */
				psConfig=fopen(psTerminalClient->cCfgName, "r");
				if(!psConfig){
					/* indicate open error */
					printf("error to open config file <%s>, but to be continue ..\n", psTerminalClient->cCfgName);
					psTerminalClient->lCfgType=CTRL_CMD_NONEEDREAD;
					continue;
				}
				
				printf("read command from the config file <%s> ..\n", psTerminalClient->cCfgName);
				lCfgFst=CTRL_CMD_READNOTFST;
			}

			/* read command from config file */
			if(!fgets( pcCmd, CTRL_CMDMAXALLCHAR, psConfig)){
				
				/* if end, toggle the type and continue */
				printf("all the command is done!\n\n");
				psTerminalClient->lCfgType=CTRL_CMD_NONEEDREAD;
				lCfgFst=CTRL_CMD_READFST;
				fclose(psConfig);
				psConfig=NULL;
				memset(psTerminalClient->cCfgName, 0,sizeof(psTerminalClient->cCfgName));
				continue;
			}

			/* if not end */
			printf("cmd > %s\n", pcCmd);
		}

		/* check valid of the command */
		if(!strlen(pcCmd)){
			/* indicate error input */
			strcpy(pcCmd, "system -q\n");
		}

		/* remove cr */
		cCmd[strlen(pcCmd)-1]=0;
		
		/* only have spaces should skip at once */	
		lSpcCnt=strspn(pcCmd, &c);
		if( lSpcCnt==strlen(pcCmd))	
			continue;

		/* for debug command */
		lCTRL_TerminalReq_GetDebugCmd(pcCmd);
		
		/* change the cCmd to lReq */
		lRet=lCTRL_TerminalReq_Cmd2Req(pcCmd, &pcReq);
		if(lRet<0){
			printf("%s\n", cINFO);
			//vCTRL_TerminalReq_ShowHelp();
			continue;
		}else if(lRet==CTRL_CMDPARAVALNOTSUPPORT){
			printf("%s\n", cINFO);	
			continue;
		}
		else if (CTRL_CMDNONEEDFEEDBACK==lRet) {
			//printf("%s", cINFO);
			//printf("no need feedback\n\n");
			continue;
		} else {
			lCh=lRet;
			debug_info("channel [ %ld ]\n", lCh);
		}
		
#ifdef DEBUG_TERMINAL_REQ
		printf("%s", cINFO);
		continue;
#endif /* DEBUG_TERMINAL_REQ */

		/* get lock and allocate reqid */
		pthread_mutex_lock(&sMutReqId);
		if(++glReqId == COMM_CMDMAXREQID)
			glReqId=0;

		/* prepare reqInfo : include reqId, ch, ui */
		lReqInfo = (COMM_SETREQINFO(glReqId, COMM_REQID) | \
					COMM_SETREQINFO(lCh, COMM_REQCH) | \
					COMM_SETREQINFO(COMM_UI_TERMINAL, COMM_REQUI));

		/* add tail and head */	
		vCOMM_CMD_AddCmdHeadTail( &pcReq, lReqInfo );
		/* release the lock */
		pthread_mutex_unlock(&sMutReqId);

		/* send to request list */
		vCOMM_LQ_Write(&sReqList, pcReq, AUTO_DETECT_STRLEN);

		/* show information */
		printf("%s\n", cINFO);
		printf("Sending command...\n");

		/** wait until get the right reqInfo **/
		do{
			vCOMM_CF_GetIdCF(&stCfCtlRetTerminal, &lCfId, COMM_COND_BLOCK);
		} while (lReqInfo!=lCfId);

		/* now, get the feedback and handle it */
		while(1) {

			/* read the feedback of the ret */
			vCOMM_CF_RdCF(&stCfCtlRetTerminal, &lCfRet);

			if(lCfRet==COMM_CF_FEEDBACKATONCESUC){
				/* first feedback, base process get the infomation */
				printf("Base process handleing...\n");

			}else if(lCfRet==COMM_CF_FEEDBACKFINISHSUC){
				/* second feedback, base process finish handle the command */
				printf("Base process done!\n");
						
				break;
			}else if(lCfRet==COMM_CF_FEEDBACKFINISHSYSQUIT){
				/* second feedback, base process finish handle the command */
				printf("console session terminate !\n");

				/* toggle the end flag */
				lEndFlag=1;
				
				break;
			}else{
				printf("abnormal\n");
				/* abnormal branch */
				if(lCfRet==COMM_COND_CFSYSRETFAIL){
					/* time out */
					printf("timeout to get feedback, the command is abnormal\n");
				} else if ( lCfRet==COMM_CF_FEEDBACKFINISHFAIL ){
					printf("base handle abnormal\n");
				} else {
					printf("unknowning bug <%ld>..\n", lCfRet);
				}
				break;
			}

			/* again, get id */
			vCOMM_CF_GetIdCF(&stCfCtlRetTerminal, &lCfId, COMM_COND_BLOCK);
		}
	}

	/* destroy the mutex */
	pthread_mutex_destroy(&mutStringPoolWt);

	/* deregist the CF */
	vCOMM_CF_DeregCF(&stCfCtlRetTerminal);

	debug_show("terminal request stop\n");
	
	return (void*) NULL;
}
	
void vCTRL_TerminalReq_ShowHelp(){
	printf("\nSupport Format :\n\n");
	vCTRL_TerminalReq_ShowHelp_System();
	vCTRL_TerminalReq_ShowHelp_Hdmi();
	vCTRL_TerminalReq_ShowHelp_Rec();
	vCTRL_TerminalReq_ShowHelp_Venc();
	vCTRL_TerminalReq_ShowHelp_Eth();
	vCTRL_TerminalReq_ShowHelp_Isp();
	vCTRL_TerminalReq_ShowHelp_Reg();
	vCTRL_TerminalReq_ShowHelp_Vda();
}

void vCTRL_TerminalReq_ShowHelp_System(){
	printf("system [option] [value]\n");
	printf("option :\n");
	printf("\t-q\t\tsystem quit\n");
	printf("\t-s\t<vi/vo/venc>/tshow info\n");
	printf("\t-f\t\tconfig file\n");
	printf("\t-r <register address hex> -rl <count in hex: default 0x1>\tdump register\n");
	printf("\t-m <memory address hex> -ml <size in hex: default 0x100>\tdump memory\n");
	printf("\t-rw <register address hex> -rv <register value hex>\twrite register\n");
	printf("\t-mw <memory address hex> -mv <memory value hex>\twrite memory\n");
	printf("\t-fb <1080p/720p/480p>\tshow frame buffer");
	printf("\t-h\t\tshow this help infomation\n");
	printf("\t-ls <client>\tshow all the client info\n");
	printf("\t-rr <1080p/720p/480p> -rs <frame count:lager than zero>\tsave vi raw data\n");
	printf("\n");
}

void vCTRL_TerminalReq_ShowHelp_Hdmi(){
	printf("hdmi [option] [value]\n");
	printf("option :\n");
	printf("\t-r\t\tresolution\n");
	printf("\n");
}

void vCTRL_TerminalReq_ShowHelp_Rec(){
	printf("rec [option] [value]\n");
	printf("option :\n");
	printf("\t-r\t\tresolution\n");
	printf("\t-s\t\tstop\n");
	printf("\n");
}

void vCTRL_TerminalReq_ShowHelp_Venc(){
	printf("venc [option] [value]\n");
	printf("option :\n");
	printf("\t-vr\t\tvideo resolution\n");
	printf("\t-vf\t\tvideo framerate\n");
	printf("\t-vg\t\tvideo gop\n");
	printf("\n");
}

void vCTRL_TerminalReq_ShowHelp_Eth(){
	printf("eth [option] [value]\n");
	printf("option :\n");
	printf("\t-i\t\tip address ( must )\n");
	printf("\t-p\t\tport ( must )\n");
	printf("\t-vr\t\tvideo resolution\n");
	printf("\t-vs\t\tvideo stop\n");
	printf("\t-a\taudio start\n");
	printf("\t-as\taudio stop\n");
	printf("\t-ai\taudio ip\n");
	printf("\t-ap\taudio port\n");
	printf("\n");
}

void vCTRL_TerminalReq_ShowHelp_Isp(){
	printf("isp [option] [value]\n");
	printf("option :\n");
	printf("\t-vr\t\tvideo resolution\n");
	printf("\t-ct\t\tcontrast\n");
	printf("\t-b\t\tbrightneaa\n");
	printf("\t-cl\t\tcolor\n");
	printf("\t-sn\t\tsharpness\n");
	printf("\t-dd\t\tDDP\n");
	printf("\t-di\t\tDIS\n");
	printf("\t-aw\t\tAWB\n");
	printf("\t-ae\t\tae\n");
	printf("\t-af\t\tAF\n");
	printf("\t-ai\t\tAI\n");
	printf("\t-gp\t\tgamma preset\n");
	printf("\n");
}

void vCTRL_TerminalReq_ShowHelp_Reg(){
	printf("reg [option] [value]\n");
	printf("option :\n");
	printf("\t-cr <resolution:1080p/720p/480p> -cb<cover begin>\n");
	printf("\t-cr <resolution:1080p/720p/480p> -cb<cover begin> -cx <cover point x> -cy <cover point y> -ch <cover height> -cw <cover width>\n");	 
	printf("\t-cr <resolution:1080p/720p/480p> -cb<cover begin> -cx <cover point x> -cy <cover point y> -ch <cover height> -cw <cover width> -cc <color:red/green/blue/black/white/default(black)>\n");	
	printf("\t-cr <resolution:1080p/720p/480p> -cx <cover point x> -cy <cover point y> -ch <cover height> -cw <cover width>\n");		 
	printf("\t-cr <resolution:1080p/720p/480p> -cx <cover point x> -cy <cover point y> -ch <cover height> -cw <cover width> -cc <color:red/green/blue/black/white/default(black)\n");
	printf("\t-cr <resolution:1080p/720p/480p> -cc <color:red/green/blue/black/white/default(black)\n");
	printf("\t-cr <resolution:1080p/720p/480p> -ce<cover end>\n");
	printf("\n");
}

void vCTRL_TerminalReq_ShowHelp_Vda(){
	printf("vda [option] [value]\n");
	printf("option :\n");
	printf("\t-r <resolution:1080p/720p/480p> -b<vda begin> -x <vda start x> -y <vda start y> -h <vda region height> -w <vda region width>\n");
	printf("\t -r <resolution:1080p/720p/480p> -e<vda end>\n");
	printf("\n");
}

LONG lCTRL_TerminalReq_Cmd2Req(char* lCmd, char** ppcReq){
	LONG lRet=0;

	/* pcOpt point to first option */
	CHAR* pcOpt=NULL;

	/* some pre error check */
	lRet=lCTRL_TerminalReq_GetCmd(lCmd, &pcOpt);
	if(lRet!=CTRL_CMDSUCCESS){
		return lRet;
	}

	/* parse the command */
	if(!strcmp(lCmd, "system")){
		/* system */
		lRet=lCTRL_TerminalReq_ParseSystemCmd(pcOpt, ppcReq);
	}else if(!strcmp(lCmd, "rec")){
		/* record */
		lRet=lCTRL_TerminalReq_ParseRecCmd(pcOpt, ppcReq);
	}else if(!strcmp(lCmd, "venc")){
		/* venc */
		lRet=lCTRL_TerminalReq_ParseVencCmd(pcOpt, ppcReq);
	}else if(!strcmp(lCmd, "eth")){
		/* eth */
		lRet=lCTRL_TerminalReq_ParseEthCmd(pcOpt, ppcReq);
	}else if(!strcmp(lCmd, "isp")){
		/* isp */
		lRet=lCTRL_TerminalReq_ParseIspCmd(pcOpt, ppcReq);
	}else if(!strcmp(lCmd, "reg")){
		/* region */
		lRet=lCTRL_TerminalReq_ParseRegCmd(pcOpt, ppcReq);
	}else{
		/* command not support */
		return CTRL_CMDNOTSUPPORT;
	}

	return lRet;
}

/* check and get the lCmd, first step of parse command */
LONG lCTRL_TerminalReq_GetCmd(char* lCmd, char** opt){
	LONG lRet=CTRL_CMDSUCCESS;
	LONG i;
	CHAR* pc;
	CHAR* pcNext;

	/* init */
	pc=lCmd;
	*opt=NULL;

	if(( pcNext=index(pc, ' ')) != NULL) {
		*pcNext='\0';
	} else {
		/* indicate, we only have command */
		lRet=CTRL_CMDNEEDPARA;
		sprintf(cINFO, "%s\tNeed option parameter !\n", pcINFO);
		return lRet;
	}

	/* first, check the valid of the command */
	for(i=0;i<CTRL_SUPPORTCMDCNT;i++){
		/* compare the command one by one */
		if(!strcmp(pc, cCOMMAND[i])){
			sprintf(cINFO, "%sThe input command %s :\n", pcINFO, lCmd);
			break; 
		}
	}

	if(i==CTRL_SUPPORTCMDCNT){
		/* indicate no such command */
		lRet=CTRL_CMDNOTSUPPORT;
		sprintf(cINFO, "%sThe input command %s is not support !\n", pcINFO, lCmd);
		return lRet;
	}

	/* update pc */
	pc=pcNext+1;

	/* check whether have parameter */
	*opt=index(pc, '-');

	if(!*opt){
		/* indicate no option */
		lRet=CTRL_CMDNEEDPARA;
		sprintf(cINFO, "%s\tNeed option parameter !\n", pcINFO);
		return lRet;
	}

	return lRet;
}
LONG lCTRL_TerminalReq_IspCmdHandle(CHAR* p,LONG lCmd,LONG lOpt,LONG lFstFlag,char** ppcReq)
{      
	LONG lRet=CTRL_CMDPARAVALNOCH;
	CHAR* pcTmp;
	LONG lVal=0;
	
	/* for command string */
	sCOMM_CMD sCmd;
	CHAR cOneCmd[COMM_CMDMAXCHAREACH];
	CHAR* pcOneCmd=cOneCmd;
	LONG nChar=0;
	//LONG lFstFlag=1;
	while(*p!=' '){
		p++;		
	}
	while(*++p==' ');
	
	/* strip the string */
	if((pcTmp=index(p,' '))!=NULL){
		/* prepare for the string */
		*pcTmp='\0';
	}
	/* parse the string and get value */
	sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);
	lVal=atol(p);

	switch(lOpt){
	case COMM_SUBCMD_ISPVIDEORESOLUTION:
		if(!strcmp(p, "1080p")){
			lVal=1;
			lRet=0;
			sprintf(cINFO, "%schannel 0\n", pcINFO);
		} else if (!strcmp(p, "720p")){
			lVal=1;
			lRet=1;
			sprintf(cINFO, "%schannel 1\n", pcINFO);
		} else if (!strcmp(p, "480p")){
			lVal=1;
			lRet=2;
			sprintf(cINFO, "%schannel 2\n", pcINFO);
		} else {
			sprintf(cINFO, "%scommand parameter value is not support !\n", pcINFO);
			return CTRL_CMDPARAVALNOTSUPPORT;
		}
		break;
	case COMM_SUBCMD_ISPVIDEOTERM: 
	case COMM_SUBCMD_ISPVIDEOGAMMA:
			lVal=1;
		break;
	case COMM_SUBCMD_ISPVIDEOCONTRAST:
	case COMM_SUBCMD_ISPVIDEOBRIGHTNESS:
	case COMM_SUBCMD_ISPVIDEOCOLOR:
		if(lVal>0&&lVal<=255){
		} else {
			sprintf(cINFO, "%svalue must be in 0~255!\n", pcINFO);
			return CTRL_CMDPARAVALNOTSUPPORT;
		}	
	 	break;
	case COMM_SUBCMD_ISPVIDEOINITBAYER:
		switch ( lVal ){
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
				lRet=COMM_CMD_VALID;
				break;
			default:
				lRet=COMM_CMD_INVALIDVAL;
				break;
		}
		break;
	case COMM_SUBCMD_ISPVIDEOWDRPIXELMAX: 
	case COMM_SUBCMD_ISPVIDEOWDRRATIO:
 	case COMM_SUBCMD_ISPVIDEONRTHRESH: 
 	case COMM_SUBCMD_ISPVIDEOISREGADDR: 
 	case COMM_SUBCMD_ISPVIDEOISREGDATA: 
 	case COMM_SUBCMD_ISPVIDEOFPGAADDR: 
 	case COMM_SUBCMD_ISPVIDEOFPGADATA: 
	case COMM_SUBCMD_ISPVIDEOGETISREGADDR: 
	case COMM_SUBCMD_ISPVIDEOGETFPGAADDR:
	case COMM_SUBCMD_ISPVIDEOINITWIDTH:
	case COMM_SUBCMD_ISPVIDEOINITHEIGHT:
	case COMM_SUBCMD_ISPVIDEOINITFRAME:
		if(lVal>0&&lVal<=255){
		} else {
			sprintf(cINFO, "%svalue must be in 0~255 !\n", pcINFO);
			return CTRL_CMDPARAVALNOTSUPPORT;
		}
		break;
	case COMM_SUBCMD_ISPVIDEOGAMMAPRESET:
	case COMM_SUBCMD_ISPVIDEOSHARPNESS:
	case COMM_SUBCMD_ISPVIDEOSHADINGGAIN:
	case COMM_SUBCMD_ISPVIDEOSHADINGVARIANCE:
	case COMM_SUBCMD_ISPVIDEODRCLEVEL:
	case COMM_SUBCMD_ISPVIDEOANTIFLICKERFREQ:
	case COMM_SUBCMD_ISPVIDEOCRSH:
	case COMM_SUBCMD_ISPVIDEOCRSV:
	case COMM_SUBCMD_ISPVIDEOWDROFFSET:
		if(lVal>0&&lVal<=255){
		} else {
			sprintf(cINFO, "%svalue must be in 0~255 !\n", pcINFO);
			return CTRL_CMDPARAVALNOTSUPPORT;
		}	
	       break;
	case COMM_SUBCMD_ISPVIDEODDP:
	case COMM_SUBCMD_ISPVIDEODIS:
	case COMM_SUBCMD_ISPVIDEOAE:
	case COMM_SUBCMD_ISPVIDEOAF:
	case COMM_SUBCMD_ISPVIDEOAI:
	case COMM_SUBCMD_ISPVIDEOAWB:
	case COMM_SUBCMD_ISPVIDEOWDR:
	case COMM_SUBCMD_ISPVIDEOSHADING:
	case COMM_SUBCMD_ISPVIDEOANTIFLICKER:
		if(lVal==0||lVal==1){
		} else {
			sprintf(cINFO, "%svalue must be 0 or 1 !\n", pcINFO);
			return CTRL_CMDPARAVALNOTSUPPORT;
		}	
		break;
		
	default:
		break;
	}
	vCOMM_CMD_InitCmd(&sCmd, lCmd, lOpt, lVal); 
	vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
	if(!lFstFlag){
		/* not null, have value already */
		nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
		if(nChar>=COMM_CMDSMAXCHAR){
			debug_info("too large commands!\n");	
		}
	} else {	
		/* first time */
		strcpy(*ppcReq, pcOneCmd); 
	}
	return 0;
}

LONG lCTRL_TerminalReq_ParseSystemCmd(char* opt, char** ppcReq){
	LONG lRet=CTRL_CMDSUCCESS;
	CHAR* p;
	CHAR* pn;
	LONG lVal=0;
	LONG lIsEnd=0;

	/* for command string */
	sCOMM_CMD sCmd;
	CHAR cOneCmd[COMM_CMDMAXCHAREACH];
	CHAR cCmd[COMM_CMDSYSMAXCNT];
	CHAR* pcOneCmd=cOneCmd;
	CHAR* pcTmp;
	LONG nChar=0;
	LONG lFstFlag=1;

	/* for check */
	eCTRL_CMD_STATE eChkRet;
	sCTRL_CMD_CHECKSYS sChkSys;

	/* initial */
	memset(&sChkSys, 0, sizeof(sCTRL_CMD_CHECKSYS));

	/* start to parse option */
	p=opt;
	
	/* only support -q */
	 while (!lIsEnd){
		p++;

		if((pn=index(p,'-'))!=NULL){
			/* prepare for the string */
			*pn='\0';
		} else {
			/* indicate is End */
			lIsEnd=1;
		}

		if(*p=='q'){
			/** quit system **/
			sprintf(cINFO, "%s\tsub command < quit >", pcINFO);
			lVal=COMM_CMDVAL_DEFAULT;
			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_SYS, COMM_SUBCMD_SYSQUIT, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}
	
			/* toggle the global end flag */
			glEndFlag=1;
		} else if(*p=='t'&&*(p+1)=='s') {
			/** thread start **/
			sprintf(cINFO, "%s\tsub command < thread start >\n", pcINFO);

			/* update p, and skip the space */
			p++;		// skip 'r'
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_SYS, COMM_SUBCMD_THREADSTART, 1); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}
		} else if(*p=='s'&&*(p+1)=='t') {
			/** set time YYYY:MM:DD:HH:MM:SS **/
			sprintf(cINFO, "%s\tsub command < set time >\n", pcINFO);

			/* update p, and skip the space */
			p++;		// skip 'r'
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			COMM_MW_StrPool_WriteEntry(COMM_GET_STRPOOL(), p, &lVal, &mutStringPoolWt);
			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_SYS, COMM_SUBCMD_SETTIME, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}
		} else if(*p=='h'){
			/* -h : help */
			sprintf(cINFO, "%s\tsub command < show help infomation >\n", pcINFO);
			vCTRL_TerminalReq_ShowHelp();			
			return CTRL_CMDNONEEDFEEDBACK;		
		}else if(*p=='s'){
			/* -s : show information */
			sprintf(cINFO, "%s\tsub command < show infomation >", pcINFO);

			/* update p, and skip the space */
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);

			/* show infomation */
			sprintf(cCmd, "cat /proc/umap/%s", p);
			system(cCmd);

			return CTRL_CMDNONEEDFEEDBACK;

		} else {
			/* wrong parameter */
			sprintf(cINFO, "%sthe option %c, is not support\n", pcINFO, *p);
			return CTRL_CMDPARAVALNOTSUPPORT;
		}

		/* update p */
		p=pn;
		
	}

	 /* check whether the must parameter is already set */
	eChkRet=lCTRL_TerminalReq_CheckSys(&sChkSys);
	if(CTRL_CMDSUCCESS!=eChkRet){
			return eChkRet;
	}
	
	return lRet;
}

LONG lCTRL_TerminalReq_ParseRecCmd(char* opt, char** ppcReq){
	LONG lRet=CTRL_CMDPARAVALNOCH;
	CHAR* p;
	CHAR* pn;
	//CHAR* pcTmp;
	LONG lVal=0;
	LONG lIsEnd=0;

	/* for command string */
	sCOMM_CMD sCmd;
	CHAR cOneCmd[COMM_CMDMAXCHAREACH];
	CHAR* pcOneCmd=cOneCmd;
	LONG nChar=0;
	LONG lFstFlag=1;

	/* start to parse option */
	p=opt;
	
	/* only support -r and -d */
	 while (!lIsEnd){
		p++;

		if((pn=index(p,'-'))!=NULL){
			/* prepare for the string */
			*pn='\0';
		} else {
			/* indicate is End */
			lIsEnd=1;
		}

		if(*p=='s'&&*(p+1)=='n'){
			/** snap **/
			sprintf(cINFO, "%s\tsub command < snap >\n", pcINFO);

                        lVal=1;
			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_RECORD, COMM_SUBCMD_RECSNAP, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}
			if(lRet==CTRL_CMDPARAVALNOCH){
				lRet=CTRL_CMDSUCCESS;
			}
		} else if(*p=='s'&&*(p+1)=='r') {
			/** subcard record **/
			sprintf(cINFO, "%s\tsub command < subcard rec >\n", pcINFO);

			lVal=1;
			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_RECORD, COMM_SUBCMD_SUBCARDREC, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}
			if(lRet==CTRL_CMDPARAVALNOCH){
				lRet=CTRL_CMDSUCCESS;
			}
		} else if(*p=='a'&&*(p+1)=='s') {
			/** record resolution **/
			sprintf(cINFO, "%s\tsub command < rec audio stop >\n", pcINFO);

			lVal=1;
			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_RECORD, COMM_SUBCMD_RECAUDIOSTOPRESOLUTION, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}
			if(lRet==CTRL_CMDPARAVALNOCH){
				lRet=CTRL_CMDSUCCESS;
			}
		}else if(*p=='a'){
		
			/** record resolution **/
			sprintf(cINFO, "%s\tsub command < rec audio start >\n", pcINFO);

                        lVal=1;
			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_RECORD, COMM_SUBCMD_RECAUDIORESOLUTION, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}
			if(lRet==CTRL_CMDPARAVALNOCH){
				lRet=CTRL_CMDSUCCESS;
			}
		} else {
			/* wrong parameter */
			sprintf(cINFO, "%sthe option %c, is not support\n", pcINFO, *p);
			return CTRL_CMDPARAVALNOTSUPPORT;
		}

		/* update p */
		p=pn;
		
	}

	 if(CTRL_CMDPARAVALNOCH==lRet){
		 sprintf(cINFO, "%serror : No channel input now\n", pcINFO);
	 }
		 
	return lRet;
}

LONG lCTRL_TerminalReq_ParseVencCmd(char* opt, char** ppcReq){
	LONG lRet=CTRL_CMDPARAVALNOCH;
	CHAR* p;
	CHAR* pn;
	CHAR* pcTmp;
	LONG lVal=0;
	LONG lIsEnd=0;

	/* for command string */
	sCOMM_CMD sCmd;
	CHAR cOneCmd[COMM_CMDMAXCHAREACH];
	CHAR* pcOneCmd=cOneCmd;
	LONG nChar=0;
	LONG lFstFlag=1;

	/* for check */
	eCTRL_CMD_STATE eChkRet;
	sCTRL_CMD_CHECKVENC sChkVenc;

	/* initial */
	memset(&sChkVenc, 0, sizeof(sCTRL_CMD_CHECKVENC));

	/* start to parse option */
	p=opt;
	
	/* support -vf -vg */
	 while (!lIsEnd){
		p++;

		if((pn=index(p,'-'))!=NULL){
			/* prepare for the string */
			*pn='\0';
		} else {
			/* indicate is End */
			lIsEnd=1;
		}

		if(*p=='v'&&*(p+1)=='r'){
			/** video resolution **/
			sprintf(cINFO, "%s\tsub command < video resolution >", pcINFO);

			if(lRet!=CTRL_CMDPARAVALNOCH && lRet!=CTRL_CMDDEFAULTCH){
				sprintf(cINFO, "%smultiple channel set !\n", pcINFO);
				return CTRL_CMDPARAVALMULTIPLECH;
			}

			/* update p, and skip the space */
			p++;		// skip 'r'
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);
			if(!strcmp(p, "1080p")){
				lVal=1;
				lRet=0;
				sprintf(cINFO, "%schannel 0\n", pcINFO);
			} else if (!strcmp(p, "720p")){
				lVal=1;
				lRet=1;
				sprintf(cINFO, "%schannel 1\n", pcINFO);
			} else if (!strcmp(p, "480p")){
				lVal=1;
				lRet=2;
				sprintf(cINFO, "%schannel 2\n", pcINFO);
			} else {
				sprintf(cINFO, "%scommand parameter value is not support !\n", pcINFO);
				return CTRL_CMDPARAVALNOTSUPPORT;
			}

			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_VENC0, COMM_SUBCMD_VENCVIDEORESOLUTION, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}

			sChkVenc.lVideoModReq=1;
		}else if(*p=='v'&&*(p+1)=='f'){
			/** video framerate **/
			sprintf(cINFO, "%s\tsub command < video framerate >", pcINFO);

			/* update p, and skip the space */
			p++;		// skip 'r'
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);
			lVal=atol(p);
			if(lVal>0&&lVal<=60){
			} else {
				sprintf(cINFO, "%scommand parameter value is not support !\n", pcINFO);
				return CTRL_CMDPARAVALNOTSUPPORT;
			}

			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_VENC0, COMM_SUBCMD_VENCVIDEOFRAMERATE, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}

			sChkVenc.lVideoMod=1;
		}else if(*p=='v'&&*(p+1)=='g'){
			/** video gop **/
			sprintf(cINFO, "%s\tsub command < video gop >", pcINFO);

			/* update p, and skip the space */
			p++;		// skip 's'
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);
			lVal=atol(p);
			if(lVal<=0||lVal>30){
				sprintf(cINFO, "%scommand parameter value is not support !\n", pcINFO);
				return CTRL_CMDPARAVALNOTSUPPORT;
			}

			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_VENC0, COMM_SUBCMD_VENCVIDEOGOP, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}
			sChkVenc.lVideoMod=1;
			}else if(*p=='v'&&*(p+1)=='t'){
			/** video encoder type **/
			sprintf(cINFO, "%s\tsub command < video encoder type >", pcINFO);
			
			/* update p, and skip the space */
			p++;		// skip 'p'
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);
			if(!strcmp(p, "cbr")){
				lVal=0;
			} else if (!strcmp(p, "vbr")){
				lVal=1;
			} else if (!strcmp(p, "fixqp")){
				lVal=2;
			} else {
				sprintf(cINFO, "%scommand parameter value is not support !\n", pcINFO);
				return CTRL_CMDPARAVALNOTSUPPORT;
			}

			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_VENC0, COMM_SUBCMD_VENCVIDEOENCODERTYPE, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}
			sChkVenc.lVideoMod=1;
		}else if(*p=='a'&&*(p+1)=='p'){
			/** audio protocol **/
			sprintf(cINFO, "%s\tsub command < audio protocol >", pcINFO);
			
			/* update p, and skip the space */
			p++;		// skip 'p'
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);
			if(!strcmp(p, "ADPCMA")){
				lVal=COMM_AENC_ADPCMA;
			} else if (!strcmp(p, "G711A")){
				lVal=COMM_AENC_G711A;
			} else if (!strcmp(p, "G726")){
				lVal=COMM_AENC_G726;
			} else if (!strcmp(p, "G711U")){
				lVal=COMM_AENC_G711U;
			}else {
				sprintf(cINFO, "%scommand parameter value is not support !\n", pcINFO);
				return CTRL_CMDPARAVALNOTSUPPORT;
			}

			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_VENC0, COMM_SUBCMD_VENCAUDIOTYPE, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}
			/*audio only*/
			if(lRet==CTRL_CMDPARAVALNOCH){
				lRet=CTRL_CMDDEFAULTCH;
			}
		}else{
			/* wrong parameter */
			sprintf(cINFO, "%sthe option %c, is not support\n", pcINFO, *p);
			return CTRL_CMDPARAVALNOTSUPPORT;
		}

		/* update p */
		p=pn;
		
	}

	/* check whether the must parameter is already set */
	eChkRet=lCTRL_TerminalReq_CheckVenc(&sChkVenc);
	if(CTRL_CMDSUCCESS!=eChkRet){
			return eChkRet;
	}

	 if(CTRL_CMDPARAVALNOCH==lRet){
		 sprintf(cINFO, "%serror : No channel input now\n", pcINFO);
	 }

	return lRet;
}

LONG lCTRL_TerminalReq_ParseEthCmd(char* opt, char** ppcReq){
	LONG lRet=CTRL_CMDPARAVALNOCH;
	CHAR* p;
	CHAR* pn;
	CHAR* pcTmp;
	LONG lVal=0;
	LONG lIsEnd=0;

	/* for command string */
	sCOMM_CMD sCmd;
	CHAR cOneCmd[COMM_CMDMAXCHAREACH];
	CHAR* pcOneCmd=cOneCmd;
	LONG nChar=0;
	LONG lFstFlag=1;

	/* for check */
	eCTRL_CMD_STATE eChkRet;
	sCTRL_CMD_CHECKETH sChkEth;

	/* initial */
	memset(&sChkEth, 0, sizeof(sCTRL_CMD_CHECKETH));

	/* start to parse option */
	p=opt;
	
	/* support -vr -vs -i -p */
	 while (!lIsEnd){
		p++;

		if((pn=index(p,'-'))!=NULL){
			/* prepare for the string */
			*pn='\0';
		} else {
			/* indicate is End */
			lIsEnd=1;
		}

		if(*p=='v'&&*(p+1)=='r'){
			/** video resolution **/
			sprintf(cINFO, "%s\tsub command < video resolution >", pcINFO);

			if(lRet!=CTRL_CMDPARAVALNOCH){
				sprintf(cINFO, "%smultiple channel set !\n", pcINFO);
				return CTRL_CMDPARAVALMULTIPLECH;
			}

			/* update p, and skip the space */
			p++;		// skip 'r'
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);
			if(!strcmp(p, "1080p")){
				lVal=1;
				lRet=0;
				sprintf(cINFO, "%schannel 0\n", pcINFO);
			} else if (!strcmp(p, "720p")){
				lVal=2;
				lRet=1;
				sprintf(cINFO, "%schannel 1\n", pcINFO);
			} else if (!strcmp(p, "480p")){
				lVal=4;
				lRet=2;
				sprintf(cINFO, "%schannel 2\n", pcINFO);
			} else {
				sprintf(cINFO, "%scommand parameter value is not support !\n", pcINFO);
				return CTRL_CMDPARAVALNOTSUPPORT;
			}

			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_ETH, COMM_SUBCMD_ETHVIDEORESOLUTION, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}

			/* check init */
			sChkEth.lVideoUse=1;
		}else if(*p=='a'&&*(p+1)=='s'){
			/** audio stop display resolution **/
			sprintf(cINFO, "%s\tsub command < audio stop play >", pcINFO);

			/* update p, and skip the space */
			p++;		// skip 's'
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);
			lRet=CTRL_CMDDEFAULTCH;
			lVal=0;

			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_ETH, COMM_SUBCMD_ETHAUDIOSTOP, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}

			/* check init */
			sChkEth.lAudioUse=1;
		}else if(*p=='v'&&*(p+1)=='s'){
			/** video stop display resolution **/
			sprintf(cINFO, "%s\tsub command < video stop display resolution >", pcINFO);

			if(lRet!=CTRL_CMDPARAVALNOCH){
				sprintf(cINFO, "%smultiple channel set !\n", pcINFO);
				return CTRL_CMDPARAVALMULTIPLECH;
			}

			/* update p, and skip the space */
			p++;		// skip 's'
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);
			if(!strcmp(p, "1080p")){
				lVal=1;
				lRet=0;
				sprintf(cINFO, "%schannel 0\n", pcINFO);
			} else if (!strcmp(p, "720p")){
				lVal=2;
				lRet=1;
				sprintf(cINFO, "%schannel 1\n", pcINFO);
			} else if (!strcmp(p, "480p")){
				lVal=4;
				lRet=2;
				sprintf(cINFO, "%schannel 2\n", pcINFO);
			} else {
				sprintf(cINFO, "%scommand parameter value is not support !\n", pcINFO);
				return CTRL_CMDPARAVALNOTSUPPORT;
			}

			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_ETH, COMM_SUBCMD_ETHSTOPRESOLUTION, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}

			/* check init */
			sChkEth.lVideoUse=1;
		}else if(!strncmp(p, "ap", 2)){
			/* port */
			sprintf(cINFO, "%s\tsub command < audio port >", pcINFO);

			/* update p, and skip the space */
			p++;
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);

			lVal=atol(p);
			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_ETH, COMM_SUBCMD_ETHAUDIOPORT, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}

			/* check init */
			sChkEth.lAudioPort=1;

		} else if(*p=='p') {
			/* port */
			sprintf(cINFO, "%s\tsub command < port >", pcINFO);

			/* update p, and skip the space */
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);

			lVal=atol(p);
			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_ETH, COMM_SUBCMD_ETHVIDEOPORT, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}

			/* check init */
			sChkEth.lVideoPort=1;
			
		}else if(!strncmp(p, "ai", 2)){
			/* audio ip address */
			sprintf(cINFO, "%s\tsub command < audio ip address >", pcINFO);

			/* update p, and skip the space */
			p++;
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);

			/* transform the ip address to long int */
			lVal=inet_addr(p);
			if(lVal==-1){
				/* indicate wrong ip address */
				sprintf(cINFO, "%s\nipaddress wrong\n", pcINFO);
				return CTRL_CMDPARAVALNOTSUPPORT;
			}

			/* correct value */
			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_ETH, COMM_SUBCMD_ETHAUDIOIPADDRESS, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}

			/* check init */
			sChkEth.lAudioIp=1;
		}else if(*p=='a'){
			/* audio start */
			sprintf(cINFO, "%s\tsub command < audio start >", pcINFO);

			/* update p, and skip the space */
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);

			/* the return value and channel */
			lVal=1;
			lRet=CTRL_CMDDEFAULTCH;

			/* correct value */
			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_ETH, COMM_SUBCMD_ETHAUDIOSTART, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}

			/* check init */
			sChkEth.lAudioUse=1;
			
		} else if(*p=='i') {
			/* ip address */
			sprintf(cINFO, "%s\tsub command < ip address >", pcINFO);

			/* update p, and skip the space */
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);

			/* transform the ip address to long int */
			lVal=inet_addr(p);
			if(lVal==-1){
				/* indicate wrong ip address */
				sprintf(cINFO, "%s\nipaddress wrong\n", pcINFO);
				return CTRL_CMDPARAVALNOTSUPPORT;
			}

			/* correct value */
			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_ETH, COMM_SUBCMD_ETHVIDEOIPADDRESS, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}

			/* check init */
			sChkEth.lVideoIp=1;
			
		} else {
			/* wrong parameter */
			sprintf(cINFO, "%sthe option %c, is not support\n", pcINFO, *p);
			return CTRL_CMDPARAVALNOTSUPPORT;
		}

		/* update p */
		p=pn;
		
	}

	/* check whether the must parameter is already set */
	eChkRet=lCTRL_TerminalReq_CheckEth(&sChkEth);
	if(CTRL_CMDSUCCESS!=eChkRet){
		return eChkRet;
	}
	
	return lRet;
}

LONG lCTRL_TerminalReq_ParseIspCmd(char* opt, char** ppcReq){
	LONG lRet;
	CHAR* p;
	CHAR* pn;
	LONG lIsEnd=0;
	LONG lFlag=1;
	/* start to parse option */
	p=opt;
	
	/* support -vf -vg */
	 while (!lIsEnd){
		p++;

		if((pn=index(p,'-'))!=NULL){
			/* prepare for the string */
			*pn='\0';
		} else {
			/* indicate is End */
			lIsEnd=1;
		}

		if(*p=='v'&&*(p+1)=='r'){
			/** video resolution **/
			sprintf(cINFO, "%s\tsub command < video resolution >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEORESOLUTION,lFlag,ppcReq);
			if(lRet==0){
				printf("video resolution success!\n");
			}
		}else if(*p=='c'&&*(p+1)=='t'){
			/** video contrast**/
			sprintf(cINFO, "%s\tsub command < video contrast >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOCONTRAST,lFlag,ppcReq);
			if(lRet==0){
				printf("video contrast set success!\n");
			}
		}else if(*p=='b'){
			/** video brightness**/
			sprintf(cINFO, "%s\tsub command < video brightness >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOBRIGHTNESS,lFlag,ppcReq);
			if(lRet==0){
				printf("video brightness set success!\n");
			}
		}else if(*p=='c'&&*(p+1)=='l'){
			/** video color**/
			sprintf(cINFO, "%s\tsub command < video color >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOCOLOR,lFlag,ppcReq);
			if(lRet==0){
				printf("video color set success!\n");
			}
		}else if(*p=='s'&&*(p+1)=='n'){
			/** video sharpness**/
			sprintf(cINFO, "%s\tsub command < video sharpness >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOSHARPNESS,lFlag,ppcReq);
			if(lRet==0){
				printf("video sharpness set success!\n");
			}
		}else if(*p=='d'&&*(p+1)=='d'){
			/** video brightness**/
			sprintf(cINFO, "%s\tsub command < video DDP >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEODDP,lFlag,ppcReq);
			if(lRet==0){
				printf("video DDP set success!\n");
			}
		}else if(*p=='d'&&*(p+1)=='i'){
			/** video brightness**/
			sprintf(cINFO, "%s\tsub command < video DIS >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEODIS,lFlag,ppcReq);
			if(lRet==0){
				printf("video DIS set success!\n");
			}
		}else if(*p=='a'&&*(p+1)=='w'){
			/** video brightness**/
			sprintf(cINFO, "%s\tsub command < video AWB >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOAWB,lFlag,ppcReq);
			if(lRet==0){
				printf("video AWB set success!\n");
			}
		}else if(*p=='a'&&*(p+1)=='e'){
			/** video brightness**/
			sprintf(cINFO, "%s\tsub command < video AE >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOAE,lFlag,ppcReq);
			if(lRet==0){
				printf("video AE set success!\n");
			}
		}else if(*p=='a'&&*(p+1)=='f'){
			/** video brightness**/
			sprintf(cINFO, "%s\tsub command < video AF >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOAF,lFlag,ppcReq);
			if(lRet==0){
				printf("video AF set success!\n");
			}
		}else if(*p=='a'&&*(p+1)=='i'){
			/** video brightness**/
			sprintf(cINFO, "%s\tsub command < video AI >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOAI,lFlag,ppcReq);
			if(lRet==0){
				printf("video AI set success!\n");
			}
		}else if(*p=='g'&&*(p+1)=='p'){
			/** video gamma preset**/
			sprintf(cINFO, "%s\tsub command < video gamma preset >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOGAMMAPRESET,lFlag,ppcReq);
			if(lRet==0){
				printf("video gamma preset set success!\n");
			}
		}else if(*p=='t'){
			/** video term**/
			sprintf(cINFO, "%s\tsub command < video term >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOTERM,lFlag,ppcReq);
			if(lRet==0){
				printf("video term success!\n");
			}
		}else if(*p=='n'&&*(p+1)=='t'){
			/** video NR thresh**/
			sprintf(cINFO, "%s\tsub command < video NR thresh>", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEONRTHRESH,lFlag,ppcReq);
			if(lRet==0){
				printf("video NR thresh set success!\n");
			}
		}else if(*p=='n'){
			/** video NR**/
			sprintf(cINFO, "%s\tsub command < video NR >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEONR,lFlag,ppcReq);
			if(lRet==0){
				printf("video NR success!\n");
			}
		}else if(*p=='s'&&*(p+1)=='d'&&*(p+2)=='g'){
			/** video shading gain**/
			sprintf(cINFO, "%s\tsub command < video shading gain>", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOSHADINGGAIN,lFlag,ppcReq);
			if(lRet==0){
				printf("video shading gain set  success!\n");
			}
		}else if(*p=='s'&&*(p+1)=='d'&&*(p+2)=='v'){
			/** video shading variance**/
			sprintf(cINFO, "%s\tsub command < video shading variance>", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOSHADINGVARIANCE,lFlag,ppcReq);
			if(lRet==0){
				printf("video shading  variance set success!\n");
			}
		}else if(*p=='s'&&*(p+1)=='d'){
			/** video shading**/
			sprintf(cINFO, "%s\tsub command < video shading >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOSHADING,lFlag,ppcReq);
			if(lRet==0){
				printf("video shading  success!\n");
			}
		}else if(*p=='d'&&*(p+1)=='r'&&*(p+2)=='l'){
			/** video shading level**/
			sprintf(cINFO, "%s\tsub command < video shading level >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEODRCLEVEL,lFlag,ppcReq);
			if(lRet==0){
				printf("video DRC level set success!\n");
			}
		}else if(*p=='d'&&*(p+1)=='r'){
			/** video DRC**/
			sprintf(cINFO, "%s\tsub command < video DRC >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEODRC,lFlag,ppcReq);
			if(lRet==0){
				printf("video DRC success!\n");
			}
		}else if(*p=='w'&&*(p+1)=='o'){
			/** video WRC offset**/
			sprintf(cINFO, "%s\tsub command < video WRC offset >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOWDROFFSET,lFlag,ppcReq);
			if(lRet==0){
				printf("video WRC offset set success!\n");
			}
		}else if(*p=='w'&&*(p+1)=='p'){
			/** video WRC pixelmax**/
			sprintf(cINFO, "%s\tsub command < video WRC pixelmax >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOWDRPIXELMAX,lFlag,ppcReq);
			if(lRet==0){
				printf("video WRC pixelmax set success!\n");
			}
		}else if(*p=='w'&&*(p+1)=='r'){
			/** video WRC ratio**/
			sprintf(cINFO, "%s\tsub command < video WRC ratio >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOWDRRATIO,lFlag,ppcReq);
			if(lRet==0){
				printf("video WRC ratio set success!\n");
			}
		}else if(*p=='w'){
			/** video WDR**/
			sprintf(cINFO, "%s\tsub command < video WDR >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOWDR,lFlag,ppcReq);
			if(lRet==0){
				printf("video WDR success!\n");
			}
		}else if(*p=='f'&&*(p+1)=='f'){
			/** video antiflicker freq**/
			sprintf(cINFO, "%s\tsub command < video antiflicker freq >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOANTIFLICKERFREQ,lFlag,ppcReq);
			if(lRet==0){
				printf("video antiflicker freq set success!\n");
			}
		}else if(*p=='c'&&*(p+1)=='r'&&*(p+2)=='h'){
			/** video CRS h_strength**/
			sprintf(cINFO, "%s\tsub command < video CRS h_strength >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOCRSH,lFlag,ppcReq);
			if(lRet==0){
				printf("video CRS h_strength set success!\n");
			}
		}else if(*p=='c'&&*(p+1)=='r'&&*(p+2)=='v'){
			/** video CRS v_strength**/
			sprintf(cINFO, "%s\tsub command < video CRS v_strength >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOCRSV,lFlag,ppcReq);
			if(lRet==0){
				printf("video CRS v_strength set success!\n");
			}
		}else if(*p=='i'&&*(p+1)=='r'&&*(p+2)=='a'){
			/** video isreg addr**/
			sprintf(cINFO, "%s\tsub command < video isreg addr >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOISREGADDR,lFlag,ppcReq);
			if(lRet==0){
				printf("video isreg addr set success!\n");
			}
		}else if(*p=='i'&&*(p+1)=='r'&&*(p+2)=='d'){
			/** video isreg data**/
			sprintf(cINFO, "%s\tsub command < video isreg data >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOISREGDATA,lFlag,ppcReq);
			if(lRet==0){
				printf("video isreg data set success!\n");
			}
		}else if(*p=='f'&&*(p+1)=='r'&&*(p+2)=='a'){
			/** video fpga addr**/
			sprintf(cINFO, "%s\tsub command < video fpga addr >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOFPGAADDR,lFlag,ppcReq);
			if(lRet==0){
				printf("video fpga addr set success!\n");
			}
		}else if(*p=='f'&&*(p+1)=='r'&&*(p+2)=='d'){
			/** video fpga data**/
			sprintf(cINFO, "%s\tsub command < video fpga data >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOFPGADATA,lFlag,ppcReq);
			if(lRet==0){
				printf("video fpga data set success!\n");
			}
		}else if(*p=='i'&&*(p+1)=='w'){
			/** video active width**/
			sprintf(cINFO, "%s\tsub command < video active width >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOINITWIDTH,lFlag,ppcReq);
			if(lRet==0){
				printf("video active width init success!\n");
			}
		}else if(*p=='i'&&*(p+1)=='h'){
			/** video active height**/
			sprintf(cINFO, "%s\tsub command < video active height>", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOINITHEIGHT,lFlag,ppcReq);
			if(lRet==0){
				printf("video active height init success!\n");
			}
		}else if(*p=='i'&&*(p+1)=='f'){
			/** video frame rate**/
			sprintf(cINFO, "%s\tsub command < video frame rate >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOINITFRAME,lFlag,ppcReq);
			if(lRet==0){
				printf("video frame rate init success!\n");
			}
		}else if(*p=='i'&&*(p+1)=='b'){
			/** video bayer**/
			sprintf(cINFO, "%s\tsub command < video bayer >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOINITBAYER,lFlag,ppcReq);
			if(lRet==0){
				printf("video bayer init success!\n");
			}
		}else if(*p=='g'&&*(p+1)=='i'&&*(p+2)=='a'){
			/** video isreg get data**/
			sprintf(cINFO, "%s\tsub command < video isreg get data >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOGETISREGADDR,lFlag,ppcReq);
			if(lRet==0){
				printf("video isreg get data success!\n");
			}
		}else if(*p=='g'&&*(p+1)=='f'&&*(p+2)=='a'){
			/** video fpga get data**/
			sprintf(cINFO, "%s\tsub command < video fpga get data >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOGETFPGAADDR,lFlag,ppcReq);
			if(lRet==0){
				printf("video fpga get datat success!\n");
			}
		}else if(*p=='g'){
			/** video gamma**/
			sprintf(cINFO, "%s\tsub command < video gamma >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOGAMMA,lFlag,ppcReq);
			if(lRet==0){
				printf("video gamma success!\n");
			}
		}else if(*p=='f'){
			/** video antiflicker**/
			sprintf(cINFO, "%s\tsub command < video antiflicker >", pcINFO);
			lRet=lCTRL_TerminalReq_IspCmdHandle(p,COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOANTIFLICKER,lFlag,ppcReq);
			if(lRet==0){
				printf("video antiflicker success!\n");
			}
		}
		else{
			/* wrong parameter */
			sprintf(cINFO, "%sthe option %c, is not support\n", pcINFO, *p);
			return CTRL_CMDPARAVALNOTSUPPORT;
		}
		lFlag=0;
		/* update p */
		p=pn;
	 	}
		 if(CTRL_CMDPARAVALNOCH==lRet){
		 sprintf(cINFO, "%serror : No channel input now\n", pcINFO);
		 }
	return lRet;
}

LONG lCTRL_TerminalReq_ParseRegCmd(char* opt, char** ppcReq){
	LONG lRet=CTRL_CMDPARAVALNOCH;
	CHAR* p;
	CHAR* pn;
	CHAR* pcTmp;
	LONG lVal=0;
	LONG lIsEnd=0;

	/* for command string */
	sCOMM_CMD sCmd;
	CHAR cOneCmd[COMM_CMDMAXCHAREACH];
	CHAR* pcOneCmd=cOneCmd;
	LONG nChar=0;
	LONG lFstFlag=1;

	/* for check */
	eCTRL_CMD_STATE eChkRet;
	sCTRL_CMD_CHECKREG sChkReg;

	/* initial */
	memset(&sChkReg, 0, sizeof(sCTRL_CMD_CHECKREG));

	/* start to parse option */
	p=opt;
	
	/* support -vf -vg */
	 while (!lIsEnd){
		p++;

		if((pn=index(p,'-'))!=NULL){
			/* prepare for the string */
			*pn='\0';
		} else {
			/* indicate is End */
			lIsEnd=1;
		}

		if(*p=='o'&&*(p+1)=='r'){
			/** overlay resolution **/
			sprintf(cINFO, "%s\tsub command < overlay resolution >", pcINFO);

			if(lRet!=CTRL_CMDPARAVALNOCH){
				sprintf(cINFO, "%smultiple channel set !\n", pcINFO);
				return CTRL_CMDPARAVALMULTIPLECH;
			}

			/* update p, and skip the space */
			p++;		// skip 'r'
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);
			if(!strcmp(p, "1080p")){
				lVal=COMM_BIT(0);
				lRet=0;
				sprintf(cINFO, "%schannel 0\n", pcINFO);
			} else if (!strcmp(p, "720p")){
				lVal=COMM_BIT(1);
				lRet=1;
				sprintf(cINFO, "%schannel 1\n", pcINFO);
			} else if (!strcmp(p, "480p")){
				lVal=COMM_BIT(2);
				lRet=2;
				sprintf(cINFO, "%schannel 2\n", pcINFO);
			} else {
				sprintf(cINFO, "%scommand parameter value is not support !\n", pcINFO);
				return CTRL_CMDPARAVALNOTSUPPORT;
			}

			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_REG, COMM_SUBCMD_OVERLAYRESOLUTION, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}
			sChkReg.lCategory|=COMM_BIT(CTRL_CMD_REGION_OVERLAY);
			sChkReg.lOverlayResolution=1;
		}else if(*p=='s'&&*(p+1)=='t'){
			/** overlay change attr **/
			sprintf(cINFO, "%s\tsub command < overlay change attr >\n", pcINFO);

			/* update p, and skip the space */
			p++;		// skip 'c'
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);
			if(!strcmp(p, "pic")){
				lVal=0;
			} else if (!strcmp(p, "str")){
				lVal=1;
			} else if (!strcmp(p, "time")){
				lVal=2;
			}else{
				printf("the input type %s is not support!\n",p);
				return CTRL_CMDPARAVALNOTSUPPORT;
			} 
			
			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_REG, COMM_SUBCMD_OVERLAYSHOWTYPE, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}
			//sChkReg.lCategory|=COMM_BIT(CTRL_CMD_REGION_OVERLAY);
			//sChkReg.lOverlayStart=1;
		}else if(*p=='o'&&*(p+1)=='c'){
			/** overlay change attr **/
			sprintf(cINFO, "%s\tsub command < overlay change attr >\n", pcINFO);

			/* update p, and skip the space */
			p++;		// skip 'c'
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			lVal=1;
			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_REG, COMM_SUBCMD_OVERLAYCHANGEATTR, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}
			sChkReg.lCategory|=COMM_BIT(CTRL_CMD_REGION_OVERLAY);

		}else if(*p=='o'&&*(p+1)=='n'){
			/** overlay region num **/
			sprintf(cINFO, "%s\tsub command < overlay region num >\n", pcINFO);

			/* update p, and skip the space */
			p++;		// skip 'b'
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			lVal=atol(p);
			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_REG, COMM_SUBCMD_OVERLAYNUM, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}
			sChkReg.lCategory|=COMM_BIT(CTRL_CMD_REGION_OVERLAY);
			sChkReg.lOverlayStart=1;
		}else if(*p=='o'&&*(p+1)=='e'){
			/** overlay stop **/
			sprintf(cINFO, "%s\tsub command < overlay stop >\n", pcINFO);

			/* update p, and skip the space */
			p++;		// skip 'e'
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			lVal=1;
			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_REG, COMM_SUBCMD_OVERLAYSTOP, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}
			sChkReg.lCategory|=COMM_BIT(CTRL_CMD_REGION_OVERLAY);
			sChkReg.lOverlayStop=1;
		}else if(*p=='o'&&*(p+1)=='x'){
			/** overlay rec startX **/
			sprintf(cINFO, "%s\tsub command < overlay rec startX >", pcINFO);

			/* update p, and skip the space */
			p++;		// skip 'x'
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);
			lVal=atol(p);

			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_REG, COMM_SUBCMD_OVERLAYX, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}
			sChkReg.lCategory|=COMM_BIT(CTRL_CMD_REGION_OVERLAY);
			sChkReg.lOverlayChange = 1;
			sChkReg.lOverlayRecX=1;
		}else if(*p=='o'&&*(p+1)=='y'){
			/** overlay rec startY **/
			sprintf(cINFO, "%s\tsub command < overlay rec startY >", pcINFO);

			/* update p, and skip the space */
			p++;		// skip 'y'
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);
			lVal=atol(p);

			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_REG, COMM_SUBCMD_OVERLAYY, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}
			sChkReg.lCategory|=COMM_BIT(CTRL_CMD_REGION_OVERLAY);
			sChkReg.lOverlayChange = 1;
			sChkReg.lOverlayRecY=1;
		}else if(*p=='f'&&*(p+1)=='a'){
			/** cover rec startX **/
			sprintf(cINFO, "%s\tsub command < overlay front alpha >", pcINFO);

			/* update p, and skip the space */
			p++;		// skip 'f'
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);
			lVal=atol(p);
			if(lVal<0||lVal>255){
				sprintf(cINFO, "%scommand parameter value is not support !\n", pcINFO);
				return CTRL_CMDPARAVALNOTSUPPORT;
			}

			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_REG, COMM_SUBCMD_OVERLAYFRONTALPHA, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}
			sChkReg.lCategory|=COMM_BIT(CTRL_CMD_REGION_OVERLAY);
			sChkReg.lOverlayChange = 1;
			sChkReg.lOverlayFAlpha=1;
			}else if(*p=='b'&&*(p+1)=='a'){
			/** cover rec startX **/
			sprintf(cINFO, "%s\tsub command < overlay front alpha >", pcINFO);

			/* update p, and skip the space */
			p++;		// skip 'f'
			while(*++p==' ');

			/* strip the string */
			if((pcTmp=index(p,' '))!=NULL){
				/* prepare for the string */
				*pcTmp='\0';
			}

			/* parse the string and get value */
			sprintf(cINFO, "%s\tval < %s >\n", pcINFO, p);
			lVal=atol(p);
			if(lVal<0||lVal>255){
				sprintf(cINFO, "%scommand parameter value is not support !\n", pcINFO);
				return CTRL_CMDPARAVALNOTSUPPORT;
			}

			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_REG, COMM_SUBCMD_OVERLAYBOTTOMALPHA, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}
			sChkReg.lCategory|=COMM_BIT(CTRL_CMD_REGION_OVERLAY);
			sChkReg.lOverlayChange = 1;
		}else if(*p=='o'&&*(p+1)=='s'){
			/** overlay begion **/
			sprintf(cINFO, "%s\tsub command < overlay begion >\n", pcINFO);

			/* update p, and skip the space */
			p++;		// skip 's'
			while(*++p==' ');

			/* parse the string and get value */
			COMM_MW_StrPool_WriteEntry(COMM_GET_STRPOOL(), p, &lVal, &mutStringPoolWt);
			vCOMM_CMD_InitCmd(&sCmd, COMM_CMD_REG, COMM_SUBCMD_OVERLAYSHOWBMP, lVal); 
			vCOMM_CMD_CmdToChar(&sCmd, &pcOneCmd);	
			if(!lFstFlag){
				/* not null, have value already */
				nChar=snprintf(*ppcReq, COMM_CMDSMAXCHAR, "%s,%s", *ppcReq, pcOneCmd);
				if(nChar>=COMM_CMDSMAXCHAR){
					debug_info("too large commands!\n");	
				}
			} else {	
				/* toggle the first flag */ 
				lFstFlag=0; 
				/* first time */
				strcpy(*ppcReq, pcOneCmd); 
			}
			sChkReg.lCategory|=COMM_BIT(CTRL_CMD_REGION_OVERLAY);
			sChkReg.lOverlayChange = 1;
			sChkReg.lOverlayShowBmp=1;
		}else{
			/* wrong parameter */
			sprintf(cINFO, "%sthe option %c, is not support\n", pcINFO, *p);
			return CTRL_CMDPARAVALNOTSUPPORT;
		}

		/* update p */
		p=pn;
		
	}

	/* check whether the must parameter is already set */
	eChkRet=lCTRL_TerminalReq_CheckReg(&sChkReg);
	if(CTRL_CMDSUCCESS!=eChkRet){
			return eChkRet;
	}

	 if(CTRL_CMDPARAVALNOCH==lRet){
		 sprintf(cINFO, "%serror : No channel input now\n", pcINFO);
	 }

	return lRet;
}

LONG lCTRL_TerminalReq_GetDebugCmd(char* pcCmd){
	CHAR cTargetCmd[CTRL_CMDMAXALLCHAR];

	/* clear target command */
	memset(cTargetCmd, 0, sizeof(cTargetCmd));

	/* make target command */
	if(COMM_STRING_EQUAL == strcmp(pcCmd, "q")) {
		strcpy(cTargetCmd, "system -q");
	} else if(COMM_STRING_EQUAL == strcmp(pcCmd, "sr")) {
		strcpy(cTargetCmd, "rec -sr");
	} else {
		/* not for debug command */
		return 1;
	}

	/* make the target command to pcCmd */
	strcpy(pcCmd, cTargetCmd);

	return 0;
}

eCTRL_CMD_STATE lCTRL_TerminalReq_CheckSys(sCTRL_CMD_CHECKSYS* psChkSys){
	eCTRL_CMD_STATE eRet=CTRL_CMDSUCCESS;

	/* check for only input memory length without memory address */
	if(psChkSys->lMemLen && (!psChkSys->lMem)){
		sprintf(cINFO, "%serror : please input memory address\n", pcINFO);
		return CTRL_CMDINFONOTFULL;
	}

	/* check for only input register length without register address */
	if(psChkSys->lRegLen && (!psChkSys->lReg)){
		sprintf(cINFO, "%serror : please input register address\n", pcINFO);
		return CTRL_CMDINFONOTFULL;
	}

	/* check for only input one of the memory write address and write value */
	if(psChkSys->lMemWt^psChkSys->lMemWtVal){
		sprintf(cINFO, "%serror : please input both memory and value \n", pcINFO);
		return CTRL_CMDINFONOTFULL;
	}

	/* check for only input one of the register write address and write value */
	if(psChkSys->lRegWt^psChkSys->lRegWtVal){
		sprintf(cINFO, "%serror : please input both register and value \n", pcINFO);
		return CTRL_CMDINFONOTFULL;
	}

	/* check for only input one of raw vi channel and raw vi frame count */
	if(psChkSys->lRawViCh^psChkSys->lRawViCnt){
		sprintf(cINFO, "%serror : please input both vi raw ch and vi raw frame count \n", pcINFO);
		return CTRL_CMDINFONOTFULL;
	}

	return eRet;
}

eCTRL_CMD_STATE lCTRL_TerminalReq_CheckEth(sCTRL_CMD_CHECKETH* psChkEth){
	eCTRL_CMD_STATE eRet=CTRL_CMDSUCCESS;

	if(psChkEth->lAudioUse){
		if(!(psChkEth->lAudioIp & psChkEth->lAudioPort)){
			sprintf(cINFO, "%serror : [ audio ] should input both ip and port\n", pcINFO);
			return CTRL_CMDINFONOTFULL;
		}
	}

	if(psChkEth->lVideoUse){
		if(!(psChkEth->lVideoIp & psChkEth->lVideoPort)){
			sprintf(cINFO, "%serror : [ video ] should input both ip and port\n", pcINFO);
			return CTRL_CMDINFONOTFULL;
		}
	}

	return eRet;

}

eCTRL_CMD_STATE lCTRL_TerminalReq_CheckHdmi(sCTRL_CMD_CHECKHDMI* psChkHdmi){
	eCTRL_CMD_STATE eRet=CTRL_CMDINFONOTFULL;

	/* check whether have modifed request and modified */
	if((psChkHdmi->lHdmiWidth == 0) && (psChkHdmi->lHdmiHeight == 0) && (psChkHdmi->lPointX == 0) && (psChkHdmi->lPointY == 0) && (psChkHdmi->lVencPicSize == 0) && (psChkHdmi->lVoResolutio == 1)){
		return CTRL_CMDSUCCESS;
	}else if((psChkHdmi->lHdmiWidth == 1) && (psChkHdmi->lHdmiHeight == 1) && (psChkHdmi->lPointX == 1) && (psChkHdmi->lPointY == 1) && (psChkHdmi->lVencPicSize == 1)){
		return CTRL_CMDSUCCESS;
	}

	return eRet;

}

eCTRL_CMD_STATE lCTRL_TerminalReq_CheckVenc(sCTRL_CMD_CHECKVENC* psChkVenc){
	eCTRL_CMD_STATE eRet=CTRL_CMDSUCCESS;

	/* check whether have modifed request and modified */
	if(psChkVenc->lVideoMod^psChkVenc->lVideoModReq){
		sprintf(cINFO, "%serror : [ venc ] should input both channel and modified content\n", pcINFO);
		return CTRL_CMDINFONOTFULL;
	}

	return eRet;

}

eCTRL_CMD_STATE lCTRL_TerminalReq_CheckReg(sCTRL_CMD_CHECKREG* psChkReg){
	eCTRL_CMD_STATE eRet=CTRL_CMDSUCCESS;

	/*cover*/
	if(COMM_BIT(CTRL_CMD_REGION_COVER)&psChkReg->lCategory){
		/* we must have resolution */
		if(!(psChkReg->lCovResolution)){
			return CTRL_CMDINFONOTFULL;
		}
		
		/* at least start or stop or change */
		if(!(psChkReg->lCovStart | psChkReg->lCovStop | psChkReg->lCovChange)){
			sprintf(cINFO, "%serror : [ cover ] should input cover start,stop,change color,change position \n", pcINFO);
			return CTRL_CMDINFONOTFULL;
		}

		if(psChkReg->lCovChange){
			if((!(psChkReg->lCovRecX & psChkReg->lCovRecY &psChkReg->lCovRecHeight
				& psChkReg->lCovRecWidth)) && (!(psChkReg->lCovColor))){
				sprintf(cINFO, "%serror : [ cover ] should input both cover resolution and start \n", pcINFO);
				return CTRL_CMDINFONOTFULL;
			}
		}
		psChkReg->lCategory&= (~ COMM_BIT(CTRL_CMD_REGION_COVER));
	}
	
	/*overlay*/
	if(COMM_BIT(CTRL_CMD_REGION_OVERLAY)&psChkReg->lCategory){
		/* we must have resolution */
		if(!(psChkReg->lOverlayResolution)){
			return CTRL_CMDINFONOTFULL;
		}
	
		/* at least start or stop or change */
		if(!(psChkReg->lOverlayStart| psChkReg->lOverlayStop| psChkReg->lOverlayChange)){
			sprintf(cINFO, "%serror : [ cover ] should input cover start,stop,change color,change position \n", pcINFO);
			return CTRL_CMDINFONOTFULL;
		}
	
		if(psChkReg->lCovChange){
			if((!(psChkReg->lOverlayRecX& psChkReg->lOverlayRecY)) &&(!(psChkReg->lOverlayWidth& psChkReg->lOverlayHeight))&& (!(psChkReg->lOverlayFAlpha))&& (!(psChkReg->lOverlayShowBmp))){
				sprintf(cINFO, "%serror : [ cover ] should input both cover resolution and start \n", pcINFO);
				return CTRL_CMDINFONOTFULL;
			}
		}
		psChkReg->lCategory&= (~ COMM_BIT(CTRL_CMD_REGION_OVERLAY));		
	}
	return eRet;

}
eCTRL_CMD_STATE lCTRL_TerminalReq_CheckVda(sCTRL_CMD_CHECKVDA* psChkVda){
	eCTRL_CMD_STATE eRet=CTRL_CMDSUCCESS;

	/* we must have resolution */
	if(!(psChkVda->lVdaResoution)){
		sprintf(cINFO, "%serror : [ vda ] should input resolution!\n", pcINFO);
		return CTRL_CMDINFONOTFULL;
	}

	/* at least start or stop or change */
	if(!(psChkVda->lVdaStart | psChkVda->lVdaStop)){
		sprintf(cINFO, "%serror : [ vda ] should input vda start,stop,\n", pcINFO);
		return CTRL_CMDINFONOTFULL;
	}

	if(psChkVda->lVdaStart &&(!(psChkVda->lVdaRecHeight & psChkVda->lVdaRecWidth & 
			psChkVda->lVdaRecX &psChkVda->lVdaRecY))){
		sprintf(cINFO, "%serror : [ vda ] should input vda start,x,y,height,width,\n", pcINFO);
		return CTRL_CMDINFONOTFULL;
	}
	return eRet;
}


#ifdef __cplusplus
}
#endif

