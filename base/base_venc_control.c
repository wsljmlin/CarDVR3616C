#include "base_comm.h"
#include "base_venc_control.h"
#include "base_mw_region.h"
#include "base_mw_modechange.h"
#include "base_mw_sd.h"
#include "base_mw_snap.h"
#include "base_venc_handle.h"
#include "base_venc_dataconsume.h"
#include "base_mw_subcard.h"
#include "base_mw_filesort.h"
#include "base_mw_boardinterface.h"
#include "base_mw_time.h"
#include "base_mw_wifi.h"
#include <assert.h>

#include <time.h>

#ifdef __cplusplus
extern "C"{
#endif

/*  The client num */
static LONG glClientNum = 0;
static eBASE_VENC_PIC_SIZE s_eVencSizeTable[BASE_VENC_CHALL] = {
	BASE_RES_HD1080,			// venc channel 0
	BASE_RES_HD720,			// venc channel 1
	BASE_RES_SD480,			// venc channel 2
};

/*************************************************************************
	define
*************************************************************************/
#define BASE_VENC_CONTROL_V_TIMESECTH 6
#define BASE_VENC_CONTROL_M_TIMESECTH 300
#define BASE_VENC_CONTROL_SUB_TIMESECTH 10

//#define BASE_VENC_CONTROL_DEBUG
#ifdef BASE_VENC_CONTROL_DEBUG
#define BVC_Debug(fmt, arg...) fprintf(stdout, "[ BVC ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define BVC_Debug(fmt, arg...)
#endif

#define BVC_FuncIn() BVC_Debug("in\n")
#define BVC_FuncOut() BVC_Debug("out\n")
#define BVC_iVal(iVal) BVC_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define BVC_lVal(lVal) BVC_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define BVC_PtVal(ptVal) BVC_Debug("pt %s @ %p\n", #ptVal, ptVal)


typedef enum {
	BASE_VENC_CONTROL_LEDSTATUS_REDOFF_GREENOFF = 0x00,
	BASE_VENC_CONTROL_LEDSTATUS_REDBLINK_GREENOFF = 0x10,
	BASE_VENC_CONTROL_LEDSTATUS_REDON_GREENOFF = 0x20,
	BASE_VENC_CONTROL_LEDSTATUS_REDOFF_GREENBLINK = 0x01,
	BASE_VENC_CONTROL_LEDSTATUS_REDBLINK_GREENBLINK = 0x11,
	BASE_VENC_CONTROL_LEDSTATUS_REDON_GREENBLINK = 0x21,
	BASE_VENC_CONTROL_LEDSTATUS_REDOFF_GREENON = 0x02,
	BASE_VENC_CONTROL_LEDSTATUS_REDBLINK_GREENON = 0x12,
	BASE_VENC_CONTROL_LEDSTATUS_REDON_GREENON = 0x22,
} eBASE_VENC_CONTROL_LEDSTATUS;

/* function declare */
void vBASE_VENC_Ctrl_Usage(CHAR *pcPrgNm);
void vBASE_VENC_Ctrl_StreamHandleSig(HI_S32 signo);
void vBASE_VENC_Ctrl_SnapHandleSig(HI_S32 signo);
void lBASE_VENC_Ctrl_HandleSig(HI_S32 signo);
LONG lBASE_VENC_Ctrl_StreamProcess(HI_CHAR cChoose);
LONG lBASE_VENC_Ctrl_SnapProcess(HI_CHAR cChoose);
void* vBASE_VENC_Ctrl_Thread(void* pvArg);
LONG lBASE_VENC_Ctrl_VencInit();
LONG lBASE_VENC_Ctrl_VencStart();
LONG lBASE_VENC_Ctrl_Process();
LONG lBASE_VENC_Ctrl_VencExit(LONG lExitCmd);
void lBASE_VENC_Ctrl_Exit();

/* global video variables */
static VB_CONF_S gs_stVbConf ={0};/* vb config define */
VI_DEV gs_ViDevBnd_curr;	/* which vi-dev is binded to vo-chn */
VI_CHN gs_ViChnBnd_curr;	/* which vi-chn is binded to vo-chn */
static HI_S32 gs_s32Cnt; /* use venc chn count.one vi chn -- one venc grp -- one venc chn */ 	

extern CHAR gcTargetFileInfo[TWO_FOLDER][32];

#define BASE_CMD_SHIFT 64

/* macro */
/**********************************************************************************/
#define BASE_VENC_CTL_DEBUG
#ifdef BASE_VENC_CTL_DEBUG
#define BVENC_Ctl_Debug(fmt, arg...) fprintf(stdout, "[ BASE VENC CTL ] : %s < %d > "fmt, __func__, __LINE__, ##arg)
#define BVENC_Ctl_FuncIn() //fprintf(stdout, "in %s\n", __func__)
#define BVENC_Ctl_FunOut() //fprintf(stdout, "out %s\n", __func__)
#define BVENC_Ctl_iVal(para) fprintf(stdout, "%s : 0x%x(%d) @ %p\n", #para, (para), (para), &(para))
#define BVENC_Ctl_lVal(para) fprintf(stdout, "%s : <%ld> @ %p\n", #para, (para), &(para))
#define BVENC_Ctl_PtVal(para) fprintf(stdout, "%s : %p\n", #para, (para))
#define BVENC_Ctl_ShowErr(err) fprintf(stderr, "%s <%d> : Error <%d>\n", __func__, __LINE__, err)
#else
#define BVENC_Ctl_Debug(fmt, args...)
#define BVENC_Ctl_FuncIn()
#define BVENC_Ctl_FunOut()
#define BVENC_Ctl_iVal(para)
#define BVENC_Ctl_lVal(para)
#define BVENC_Ctl_PtVal(para)
#define BVENC_Ctl_ShowErr(err)
#endif

/*------------------------------------------------------------------------------*/
/*!
	@brief	Venc thread,include system init,vi,vo,venc.etc.start , hdmi and system process
    
	@param	LONG lcmd							[in]	The command
			LONG lopt							[in]	The command option
			LONG lch							[in]	The channel

	@retval	LONG										return the commandID for given command,option,and ch
*/
/* -----------------------------------------------------------------------------*/
__inline__ LONG lBASE_GET_PARAID(LONG lcmd,LONG lopt,LONG lch)
{
	return((((lcmd==COMM_CMD_ISP0)||(lcmd==COMM_CMD_VENC0))?\
	((lcmd+lch*COMM_CMDCHSTEP)*BASE_CMD_SHIFT+lopt):(lcmd*BASE_CMD_SHIFT+lopt)));
}

/*------------------------------------------------------------------------------*/
/*!
	@brief	Venc thread,include system init,vi,vo,venc.etc.start , hdmi and system process
    
	@param	void* pvArg							[in]	The parameraters from AVMsgment

	@retval	LONG										When venc thread exit,will return NULL
*/
/* -----------------------------------------------------------------------------*/
void* vBASE_VENC_Ctrl_Thread(void* pvArg)
{
	debug_show("venc start\n");
	
	/* venc and audio init */
	lBASE_VENC_Ctrl_VencInit();

	/* venc and audio start */
	lBASE_VENC_Ctrl_VencStart();

	/* vencCtrl thread process */
	lBASE_VENC_Ctrl_Process();

	debug_show("venc stop\n");

	return (void*)NULL;
}

/*------------------------------------------------------------------------------*/
/*!
	@brief	System process, get system updated parameters and process,with AVMsgment
			Now only support system quit

	@retval	LONG				BASE_RET_SUCCESS indicate system process success 
								or have no parameters to process
								BASE_RET_FAIL indicate system process error
*/
/* -----------------------------------------------------------------------------*/
LONG lBASE_VENC_Ctrl_SystemProcess()
{	
	LONG lRet = BASE_RET_SUCCESS;
	
	BVC_Debug("CommandType is System\n");
#if 1
	/* get system parameter */
	memset(&gsSystemAttr,0,sizeof(gsSystemAttr));
	lBASE_VENC_GetSystemUpdatedParam(plCurrentChangeParamsArray,&gsSystemAttr);

	/* handle */
	lRet = lBASE_VENC_SystemParaHandle(&gsSystemAttr);
	if(BASE_RET_SYSTEMQUIT == lRet)
	{
		/* response to AVMsgment */
		vCOMM_CF_WtCF( &stCfBaseRet, COMM_COND_CFSYSQUIT );
		debug_info("[ VencCtrl ] success response to AVMsgment\n");
		return BASE_RET_SYSTEMQUIT;
		
	}
	else if(BASE_RET_SUCCESS == lRet)
	{
		/* response to AVMsgment */
		vCOMM_CF_WtCF( &stCfBaseRet, COMM_COND_CFSYSRETSUC );
		debug_info("[ VencCtrl ] success response to AVMsgment\n");
		return BASE_RET_SUCCESS;

	}
	else
	{
		debug_info("[%s:%d] system para handle error\n",__FUNCTION__,__LINE__);
		/* response to AVMsgment */
		vCOMM_CF_WtCF( &stCfBaseRet, COMM_COND_CFSYSRETFAIL );
		debug_info("[ VencCtrl ] success response to AVMsgment\n");
		return BASE_RET_FAIL;
	}
#endif

}	

/*------------------------------------------------------------------------------*/
/*!
	@brief	Venc contrl process
	@param	VENC_CHN VencChn	[in]	Venc Channel
    

	@retval	LONG						BASE_RET_SUCCESS indicate venc contrl handle success 
										BASE_RET_FAIL indicate venc contrl handle error
*/
/* -----------------------------------------------------------------------------*/
LONG lBASE_VENC_Ctrl_VencProcess( VENC_CHN VencChn)
{
	LONG s32Ret = BASE_RET_SUCCESS;	
	
	/* get venc updated parameters */
	s32Ret = lBASE_VENC_GetVencUpdatedParam(plCurrentChangeParamsArray,&(gsVencAttr[VencChn]));
	if(BASE_RET_SUCCESS != s32Ret)
	{
		debug_info("[%s:%d] VENC para handle error\n",__FUNCTION__,__LINE__);
		// response to AVMsgment
		vCOMM_CF_WtCF( &stCfBaseRet, COMM_COND_CFSYSRETFAIL );
		return BASE_RET_FAIL;
	}

	/* abnoraml handle case */
	if(!(BASE_COMM_GETTYPEVAL(geModeFlag,BASE_MODEFLAG_VENC)&(1<<VencChn)))
    	{
    		debug_info("[%s:%d] The resolution have not started!\n",__FUNCTION__,__LINE__);
		// response to AVMsgment
		vCOMM_CF_WtCF( &stCfBaseRet, COMM_COND_CFSYSRETFAIL );
		return BASE_RET_FAIL;
	}
	
	/* handle venc process */
	debug_info("CommandType is VENC,ch[%ld]\n",(LONG)VencChn);
	s32Ret = lBASE_VENC_ParaHandle(VencChn,&(gsVencAttr[VencChn]));
	if(BASE_RET_SUCCESS != s32Ret)
	{
		debug_info("[%s:%d] VENC para handle error\n",__FUNCTION__,__LINE__);
		// response to AVMsgment
		vCOMM_CF_WtCF( &stCfBaseRet, COMM_COND_CFSYSRETFAIL );
		return BASE_RET_FAIL;
	}

	// response to AVMsgment
	vCOMM_CF_WtCF( &stCfBaseRet, COMM_COND_CFSYSRETSUC );
	debug_info("[ VencCtrl ] successfully response to AVMsgment\n");
			
	return BASE_RET_SUCCESS;
	
}

/*------------------------------------------------------------------------------*/
/*!
	@brief	Eth handle,mainly change mode.
	@param	eBASE_CH_CHANGE eChangeCh	[in]	command channnel,such as 1080p,720p.etc.
			eBASE_MODEFLAG* peModeFlag	[out]   mangement and record vi,vo,venc channel and
												mode flag

	@retval	LONG						BASE_RET_SUCCESS indicate Eth  handle success 
										BASE_RET_FAIL indicate Eth  handle error
*/
/* -----------------------------------------------------------------------------*/
LONG lBASE_VENC_Ctrl_EthHandle(eBASE_CH_CHANGE eChangeCh,eBASE_MODEFLAG* peModeFlag)
{
	LONG lRet=BASE_RET_SUCCESS;
	eBASE_MODEFLAG eModeFlagbak = *peModeFlag;

	if(-1 != eChangeCh)
	{
		lRet = lBASE_MODE_CHANGE(peModeFlag,eChangeCh);
		if(BASE_RET_SUCCESS != lRet)
		{
			/* if mode change failed,recover mode change flag */
			*peModeFlag = eModeFlagbak;
			debug_info("[ ETH ] mode change error! \n");
			return BASE_RET_FAIL;

		}
	}
	return BASE_RET_SUCCESS;

}
	
/*------------------------------------------------------------------------------*/
/*!
	@brief	Eth contrl process,get and save updated rtp parameters,notify VencGetstream
			and AudioGetstream thread,read their feedback,response to AVMsgment
	@param	VENC_CHN VencChn	[in]	Venc Channel
    

	@retval	LONG						BASE_RET_SUCCESS indicate Eth contrl handle success 
										BASE_RET_FAIL indicate Eth contrl handle error
*/
/* -----------------------------------------------------------------------------*/
LONG lBASE_VENC_Ctrl_EthProcess()
{
	LONG lCfRet;
	LONG lVch=-1;
	LONG lAch=-1;
	LONG lRet = BASE_RET_SUCCESS;

	eBASE_CH_CHANGE eChangeCh = -1;

	memset(&gsRtpAttr,0,sizeof(gsRtpAttr));

	/* get rtp parameters */
	lBASE_VENC_GetRtpUpdatedParam(&eChangeCh, plCurrentChangeParamsArray,&lVch,&lAch,gsRtpAttr);

	lRet = lBASE_VENC_Ctrl_EthHandle(eChangeCh,&geModeFlag);
	if(BASE_RET_SUCCESS != lRet)
	{
		/* Response to AVMsgment */
		vCOMM_CF_WtCF(&stCfBaseRet,COMM_COND_CFSYSRETFAIL);
		debug_info("[ VencCtrl ] successfully response success to AVMsgment\n");
		return lRet;		
	}
	
	/* video rtp process */
	if((-1!=lVch)&&(BASE_RTP_VIDEO == gsRtpAttr[lVch].lRtpCmdType))
	{
		/* the encoder have not start */
		if(!(BASE_COMM_GETTYPEVAL(geModeFlag,BASE_MODEFLAG_VENC)&(1<<lVch)))
    	{
			/* Response to AVMsgment */
			vCOMM_CF_WtCF(&stCfBaseRet,COMM_COND_CFSYSRETFAIL);
			printf("[ RTP ] have not start the rtp resolution\n");
			debug_info("[ VencCtrl ] successfully response success to AVMsgment\n");
    	}
		
		vCOMM_CF_SetIdCF(&(stCfVencCtrlCh[lVch]),COMM_CMD_ETH,COMM_COND_NOBLOCK);
		/* get response of venc getstream  */
		vCOMM_CF_RdCF(&(stCfVencCtrlCh[lVch]), &lCfRet);

		if(COMM_COND_CFSYSRETFAIL == lCfRet)
		{
			debug_info("[ VencCtrl ] timeout receive VENC GetStream! \n");
			vCOMM_CF_WtCF(&stCfBaseRet,COMM_COND_CFSYSRETFAIL);
			return BASE_RET_FAIL;
		}
		
	}

	/* Response to AVMsgment */
	vCOMM_CF_WtCF(&stCfBaseRet,COMM_COND_CFSYSRETSUC);
	debug_info("[ VencCtrl ] successfully response success to AVMsgment\n");
	

	return BASE_RET_SUCCESS;
}

/*------------------------------------------------------------------------------*/
/*!
	@brief	Record contrl process,get and save updated rtp parameters,notify VencGetstream
			and AudioGetstream thread,read their feedback,response to AVMsgment
    

	@retval	LONG						BASE_RET_SUCCESS indicate Record contrl handle success 
										BASE_RET_FAIL indicate Record contrl handle error
*/	
/* -----------------------------------------------------------------------------*/
LONG lBASE_VENC_Ctrl_RecordProcess()
{	
	LONG lCfRet;
	LONG lRet=BASE_RET_SUCCESS;
	LONG lACh=-1;
	LONG lVCh=-1;
	eBASE_MW_SUBCARD_Ret eSubcardRet = BASE_MW_SUBCARD_SUCCESS;

	/* sd check */
	//lRet = eBASE_MW_SD_CHECK(&gsSDStat);
	if(BASE_RET_SUCCESS != lRet)
	{
		vCOMM_CF_WtCF(&stCfBaseRet,COMM_COND_CFSYSRETFAIL);
		printf("[ REC ] NO SD card,file please insert sd!\n");
		lRet = BASE_RET_FAIL;
		return lRet;
	}
		
	lBASE_VENC_GetRecordUpdatedParam(plCurrentChangeParamsArray,&lACh, &lVCh,&gsFileAttr);
	
	/* video record process */
	if((-1!=lVCh)&&( COMM_BIT(BASE_RECORD_VIDEO)&(gsFileAttr.lRecordCmdType)))
	{
		gsFileAttr.lRecordCmdType &= (~ COMM_BIT(BASE_RECORD_VIDEO));
		/* the encoder have not start */
		if(!(BASE_COMM_GETTYPEVAL(geModeFlag,BASE_MODEFLAG_VENC)&(1<<lVCh)))
    		{
			/* Response to AVMsgment */
			vCOMM_CF_WtCF(&stCfBaseRet,COMM_COND_CFSYSRETFAIL);
			printf("[ REC ] have not start the recorded resolution\n");
			debug_info("[ VencCtrl ] successfully response success to AVMsgment\n");
			return BASE_RET_FAIL;
    		}
		/* notify venc getstream */
		vCOMM_CF_SetIdCF(&(stCfVencCtrlCh[lVCh]),COMM_CMD_RECORD,COMM_COND_NOBLOCK);
		/* get response of venc getstream  */	
		vCOMM_CF_RdCF(&(stCfVencCtrlCh[lVCh]), &lCfRet);
		if(COMM_COND_CFSYSRETFAIL == lCfRet)
		{
			debug_info("[ VencCtrl ] timeout receive VENC GetStream! \n");
			vCOMM_CF_WtCF(&stCfBaseRet,COMM_COND_CFSYSRETFAIL);
			return BASE_RET_FAIL;
		}

		/* get feedback */
		debug_info("[ VencCtrl ] successfully receive venc GetStream! \n");

	}

	if( COMM_BIT(BASE_RECORD_SNAP)&(gsFileAttr.lRecordCmdType))
	{
		gsFileAttr.lRecordCmdType &= (~ COMM_BIT(BASE_RECORD_SNAP));
		//printf("<Enter into lBASE_VENC_SnapHandle>\n");
		lRet=lBASE_VENC_SnapHandle(&gsSnap);
		//printf("<Exit out lBASE_VENC_SnapHandle>\n");
		if(BASE_RET_SUCCESS != lRet) {
			debug_info("snap handle error\n");
			vCOMM_CF_WtCF(&stCfBaseRet,COMM_COND_CFSYSRETFAIL);
			debug_info("[ VencCtrl ] failure response success to AVMsgment\n");
			return lRet;
		}
	}

	/* subcard rec */
	if( COMM_BIT(BASE_RECORD_SUBCARD)&(gsFileAttr.lRecordCmdType))
	{
		gsFileAttr.lRecordCmdType &= (~ COMM_BIT(BASE_RECORD_SUBCARD));
		eSubcardRet = eBASE_MW_SUBCARD_NewSignal(&sSubCardInfo);
		if(BASE_MW_SUBCARD_SUCCESS != eSubcardRet) {
			BVC_Debug("sub card rec error %d\n", eSubcardRet);
			vCOMM_CF_WtCF(&stCfBaseRet,COMM_COND_CFSYSRETFAIL);
			return lRet;
		}

		/* get feedback */
		BVC_Debug("[ VencCtrl ] successfully receive subcard record ! \n");
	}

	/* buffer time set */
	if (COMM_BIT(BASE_RECORD_BUFFERTIME) & (gsFileAttr.lRecordCmdType)) {

		BVC_Debug("set buffer time\n");

		/* clear the command */
		gsFileAttr.lRecordCmdType &= (~ COMM_BIT(BASE_RECORD_BUFFERTIME));
		
		/* update the buffer time */
		eBASE_VENC_DATACONSUME_UpdateTimeThreshold(&gsDataConsume,
			BASE_VENC_DATACONSUME_DEFAULTTIME, gsFileAttr.lBufferTime + 1);

		/* reset the buffer time */
		gsFileAttr.lBufferTime = 0;
	}

	/* whole time set */
	if (COMM_BIT(BASE_RECORD_WHOLETIME) & (gsFileAttr.lRecordCmdType)) {

		BVC_Debug("set whole time\n");
		
		/* clear the command */
		gsFileAttr.lRecordCmdType &= (~ COMM_BIT(BASE_RECORD_WHOLETIME));
		
		/* update the whole time */
		eBASE_MW_SUBCARD_UpdateTimeTh(&sSubCardInfo, gsFileAttr.lWholeTime);

		/* reset the whole time */
		gsFileAttr.lWholeTime = 0;
	}

	/* subcard overlay type */
	if (COMM_BIT(BASE_RECORD_OVERLAYTYPE) & (gsFileAttr.lRecordCmdType)) {

		BVC_Debug("set overlay type\n");

		/* clear the command */
		gsFileAttr.lRecordCmdType &= (~ COMM_BIT(BASE_RECORD_OVERLAYTYPE));
		
		/* update the overlay type */
		eBASE_MW_SUBCARD_UpdateOverlayType(&sSubCardInfo, gsFileAttr.lOverLayType);

		/* reset the overlay type */
		gsFileAttr.lOverLayType = 0;
	}
	
	/* Response to AVMsgment */
	vCOMM_CF_WtCF(&stCfBaseRet,COMM_COND_CFSYSRETSUC);
	debug_info("[ VencCtrl ] successfully response success to AVMsgment\n");
	
	return BASE_RET_SUCCESS;		
}


/*------------------------------------------------------------------------------*/
/*!
	@brief	Region process

	@retval	LONG						BASE_RET_SUCCESS indicate regioin handle success 
										BASE_RET_FAIL indicate regioin handle error
*/	
/* -----------------------------------------------------------------------------*/
LONG lBASE_VENC_Ctrl_Region_Process()
{
	LONG lRet = BASE_RET_SUCCESS;
	LONG lVencCh = BASE_INVALID_CH;
	
	/* get region updated param */
	eBASE_MW_REGION_GetUpdatedParam(plCurrentChangeParamsArray,&lVencCh,gsRegCov,&gsOverlayAttr);

	/* handle */
	lRet = eBASE_MW_REGION_Handle(lVencCh,gsRegCov,&gsOverlayAttr);
	if(BASE_RET_SUCCESS != lRet)
	{
		debug_info("[%s:%d]:region handle error\n",__FUNCTION__,__LINE__);
		
		/* Response to AVMsgment */
		vCOMM_CF_WtCF(&stCfBaseRet,COMM_COND_CFSYSRETFAIL);
		return BASE_RET_FAIL;
	}
	
	/* Response to AVMsgment */
	vCOMM_CF_WtCF(&stCfBaseRet,COMM_COND_CFSYSRETSUC);
	debug_info("[ VencCtrl ] successfully response success to AVMsgment\n");
	
	return BASE_RET_SUCCESS;	
}


void vBASE_VENC_Ctrl_AttrInit()
{
	/* venc attr init */
	gsVencAttr[BASE_VENC_CH0].lGop = 30;
	gsVencAttr[BASE_VENC_CH0].lFrameRate = BASE_VI_TARGET_FRAMERATE;
	gsVencAttr[BASE_VENC_CH0].enType = PT_H264;
	gsVencAttr[BASE_VENC_CH0].enSize = BASE_RES_HD1080;
	gsVencAttr[BASE_VENC_CH0].enRcMode = BASE_RC_VBR;	

	gsVencAttr[BASE_VENC_CH1].lGop = 30;
	gsVencAttr[BASE_VENC_CH1].lFrameRate = BASE_VI_TARGET_FRAMERATE;
	gsVencAttr[BASE_VENC_CH1].enType = PT_H264;
	gsVencAttr[BASE_VENC_CH1].enSize = BASE_RES_HD720;
	gsVencAttr[BASE_VENC_CH1].enRcMode = BASE_RC_VBR;

	gsVencAttr[BASE_VENC_CH2].lGop = 30;
	gsVencAttr[BASE_VENC_CH2].lFrameRate = BASE_VI_TARGET_FRAMERATE;
	gsVencAttr[BASE_VENC_CH2].enType = PT_H264;
	gsVencAttr[BASE_VENC_CH2].enSize = BASE_RES_SD480;
	gsVencAttr[BASE_VENC_CH2].enRcMode = BASE_RC_VBR;

}
/*------------------------------------------------------------------------------*/
/*!
	@brief	Venc contrl init, include venc relative global parameters init ,system init
    

	@retval	LONG							BASE_RET_SUCCESS indicate venc contrl init success 
											BASE_RET_FAIL indicate venc contrl init error
*/
/* -----------------------------------------------------------------------------*/
LONG lBASE_VENC_Ctrl_VencInit()
{
	LONG s32Ret = BASE_RET_SUCCESS;	
	VENC_CHN VencChn = 0;

#ifdef USE_720P
	geModeFlag = BASE_MODE_VENC720_VONULL_L;
#else
	geModeFlag = BASE_MODE_VENC1080_VONULL_L;
#endif
	/* init global  variable */
	gs_s32Cnt = 3; /* venc chn count */

	vBASE_VENC_Ctrl_AttrInit();

	/* define VB Block size */
	gs_stVbConf.u32MaxPoolCnt = 128;

	for (VencChn = 0; VencChn < gs_s32Cnt; VencChn++)
	{
		s32Ret = lBASE_SYS_DefVb(VencChn * 2, VencChn * 2 + 1, s_eVencSizeTable[ VencChn ], &gs_stVbConf);
		if (BASE_RET_SUCCESS != s32Ret)
		{
			printf("Error Vb\n");
			return BASE_RET_FAIL;
		}
	}
	/* hist buf*/
	gs_stVbConf.astCommPool[3].u32BlkSize = (196*4);
	gs_stVbConf.astCommPool[3].u32BlkCnt = 6;
   
	/* mpp system init. note: in this case, vb cfg is ineffective. */  
	s32Ret = lBASE_SYS_Init(&gs_stVbConf);
	if (BASE_RET_SUCCESS != s32Ret)
	{
		debug_info("%s: system init failed with %ld!\n", __FUNCTION__, s32Ret);
		return BASE_RET_FAIL;
	}

	return BASE_RET_SUCCESS;
}


/*------------------------------------------------------------------------------*/
/*!
	@brief	Venc contrl start,include vi,vo,venc.etc.module start(include init),rtp
			send(send contrl ,update,communicate with AVMsgment)
    

	@retval	LONG							BASE_RET_SUCCESS indicate venc control start success 
											BASE_RET_FAIL indicate venc control start error
*/		
/* -----------------------------------------------------------------------------*/
LONG lBASE_VENC_Ctrl_VencStart()
{
	LONG s32Ret = BASE_RET_SUCCESS;	

	/* init vi prepare */
	lBASE_VI_Init();

	/* init */
	lBASE_MODE_INIT(geModeFlag);

	/* start */
	lBASE_MODE_START();

	/* init venc handle */
	eBASE_VENC_HANDLE_ResInit();

	/* init data consume thread */
	eBASE_VENC_DATACONSUME_Init(&gsDataConsume, &glPoolId[BASE_POOL_VENC], 
		BASE_VENC_CONTROL_V_TIMESECTH, BASE_VENC_CONTROL_M_TIMESECTH, BASE_MW_SD_MAINCARDPATH);	
	// eBASE_VENC_DATACONSUME_StartMConsumer(&gsDataConsume);
	// eBASE_VENC_DATACONSUME_StartVConsumer(&gsDataConsume);

	/* init sub card data */
	eBASE_MW_SUBCARD_Init(&sSubCardInfo, glPoolId[BASE_POOL_VENC], 
		BASE_VENC_CONTROL_SUB_TIMESECTH, 
		BASE_VENC_SUBCARDPATHFIR);
	// eBASE_MW_SUBCARD_StartSigProcess(&sSubCardInfo);

	/* start getstream */
    s32Ret = lBASE_VENC_StartGetStream(gs_s32Cnt);
    if (BASE_RET_SUCCESS != s32Ret)
    {
		debug_info("[%s:%d] Start Venc failed!\n",__FUNCTION__,__LINE__);
		lBASE_VENC_Ctrl_VencStop(BASE_VENC_VENC_STOP);
		return s32Ret;
    }

#ifndef CLOSE_RTP
	/* start realtime handle thread */
	s32Ret = eBASE_VENC_HIGHHANDLE_StartThread(&gsHighThreadArgv);
	if (BASE_RET_SUCCESS != s32Ret) {
		eBASE_VENC_HANDLE_ResDeInit();
		debug_info("[%s:%d] Start realtime thread failed!\n",__FUNCTION__,__LINE__);
		return s32Ret;
	}
#endif

    return BASE_RET_SUCCESS;
}
/*------------------------------------------------------------------------------*/
/*!
	@brief	Venc  stop
	@param	LONG lExitCmd						[in]	The exit command, when vi,vo or venc module start
														error,	system will stop before have inited module.
														Start in this turn:
														SYS->VI->VO->VENC->VENCTGETSTREAM
														Stop will reverse the turn;
														ex:If VO start error,system will stop VI and SYS

	@retval	LONG										BASE_RET_SUCCESS indicate stop successfully
														BASE_RET_FAIL indicate stop failed
*/
/* -----------------------------------------------------------------------------*/
LONG lBASE_VENC_Ctrl_VencStop(eBASE_VENC_STOP_CMD eVencStopCmd)
{
	LONG lRet=BASE_RET_SUCCESS;
	LONG lCnt;
	LONG lRegionTmp;

	switch(eVencStopCmd)
	{
		case BASE_VENC_VENCHANDLE_STOP:
			lRet = eBASE_MW_SUBCARD_Deinit(&sSubCardInfo);
			if(BASE_RET_SUCCESS!=lRet)
			{
				printf("[%s:%d]: stop eBASE_MW_SUBCARD_Deinit error\n",__FUNCTION__,__LINE__);
				return BASE_RET_FAIL;
			}
			
			lRet = eBASE_VENC_HANDLE_ResDeInit();
			if(BASE_RET_SUCCESS!=lRet)
			{
				printf("[%s:%d]: stop HANDLE_ResDeInit error\n",__FUNCTION__,__LINE__);
				return BASE_RET_FAIL;
			}

			#ifndef CLOSE_RTP
			lRet = eBASE_VENC_HIGHHANDLE_StopThread();
			if(BASE_RET_SUCCESS!=lRet)
			{
				printf("[%s:%d]: stop HANDLE_StopRealTimeThread error\n",__FUNCTION__,__LINE__);
				return BASE_RET_FAIL;
			}
			#endif

			lRet = eBASE_VENC_DATACONSUME_Deinit(&gsDataConsume);
			if(BASE_RET_SUCCESS!=lRet)
			{
				printf("[%s:%d]: stop eBASE_VENC_DATACONSUME_Deinit error\n",__FUNCTION__,__LINE__);
				return BASE_RET_FAIL;
			}
			
		case BASE_VENC_VENCTHREAD_STOP:
		case BASE_VENC_GETSTREAM:
			lRet = lBASE_VENC_StopGetStream();
			if(BASE_RET_SUCCESS!=lRet)
			{
				printf("[%s:%d]:StopGetStream error\n",__FUNCTION__,__LINE__);
				return BASE_RET_FAIL;
			}

		case BASE_VENC_REGSTOP:
			for(lCnt = 0;lCnt <BASE_MW_REGION_CH_ALL;lCnt++)
			{
				lRet = eBASE_MW_REGION_STOP(&(gsRegCov[lCnt]));
				if(BASE_RET_SUCCESS!=lRet)
				{
					printf("[%s:%d]:stop region error\n",__FUNCTION__,__LINE__);
					return BASE_RET_FAIL;
				}
			}
	
		case BASE_VENC_OVERLAYSTOP:
			for(lCnt=0;lCnt<3;lCnt++) {
				for(lRegionTmp=0; lRegionTmp!=BASE_MW_OVERLAY_REGIONCNT; lRegionTmp++) {
					if(gsOverlayAttr.sOverlayUse[lRegionTmp].eState[lCnt] != BASE_MW_OVERLAY_ISSTART) 
						continue;
					gsOverlayAttr.lGroupCh=lCnt;
					gsOverlayAttr.lRgnHandle=lRegionTmp;
					lRet=eBASE_MW_OVERLAY_STOP(&gsOverlayAttr);
					if(BASE_RET_SUCCESS!=lRet)
					{
						printf("[%s:%d]:stop region error\n",__FUNCTION__,__LINE__);
						return BASE_RET_FAIL;
					}
				}
			}
			
		case BASE_VENC_VENC_STOP:
			lRet = lBASE_VENC_STOP(gsVenc);
			if(BASE_RET_SUCCESS!=lRet)
			{
				printf("[%s:%d]:venc stop error\n",__FUNCTION__,__LINE__);
				return BASE_RET_FAIL;
			}
			
		case BASE_VENC_VPSS_STOP:
			for ( lCnt = 0; lCnt != BASE_VPSS_CHALL; lCnt ++ ) {
				lBASE_VPSS_STOP_CH(&gsVpss[lCnt]);
			}
			break;
			
		default:
			printf("error venc stop command type\n");
			return BASE_RET_FAIL;
			break;
	}	
	return BASE_RET_SUCCESS;
}
void lBASE_VENC_Ctrl_Exit(){
	/* exit the ctr */
	lBASE_VENC_Ctrl_VencStop(BASE_VENC_VENCHANDLE_STOP);
	lBASE_VI_Deinit();
	vBASE_SYS_Exit();
}

/*------------------------------------------------------------------------------*/
/*!
	@brief	Venc contrl process,include HDMI process,system process.etc.
    

	@retval	LONG							BASE_RET_SUCCESS indicate venc contrl process success 

											BASE_RET_FAIL indicate venc contrl process error
*/
/* -----------------------------------------------------------------------------*/
LONG lBASE_VENC_Ctrl_Process()
{
	LONG lParaType;
	LONG lSystemQuitFlag = 0;

	VENC_CHN VencChn;
 	LONG s32Ret = BASE_RET_SUCCESS;
	eBASE_MW_SD_ERROR eRet = BASE_MW_SD_SUCCESS;
	eBASE_MW_FILESORT_RET eFilesortRet = BASE_MW_FILESORT_SUCCESS;
	LONG lRet;
	
	/* regist the cf */
	vCOMM_CF_RegCF(&stCfBaseRet);
	gsSnap.sFileSortInfo.lFileMaxLen = TFCARD_MAX_FILE_NUM;
	sBASE_MW_SD_STAT* psSDStat = &gsSDStat;
	sBASE_API_LEDCTL_Info* psLedCtlInfo = &gsLedCtlInfo;
	eBASE_API_LEDCTL_LedNewStatus eStatus = BASE_API_LEDCTL_NORMAL;

	//add by yudong
	/************************************************************************/
#ifdef SD_CHECK_DEBUG
	/* check whether enough space */
	lRet = pthread_mutex_lock(&psSDStat->sMutStatus);
	if( (lRet != 0) ){
		BVENC_Ctl_Debug("thread mutex lock failed\n");
	}
	
	if (BASE_MW_SD_SUCCESS != psSDStat->lSdState) 
	{
		eRet = eBASE_SD_MW_GETSPACE(psSDStat, TFCARD, ".jpg", 1);
		if(eRet != BASE_MW_SD_SUCCESS) {
			psSDStat->lSdState = BASE_MW_SD_INVALID;
			psSDStat->eSnapStat= BASE_MW_SNAP_NOREC;			
			BVENC_Ctl_Debug("get space error <%d>\n", eRet);
			eStatus = BASE_API_LEDCTL_SDFAIL;
			eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);			
		} else {
			psSDStat->lSdState = BASE_MW_SD_SUCCESS;
			psSDStat->eSnapStat= BASE_MW_SNAP_REC;		
		}
	}
	
	//Init file sort for snap dir
	if (BASE_MW_SD_SUCCESS == psSDStat->lSdState) {
		eFilesortRet = eBASE_MW_FILESORT_Init(&(gsSnap.sFileSortInfo), gsSnap.sFileSortInfo.lFileMaxLen, RECSNAPPATH);
		if (eFilesortRet == BASE_MW_FILESORT_SUCCESS) {
			printf("--- Snap:eBASE_MW_FILESORT_Init() OK -- \n");
		} else {
			printf("--- Snap:eBASE_MW_FILESORT_Init() Failed -- \n");	
		}
	}	

	lRet = pthread_mutex_unlock(&psSDStat->sMutStatus);
	if( (lRet != 0) ){
		BVENC_Ctl_Debug("thread mutex unlock failed\n");
	}		
#endif

	BVC_Debug("process ready to work !\n");

	/*************************************************************************/

	while(1)    
	{	

		//lParaFlag record Param type
		vCOMM_CF_GetIdCF( &stCfBaseRet, &lParaType, COMM_COND_BLOCK );
		BVC_Debug("Coditionflag = %ld\n", lParaType);

		switch(lParaType)
		{
			case COMM_CMD_SYS:
				/* system process */
				s32Ret = lBASE_VENC_Ctrl_SystemProcess();
				if( BASE_RET_SYSTEMQUIT == s32Ret)
				{
					/* indicate system terminate */
					printf("Base system will exit...\n");
					lSystemQuitFlag = 1;
				}
				break;
				
			case COMM_CMD_VENC0:
			case COMM_CMD_VENC1:
			case COMM_CMD_VENC2:
				/* venc process */
				VencChn = (lParaType - COMM_CMD_VENC0)/COMM_CMDCHSTEP;
				s32Ret = lBASE_VENC_Ctrl_VencProcess(VencChn);
				if (BASE_RET_SUCCESS != s32Ret)	
				{	        
					printf("[%s:%d]: Venc process error!\n", __FUNCTION__,__LINE__);	 
				}
				break;
				
			case COMM_CMD_ETH:
				/* Eth process */
				s32Ret = lBASE_VENC_Ctrl_EthProcess();
				if (BASE_RET_SUCCESS != s32Ret)	
				{	        
					printf("[%s:%d]: Eth process error!\n", __FUNCTION__,__LINE__);
				}
				break;
				
			case COMM_CMD_RECORD:
				/* record process */
				s32Ret = lBASE_VENC_Ctrl_RecordProcess();
				if (BASE_RET_SUCCESS != s32Ret)	
				{	        
					printf("[%s:%d]: Record process error!\n", __FUNCTION__,__LINE__);
				}
				break;	
				
			case COMM_CMD_REG:
				/* region cover process */
				s32Ret = lBASE_VENC_Ctrl_Region_Process();
				if (BASE_RET_SUCCESS != s32Ret)	
				{	        
					printf("[%s:%d]: region cover process error!\n", __FUNCTION__,__LINE__);
				}
				break;

			default:break;				
		}	
		/* judge whether system quit */
		if(1 == lSystemQuitFlag){
			/* indicate system quit */
			break;
		}
	}
	
#ifdef SD_CHECK_DEBUG
	//add by yudong
	if (gsSnap.sFileSortInfo.lState == BASE_MW_FILESORT_Valid) {
		eRet = eBASE_MW_FILESORT_DeInit(&(gsSnap.sFileSortInfo));
		if (eRet == BASE_MW_FILESORT_SUCCESS) {
			BVENC_Ctl_Debug("eBASE_MW_FILESORT_DeInit() OK \n");
		} else {
			BVENC_Ctl_Debug("eBASE_MW_FILESORT_DeInit() Failed\n");
		}		
	}

	/* check whether enough space */
	BVENC_Ctl_Debug("mutex lock enter !\n");
	lRet = pthread_mutex_lock(&psSDStat->sMutStatus);
	if( (lRet != 0) ){
		BVENC_Ctl_Debug("thread mutex lock failed\n");
		s32Ret = BASE_RET_LOCKFAIL;
		return s32Ret;		
	}

	if (BASE_MW_SNAP_REC != psSDStat->eSnapStat) {
		psSDStat->eSnapStat = BASE_MW_SNAP_NOREC;
	}
	lRet = pthread_mutex_unlock(&psSDStat->sMutStatus);
	if( (lRet != 0) ){
		BVENC_Ctl_Debug("thread mutex unlock failed\n");
		s32Ret = BASE_RET_UNLOCKFAIL;
		return s32Ret;		
	}		
	BVENC_Ctl_Debug("mutex lock exit !\n");
#endif

	/* deregist the cf */
	vCOMM_CF_DeregCF(&stCfBaseRet);

	return s32Ret;
}

LONG lBASE_VENC_GetSystemUpdatedParam(LONG *plPara,sBASE_SYSTEM_PARA_ATTR* psSystemAttr)
{

	LONG lCmdID;
	LONG lCmdVal;
	LONG lStrPoolPos = 0;
	
	/* -1 indicate the following data is invalid */
	while(plPara[0] != BASE_INVALID_PARA_ID)
	{		
		/* get valid paramerater ID and value */
		lCmdID = plPara[0];
		lCmdVal = plPara[1];
		//debug_info("\n\nlCmdID = %ld, lCmdVal = %ld\n\n", lCmdID, lCmdVal);
		switch(lCmdID & (COMM_CMDMAXSUBCNT-1))
		{
			case COMM_SUBCMD_SYSQUIT :
				psSystemAttr->lSystemCmdType |= COMM_BIT(BASE_SYSTEM_EXIT);
				debug_info("%s Receive system exit command !\n",__FUNCTION__);
				break;

			case COMM_SUBCMD_SETTIME:
				psSystemAttr->lSystemCmdType |= COMM_BIT(BASE_SYSTEM_SETTIME);
				lStrPoolPos = lCmdVal;
				memset(psSystemAttr->cSetTime, 0, sizeof(psSystemAttr->cSetTime));
				COMM_MW_StrPool_ReadEntry(COMM_GET_STRPOOL(), psSystemAttr->cSetTime, lStrPoolPos);				
				break;

			case COMM_SUBCMD_CLEARSUBCARD :
				psSystemAttr->lSystemCmdType |= COMM_BIT(BASE_SYSTEM_CLEARSUBCARD);
				break;

			case COMM_SUBCMD_WIFIUSERNAME :
				psSystemAttr->lSystemCmdType |= COMM_BIT(BASE_SYSTEM_WIFIUSERNAME);
				lStrPoolPos = lCmdVal;
				memset(psSystemAttr->cWifiUserName, 0, sizeof(psSystemAttr->cWifiUserName));
				COMM_MW_StrPool_ReadEntry(COMM_GET_STRPOOL(), psSystemAttr->cWifiUserName, lStrPoolPos);
				break;

			case COMM_SUBCMD_WIFIPASSWORD :
				psSystemAttr->lSystemCmdType |= COMM_BIT(BASE_SYSTEM_WIFIPASSWORD);
				lStrPoolPos = lCmdVal;
				memset(psSystemAttr->cWifiPassWord, 0, sizeof(psSystemAttr->cWifiPassWord));
				COMM_MW_StrPool_ReadEntry(COMM_GET_STRPOOL(), psSystemAttr->cWifiPassWord, lStrPoolPos);
				break;

			case COMM_SUBCMD_WIFITYPE :
				psSystemAttr->lSystemCmdType |= COMM_BIT(BASE_SYSTEM_WIFITYPE);
				psSystemAttr->lWifiType = lCmdVal;
				break;

			case COMM_SUBCMD_LED :
				BVC_Debug("led sub command, val <%ld>\n", lCmdVal);
				psSystemAttr->lSystemCmdType |= COMM_BIT(BASE_SYSTEM_LED);
				psSystemAttr->lLedStatus = lCmdVal;
				break;

			case COMM_SUBCMD_THREADSTART :
				BVC_Debug("start thread sub command\n");
				psSystemAttr->lSystemCmdType |= COMM_BIT(BASE_SYSTEM_THREADSTART);
				break;				
			default :
				psSystemAttr->lSystemCmdType = BASE_SYSTEM_INVALID;
				break;
		}
		/* Move pointer position to get next paramerater ID and value */
		plPara += 2;
	}

		return BASE_RET_SUCCESS;
}

#ifdef RECONFIGTIME
/* boot config system time */
LONG lBASE_VENC_ConfigTime()
{
	LONG lRet = BASE_RET_SUCCESS;
	LONG lRetVal = 0;
	time_t sTime;
	
	/* check whether it is the first boot */
	if(!access(SAVEFILE_TIMESTAMP, F_OK)){
		/* compare file time and config time */
		lRetVal = lBASE_MW_TIME_CompareTime(&gsTime, SAVEFILE_TIMESTAMP);
		if(lRetVal < 0 ) {
			/* use savefile time */
			eCOMM_UTIL_LoadTimeCalendarfile(SAVEFILE_TIMESTAMP, NULL, &sTime);
			/* set time */
			sTime += CONFIGTIMEDELTA;
			stime(&sTime);
		} else {
			/* use config time */
			eBASE_MW_TIME_ConfigTimeFromCmd(&gsTime);
		}
	} else {
		/* first boot config time*/
		eBASE_MW_TIME_ConfigTimeFromCmd(&gsTime);
	}

	//system("date");
	
	return lRet;
}

/*  reconfig system time after boot config */
LONG lBASE_VENC_ReConfigTime()
{
	LONG lRet = BASE_RET_SUCCESS;
	LONG lRetVal = 0;
	time_t sTime; 

	/* load savefile stamp */
	eCOMM_UTIL_LoadTimeCalendarfile(SAVEFILE_TIMESTAMP, NULL, &sTime);

	/* compare time */
	lRetVal = lBASE_MW_TIME_CompareTime(&gsTime, SAVEFILE_TIMESTAMP);
	if(lRetVal < 0) {
		debug_info("config time is invalid!\n");
		return BASE_RET_FAIL;
	}
	/* set system time */
	lRet = eBASE_MW_TIME_SetTime(&gsTime);	
	if(lRet != BASE_RET_SUCCESS) {
		printf("set time error! err <%ld> \n", lRet);
		return BASE_RET_FAIL;
	}
	//system("date");
	
	return lRet;
}
#endif

LONG lBASE_VENC_SystemParaHandle(sBASE_SYSTEM_PARA_ATTR* psSystemAttr)
{
	LONG lSystemCmdType = psSystemAttr->lSystemCmdType;

	LONG lRet = BASE_RET_SUCCESS;

	CHAR* pcSetTime = NULL;
	static LONG lThreadIsStart = 0;
#ifdef RECONFIGTIME
	static LONG lConfigured = 0;
#endif
	/* system exit process*/
	if(lSystemCmdType & COMM_BIT(BASE_SYSTEM_EXIT))
	{
		BVC_Debug("system exit command\n");
		
		lSystemCmdType &=(~(COMM_BIT(BASE_SYSTEM_EXIT)));
		psSystemAttr->lSystemCmdType = BASE_SYSTEM_INVALID;
		
		/* terminal */
		lBASE_VENC_Ctrl_Exit();	
		//printf("it will be to halt...................\n");
		system("halt");
		return BASE_RET_SYSTEMQUIT;
	}

	/* set time */
	if(lSystemCmdType & COMM_BIT(BASE_SYSTEM_SETTIME)) {
		BVC_Debug("system set time command\n");

		/* clear the command */
		psSystemAttr->lSystemCmdType &= (~ (COMM_BIT(BASE_SYSTEM_SETTIME)));
		
		pcSetTime = psSystemAttr->cSetTime;
		eBASE_MW_TIME_GetTime(&gsTime, pcSetTime);
#ifdef RECONFIGTIME
		if(lConfigured) {
			/* reconfig system time */
			lRet = lBASE_VENC_ReConfigTime();
			/* save set time stamp */
			eCOMM_UTIL_SaveCurCalendarTime2file(PREVSET_TIMESTAMP);
			if(BASE_RET_SUCCESS != lRet)  {
				return lRet;
			}
		} else {
			/* config system time when boot */
			lBASE_VENC_ConfigTime();
			lConfigured = 1;
			/* save set time stamp */
			eCOMM_UTIL_SaveCurCalendarTime2file(PREVSET_TIMESTAMP);
		}
		
#else
		/* configure system time using command */
		eBASE_MW_TIME_ConfigTimeFromCmd(&gsTime);
#endif
	}


	/* clear subcard */
	if (lSystemCmdType & COMM_BIT(BASE_SYSTEM_CLEARSUBCARD)) {

		BVC_Debug("system clear sub card\n");

		/* clear the command */
		psSystemAttr->lSystemCmdType &= (~ (COMM_BIT(BASE_SYSTEM_SETTIME)));

		/* clear the subcard */
		CHAR cClearSubCardCmd[128];
		memset(cClearSubCardCmd, 0, sizeof(cClearSubCardCmd));
		//sprintf(cClearSubCardCmd, "rm -rf %s", BASE_MW_SD_SUBCARDPATH);
		sprintf(cClearSubCardCmd, "rm -rf %s* %s*", gcTargetFileInfo[0],  gcTargetFileInfo[1]);
		system(cClearSubCardCmd);
	}

	/* wifi connect */
	if(lSystemCmdType & COMM_BIT(BASE_SYSTEM_WIFITYPE)) {

		BVC_Debug("system wifitype command\n");

		/* precondiction */
		assert(lSystemCmdType & COMM_BIT(BASE_SYSTEM_WIFIUSERNAME));
		assert(lSystemCmdType & COMM_BIT(BASE_SYSTEM_WIFIPASSWORD));

		/* clear the command */
		psSystemAttr->lSystemCmdType &= (~ (COMM_BIT(BASE_SYSTEM_WIFITYPE)));
		psSystemAttr->lSystemCmdType &= (~ (COMM_BIT(BASE_SYSTEM_WIFIUSERNAME)));
		psSystemAttr->lSystemCmdType &= (~ (COMM_BIT(BASE_SYSTEM_WIFIPASSWORD)));

		BVC_Debug("wifi type : %ld\n", psSystemAttr->lWifiType);
		BVC_Debug("user name : %s\n", psSystemAttr->cWifiUserName);
		BVC_Debug("pass word : %s\n", psSystemAttr->cWifiPassWord);

		/* connect or disconnect the wifi according to wifi type */
		if (psSystemAttr->lWifiType == BASE_MW_BOARDINTERFACE_WIFITYPE_Connect) {
			/* connect */
#ifdef WIFI_ENABLE				
			eBASE_MW_WIFI_Update(&gsWifiInfo, psSystemAttr->cWifiUserName, psSystemAttr->cWifiPassWord);
			eBASE_MW_WIFI_Connect(&gsWifiInfo);
#endif			
		} else {
			/* disconnect */
#ifdef WIFI_ENABLE				
			eBASE_MW_WIFI_Update(&gsWifiInfo, psSystemAttr->cWifiUserName, psSystemAttr->cWifiPassWord);
			eBASE_MW_WIFI_DisConnect(&gsWifiInfo);
#endif			
		}
	}

	/* led */
	if (lSystemCmdType & COMM_BIT(BASE_SYSTEM_LED)) {

		BVC_Debug("led control\n");

		/* clear the command */
		psSystemAttr->lSystemCmdType &= (~ (COMM_BIT(BASE_SYSTEM_LED)));

		LONG lLedStatus = psSystemAttr->lLedStatus;
		switch (lLedStatus) {
			case BASE_VENC_CONTROL_LEDSTATUS_REDBLINK_GREENBLINK :
				/* red blink and green blink */
				BVC_Debug("LED red blink, green blink\n");

				/* change status now */
				sBASE_API_LEDCTL_Info* psLedCtlInfo = &gsLedCtlInfo;
				eBASE_API_LEDCTL_LedNewStatus eStatus = BASE_API_LEDCTL_NOFEEDBACK;
				eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg, eStatus);
			
				break;
			default :
				BVC_Debug("the led status : <%ld> is not supported now\n", lLedStatus);
				return BASE_RET_FAIL;
				break;
		}
	}

	/* thread start */
	if (lSystemCmdType & COMM_BIT(BASE_SYSTEM_THREADSTART)) {

		BVC_Debug("thread start\n");

		/* clear the command */
		psSystemAttr->lSystemCmdType &= (~ (COMM_BIT(BASE_SYSTEM_THREADSTART)));

		/* start the CARDVR core thread */
		if (!lThreadIsStart) {
			eBASE_VENC_DATACONSUME_StartMConsumer(&gsDataConsume);
			eBASE_VENC_DATACONSUME_StartVConsumer(&gsDataConsume);
			eBASE_MW_SUBCARD_StartSigProcess(&sSubCardInfo);
			lThreadIsStart = 1;
		}
		
	}

	return BASE_RET_SUCCESS;

}

LONG lBASE_VENC_GetVencUpdatedParam(LONG *plPara,sBASE_VENC_PARA_ATTR* psVencAttr)
{
	LONG lCmdID;
	LONG lCmdVal;

	/*LONG *pTemp = plPara;
	while(*pTemp != BASE_INVALID_PARA_ID){
		debug_info("\n\nlpTemp = %ld\n\n", *pTemp);
		pTemp++;
	}*/
	/* -1 indicate the following data is invalid */
	while(plPara[0] != BASE_INVALID_PARA_ID)
	{		
		/* get valid paramerater ID and value */
		lCmdID = plPara[0];
		lCmdVal = plPara[1];
		//debug_info("\n\nlCmdID = %ld, lCmdVal = %ld\n\n", lCmdID, lCmdVal);
		switch(lCmdID & (COMM_CMDMAXSUBCNT-1))
		{
			case COMM_SUBCMD_VENCVIDEORESOLUTION :
				break;
			case COMM_SUBCMD_VENCVIDEOENCODERTYPE :
				psVencAttr->enRcMode = lCmdVal;
				psVencAttr->eAttrType |= COMM_BIT(BASE_STATIC_ATTR);
				psVencAttr->eType = BASE_VENC_VIDEO;
				debug_info("%s new encoder type : <%ld>\n", __func__, lCmdVal);
				break;
			case COMM_SUBCMD_VENCVIDEOBITRATE :
				psVencAttr->eType = BASE_VENC_VIDEO;
				break;
			case COMM_SUBCMD_VENCVIDEOFRAMERATE :
				if(lCmdVal<=0 || lCmdVal > BASE_VI_TARGET_FRAMERATE) {
					printf("error to set framerate with value <%ld>\n", lCmdVal);
					return BASE_RET_FAIL;
				}
				psVencAttr->lFrameRate = lCmdVal;
				psVencAttr->eAttrType |= COMM_BIT(BASE_DYNAMIC_ATTR);
				psVencAttr->eType = BASE_VENC_VIDEO;
				debug_info("%s  Receive frameRate  :%ld\n",__FUNCTION__,lCmdVal);
				break;
			case COMM_SUBCMD_VENCVIDEOGOP :
				if(lCmdVal<=0 || lCmdVal > 60) {
					printf("error to set gop with value <%ld>\n", lCmdVal);
					return BASE_RET_FAIL;
				}
				psVencAttr->lGop = lCmdVal;
				psVencAttr->eAttrType |= COMM_BIT(BASE_DYNAMIC_ATTR);
				psVencAttr->eType = BASE_VENC_VIDEO;
				debug_info("%s  Receive gop :%ld\n",__FUNCTION__,lCmdVal);
				break;
			case COMM_SUBCMD_VENCVIDEOCONTROLTYPE :
				break;
			case COMM_SUBCMD_VENCVIDEOPROFILE :
				break;
			case COMM_SUBCMD_VENCVIDEOCOVER :
				break;
			default :
				psVencAttr->eAttrType |= COMM_BIT(BASE_INVALID_ATTR);
				printf("Invalid venc command\n");
				break;
		}
		/* Move pointer position to get next paramerater ID and value */
		plPara += 2;
	}

		return BASE_RET_SUCCESS;

}

LONG lBASE_VENC_ParaHandle(VENC_CHN VencChn,sBASE_VENC_PARA_ATTR* psVencAttr)
{
	LONG lRet = BASE_RET_SUCCESS;
	VENC_CHN_ATTR_S stChnAttr;

	/* deal dynamic attr command,such as gop,framerate.etc. */
	if(COMM_BIT(BASE_DYNAMIC_ATTR)&( psVencAttr->eAttrType))
	{
		/* reset the vencCommand of global buffer*/
		psVencAttr->eAttrType &=(~ COMM_BIT(BASE_DYNAMIC_ATTR));

		/* Get current vencoder attr, merge new attr,set merged attr */
		memset(&stChnAttr,0,sizeof(stChnAttr));
		lRet = HI_MPI_VENC_GetChnAttr(VencChn,&stChnAttr);
		if (BASE_RET_SUCCESS != lRet)
		{
			debug_info("[%s:%d] get channel attr error\n",__FUNCTION__,__LINE__);
			return BASE_RET_FAIL;
		}
		
		lBASE_VENC_ParaToVencPara(&stChnAttr,psVencAttr);
		if (BASE_RET_SUCCESS != lRet)
		{
			debug_info("[%s:%d] modify channel attr error\n",__FUNCTION__,__LINE__);
			return BASE_RET_FAIL;
		}
			
		lRet = HI_MPI_VENC_SetChnAttr(VencChn,&stChnAttr);
		if (BASE_RET_SUCCESS != lRet)
		{
			debug_info("[%s:%d] set channel attr error!\n",__FUNCTION__,__LINE__);
			return BASE_RET_FAIL;
		}
		
	}
	/* deal static attr command,such as encoder attr,need restart,now no support */
	 if(COMM_BIT(BASE_STATIC_ATTR)&( psVencAttr->eAttrType))
	{
		/* reset the vencCommand of global buffer*/
		psVencAttr->eAttrType &=(~ COMM_BIT(BASE_STATIC_ATTR));

		if(psVencAttr->eType==BASE_VENC_VIDEO){
			/* stop the curent venc ,and restart from new venc paramerters*/
			lBASE_VENC_StreamStop(VencChn,BASE_SOFT_STOP);

			lBASE_VENC_StreamStart(VencChn, psVencAttr->enType,psVencAttr->enSize,
								HI_TRUE,psVencAttr->enRcMode,psVencAttr);
			if (BASE_RET_SUCCESS != lRet)
		 	{
				debug_info("[%s:%d] Restart venc start error\n",__FUNCTION__,__LINE__);
				lBASE_VENC_StreamStop(VencChn,BASE_FORCE_STOP);
				return BASE_RET_FAIL;
		 	}
		}
	}
	if(COMM_BIT(BASE_INVALID_ATTR)&( psVencAttr->eAttrType))
	{
		psVencAttr->eAttrType &=(~ COMM_BIT(BASE_INVALID_ATTR));
		printf("[ VNEC ]:invalid command!\n");
	}

	return BASE_RET_SUCCESS;
}

LONG lBASE_VENC_GetRtpUpdatedParam(eBASE_CH_CHANGE* peChangeCh,LONG *plPara,LONG* plVch,LONG* plAch,sBASE_PARA_RTP* psRtpAttr)
{

	LONG lCmdID;
	LONG lCmdVal;

	LONG lCh =BASE_INVALID_CH;

	LONG lPara_RtpIP_tmp;
	CHAR* pcIP;
	struct in_addr addr;

	/*LONG *pTemp = plPara;
	while(*pTemp != BASE_INVALID_PARA_ID){
		debug_info("\n\nlpTemp = %ld\n\n", *pTemp);
		pTemp++;
	}*/
	/* -1 indicate the following data is invalid */
	while(plPara[0] != BASE_INVALID_PARA_ID)
	{		
		/* get valid paramerater ID and value */
		lCmdID = plPara[0];
		lCmdVal = plPara[1];
		//debug_info("\n\nlCmdID = %ld, lCmdVal = %ld\n\n", lCmdID, lCmdVal);
		switch(lCmdID & (COMM_CMDMAXSUBCNT-1))
		{
			case COMM_SUBCMD_ETHVIDEOIPADDRESS :
				if(BASE_INVALID_CH == lCh)
				{
					debug_info("Invalid rtp data\n");
					return BASE_RET_FAIL;	
				}
				
				lPara_RtpIP_tmp = lCmdVal;
				if(lPara_RtpIP_tmp != -1)
				{
					/* Tansform the IP */
					addr.s_addr = (ULONG)lPara_RtpIP_tmp;
					if((pcIP = inet_ntoa(addr)) == NULL)
					{
						debug_info("[%s:%d] video IP error\n",__FUNCTION__,__LINE__);
						return BASE_RET_FAIL;	
					}
					debug_info("%s Receive videi char IP : %s \n",__FUNCTION__,pcIP);
					strncpy(psRtpAttr[lCh].cClientIp,pcIP,COMM_IP_ADDRESS_LENGTH);
				}
				else
				{
					/* "0" indicate no valid ip data ,also indicate no valid data for rtp*/
					strncpy(psRtpAttr[lCh].cClientIp,"0",COMM_IP_ADDRESS_LENGTH);
				}
				psRtpAttr[lCh].lRtpCmdType = BASE_RTP_VIDEO;
				break;	
				
			case COMM_SUBCMD_ETHAUDIOIPADDRESS :
				if(BASE_INVALID_CH == lCh)
				{
					debug_info("Invalid rtp data\n");
					return BASE_RET_FAIL;	
				}
				
				lPara_RtpIP_tmp = lCmdVal;
				if(lPara_RtpIP_tmp != -1)
				{
					/* Tansform the IP */
					addr.s_addr = (ULONG)lPara_RtpIP_tmp;
					if((pcIP = inet_ntoa(addr)) == NULL)
					{
						debug_info("[%s:%d] audio IP error\n",__FUNCTION__,__LINE__);
						return BASE_RET_FAIL;	
					}
					debug_info("%s Receive audio char IP : %s \n",__FUNCTION__,pcIP);
					strncpy(psRtpAttr[lCh].cClientIp,pcIP,COMM_IP_ADDRESS_LENGTH);
				}
				else
				{
					/* "0" indicate no valid ip data ,also indicate no valid data for rtp*/
					strncpy(psRtpAttr[lCh].cClientIp,"0",COMM_IP_ADDRESS_LENGTH);
				}
					
				psRtpAttr[lCh].lRtpCmdType = BASE_RTP_AUDIO;

				*plAch = BASE_AUDIO_CH;
				break;
			case COMM_SUBCMD_ETHVIDEOPORT :
				if(BASE_INVALID_CH == lCh)
				{
					debug_info("Invalid rtp data\n");
					return BASE_RET_FAIL;	
				}
				psRtpAttr[lCh].rmt_port = lCmdVal;
				psRtpAttr[lCh].lRtpCmdType = BASE_RTP_VIDEO;
				debug_info("%s Receive video remote port :%ld\n",__FUNCTION__,lCmdVal);
				break;
				
			case COMM_SUBCMD_ETHAUDIOPORT :
				if(BASE_INVALID_CH == lCh)
				{
					debug_info("Invalid rtp data\n");
					return BASE_RET_FAIL;	
				}
				
				psRtpAttr[lCh].rmt_port = lCmdVal;
				psRtpAttr[lCh].lRtpCmdType = BASE_RTP_AUDIO;
				debug_info("%s Receive audio remote port :%ld\n",__FUNCTION__,lCmdVal);
				break;
				
			case COMM_SUBCMD_ETHVIDEORESOLUTION :
				lCh = lCmdVal>>1;
				psRtpAttr[lCh].eRtpCmd = BASE_RTP_VIDEO_START;
				psRtpAttr[lCh].lRtpCmdType = BASE_RTP_VIDEO;
				//psRtpAttr[lCh].eVencMode= BASE_VENC_NORMAL;
				*peChangeCh = BASE_CH_VENC1080P+lCh;
				debug_info("%s Receive rtp video connect ch[%ld] \n",__FUNCTION__,lCh);
				*plVch = lCh;
				break;
				
			case COMM_SUBCMD_ETHAUDIOSTART:
				lCh = BASE_AUDIO_CH;
				psRtpAttr[lCh].eRtpCmd = BASE_RTP_AUDIO_START;
				psRtpAttr[lCh].lRtpCmdType = BASE_RTP_AUDIO;
				debug_info("%s Receive rtp audio connect \n",__FUNCTION__);
				*plAch = BASE_AUDIO_CH;
				break;
				
			case  COMM_SUBCMD_ETHSTOPRESOLUTION :
				lCh = lCmdVal>>1;
				psRtpAttr[lCh].eRtpCmd = BASE_RTP_VIDEO_STOP;
				psRtpAttr[lCh].lRtpCmdType = BASE_RTP_VIDEO;
				//psRtpAttr[lCh].eVencMode= BASE_VENC_NORMAL;
				//*peChangeCh = BASE_CH_VENC1080P+lCh;
				debug_info("%s Receive rtp video ch[%ld]disconnect\n",__FUNCTION__,lCh);
				*plVch = lCh;
				break;
				
			case COMM_SUBCMD_ETHAUDIOSTOP :
				lCh = BASE_AUDIO_CH;
				psRtpAttr[lCh].eRtpCmd = BASE_RTP_AUDIO_STOP;
				debug_info("%s Receive rtp audio disconnect\n",__FUNCTION__);
				*plAch = BASE_AUDIO_CH;
				break;
			default :
				break;
		}
		/* Move pointer position to get next paramerater ID and value */
		plPara += 2;
	}

		return BASE_RET_SUCCESS;

}

LONG lBASE_VENC_GetRecordUpdatedParam(LONG *plPara,LONG* plACh, LONG* plVCh, sBASE_FILE_PARA_ATTR* psFileAttr)
{
	LONG lCmdID;
	LONG lCmdVal;
	
	/* -1 indicate the following data is invalid */
	while(plPara[0] != BASE_INVALID_PARA_ID)
	{		
		/* get valid paramerater ID and value */
		lCmdID = plPara[0];
		lCmdVal = plPara[1];
		//debug_info("\n\nlCmdID = %ld, lCmdVal = %ld\n\n", lCmdID, lCmdVal);
		switch(lCmdID & (COMM_CMDMAXSUBCNT-1))
		{	
				
			case COMM_SUBCMD_RECSNAP:
				psFileAttr->lRecordCmdType  |=  COMM_BIT(BASE_RECORD_SNAP);
				break;

			case COMM_SUBCMD_SUBCARDREC:
				psFileAttr->lRecordCmdType  |= COMM_BIT(BASE_RECORD_SUBCARD);
				break;

			case COMM_SUBCMD_BUFFER_TIME:
				psFileAttr->lRecordCmdType |= COMM_BIT(BASE_RECORD_BUFFERTIME);
				psFileAttr->lBufferTime = lCmdVal;
				break;

			case COMM_SUBCMD_WHOLE_TIME:
				psFileAttr->lRecordCmdType |= COMM_BIT(BASE_RECORD_WHOLETIME);
				psFileAttr->lWholeTime = lCmdVal;
				break;

			case COMM_SUBCMD_OVERLAY_TYPE:
				psFileAttr->lRecordCmdType |= COMM_BIT(BASE_RECORD_OVERLAYTYPE);
				psFileAttr->lOverLayType = lCmdVal;
				break;
				
			default:
				psFileAttr->lRecordCmd   |=  COMM_BIT(BASE_RECORD_INVALID);
				debug_info("%s Receive invalid record command!\n",__FUNCTION__);
				break;
		}
		/* Move pointer position to get next paramerater ID and value */
		plPara += 2;
	}
	return BASE_RET_SUCCESS;
	
}

LONG lBASE_VENC_ParaToVencPara(VENC_CHN_ATTR_S* psChnAttr,sBASE_VENC_PARA_ATTR* psVencAttr)
{

	switch(psVencAttr->enType)
	{
		case PT_H264:
			if(psVencAttr->enRcMode == BASE_RC_FIXQP)
			{
				debug_info("FIXQP mode\n");
                psChnAttr->stRcAttr.stAttrH264FixQp.u32Gop = psVencAttr->lGop;
                psChnAttr->stRcAttr.stAttrH264FixQp.fr32TargetFrmRate = psVencAttr->lFrameRate;	
			}
			else if(psVencAttr->enRcMode == BASE_RC_CBR)
			{
				debug_info("CBR mode\n");
                psChnAttr->stRcAttr.stAttrH264Cbr.u32Gop            = psVencAttr->lGop;
                psChnAttr->stRcAttr.stAttrH264Cbr.fr32TargetFrmRate = psVencAttr->lFrameRate;/* target frame rate */		
			}
			else if(psVencAttr->enRcMode == BASE_RC_VBR)
			{
				debug_info("VBR mode\n");
                psChnAttr->stRcAttr.stAttrH264Vbr.u32Gop = psVencAttr->lGop;
                psChnAttr->stRcAttr.stAttrH264Vbr.fr32TargetFrmRate = psVencAttr->lFrameRate;
			}
			else
			{
				debug_info("[%s:%d] error RC mode \n",__FUNCTION__,__LINE__);
				return BASE_RET_FAIL;
			}
			
			break;
		
		case PT_MP4VIDEO:
			break;

		case PT_MJPEG:
			break;
		
		case PT_JPEG:
			break;

		default:
			debug_info("[%s:%d] error Encode Type \n",__FUNCTION__,__LINE__);
			break;
	}
	return BASE_RET_SUCCESS;
}

LONG lBASE_VENC_RtpUpdate(LONG lCh,LONG* pRtpCh,LONG* lParamRtpUpdateFlag)
{
		
		//LONG lParaRtpChID;
		LONG lParamType = -1;
		
		LONG lRet = BASE_RET_SUCCESS;

		/* record whether to response to AVMsgment,
		'-1' indicate not receive relative command and no response to AVMsgment */
		*lParamRtpUpdateFlag = -1;
		
		//get paramerater update id
		vCOMM_CF_GetIdCF( &(stCfVencCtrlCh[lCh]), &lParamType, COMM_COND_NOBLOCK );
		
		if( COMM_CMD_ETH == lParamType )
		{
			debug_info("Command Type is ETH%ld !\n",lCh);
			/* notified to this module, reset the id */
			vCOMM_CF_SetIdCF( &(stCfVencCtrlCh[lCh]),
					COMM_COND_CFIDINVALID, COMM_COND_NOBLOCK);
			
			/* handle Rtp parameters  */
			lRet = lBASE_VENC_RtpParaHandle(lCh,pRtpCh,lParamRtpUpdateFlag,&(gsRtpAttr[lCh]));
			if( BASE_RET_SUCCESS != lRet)
			{
				debug_info("[%s:%d] Rtp parameters handle error\n",__FUNCTION__,__LINE__);
				return BASE_RET_FAIL;
			}
		}
				
		return BASE_RET_SUCCESS;
}

LONG lBASE_VENC_RtpServerInit(void** ppv, LONG lCh)
{

	LONG lRet;

	USHORT local_video_rtp_port;
	local_video_rtp_port = GetRtpLocalPort(gsParaRtp,lCh);
	
	debug_info("\n[ VENC ] : RTP  \n ch %ld\nlocal port %d\n\n", lCh, local_video_rtp_port);
		
	/* Rtp initialize */
	lRet = RTPSession_serverinit(ppv, local_video_rtp_port, BASE_RTP_FRE);

	debug_info("[ VENC ] : RTP server init p %p\n", *ppv);
	if( (lRet < 0) ){	
		debug_info("[ VENC ] : RTP  serverinit error!\n");
		return BASE_RET_FAIL;
	}

	/* Set rtp playload type */
	lRet = RTPSession_setplayloadtype(*ppv, BASE_DEFCODEC);
	debug_info("[ VENC ] : RTP  set play load type p %p\n", *ppv);
	if( (lRet < 0) ){
		debug_info("[ VENC ] : RTP  setplayloadtype failed!\n");
		return BASE_RET_FAIL;

	}

	return BASE_RET_SUCCESS;
}

LONG lBASE_VENC_RtpInitFromIplist(LONG* plRtpCh, eBASE_MODEFLAG eModeFlag, sCOMM_IP_LIST* psIPList, sBASE_PARA_RTP* psParaRtp)
{
	LONG lRet=BASE_RET_SUCCESS;
	LONG lLenCnt=0;
	LONG lRtpConnectNum=0;
	void* ppvSession=NULL;

	sCOMM_IPLIST_ARRAY sIplistArry[BASE_IPLIST_CNT];
	sCOMM_IPLIST_ARRAY* psIplistArry=sIplistArry;

	memset(sIplistArry,0,sizeof(sIplistArry));

	/* get array of ip and port from iplist */
	lRet = eCOMM_IPList_ToArray(psIPList, psIplistArry, &lRtpConnectNum);
	if(COMM_IPLIST_SUCCESS != lRet)
	{
		debug_info("IPList is null,cannot recover from iplist!\n");
		return lRet;
	}
	
	/* iplist modify */
	if(eModeFlag&0x01)// if high mode,modify iplist
	{
#ifdef SHOW_INFO
		vCOMM_IPList_Tranversal(psIPList);
#endif
		eCOMM_IPList_ChangeCH_ByArray(psIPList,psIplistArry,lRtpConnectNum,
					BASE_COMM_GETTYPEVAL(eModeFlag,BASE_MODEFLAG_VENC)>>1,BASE_VENC_CH2);
		*plRtpCh = lRtpConnectNum<<(BASE_CH_CNT_SHIFT*(BASE_COMM_GETTYPEVAL(eModeFlag,BASE_MODEFLAG_VENC)>>1));
#ifdef SHOW_INFO
		vCOMM_IPList_Tranversal(psIPList);
#endif
	}
	/* init rtp session from ip and port array */
	for(lLenCnt = 0;lLenCnt < lRtpConnectNum;lLenCnt++)
	{
		 /* get current sesion */
		 if(sIplistArry[lLenCnt].lCh>BASE_VIDEO_CH2)
		 {
		 	continue;
		 }
		 ppvSession = *(GetRtpSession(psParaRtp, sIplistArry[lLenCnt].lCh));
		 /* add ip and port */
		 lRet = RTPSession_addport(ppvSession, 
		 		sIplistArry[lLenCnt].pcIP, sIplistArry[lLenCnt].lPort);
		if (BASE_RET_SUCCESS != lRet)
		{
			debug_info("[%s:%d]:restore to add ip and port error\n",
				__FUNCTION__,__LINE__);
			return BASE_RET_FAIL;
		}
	}
	
	return BASE_RET_SUCCESS;
}

LONG lBASE_VENC_RtpParaHandle(LONG lCh,LONG* pRtpCh,LONG* lParamRtpUpdateFlag,
					sBASE_PARA_RTP* psRtpAttr)
{
		LONG lRet = BASE_RET_SUCCESS;
		
		LONG lRtpPort  = -1;
		CHAR* pcIp = NULL;
		void* pvSession;
		LONG lTmpch;

		/* update client ip and port */
		pcIp =  psRtpAttr->cClientIp;			
		lRtpPort = psRtpAttr->rmt_port;

		/* checki whether ip is valid*/
		if(!strcmp(pcIp,"0"))
		{
			debug_info("[ RTP ]:Invalid IP\n");
			*lParamRtpUpdateFlag = BASE_RTP_VIDEO_STOP;
			return BASE_RET_FAIL;
		}
		
		/* disconnect the rtp connnect of given ip and port */
		if((BASE_RTP_VIDEO_STOP == psRtpAttr->eRtpCmd) ||
			(BASE_RTP_AUDIO_STOP == psRtpAttr->eRtpCmd))
		{				
			//record this channel to response to AVMsgment
			/* video need to control the num of connnection */
			if(BASE_RTP_VIDEO_STOP == psRtpAttr->eRtpCmd)
			{
				*lParamRtpUpdateFlag = BASE_RTP_VIDEO_STOP;
				/* decrease client num */
				glClientNum--;
				if(glClientNum < 0)
				{	
					glClientNum = 0;
					printf("[ RTP ]:No client rtp connect,cannot disconnect\n");
					return BASE_RET_SUCCESS;
				}
			}
			else
			{
				*lParamRtpUpdateFlag = BASE_RTP_AUDIO_STOP;
			}

			if(!((*pRtpCh) & (0xf << (BASE_CH_CNT_SHIFT * lCh))))
			{
				debug_info("[ RTP ]: channel[%ld] have no rtp requrest\n",lCh);
				return BASE_RET_SUCCESS;
			}
			
			/* remove the specific port */
			lRet = lCOMM_IPList_Del(&gIPList, pcIp, lRtpPort);
#ifdef SHOW_INFO
			vCOMM_IPList_Tranversal(&gIPList);			
#endif
			
			/* check the valid. If the ip and port not exit,
			it will retrun -1 */
			if ( BASE_RET_FAIL == lRet )
			{
				printf("[ Warning ] the ip and port is not exit!\n");
				glClientNum++;
				return BASE_RET_SUCCESS;
			}

			/* if valid, delete */
			pvSession = *(GetRtpSession(gsParaRtp,lCh));
			lRet = RTPSession_delport( pvSession, pcIp, lRtpPort);
			if ( BASE_RET_FAIL == lRet )
			{
				debug_info("[ RTP ]:Delete the given ip:%s port %ld error!\n",
							pcIp,lRtpPort);
				return BASE_RET_FAIL;
			}
			
			//decrease the channel connect count
			*pRtpCh -= 1 << (BASE_CH_CNT_SHIFT * lCh) ;
			debug_info("[ RTP ]: Have delete the given ip:%s port %ld !\n",
							pcIp,lRtpPort);
				

		}
		/* connect the rtp of given ip and port */
		else if((BASE_RTP_VIDEO_START == psRtpAttr->eRtpCmd) ||
			(BASE_RTP_AUDIO_START == psRtpAttr->eRtpCmd))
		{
			//record this channel to response to AVMsgment	
			/* video need to control the num of connnection */
			if(BASE_RTP_VIDEO_START == psRtpAttr->eRtpCmd)
			{
				*lParamRtpUpdateFlag = BASE_RTP_VIDEO_START;

				/* increase client num */
				glClientNum++;
				if(glClientNum > BASE_MAX_CLIENT)
				{
					glClientNum = BASE_MAX_CLIENT;
					printf("[ RTP ]:Rtp client num have rechived the max num\n");
#ifdef SHOW_INFO
				vCOMM_IPList_Tranversal(&gIPList);			
#endif
					return BASE_RET_SUCCESS;
				}
			}
			else
			{
				*lParamRtpUpdateFlag = BASE_RTP_AUDIO_START;
			}
			
			/* Add the ip ,ch and port to iplist */
			lRet = lCOMM_IPList_Add(&gIPList, pcIp, lCh, lRtpPort);

			/* check the port whether already in use */
			if ( BASE_RET_SUCCESS != lRet )
			{
				debug_info("[ RTP ] warning : ip %s, port %ld already in use, but to be continue..\n",
					pcIp, lRtpPort);
				
				lTmpch = lCOMM_IPList_Find( &gIPList, pcIp, lRtpPort );
				if ( lTmpch == -1 ){
					debug_info("Error, not find the channel!\n");
					return BASE_RET_FAIL;
				}
				
				debug_info(" will close ch [ %ld ] first\n", lTmpch );	

				/* video need to control the num of connnection */
				if(BASE_RTP_VIDEO_START == psRtpAttr->eRtpCmd)
				{
					/* decrease client num */
					glClientNum--;
					/* This should never exec */
					if(glClientNum < 0)
					{	
						glClientNum = 0;
						printf("[ RTP ]:No client rtp connect,cannot disconnect\n");
						return BASE_RET_FAIL;
					}
				}
				
				/* get been used session and delete the port */
				pvSession = *(GetRtpSession(gsParaRtp,lTmpch));
				lRet = RTPSession_delport(pvSession, pcIp,lRtpPort);
				if (BASE_RET_SUCCESS != lRet)
				{
					debug_info("[ RTP ]:Delte port for ch[%ld] error\n",lTmpch);
				    return BASE_RET_FAIL;
				}
				
				/* decrease the channel connect count.but when modify ch,maybe no
				 client connect,no decress */
				if(((*pRtpCh)&(0xf<<(BASE_CH_CNT_SHIFT * lTmpch)))>0)
				{
					*pRtpCh -= 1 << (BASE_CH_CNT_SHIFT * lTmpch) ;
				}
				
				lCOMM_IPList_Del(&gIPList, pcIp, lRtpPort);
				lCOMM_IPList_Add(&gIPList, pcIp, lCh, lRtpPort);
			}

#ifdef SHOW_INFO
			vCOMM_IPList_Tranversal(&gIPList);			
#endif
			
			/* get current sesion */
			 pvSession = *(GetRtpSession(gsParaRtp,lCh));
			 lRet = RTPSession_addport(pvSession, pcIp,lRtpPort);
			if (BASE_RET_SUCCESS != lRet)
			{
				debug_info("[ RTP ]:add port for ch[%ld] error\n",lCh);
				return BASE_RET_FAIL;
			}
			//increase the channel connect count
			*pRtpCh += 1 << (BASE_CH_CNT_SHIFT * lCh) ;
			
			debug_info("[ RTP ]:add port for ch[%ld] success(pRtpCh = %ld)\n",lCh,*pRtpCh);
		   
		}
		else
		{
			debug_info("Invalid command rtp type\n");
		}
			
		return BASE_RET_SUCCESS;
}

LONG lBASE_SYS_GetPicSize(VIDEO_NORM_E enNorm, eBASE_VENC_PIC_SIZE enPicSize, SIZE_S *pstSize)
{
	switch (enPicSize)
	{
		case BASE_RES_HD1080:
#ifdef TEST_CHANNEL0_720P
		    pstSize->u32Width = 1280; //1920;
		    pstSize->u32Height = 720; //1080;
#else
			pstSize->u32Width = 1920;
			pstSize->u32Height = 1080;
#endif
		    break;
		case BASE_RES_HD720:
		    pstSize->u32Width = 1280;
		    pstSize->u32Height = 720;
		    break;
		case BASE_RES_SD480:
		    pstSize->u32Width = 720;
		    pstSize->u32Height = 480;
		    break;
		default:
			return BASE_RET_FAIL;
	}
	return BASE_RET_SUCCESS;
}

/**************************************************************
 * func : lBASE_MW_COMM_IsISlice(UCHAR* pucHeader, LONG lLen)
 * arg : UCHAR* pucHeader, LONG lLen
 * ret : LONG
 * note : true if I slice, else return false
 *************************************************************/
LONG lBASE_MW_COMM_IsISlice(UCHAR* pucHeader, LONG lLen) {
	LONG lRet = 0;
	UCHAR* pucTmp = pucHeader;
	LONG lPrefixCnt = 0;

	BVC_FuncIn();

	{
		/* skip prefix 00 */
		while(*pucTmp == 0) {
			pucTmp ++;
			lPrefixCnt ++;
		}

		/* check valid */
		assert(*pucTmp == 1);
		assert(lPrefixCnt == 3 || lPrefixCnt ==2);

		/* skip 01 */
		pucTmp ++;

		/* judge */
		if((*pucTmp & BASE_MW_COMM_NALTYPEMASK) != BASE_MW_COMM_NALTYPE_PSLICE) {
			BVC_Debug("found I Slice\n");
			lRet = 1;
		}
	}

	BVC_FuncOut();

	return lRet;
}

#ifdef __cplusplus
}
#endif

