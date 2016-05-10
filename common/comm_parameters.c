#include "comm_parameters.h"

#ifdef __cplusplus
extern "C"{
#endif




/* global parameters config filename */
static CHAR cFileName[]="para.cfg";

/* extern parameter list */
extern sCOMM_PARA gsParas[ ];

/* extern function */
extern LONG lCOMM_CMD_CheckCmd(sCOMM_CMD* pstCmd);
extern void vCOMM_CMD_InitCmd(sCOMM_CMD* pstCmd, LONG lCmd, LONG lOption, LONG lValue);

void vCOMM_PARA_InitParas( sCOMM_PARA** ppParas){
	/* return */
	LONG lRet=COMM_RET_SUCCESS;

	/* init mutex */
	pthread_mutex_init( &muxPara, NULL );

	/* parameter list */
	lRet=lCOMM_PARA_InitCfgParas(ppParas);
	if(lRet!=COMM_RET_SUCCESS){
		printf("wrong with config files, with use default parameters\n");
		vCOMM_PARA_InitDefaultParas(ppParas);
	}

	debug_info("the parameter is init\n");
}

LONG lCOMM_PARA_InitCfgParas( sCOMM_PARA** ppParas){
	/* return */
	LONG lRet=COMM_RET_SUCCESS;

	/* parameter list */
	sCOMM_PARA* psParas=gsParas;
	
	/* buffer to hold file content */
	CHAR cContent[COMM_CMDMAXCMTCHAR];
	CHAR* pcCt=cContent;
	CHAR* pcRes;

	/* config file */
	FILE* pfParaCfg=NULL;

	/* parameter for each handle */
	LONG lParaId;
	LONG lParaVal;
	sCOMM_PARA sPara;

	/* check the command */
	sCOMM_CMD sCmd;

	/* open the config file */
	pfParaCfg = fopen(cFileName, "r");
	if( pfParaCfg == NULL ){
		printf("can not open config parameter file %s\n", cFileName);
		return COMM_RET_FAIL;
	}

	/* clear the buffer */
	memset(pcCt, 0, COMM_CMDMAXCMTCHAR);

	/* write back the parameter one by one */
	while( fgets( pcCt, COMM_CMDMAXCMTCHAR, pfParaCfg) ){
		pcCt[strlen(pcCt)-1]=0;
		if(*pcCt=='#'){
			/* skip the comment */
			memset(pcCt, 0, COMM_CMDMAXCMTCHAR);
			continue;
		}

		/* skip reserve item */
		if((pcRes=index(pcCt, 'r'))!=NULL){
			if( ! strncmp(pcRes, "reserve", strlen("reserve"))){
				/* get paraId and paraVal */
				lParaId=atol(strtok(pcCt, " \t"));
				lParaVal=COMM_PARAINVALIDVAL;

				/* set the para and update the paras */
				vCOMM_PARA_SetPara(&sPara, lParaId, lParaVal);
				lCOMM_PARA_UpdateParas(&psParas, &sPara);
				memset(pcCt, 0, COMM_CMDMAXCMTCHAR);
				continue;
			}
		}

		/* get paraId and paraVal */
		lParaId=atol(strtok(pcCt, " \t"));
		lParaVal=atol(strtok(NULL, " \t"));

		/* construct the command and check the valid */
		vCOMM_CMD_InitCmd(&sCmd, lParaId>>COMM_CMDMAXSUBSHIFT, lParaId&((1<<COMM_CMDMAXSUBSHIFT)-1), lParaVal);
		lRet=lCOMM_CMD_CheckCmd(&sCmd);
		if(lRet<0){
			vCOMM_PARA_ParaIdToComment(lParaId, pcCt);
			printf("wrong config file : %s\n\tpara[%ld] : %ld\n\tret [%ld]\n", pcCt, lParaId, lParaVal, lRet);
			vCOMM_PARA_SetPara(&sPara, lParaId, COMM_PARAINVALIDVAL);
			lCOMM_PARA_UpdateParas(&psParas, &sPara);
			memset(pcCt, 0, COMM_CMDMAXCMTCHAR);
			lRet=COMM_RET_FAIL;
			break;
		}

		/* set the para and update the paras */
		vCOMM_PARA_SetPara(&sPara, lParaId, lParaVal);
		lCOMM_PARA_UpdateParas(&psParas, &sPara);
		
		/* clear the buffer */
		memset(pcCt, 0, COMM_CMDMAXCMTCHAR);
	}

	/* close the file */
	fclose(pfParaCfg);

	debug_info("the parameter is init\n");

	return lRet;
}

void vCOMM_PARA_InitDefaultParas( sCOMM_PARA** ppParas){
	LONG lParaId=0;

	/* first, init all the value to invalid */
	while(lParaId != COMM_MAXPARACNT){
		vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId, COMM_PARAINVALIDVAL);
		lParaId++;
	}

	/* second, set the support parameters */
	/* COMM_CMD_SYS 0 */
	
	/* COMM_CMD_VENC0 1 */
	lParaId=COMM_PARAGETID(COMM_CMD_VENC0,COMM_SUBCMD_VENCVIDEORESOLUTION);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,1);		// resolution ch0 on
	lParaId=COMM_PARAGETID(COMM_CMD_VENC0,COMM_SUBCMD_VENCVIDEOFRAMERATE);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,25);		// framerate 25
	lParaId=COMM_PARAGETID(COMM_CMD_VENC0,COMM_SUBCMD_VENCVIDEOGOP);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,25);		// gop 25

	/* COMM_CMD_ETH0 2 */
	lParaId=COMM_PARAGETID(COMM_CMD_ETH,COMM_SUBCMD_ETHVIDEORESOLUTION);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,1);		// resolution ch0 off
	lParaId=COMM_PARAGETID(COMM_CMD_ETH,COMM_SUBCMD_ETHSTOPRESOLUTION);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// video display ch0 off
	
	/* COMM_CMD_ISP0 3 */
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEORESOLUTION);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,1);		// resolution ch0 on
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOCONTRAST);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,50);		// isp video contrast
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOBRIGHTNESS);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,50);		// isp video brightness
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOCOLOR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,100);	// isp video color
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOSHARPNESS);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video sharpness
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEODDP);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video DDP
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEODIS);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video DIS
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOAWB);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video AWB
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOAE);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video AE
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOAF);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video AF
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOAI);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video AI
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOGAMMAPRESET);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video gamma preset	
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOSHADING);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video shading
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEODRC);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video DRC
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOANTIFLICKER);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video antiflicker
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEONR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video NR
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOWDR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video WDR	
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOSHADINGGAIN);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video shading gain
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOSHADINGVARIANCE);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video shading variance
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEODRCLEVEL);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video DRC level
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOANTIFLICKERFREQ);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video antiflicker freq
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOCRSH);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video CRS h_strength
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOCRSV);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video CRS v_strength
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOWDROFFSET);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video WDR offset
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOWDRPIXELMAX);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video WDR pixrlmax
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOWDRRATIO);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video WDR ratio
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEONRTHRESH);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video NR threash
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOISREGADDR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video isreg addr
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOISREGDATA);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video isreg data
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOFPGAADDR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video FPGA addr
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOFPGADATA);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video FPGA data
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOGETISREGADDR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video get isreg addr
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOGETFPGAADDR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video get FPGA addr
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOINITFRAME);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video frame rate
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOINITHEIGHT);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video active height
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOINITWIDTH);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video active width
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOINITBAYER);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);	// isp video bayer
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOGAMMA);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);	// isp video gamma
	lParaId=COMM_PARAGETID(COMM_CMD_ISP0,COMM_SUBCMD_ISPVIDEOTERM);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);	// isp video term
	
	/* COMM_CMD_RECORD 4 */
	lParaId=COMM_PARAGETID(COMM_CMD_RECORD,COMM_SUBCMD_RECAUDIORESOLUTION);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,-1);		// audio rec start none
	lParaId=COMM_PARAGETID(COMM_CMD_RECORD,COMM_SUBCMD_RECAUDIOSTOPRESOLUTION);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,-1);		// audio rec stop none
	lParaId=COMM_PARAGETID(COMM_CMD_RECORD,COMM_SUBCMD_RECSNAP);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,-1);		// snap
	
	/* COMM_CMD_VENC1 5 */
	lParaId=COMM_PARAGETID(COMM_CMD_VENC1,COMM_SUBCMD_VENCVIDEORESOLUTION);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,1);		// resolution ch0 on
	lParaId=COMM_PARAGETID(COMM_CMD_VENC1,COMM_SUBCMD_VENCVIDEOFRAMERATE);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,25);		// framerate 25
	lParaId=COMM_PARAGETID(COMM_CMD_VENC1,COMM_SUBCMD_VENCVIDEOGOP);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,25);		// gop 25
	
	/* COMM_CMD_ISP1 7 */
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEORESOLUTION);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,1);		// resolution ch0 on
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOCONTRAST);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,50);		// isp video contrast
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOBRIGHTNESS);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,50);		// isp video brightness
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOCOLOR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,100);	// isp video color	
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOSHARPNESS);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video sharpness
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEODDP);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video DDP
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEODIS);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video DIS
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOAWB);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video AWB
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOAE);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video AE
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOAF);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video AF
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOAI);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video AI
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOGAMMAPRESET);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video gamma preset	
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOSHADING);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video shading
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEODRC);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video DRC
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOANTIFLICKER);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video antiflicker
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEONR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video NR
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOWDR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video WDR	
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOSHADINGGAIN);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video shading gain
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOSHADINGVARIANCE);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video shading variance
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEODRCLEVEL);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video DRC level
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOANTIFLICKERFREQ);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video antiflicker freq
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOCRSH);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video CRS h_strength
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOCRSV);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video CRS v_strength
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOWDROFFSET);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video WDR offset
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOWDRPIXELMAX);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video WDR pixrlmax
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOWDRRATIO);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video WDR ratio
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEONRTHRESH);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video NR threash
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOISREGADDR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video isreg addr
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOISREGDATA);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video isreg data
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOFPGAADDR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video FPGA addr
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOFPGADATA);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video FPGA data
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOGETISREGADDR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video get isreg addr
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOGETFPGAADDR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video get FPGA addr
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOINITFRAME);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video frame rate
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOINITHEIGHT);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video active height
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOINITWIDTH);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video active width
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOINITBAYER);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);	// isp video bayer
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOGAMMA);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);	// isp video gamma
	lParaId=COMM_PARAGETID(COMM_CMD_ISP1,COMM_SUBCMD_ISPVIDEOTERM);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);	// isp video term
	
	/* COMM_CMD_HDMI 8*/
	
	/* COMM_CMD_VENC2 9 */
	lParaId=COMM_PARAGETID(COMM_CMD_VENC2,COMM_SUBCMD_VENCVIDEORESOLUTION);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,1);		// resolution ch0 on
	lParaId=COMM_PARAGETID(COMM_CMD_VENC2,COMM_SUBCMD_VENCVIDEOFRAMERATE);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,25);		// framerate 25
	lParaId=COMM_PARAGETID(COMM_CMD_VENC2,COMM_SUBCMD_VENCVIDEOGOP);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,25);		// gop 25
	
	/* COMM_CMD_ISP2 11 */
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEORESOLUTION);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,1);		// resolution ch0 on
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOCONTRAST);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,50);		// isp video contrast
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOBRIGHTNESS);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,50);		// isp video brightness
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOCOLOR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,100);	// isp video color	
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOSHARPNESS);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video sharpness
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEODDP);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video DDP
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEODIS);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video DIS
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOAWB);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video AWB
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOAE);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video AE
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOAF);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video AF
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOAI);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video AI	
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOGAMMAPRESET);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video gamma preset	
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOSHADING);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video shading
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEODRC);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video DRC
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOANTIFLICKER);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video antiflicker
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEONR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video NR
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOWDR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);		// isp video WDR
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOSHADINGGAIN);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video shading gain
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOSHADINGVARIANCE);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video shading variance
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEODRCLEVEL);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video DRC level
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOANTIFLICKERFREQ);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video antiflicker freq
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOCRSH);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video CRS h_strength
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOCRSV);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video CRS v_strength
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOWDROFFSET);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video WDR offset
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOWDRPIXELMAX);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video WDR pixrlmax
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOWDRRATIO);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video WDR ratio
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEONRTHRESH);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video NR threash
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOISREGADDR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video isreg addr
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOISREGDATA);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video isreg data
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOFPGAADDR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video FPGA addr
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOFPGADATA);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video FPGA data
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOGETISREGADDR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video get isreg addr
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOGETFPGAADDR);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video get FPGA addr
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOINITFRAME);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video frame rate
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOINITHEIGHT);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video active height
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOINITWIDTH);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,170);	// isp video active width
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOINITBAYER);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);	// isp video bayer	
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOGAMMA);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);	// isp video gamma
	lParaId=COMM_PARAGETID(COMM_CMD_ISP2,COMM_SUBCMD_ISPVIDEOTERM);
	vCOMM_PARA_SetPara(*ppParas+lParaId,lParaId,0);	// isp video term
}

void vCOMM_PARA_SetPara( sCOMM_PARA* pPara, LONG lId, LONG lVal){
	/* get lock and set para, release when finish */
	pthread_mutex_lock( &muxPara );
	pPara->lParaId=lId;
	pPara->lParaVal=lVal;
	pthread_mutex_unlock( &muxPara );
}

LONG lCOMM_PARA_GetParaId( sCOMM_PARA* pPara){
	LONG lParaId;

	/* get lock and release when finish */
	pthread_mutex_lock( &muxPara );
	lParaId=pPara->lParaId;
	pthread_mutex_unlock( &muxPara );
	
	return lParaId;
}

LONG lCOMM_PARA_GetParaVal( sCOMM_PARA* pPara){
	LONG lParaVal;

	/* get lock and release when finish */
	pthread_mutex_lock( &muxPara );
	lParaVal=pPara->lParaVal;
	pthread_mutex_unlock( &muxPara );

	return lParaVal;
}

void vCOMM_PARA_WriteParasBack( sCOMM_PARA* paras){
	/* buffer to hold file content */
	CHAR cContent[COMM_CMDMAXCMTCHAR];
	CHAR* pcCt=cContent;
	CHAR* pcCtTmp=pcCt;
	LONG lParaId=0;

	/* config file */
	FILE* pfParaCfg=NULL;

	/* parameter for each handle */
	sCOMM_PARA* psPara;

	/* open the config file */
	pfParaCfg = fopen(cFileName, "w");
	if( pfParaCfg == NULL ){
		printf("can not open config parameter file %s\n", cFileName);
		exit(-1);
	}

	/* write the head */
	memset(cContent, 0,sizeof(cContent));
	strcpy(pcCt, "#PARAID\tPARAVAL\tCOMMENT\n");
	fwrite(pcCt, strlen(pcCt), 1, pfParaCfg);

	/* write back the parameter one by one */
	while( lParaId != COMM_MAXPARACNT ){
		/* clear the buffer */
		memset(cContent, 0,sizeof(cContent));
		
		/* paraId and paraVal */
		psPara=paras+lParaId;
		pcCtTmp=pcCt;
		pcCtTmp+=sprintf(pcCt, "%ld\t%ld\t", lParaId, lCOMM_PARA_GetParaVal(psPara));

		/* add comment */
		vCOMM_PARA_ParaIdToComment(lParaId, pcCtTmp);

		/* add tail [cr] */
		strcat(pcCt, "\n");

		/* write back to the config file */
		fwrite(pcCt, strlen(pcCt), 1, pfParaCfg);

		/* go to the next para */
		lParaId++;
	}

	/* close the file */
	fclose(pfParaCfg);

	/* destroy the mutex */
	pthread_mutex_destroy( &muxPara );
}

void vCOMM_PARA_ParaIdToSubCommentSystem( LONG lParaId, CHAR* pcSubCmt){
	CHAR* pc=pcSubCmt;

	switch(lParaId & ((1<<COMM_CMDMAXSUBSHIFT)-1)){

		case COMM_SUBCMD_SYSQUIT : 
			strcpy(pc, "[ system quit ]");
			pc+=strlen(pc);
			break;
		default:
			strcpy(pc, "[ reserve ]");
			pc+=strlen(pc);
			break;
	}
}
void vCOMM_PARA_ParaIdToSubCommentVenc( LONG lParaId, CHAR* pcSubCmt){
	CHAR* pc=pcSubCmt;

	switch(lParaId & ((1<<COMM_CMDMAXSUBSHIFT)-1)){
		case COMM_SUBCMD_VENCVIDEOFRAMERATE : 
			strcpy(pc, "[ video framerate ]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_VENCVIDEOGOP : 
			strcpy(pc, "[ video gop ]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_VENCVIDEORESOLUTION : 
			strcpy(pc, "[ venc video resolution ]");
			pc+=strlen(pc);
			break;
		default:
			strcpy(pc, "[ reserve ]");
			pc+=strlen(pc);
			break;
	}
}

void vCOMM_PARA_ParaIdToSubCommentEth( LONG lParaId, CHAR* pcSubCmt){
	CHAR* pc=pcSubCmt;

	switch(lParaId & ((1<<COMM_CMDMAXSUBSHIFT)-1)){
		case COMM_SUBCMD_ETHVIDEOIPADDRESS : 
			strcpy(pc, "[ ip address ]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ETHVIDEOPORT : 
			strcpy(pc, "[ ip port ]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ETHVIDEORESOLUTION : 
			strcpy(pc, "[ rtp video resolution ]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ETHSTOPRESOLUTION : 
			strcpy(pc, "[ rtp video stop display resolution ]");
			pc+=strlen(pc);
			break;
		default:
			strcpy(pc, "[ reserve ]");
			pc+=strlen(pc);
			break;
	}
}

void vCOMM_PARA_ParaIdToSubCommentISP( LONG lParaId, CHAR* pcSubCmt){
	CHAR* pc=pcSubCmt;

	switch(lParaId & ((1<<COMM_CMDMAXSUBSHIFT)-1)){
		case COMM_SUBCMD_ISPVIDEOCONTRAST: 
			strcpy(pc, "[ contrast ]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEORESOLUTION : 
			strcpy(pc, "[ isp video resolution ]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOBRIGHTNESS: 
			strcpy(pc, "[ brightness ]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOCOLOR: 
			strcpy(pc, "[ color ]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOSHARPNESS: 
			strcpy(pc, "[ sharpness]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEODDP: 
			strcpy(pc, "[ DDP]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEODIS: 
			strcpy(pc, "[ DIS]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOAWB: 
			strcpy(pc, "[ AWB]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOAE: 
			strcpy(pc, "[ AE]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOAF: 
			strcpy(pc, "[ AF]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOAI: 
			strcpy(pc, "[ AI]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOGAMMAPRESET: 
			strcpy(pc, "[ gamma preset]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOSHADING: 
			strcpy(pc, "[ shading]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEODRC: 
			strcpy(pc, "[ DRC]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOANTIFLICKER: 
			strcpy(pc, "[ antiflicker]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEONR: 
			strcpy(pc, "[ NR]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOWDR: 
			strcpy(pc, "[ WDR]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOSHADINGGAIN: 
			strcpy(pc, "[ shading gain]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOSHADINGVARIANCE: 
			strcpy(pc, "[ shading variance]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEODRCLEVEL: 
			strcpy(pc, "[ DRC level]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOANTIFLICKERFREQ: 
			strcpy(pc, "[ antiflicker freq]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOCRSH: 
			strcpy(pc, "[ CRS h_strength]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOCRSV: 
			strcpy(pc, "[ CRS v_strength]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOWDROFFSET: 
			strcpy(pc, "[ WDR offset]");
			pc+=strlen(pc);
			break;	
		case COMM_SUBCMD_ISPVIDEOWDRPIXELMAX: 
			strcpy(pc, "[ WDR pixelmax]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOWDRRATIO: 
			strcpy(pc, "[ WDR ratio]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEONRTHRESH: 
			strcpy(pc, "[ NR thresh]");
			pc+=strlen(pc);
			break;	
		case COMM_SUBCMD_ISPVIDEOISREGADDR: 
			strcpy(pc, "[ isreg addr]");
			pc+=strlen(pc);
			break;	
		case COMM_SUBCMD_ISPVIDEOISREGDATA: 
			strcpy(pc, "[ isreg data]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOFPGAADDR: 
			strcpy(pc, "[ fpga addr]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOFPGADATA: 
			strcpy(pc, "[ fpga data]");
			pc+=strlen(pc);
			break;

		case COMM_SUBCMD_ISPVIDEOGETISREGADDR: 
			strcpy(pc, "[ get isreg addr]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOGETFPGAADDR: 
			strcpy(pc, "[ get fpga addr]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOINITWIDTH: 
			strcpy(pc, "[ active width]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOINITHEIGHT: 
			strcpy(pc, "[ active high]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOINITFRAME: 
			strcpy(pc, "[ frame rate]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOINITBAYER: 
			strcpy(pc, "[ bayer]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOGAMMA: 
			strcpy(pc, "[ gamma]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_ISPVIDEOTERM: 
			strcpy(pc, "[ term]");
			pc+=strlen(pc);
			break;
			
		default:
			strcpy(pc, "[ reserve ]");
			pc+=strlen(pc);
			break;
	}
}
void vCOMM_PARA_ParaIdToSubCommentRec( LONG lParaId, CHAR* pcSubCmt){
	CHAR* pc=pcSubCmt;

	switch(lParaId & ((1<<COMM_CMDMAXSUBSHIFT)-1)){
		case COMM_SUBCMD_RECAUDIORESOLUTION : 
			strcpy(pc, "[ record audio start ]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_RECAUDIOSTOPRESOLUTION : 
			strcpy(pc, "[ record audio stop ]");
			pc+=strlen(pc);
			break;
		case COMM_SUBCMD_RECSNAP: 
			strcpy(pc, "[ snap ]");
			pc+=strlen(pc);
			break;
		default:
			strcpy(pc, "[ reserve ]");
			pc+=strlen(pc);
			break;
	}
}
void vCOMM_PARA_ParaIdToSubCommentHdmi( LONG lParaId, CHAR* pcSubCmt){
	CHAR* pc=pcSubCmt;

	switch(lParaId & ((1<<COMM_CMDMAXSUBSHIFT)-1)){

		default:
			strcpy(pc, "[ reserve ]");
			pc+=strlen(pc);
			break;
	}
}

void vCOMM_PARA_ParaIdToComment( LONG lParaId, CHAR* pcCmt){	
	CHAR* pc=pcCmt;

	/* check paraId */
	if(lParaId>=COMM_MAXPARACNT){
		/* check the valid */
		debug_info("wrong lParaId : %ld\n", lParaId);
		exit(-1);
	}

	/* the '#' */
	*pc++='#';
	*pc++=' ';

	switch( lParaId >> COMM_CMDMAXSUBSHIFT ) {
		case COMM_CMD_SYS :
			strcpy(pc, "SYSTEM");
			pc+=strlen(pc);
			*pc++=' ';
			vCOMM_PARA_ParaIdToSubCommentSystem(lParaId, pc);
			break;
		case COMM_CMD_VENC0 :
			strcpy(pc, "VENC ch0");
			pc+=strlen(pc);
			*pc++=' ';
			vCOMM_PARA_ParaIdToSubCommentVenc(lParaId, pc);
			break;
		case COMM_CMD_ETH :
			strcpy(pc, "ETH");
			pc+=strlen(pc);
			*pc++=' ';
			vCOMM_PARA_ParaIdToSubCommentEth(lParaId, pc);
			break;
		case COMM_CMD_ISP0 :
			strcpy(pc, "ISP ch0");
			pc+=strlen(pc);
			*pc++=' ';
			vCOMM_PARA_ParaIdToSubCommentISP(lParaId, pc);
			break;
		case COMM_CMD_VENC1 :
			strcpy(pc, "VENC ch1");
			pc+=strlen(pc);
			*pc++=' ';
			vCOMM_PARA_ParaIdToSubCommentVenc(lParaId, pc);
			break;
		case COMM_CMD_ISP1 :
			strcpy(pc, "ISP ch1");
			pc+=strlen(pc);
			*pc++=' ';
			vCOMM_PARA_ParaIdToSubCommentISP(lParaId, pc);
			break;
		case COMM_CMD_VENC2 :
			strcpy(pc, "VENC ch2");
			pc+=strlen(pc);
			*pc++=' ';
			vCOMM_PARA_ParaIdToSubCommentVenc(lParaId, pc);
			break;
		case COMM_CMD_ISP2 :
			strcpy(pc, "ISP ch2");
			pc+=strlen(pc);
			*pc++=' ';
			vCOMM_PARA_ParaIdToSubCommentISP(lParaId, pc);
			break;
		case COMM_CMD_RECORD :
			strcpy(pc, "RECORD");
			pc+=strlen(pc);
			*pc++=' ';
			vCOMM_PARA_ParaIdToSubCommentRec(lParaId, pc);
			break;
		case COMM_CMD_HDMI :
			strcpy(pc, "HDMI");
			pc+=strlen(pc);
			*pc++=' ';
			vCOMM_PARA_ParaIdToSubCommentHdmi(lParaId, pc);
			break;
		default :
			strcpy(pc, "reserve");
			break;	
	}
}

LONG lCOMM_PARA_UpdateParas( sCOMM_PARA** ppParas,  sCOMM_PARA* pPara){
	LONG lParaId;
	LONG lVal;
	LONG lRet=-1;

	/* get the value */
	lParaId=lCOMM_PARA_GetParaId(pPara);
	if(lParaId>=COMM_MAXPARACNT){
		/* check the valid */
		debug_info("wrong lParaId : %ld\n", lParaId);
		exit(-1);
	}

	lVal=lCOMM_PARA_GetParaVal(pPara);
	/* set back to the parameter list and return the old value */
	lRet=lCOMM_PARA_GetParaVal(*ppParas+lParaId);
	vCOMM_PARA_SetPara(*ppParas+lParaId, lParaId, lVal);

	return lRet;
}

#ifdef SHOW_INFO
void vCOMM_PARA_ShowPara( sCOMM_PARA* pPara){
	CHAR cContent[COMM_CMDMAXCMTCHAR];
	vCOMM_PARA_ParaIdToComment(pPara->lParaId, cContent);
	printf("sPara info [%ld]\t%ld\t%s\n", pPara->lParaId, pPara->lParaVal, cContent);
}
#endif /* SHOW_INFO */

#ifdef SHOW_INFO
void vCOMM_PARA_ShowParas( sCOMM_PARA** ppParas){
	 sCOMM_PARA* pPara=*ppParas;
	LONG i=0;
	printf("\nparas info :\n");
	while(i!=COMM_MAXPARACNT)
		vCOMM_PARA_ShowPara(pPara+i++);
}
#endif /* SHOW_INFO */

void vCOMM_PARA_ParaToChar( sCOMM_PARA* pPara, CHAR* cpPara){
	LONG n;
	
	n=snprintf(cpPara, COMM_MAXPARACHAR, "%ld %ld", pPara->lParaId, pPara->lParaVal);
	if(COMM_MAXPARACHAR==n){
		printf("warning : parameter char reach top limit!\n");
	}
}

void vCOMM_PARA_CharToPara( sCOMM_PARA* pPara, CHAR* cpPara){
	sscanf(cpPara, "%ld %ld", &(pPara->lParaId), &(pPara->lParaVal));
}

void vCOMM_PARA_ParasToChar( sCOMM_PARA** ppParas, CHAR* cpPara){
	LONG lParaId=0;
	CHAR* pcParaTmp=cpPara;

	while(COMM_MAXPARACNT!=lParaId){
		vCOMM_PARA_ParaToChar(*ppParas+lParaId, pcParaTmp);
		pcParaTmp+=strlen(pcParaTmp);
		*pcParaTmp++='&';
		lParaId++;
	}
}


#ifdef __cplusplus
}
#endif


