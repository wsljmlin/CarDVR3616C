#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************************
	include
**********************************************************************************/
#include "base_mw_modechange.h"
#include "base_venc_handle.h"
#include "base_mw_sd.h"
#include <assert.h>

/*********************************************************************************
	struct
**********************************************************************************/


/*********************************************************************************
	macro
**********************************************************************************/
//#define BASE_VENC_HIGHHANDLE_DEBUG
#ifdef BASE_VENC_HIGHHANDLE_DEBUG
#define BVH_Debug(fmt, args...) fprintf(stdout, "[ VENC HANDLE ] : "fmt, ##args)
#define BVH_FuncIn() //fprintf(stdout, "in %s\n", __func__)
#define BVH_FunOut() //fprintf(stdout, "out %s\n", __func__)
#define BVH_iVal(para) fprintf(stdout, "%s : 0x%x(%d) @ %p\n", #para, (para), (para), &(para))
#define BVH_lVal(para) fprintf(stdout, "%s : <%ld> @ %p\n", #para, (para), &(para))
#define BVH_PtVal(para) fprintf(stdout, "%s : %p\n", #para, (para))
#define BVH_ShowErr(err) fprintf(stderr, "%s <%d> : Error <%d>\n", __func__, __LINE__, err)
#else
#define BVH_Debug(fmt, args...)
#define BVH_FuncIn()
#define BVH_FunOut()
#define BVH_iVal(para)
#define BVH_lVal(para)
#define BVH_PtVal(para)
#define BVH_ShowErr(err)
#endif
/**********************************************************************************
	enum 
**********************************************************************************/

/**********************************************************************************
	variable
**********************************************************************************/

/**********************************************************************************
	prototype
**********************************************************************************/
eBASE_VENC_HANDLE_ERR eBASE_VENC_HIGHHANDLE_RtpInit(sBASE_VENC_HIGHHANDLE_ThreadRtpArgv* psRTPThdArgv);
eBASE_VENC_HANDLE_ERR eBASE_VENC_HANDLE_RtpCmdHandle(LONG* plRtpCh, LONG* plParamRtpUpdateFlag);
eBASE_VENC_HANDLE_ERR eBASE_VENC_HIGHHANDLE_ThreadInit(sBASE_VENC_HIGHHANDLE_ThreadArgv* psThreadArgv);
eBASE_VENC_HANDLE_ERR eBASE_VENC_HIGHHANDLE_ThreadDeInit(sBASE_VENC_HIGHHANDLE_ThreadArgv* psThreadArgv);

void* pvBASE_VENC_HITHHANDLE_DataHandle(void* pvArgv);
void vBASE_VENC_HIGHHANDLE_RtpSend(UCHAR* pucVenBuf, LONG lLen, LONG lVench, sBASE_VENC_HIGHHANDLE_RTPSendArgv* psRtpArgv);
void* pvBASE_VENC_HIGHHANDLE_ThreadHandle(void* pvArgv);

/**********************************************************************************
	extern
**********************************************************************************/
extern LONG lBASE_VENC_RtpUpdate(LONG lCh, LONG* pRtpCh, LONG* lParamRtpUpdateFlag);
extern LONG lBASE_VENC_RtpServerInit(void** ppv, LONG lCh);
extern LONG lBASE_VENC_RtpInitFromIplist(LONG* plRtpCh, eBASE_MODEFLAG eModeFlag, sCOMM_IP_LIST* psIPList, sBASE_PARA_RTP* psParaRtp);


/**********************************************************************************
	function
**********************************************************************************/

/*******************************************************************************
* @brief		: eBASE_VENC_HANDLE_ResInit()
* @param		: NULL
* @retval		: eBASE_VENC_HANDLE_ERR
* @note		: init resource 
*******************************************************************************/
eBASE_VENC_HANDLE_ERR eBASE_VENC_HANDLE_ResInit()
{
	eBASE_VENC_HANDLE_ERR eRet = BASE_RET_SUCCESS;
	BVH_FuncIn();
	{		
		/* init a poll */
		eRet = eCOMM_POOL_Init(&glPoolId[BASE_POOL_VENC], BASE_VENC_HANDLE_VENCBUFSIZEALL,
								BASE_VENC_HANDLE_VENCBUFCNT, BASE_VENC_HANDLE_VENCBUFMAX);
		if(BASE_RET_SUCCESS != eRet)
		{
			BVH_Debug("Pool init error!\n");
		}
		printf("glPoolId[BASE_POOL_VENC]=%ld\n",glPoolId[BASE_POOL_VENC]);
	}

	BVH_FunOut();
	return eRet;
}

/*******************************************************************************
* @brief		: eBASE_VENC_HANDLE_ResDeInit()
* @param		: NULL
* @retval		: eBASE_VENC_HANDLE_ERR
* @note		: deinit resourece
*******************************************************************************/
eBASE_VENC_HANDLE_ERR eBASE_VENC_HANDLE_ResDeInit()
{
	eBASE_VENC_HANDLE_ERR eRet = BASE_RET_SUCCESS;
	BVH_FuncIn();
	{		
		/* init a poll */
		eRet = eCOMM_POOL_Deinit(glPoolId[BASE_POOL_VENC]);
		if(BASE_RET_SUCCESS != eRet)
		{
			BVH_Debug("Pool deinit error ret=%d!\n",eRet);
		}
	}

	BVH_FunOut();
	return eRet;
}
/*******************************************************************************
* @brief		: eBASE_VENC_HIGHHANDLE_RtpInit(sBASE_VENC_HIGHHANDLE_ThreadRtpArgv* psThreadRtpArgv)
* @param		: sBASE_VENC_HIGHHANDLE_ThreadRtpArgv* psThreadRtpArgv
* @retval		: eBASE_VENC_HANDLE_ERR
* @note		: rtp init of start realtime handle thread
*******************************************************************************/
eBASE_VENC_HANDLE_ERR eBASE_VENC_HIGHHANDLE_RtpInit(sBASE_VENC_HIGHHANDLE_ThreadRtpArgv* psThreadRtpArgv)
{
	eBASE_VENC_HANDLE_ERR eRet = BASE_RET_SUCCESS;

	BVH_FuncIn();
	{
		LONG lVencChTmp;
		void** ppvSession = NULL;
		eBASE_MODEFLAG eModeFlag = geModeFlag;
		sBASE_VENC_HIGHHANDLE_ThreadRtpArgv* psArgv = psThreadRtpArgv;
		LONG* plRtpCh = &(psArgv->lRtpCh);
		
		/* rtp server local init */
		psArgv->psRtpPara = gsParaRtp;
		psArgv->psIplist = &gIPList;
		
		for (lVencChTmp = BASE_VIDEO_CH0; lVencChTmp <= BASE_VIDEO_CH2; lVencChTmp++)
		{
			ppvSession = GetRtpSession(psArgv->psRtpPara,lVencChTmp);
			eRet = lBASE_VENC_RtpServerInit(ppvSession, lVencChTmp);
			if (BASE_RET_SUCCESS != eRet)
		    {
		        BVH_Debug("[ VENC ]: RTP init ch[%ld]failed!\n", lVencChTmp);
				continue;
		    }		
		}	
		
		/* rtp init  from iplist */
		eRet = lBASE_VENC_RtpInitFromIplist(plRtpCh, eModeFlag,
				psArgv->psIplist, psArgv->psRtpPara);
		if (BASE_RET_SUCCESS != eRet)
	    {
	    	eRet = BASE_RET_SUCCESS;
	        BVH_Debug(" can not restore session from iplist \n");
	    }

	}
	BVH_FunOut();
	return eRet;
}

/*******************************************************************************
* @brief		: eBASE_VENC_HANDLE_RtpCmdHandle(LONG* plRtpCh,LONG* plParamRtpUpdateFlag)
* @param		: LONG* plRtpCh,LONG* plParamRtpUpdateFlag
* @retval		: eBASE_VENC_HANDLE_ERR
* @note		: rtp command handle
*******************************************************************************/
eBASE_VENC_HANDLE_ERR eBASE_VENC_HANDLE_RtpCmdHandle(LONG* plRtpCh, LONG* plParamRtpUpdateFlag)
{
	eBASE_VENC_HANDLE_ERR eRet = BASE_VENC_HANDLE_SUCCESS;
	LONG lVencChTmp;
	
	BVH_FuncIn();
	{	
		for(lVencChTmp = BASE_VENC_CH0; lVencChTmp <= BASE_VENC_CH2; lVencChTmp++)
		{
			/* get command */
			eRet = lBASE_VENC_RtpUpdate(lVencChTmp,plRtpCh,plParamRtpUpdateFlag);
			/* Response to AVMsgment */
			if((BASE_RTP_VIDEO_STOP == *plParamRtpUpdateFlag)||
				(BASE_RTP_VIDEO_START == *plParamRtpUpdateFlag))
			{
				/* Response to VencCtrl Thread */
				debug_info("[ Video ]: RTP successfully response to VencCtrl Thread\n");
				vCOMM_CF_WtCF( &(stCfVencCtrlCh[lVencChTmp]), COMM_COND_CFSYSRETSUC );
			}
		}
	}
	
	BVH_FunOut();	
	return eRet;
}

/*******************************************************************************
* @brief		: pvBASE_VENC_HIGHHANDLE_ThreadHandle(void* pvArgv)
* @param		: void* pvArgv
* @retval		: NULL
* @note		: realtime handle thread
*******************************************************************************/
void* pvBASE_VENC_HIGHHANDLE_ThreadHandle(void* pvArgv)
{
	eBASE_VENC_HANDLE_ERR eRet;
	sBASE_VENC_HIGHHANDLE_HandleArgv* psRtArgv = (sBASE_VENC_HIGHHANDLE_HandleArgv*)pvArgv;
	
	BVH_FuncIn();
	{	
		LONG lConsKey;
		LONG lPoolId = psRtArgv->lPoolID;
		LONG* pRtpch = &(psRtArgv->sRtpArgv.lRtpCh); 
		LONG lParamRtpUpdateFlag;
		
		eRet = eCOMM_POOL_Attach(lPoolId,&lConsKey, "highhandle");
		if(COMM_POOL_SUCCESS != eRet)
		{
			BVH_Debug(" Attach to Pool %ld error!\n",lPoolId);
		}
		
		while(1)
		{
			/* receive rtp command */
			eRet = eBASE_VENC_HANDLE_RtpCmdHandle(pRtpch, &lParamRtpUpdateFlag);
			if(eRet != BASE_VENC_HANDLE_SUCCESS) 
			{
				BVH_Debug("rtp cmd error!\n");
			}
			/* read video data */
			eRet = eCOMM_POOL_Read(lPoolId, (void*)pvBASE_VENC_HITHHANDLE_DataHandle, (void*)psRtArgv, lConsKey);
			if(eRet == COMM_POOL_NEEDTOQUIT) 
			{
				psRtArgv->eThreadStat=  BASE_VENC_HANDLE_THREADEXIT;
				break;
			}
		}
		BVH_Debug("%s thread exit!\n",__FUNCTION__);
		
	}
	BVH_FunOut();
	
	return NULL;
}

/*******************************************************************************
* @brief		: eBASE_VENC_HIGHHANDLE_ThreadInit(sBASE_VENC_HIGHHANDLE_ThreadArgv* psRealTimeThreadArgv)
* @param		: sBASE_VENC_HIGHHANDLE_ThreadArgv* psThreadArgv
* @retval		: eBASE_VENC_HANDLE_ERR
* @note		: rtp init of realtime handle
*******************************************************************************/
eBASE_VENC_HANDLE_ERR eBASE_VENC_HIGHHANDLE_ThreadInit(sBASE_VENC_HIGHHANDLE_ThreadArgv* psThreadArgv)
{
	eBASE_VENC_HANDLE_ERR eRet = BASE_VENC_HANDLE_SUCCESS;

	BVH_FuncIn();
	{	
		sBASE_VENC_HIGHHANDLE_ThreadArgv* psArgv = psThreadArgv;
		sBASE_VENC_HIGHHANDLE_HandleArgv* psHandleArgv = &(psArgv->sHandleArgv);
		
		psHandleArgv->lPoolID = glPoolId[BASE_POOL_VENC];

		/* rtp para, rtpch record the channel that need rtpsend */
		psArgv->sRtpThreadArgv.lRtpCh = psHandleArgv->sRtpArgv.lRtpCh;
		
		/* rtp init */
		eRet = eBASE_VENC_HIGHHANDLE_RtpInit(&(psArgv->sRtpThreadArgv));
		if(BASE_VENC_HANDLE_SUCCESS != eRet)
		{
			BVH_Debug("eBASE_VENC_HIGHHANDLE_RtpInit error !\n");
		}	
	}
	BVH_FunOut();
	return eRet;
}

/*******************************************************************************
* @brief		: eBASE_VENC_HIGHHANDLE_ThreadDeInit(sBASE_VENC_HIGHHANDLE_ThreadArgv* psThreadArgv)
* @param		: sBASE_VENC_HIGHHANDLE_ThreadArgv* psThreadArgv
* @retval		: eBASE_VENC_HANDLE_ERR
* @note		: rtp deinit of stop realtime thread
*******************************************************************************/
eBASE_VENC_HANDLE_ERR eBASE_VENC_HIGHHANDLE_ThreadDeInit(sBASE_VENC_HIGHHANDLE_ThreadArgv* psThreadArgv)
{
	eBASE_VENC_HANDLE_ERR eRet = BASE_VENC_HANDLE_SUCCESS;

	BVH_FuncIn();
	{	
		sBASE_VENC_HIGHHANDLE_ThreadRtpArgv* psRtpArgv = &(psThreadArgv->sRtpThreadArgv);
		LONG lVencCh;
		void* pSession = NULL;

		/* rtp deinit */
	 	for (lVencCh = BASE_VIDEO_CH0; lVencCh <= BASE_VIDEO_CH2; lVencCh++)
    		{	
	    		pSession = *GetRtpSession(psRtpArgv->psRtpPara, lVencCh);
			if(NULL != pSession)
			{
				RTPSession_destroy(pSession, 10, 0);
			}
	 	}
		
	}
	BVH_FunOut();
	return eRet;
}
/*******************************************************************************
* @brief		: eBASE_VENC_HIGHHANDLE_StartThread(sBASE_VENC_HIGHHANDLE_ThreadArgv* psRealTimeThreadArgv)
* @param		: sBASE_VENC_HIGHHANDLE_ThreadArgv* psThreadArgv
* @retval		: eBASE_VENC_HANDLE_ERR
* @note		: start realtime handle thread
*******************************************************************************/
eBASE_VENC_HANDLE_ERR eBASE_VENC_HIGHHANDLE_StartThread(sBASE_VENC_HIGHHANDLE_ThreadArgv* psThreadArgv)
{
	eBASE_VENC_HANDLE_ERR eRet = BASE_VENC_HANDLE_SUCCESS;

	BVH_FuncIn();
	{	
		sBASE_VENC_HIGHHANDLE_ThreadArgv* psArgv = psThreadArgv;
		sBASE_VENC_HIGHHANDLE_HandleArgv* psHandleArgv = &(psArgv->sHandleArgv);

		eRet = eBASE_VENC_HIGHHANDLE_ThreadInit(psArgv);
		if(BASE_VENC_HANDLE_SUCCESS != eRet)
		{
			BVH_Debug("eBASE_VENC_HIGHHANDLE_ThreadInit error !\n");
			return BASE_VENC_HANDLE_ERROR;
		}
		
		/* create realtime handle thread */
		psHandleArgv->eThreadStat = BASE_VENC_HANDLE_THREADRUNNING;
		eRet = pthread_create(&(psArgv->PidHighHandle), NULL, (void*)pvBASE_VENC_HIGHHANDLE_ThreadHandle, (void*)psHandleArgv);
		if(eRet!=0)
		{
			printf("create pvBASE_VENC_HIGHHANDLE_ThreadHandle error !\n");
		}
		BVH_Debug("create realtime thread sucessfully!\n");
	}
	BVH_FunOut();
	return eRet;
}

/*******************************************************************************
* @brief		: eBASE_VENC_HIGHHANDLE_StopThread()
* @param		: void* pvArgv
* @retval		: NULL
* @note		: rtp handle thread
*******************************************************************************/
eBASE_VENC_HANDLE_ERR eBASE_VENC_HIGHHANDLE_StopThread()
{
	eBASE_VENC_HANDLE_ERR eRet = BASE_VENC_HANDLE_SUCCESS;
	BVH_FuncIn();
	{
		sBASE_VENC_HIGHHANDLE_ThreadArgv* psArgv = &(gsHighThreadArgv);
		if (BASE_VENC_HANDLE_THREADRUNNING == psArgv->sHandleArgv.eThreadStat)
	    {    
			psArgv->sHandleArgv.eThreadStat = BASE_VENC_HANDLE_THREADEXIT;
	    }
		
		pthread_join(psArgv->PidHighHandle, 0);

		/* release resource */
		eRet = eBASE_VENC_HIGHHANDLE_ThreadDeInit(psArgv);
		if(BASE_VENC_HANDLE_SUCCESS != eRet)
		{
			BVH_Debug("realtime thread deinit error!, ret=%d\n",eRet);
			return eRet;
		}
		BVH_Debug("realtimethread exit!\n");
	}
	BVH_FunOut();
	return eRet;
}

/*******************************************************************************
* @brief		: pvBASE_VENC_HITHHANDLE_DataHandle(void* pvArgv)
* @param		: void* pvArgv
* @retval		: NULL
* @note		: the callback of rtp handle,deal the rtp packet send
*******************************************************************************/
void* pvBASE_VENC_HITHHANDLE_DataHandle(void* pvArgv)
{
	sCOMM_POOL_CBFuncArg* psCkArg = (sCOMM_POOL_CBFuncArg*)pvArgv;
	sBASE_VENC_HIGHHANDLE_HandleArgv* psRTArgv = (sBASE_VENC_HIGHHANDLE_HandleArgv*)(psCkArg->pvArg);
	sBASE_VENC_HIGHHANDLE_RTPSendArgv* psRtpSendArgv = &(psRTArgv->sRtpArgv);
	BVH_FuncIn();
	{
		/* get channel,buf,buf len */
		UCHAR* pucVencBuf = psCkArg->pucHeader;
		LONG lVencCh = *pucVencBuf++;
		LONG lVencLen = psCkArg->lLen - 1;
		
		/* rtp handle */
		vBASE_VENC_HIGHHANDLE_RtpSend(pucVencBuf, lVencLen,lVencCh, psRtpSendArgv);
	}
	BVH_FunOut();
	return NULL;
}

/*******************************************************************************
* @brief		: vBASE_VENC_HIGHHANDLE_RtpSend(void* pvArgv)
* @param		: void* pvArgv
* @retval		: NULL
* @note		: rtp send
*******************************************************************************/
void vBASE_VENC_HIGHHANDLE_RtpSend(UCHAR* pucVenBuf, LONG lLen, LONG lVench, sBASE_VENC_HIGHHANDLE_RTPSendArgv* psRtpArgv)
{
	
	BVH_FuncIn();
	{	
		void* pvSess = *GetRtpSession(gsParaRtp,lVench);;
		LONG lFrameRate = gsVencAttr[lVench].lFrameRate;
		eBASE_VENC_HANDLE_ERR* peErr = &(psRtpArgv->eErr);
		*peErr = BASE_VENC_HANDLE_SUCCESS;
		
		/* rtp send */
		if ((psRtpArgv->lRtpCh)&( 0xf << (BASE_CH_CNT_SHIFT * lVench)))
		{
			*peErr = RTPSession_send(pvSess, (int)BASE_RTP_FRE/lFrameRate, (void *)pucVenBuf, lLen);
		}
	}
	BVH_FunOut();
}

#ifdef __cplusplus
}
#endif

