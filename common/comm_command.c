#ifdef __cplusplus
extern "C"{
#endif

#include "comm_command.h"


/* implements */
void vCOMM_CMD_InitCmd(sCOMM_CMD* pstCmd, LONG lCmd, LONG lOption, LONG lValue){
	/* init command struct */
	pstCmd->lCmd=lCmd;
	pstCmd->lOpt=lOption;
	pstCmd->lVal=lValue;
}

#ifdef SHOW_INFO
void vCOMM_CMD_ShowCmd(sCOMM_CMD* pstCmd){
	printf("cmd info : \n");
	printf("\tcmd\t%ld\n", pstCmd->lCmd);
	printf("\topt\t%ld\n", pstCmd->lOpt);
	printf("\tval\t%ld\n", pstCmd->lVal);
}
#endif /* SHOW_INFO */

void vCOMM_CMD_CmdToChar(sCOMM_CMD* pstCmd, CHAR** ppcCmd){
	LONG lChar=0;
	lChar=snprintf(*ppcCmd,COMM_CMDMAXCHAREACH, "%ld,%ld,%ld", pstCmd->lCmd, pstCmd->lOpt, pstCmd->lVal);
	if(lChar>=COMM_CMDMAXCHAREACH){
		debug_info("Too large command\n");
	}
}

void vCOMM_CMD_CharToCmd(sCOMM_CMD* pstCmd, CHAR* pcCmd){
	LONG lCmd;
	LONG lOpt;
	LONG lVal;

	/* get command, option, value from pcCmd */
	sscanf(pcCmd, "%ld,%ld,%ld", &lCmd, &lOpt, &lVal);

	/* construct ppstCmd */
	vCOMM_CMD_InitCmd(pstCmd, lCmd, lOpt, lVal);
}

LONG lCOMM_CMD_CheckCmds(CHAR* pcCmds){
	LONG lRet=COMM_CMD_INVALID;
	CHAR* pcLocalCmds;
	CHAR* pc;
	CHAR* pcNext;
	sCOMM_CMD stCmd;

	return COMM_CMD_VALID;

	/* first, make a copy of the cmds */
	pcLocalCmds=strdup(pcCmds);

	/* parse the commands one by one */
	pc=pcLocalCmds;
	while((pcNext=index(pc,'&'))!=NULL){
		*pcNext='\0';

		/* get command */
		vCOMM_CMD_CharToCmd(&stCmd, pc);

		/* check the command and check valid */
		lRet=lCOMM_CMD_CheckCmd(&stCmd);
		if( lRet != COMM_CMD_VALID){
			/* wrong command, give up the commands */
			break;
		}

		if(*++pcNext=='\0'){
			/* end of the commands */
			lRet=COMM_CMD_VALID;
			break;
		}

		/* update pc */
		pc=pcNext;
	}

	free( pcLocalCmds );
	return lRet;
}

LONG lCOMM_CMD_CheckCmd(sCOMM_CMD* pstCmd){

	LONG lRet=COMM_CMD_INVALID;
	//vCOMM_CMD_ShowCmd(pstCmd);
	switch(pstCmd->lCmd){
		case COMM_CMD_SYS:
			lRet=lCOMM_CMD_CheckCmdSystem(pstCmd);
			break;
		case COMM_CMD_VENC0:
		case COMM_CMD_VENC1:
		case COMM_CMD_VENC2:
			lRet=lCOMM_CMD_CheckCmdVenc(pstCmd);
			break;
		case COMM_CMD_ETH:
			lRet=lCOMM_CMD_CheckCmdEth(pstCmd);
			break;
		case COMM_CMD_ISP0:
		case COMM_CMD_ISP1:
		case COMM_CMD_ISP2:
			lRet=lCOMM_CMD_CheckCmdIsp(pstCmd);
			break;
		case COMM_CMD_RECORD:
			lRet=lCOMM_CMD_CheckCmdRec(pstCmd);
			break;
		case COMM_CMD_HDMI:
			lRet=lCOMM_CMD_CheckCmdHdmi(pstCmd);
			break;
		case COMM_CMD_VDA:
			lRet=lCOMM_CMD_CheckCmdVda(pstCmd);
			break;
		default:
			lRet=COMM_CMD_INVALIDCMD;
			break;
	};
	return lRet;
}
LONG lCOMM_CMD_CheckCmdVenc(sCOMM_CMD* pstCmd){
	LONG lRet=COMM_CMD_INVALID;
	LONG lVal=pstCmd->lVal;

	switch( pstCmd->lOpt ){
		case COMM_SUBCMD_VENCVIDEORESOLUTION :
			switch ( lVal ){
				case 0:
				case 1:
					lRet=COMM_CMD_VALID;
					break;
				default:
					lRet=COMM_CMD_INVALIDVAL;
					break;
			}
			break;
		case COMM_SUBCMD_VENCVIDEOENCODERTYPE : 
			lRet=COMM_CMD_VALID;
			break;
		case COMM_SUBCMD_VENCVIDEOBITRATE : 
			lRet=COMM_CMD_VALID;
			break; 
		case COMM_SUBCMD_VENCVIDEOFRAMERATE : 
			if(lVal>0&&lVal<=30){
				lRet=COMM_CMD_VALID;
			}
			break;
		case COMM_SUBCMD_VENCVIDEOGOP : 
			if(lVal>0&&lVal<=30){
				lRet=COMM_CMD_VALID;
			}
			break;
		case COMM_SUBCMD_VENCVIDEOCONTROLTYPE : 
			lRet=COMM_CMD_VALID;
			break;
		case COMM_SUBCMD_VENCVIDEOPROFILE : 
			lRet=COMM_CMD_VALID;
			break;
		case COMM_SUBCMD_VENCVIDEOCOVER : 
			lRet=COMM_CMD_VALID;
			break;
		case COMM_SUBCMD_VENCAUDIOTYPE : 
			lRet=COMM_CMD_VALID;
			break;
		default:
			lRet=COMM_CMD_INVALIDOPT;
			break;
	}
	
	return lRet;
}

LONG lCOMM_CMD_CheckCmdEth(sCOMM_CMD* pstCmd){
	LONG lRet=COMM_CMD_INVALID;
	LONG lVal=pstCmd->lVal;

	switch( pstCmd->lOpt ){
		case COMM_SUBCMD_ETHVIDEOIPADDRESS :
			lRet=COMM_CMD_VALID;
			break;
		case COMM_SUBCMD_ETHVIDEOPORT :
			lRet=COMM_CMD_VALID;
			break;
		case COMM_SUBCMD_ETHVIDEORESOLUTION :
			switch ( lVal ){
				case 1:
				case 2:
				case 4:
					lRet=COMM_CMD_VALID;
					break;
				default:
					lRet=COMM_CMD_INVALIDVAL;
					break;
			}
			break;
		case COMM_SUBCMD_ETHSTOPRESOLUTION :
				switch ( lVal ){
				case 1:
				case 2:
				case 4:
					lRet=COMM_CMD_VALID;
					break;
				default:
					lRet=COMM_CMD_INVALIDVAL;
					break;
			}
			break;
		default:
			lRet=COMM_CMD_INVALIDOPT;
			break;
	}
	
	return lRet;
}

LONG lCOMM_CMD_CheckCmdHdmi(sCOMM_CMD* pstCmd){
	LONG lRet=COMM_CMD_INVALID;
	//LONG lVal=pstCmd->lVal;

	switch( pstCmd->lOpt ){

		default:
			lRet=COMM_CMD_INVALIDOPT;
			break;
	}
	
	return lRet;
}

LONG lCOMM_CMD_CheckCmdVda(sCOMM_CMD* pstCmd){
	LONG lRet=COMM_CMD_INVALID;
	//LONG lVal=pstCmd->lVal;

	switch( pstCmd->lOpt ){

		default:
			lRet=COMM_CMD_INVALIDOPT;
			break;
	}
	
	return lRet;
}
LONG lCOMM_CMD_CheckCmdSystem(sCOMM_CMD* pstCmd){
	LONG lRet=COMM_CMD_INVALID;

	switch( pstCmd->lOpt ){
		case COMM_SUBCMD_SYSQUIT : 
			lRet=COMM_CMD_VALID;
			break;
		default:
			lRet=COMM_CMD_INVALIDOPT;
			break;
	}

	return lRet;
}
LONG lCOMM_CMD_CheckCmdIsp(sCOMM_CMD* pstCmd){
	LONG lRet=COMM_CMD_INVALID;
	LONG lVal=pstCmd->lVal;
	
	switch( pstCmd->lOpt ){
		case COMM_SUBCMD_ISPVIDEORESOLUTION :
			switch ( lVal ){
				case 0:
				case 1:
					lRet=COMM_CMD_VALID;
					break;
				default:
					lRet=COMM_CMD_INVALIDVAL;
					break;
			}
			break;
		case COMM_SUBCMD_ISPVIDEOTERM: 
		case COMM_SUBCMD_ISPVIDEOGAMMA:
				lRet=COMM_CMD_VALID;
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
		case COMM_SUBCMD_ISPVIDEOCONTRAST: 
		case COMM_SUBCMD_ISPVIDEOBRIGHTNESS:
		case COMM_SUBCMD_ISPVIDEOCOLOR: 
			if(lVal>=-128&&lVal<=127){
				lRet=COMM_CMD_VALID;
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
				lRet=COMM_CMD_VALID;
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
				lRet=COMM_CMD_VALID;
			}
			break;
		case COMM_SUBCMD_ISPVIDEODDP:
		case COMM_SUBCMD_ISPVIDEODIS:
		case COMM_SUBCMD_ISPVIDEOAWB:
		case COMM_SUBCMD_ISPVIDEOAE:
		case COMM_SUBCMD_ISPVIDEOAF:
		case COMM_SUBCMD_ISPVIDEOAI:
		case COMM_SUBCMD_ISPVIDEOSHADING:
		case COMM_SUBCMD_ISPVIDEODRC:
		case COMM_SUBCMD_ISPVIDEOANTIFLICKER:
		case COMM_SUBCMD_ISPVIDEONR:
		case COMM_SUBCMD_ISPVIDEOWDR:
			if(lVal==0||lVal==1){
				lRet=COMM_CMD_VALID;
			}
			break;
		default:
			lRet=COMM_CMD_INVALIDOPT;
			break;
	}

	return lRet;
}
LONG lCOMM_CMD_CheckCmdRec(sCOMM_CMD* pstCmd){
	LONG lRet=COMM_CMD_INVALID;
	//LONG lVal=pstCmd->lVal;

	switch( pstCmd->lOpt ){
		case COMM_SUBCMD_RECAUDIORESOLUTION: 
			lRet=COMM_CMD_VALID;
			break;
		case COMM_SUBCMD_RECAUDIOSTOPRESOLUTION: 
			lRet=COMM_CMD_VALID;
			break;
		case COMM_SUBCMD_RECSNAP:
			lRet=COMM_CMD_VALID;
			break;
		default:
			lRet=COMM_CMD_INVALIDOPT;
			break;
	}
	
	return lRet;
}

void vCOMM_CMD_AddCmdHeadTail(CHAR** ppcCmds, LONG lReqInfo){

	LONG lChar=0;
	CHAR cHead[COMM_CMDMAXCHAREACH];
	CHAR cTail[COMM_CMDMAXCHAREACH];

	CHAR cMidBuf[COMM_CMDSMAXCHAR];

	/* clear the buffer */
	memset(cHead, 0,sizeof(cHead));
	memset(cTail, 0,sizeof(cTail));

	/* save the command */	
	strcpy(cMidBuf,*ppcCmds);

	/* construct head node */
	lChar=snprintf(cHead, COMM_CMDMAXCHAREACH, "req,head,%ld", lReqInfo);
	if(lChar>=COMM_CMDMAXCHAREACH){
		debug_info("Too large command head\n");
	}

	/* construct tail node */
	lChar=snprintf(cTail, COMM_CMDMAXCHAREACH, "req,tail,%ld", lReqInfo);
	if(lChar>=COMM_CMDMAXCHAREACH){
		debug_info("Too large command tail\n");
	}

	/* construct the packet commands */
	memset(*ppcCmds, 0,sizeof(*ppcCmds));
	lChar=snprintf(*ppcCmds, COMM_CMDSMAXCHAR, "%s,%s,%s", cHead, cMidBuf, cTail);
		
	if(lChar>=COMM_CMDSMAXCHAR){
		debug_info("Too large commands\n");
	}
}

void vCOMM_CMD_Split(CHAR *pcCmd,CHAR *pcDelim, CHAR **ppcCharArray, LONG *plArraySize)
{
	/* the beginning of each sub array */
	CHAR* pcToken;
	LONG lCnt=0;

	pcToken=strtok(pcCmd,pcDelim);

	/* construct the output array */
	while(pcToken!=NULL){
		if( (lCnt+1) >= (COMM_CMD_ARRAYSIZE + 2)*3 ){
			debug_info("ERROR: commands length is bigger than COMM_PARAM_LENGTH\n");
			break;
		}
		ppcCharArray[lCnt++]=pcToken;	
		pcToken=strtok(NULL,pcDelim);
	}

	/* return the count */
	*plArraySize=lCnt;
}


void vCOMM_CMD_AddReqInfo(CHAR* pcReq, eCOMM_UI eUi) {
	LONG lReqInfo = 0;
	LONG lCh = 0;
	
	/* get lock and allocate reqid */
	pthread_mutex_lock(&sMutReqId);
	if(++glReqId == COMM_CMDMAXREQID)
		glReqId=0;

	/* prepare reqInfo : include reqId, ch, ui */
	lReqInfo = (COMM_SETREQINFO(glReqId, COMM_REQID) | \
				COMM_SETREQINFO(lCh, COMM_REQCH) | \
				COMM_SETREQINFO(eUi, COMM_REQUI));

	/* add tail and head */ 
	vCOMM_CMD_AddCmdHeadTail( &pcReq, lReqInfo);
	
	/* release the lock */
	pthread_mutex_unlock(&sMutReqId);
}


#ifdef __cplusplus
}
#endif


