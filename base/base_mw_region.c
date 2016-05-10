#ifdef __cplusplus
	extern "C"{
#endif

#include "base_mw_region.h"
#include "base_mw_modechange.h"

/*two align*/
#define BASE_MW_REGION_RECT(val) ((val)=((val)%2==0)?(val):((val)+1))
eBASE_MW_REGION_ERROR eBASE_MW_REGION_INIT(sBASE_MW_REGION* psReg,  LONG lRegionCh, LONG lViCh){
	eBASE_MW_REGION_ERROR eRet=BASE_MW_REGION_SUCCESS;
	sBASE_MW_REGION_ATTR* psAttr;

	/* init the struct */
	psReg->eState=BASE_MW_REGION_ISUSE;
	psReg->sCoverChn.enModId=HI_ID_VIU;
	psReg->sCoverChn.s32ChnId=lViCh;
	psReg->lRegionCh=lRegionCh;
	psReg->sCoverChn.s32DevId=0;
	psReg->sCovHandle=0+lViCh;

	/* init attribute */
	psAttr=&gsRegCovAttr[lRegionCh];
	SETDEFAULTVAL(psAttr->lShow, 1);
	SETDEFAULTVAL(psAttr->lLayer, 0);
	SETDEFAULTVAL(psAttr->lColor, BASE_MW_REGION_DEFAULT);
	SETDEFAULTVAL(psAttr->sRegRect.lStartX, 0);
	SETDEFAULTVAL(psAttr->sRegRect.lStartY, 0);
	SETDEFAULTVAL(psAttr->sRegRect.lWidth, 160);
	SETDEFAULTVAL(psAttr->sRegRect.lHeight, 160);
	SETDEFAULTVAL(psAttr->lCmdType, 0);

	/* init psReg Attr */
	psReg->psAttr=psAttr;

	return eRet;
}

eBASE_MW_REGION_ERROR eBASE_MW_REGION_getTwoAlignVal(sBASE_MW_REGION_ATTR* psAttr) {
	eBASE_MW_REGION_ERROR eRet=BASE_MW_REGION_SUCCESS;


		 BASE_MW_REGION_RECT(psAttr->sRegRect.lWidth);
		 BASE_MW_REGION_RECT(psAttr->sRegRect.lHeight);
		 BASE_MW_REGION_RECT(psAttr->sRegRect.lStartX);
		 BASE_MW_REGION_RECT(psAttr->sRegRect.lStartY);
		 if(psAttr->sRegRect.lHeight<16)
			psAttr->sRegRect.lHeight=16;
		 if(psAttr->sRegRect.lWidth<16)
			psAttr->sRegRect.lWidth=16;
		 
	
	return eRet;
}

eBASE_MW_REGION_ERROR eBASE_MW_REGION_START(sBASE_MW_REGION* psReg){
	eBASE_MW_REGION_ERROR eRet=BASE_MW_REGION_SUCCESS;
	sBASE_MW_REGION_ATTR* psAttr=psReg->psAttr;

	HI_S32 s32Ret = HI_FAILURE;
	RGN_ATTR_S stCoverAttr;
	RGN_CHN_ATTR_S stCoverChnAttr;

	stCoverAttr.enType = COVER_RGN;
	s32Ret = HI_MPI_RGN_Create(psReg->sCovHandle, &stCoverAttr);
	if(HI_SUCCESS != s32Ret)
	{
		debug_info("Fun:%s line:%u failed with %#x!\n", __FUNCTION__, __LINE__, s32Ret);
		return BASE_MW_REGION_FAIL;
	}
	eBASE_MW_REGION_getTwoAlignVal(psAttr);
	stCoverChnAttr.bShow = psAttr->lShow;
	stCoverChnAttr.enType = COVER_RGN;
	stCoverChnAttr.unChnAttr.stCoverChn.stRect.s32X = psAttr->sRegRect.lStartX;
	stCoverChnAttr.unChnAttr.stCoverChn.stRect.s32Y = psAttr->sRegRect.lStartY;
	stCoverChnAttr.unChnAttr.stCoverChn.stRect.u32Width = psAttr->sRegRect.lWidth;
	stCoverChnAttr.unChnAttr.stCoverChn.stRect.u32Height = psAttr->sRegRect.lHeight;
	stCoverChnAttr.unChnAttr.stCoverChn.u32Color = psAttr->lColor;
	stCoverChnAttr.unChnAttr.stCoverChn.u32Layer = psAttr->lLayer; 	  
	s32Ret = HI_MPI_RGN_AttachToChn(psReg->sCovHandle, &psReg->sCoverChn, &stCoverChnAttr);
	if(HI_SUCCESS != s32Ret)
	{
		debug_info("Fun:%s line:%u failed with %#x!\n", __FUNCTION__, __LINE__, s32Ret);
		return BASE_MW_REGION_FAIL;
	}
	psReg->eState=BASE_MW_REGION_ISSTART;
	return eRet;
}

eBASE_MW_REGION_ERROR eBASE_MW_REGION_CHANGE(sBASE_MW_REGION* psReg){
	eBASE_MW_REGION_ERROR eRet=BASE_MW_REGION_SUCCESS;
	sBASE_MW_REGION_ATTR* psAttr=psReg->psAttr;

	HI_S32 s32Ret = HI_FAILURE;
	RGN_CHN_ATTR_S stCoverChnAttr;

	/* get current display attribute */
	HI_MPI_RGN_GetDisplayAttr(psReg->sCovHandle, &psReg->sCoverChn,  &stCoverChnAttr);
	stCoverChnAttr.bShow = psAttr->lShow;
	stCoverChnAttr.enType = COVER_RGN;
	stCoverChnAttr.unChnAttr.stCoverChn.stRect.s32X = psAttr->sRegRect.lStartX;
	stCoverChnAttr.unChnAttr.stCoverChn.stRect.s32Y = psAttr->sRegRect.lStartY;
	stCoverChnAttr.unChnAttr.stCoverChn.stRect.u32Width = psAttr->sRegRect.lWidth;
	stCoverChnAttr.unChnAttr.stCoverChn.stRect.u32Height = psAttr->sRegRect.lHeight;
	stCoverChnAttr.unChnAttr.stCoverChn.u32Color = psAttr->lColor;
	stCoverChnAttr.unChnAttr.stCoverChn.u32Layer = psAttr->lLayer; 	  

	s32Ret = HI_MPI_RGN_SetDisplayAttr(psReg->sCovHandle, &psReg->sCoverChn, &stCoverChnAttr);
	if(HI_SUCCESS != s32Ret)
	{
		debug_info("Fun:%s line:%u failed with %#x!\n", __FUNCTION__, __LINE__, s32Ret);
		return BASE_MW_REGION_FAIL;
	}

	return eRet;
}

eBASE_MW_REGION_ERROR eBASE_MW_REGION_STOP(sBASE_MW_REGION* psReg)
{
	LONG lRet = BASE_MW_REGION_SUCCESS;
	sBASE_MW_REGION* psRegTmp = psReg;
	
	if(BASE_MW_REGION_ISSTART != psRegTmp->eState)
	{
		debug_info("cover have not start\n");
		return BASE_MW_REGION_SUCCESS;
	}
	psRegTmp->sCovHandle=0+psRegTmp->lRegionCh;
	/* detach */
	lRet = HI_MPI_RGN_DetachFrmChn(psRegTmp->sCovHandle, 
						&(psRegTmp->sCoverChn));
	if(BASE_MW_REGION_SUCCESS != lRet)
	{
		debug_info("[%s:%d]:region detach for vi ch[%d] error,ret=%#lx!\n",__FUNCTION__,
			__LINE__,psRegTmp->sCoverChn.s32ChnId,lRet);
		return BASE_MW_REGION_FAIL;
	}

	/* destory region */
		lRet = HI_MPI_RGN_Destroy(psRegTmp->sCovHandle);
	if(BASE_MW_REGION_SUCCESS != lRet)
	{
		debug_info("[%s:%d]:region destory for vi ch[%d] error,ret=%#lx!\n",__FUNCTION__,
			__LINE__,psRegTmp->sCoverChn.s32ChnId,lRet);
		return BASE_MW_REGION_FAIL;
	}

	psRegTmp->eState = BASE_MW_REGION_NOTUSE;
	
	return BASE_MW_REGION_SUCCESS;
}

eBASE_MW_REGION_ERROR eBASE_MW_REGION_GetUpdatedParam(LONG *plPara,LONG* plVench,sBASE_MW_REGION* psReg,sBASE_MW_OVERLAY_ATTR* psOverlayAttr)
{
	LONG lCmdID;
	LONG lCmdVal;
	LONG lCh = BASE_INVALID_CH;
	psOverlayAttr->sRegRect.lStartX=-1;
	psOverlayAttr->sRegRect.lStartY=-1;
	CHAR* pcOsdString = NULL;
		
	/* -1 indicate the following data is invalid */
	while(plPara[0] != BASE_INVALID_PARA_ID)
	{		
		/* get valid paramerater ID and value */
		lCmdID = plPara[0];
		lCmdVal = plPara[1];
		//debug_info("\n\nlCmdID = %ld, lCmdVal = %ld\n\n", lCmdID, lCmdVal);
		switch(lCmdID & (COMM_CMDMAXSUBCNT-1))
		{
		
			case COMM_SUBCMD_OVERLAYRESOLUTION:	
				lCh = lCmdVal>>1;
				psOverlayAttr->lGroupCh= lCh;
				*plVench = lCh;
				break;

			case COMM_SUBCMD_OVERLAYNUM:
				if(BASE_INVALID_CH == lCh)
				{
					debug_info("Invalid region overlay start\n");
					return BASE_RET_FAIL;	
				}
				psOverlayAttr->lRgnHandle=lCmdVal;
				break;
				
			case COMM_SUBCMD_OVERLAYSTOP:
				if(BASE_INVALID_CH == lCh)
				{
					debug_info("Invalid region overlay stop\n");
					return BASE_RET_FAIL;	
				}
				printf("cover stop\n");
				psOverlayAttr->lCmdType  |= COMM_BIT(BASE_MW_OVERLAY_STOP);
				break;
				
			case COMM_SUBCMD_OVERLAYX:
				if(BASE_INVALID_CH == lCh)
				{
					debug_info("Invalid region overlay ox\n");
					return BASE_RET_FAIL;	
				}
				psOverlayAttr->sRegRect.lStartX= lCmdVal;
				psOverlayAttr->eAttrType=BASE_MW_OVERLAY_DYNAMIC_ATTR;
				psOverlayAttr->eChangeType=BASE_MW_OVERLAY_CHANGE_TYPE_POSITION;
				break;
				
			case COMM_SUBCMD_OVERLAYY:
				if(BASE_INVALID_CH == lCh)
				{
					debug_info("Invalid region overlay oy\n");
					return BASE_RET_FAIL;	
				}
				psOverlayAttr->sRegRect.lStartY= lCmdVal;
				psOverlayAttr->eAttrType=BASE_MW_OVERLAY_DYNAMIC_ATTR;
				psOverlayAttr->eChangeType=BASE_MW_OVERLAY_CHANGE_TYPE_POSITION;
				break;
				
			case COMM_SUBCMD_OVERLAYFRONTALPHA:
				if(BASE_INVALID_CH == lCh)
				{
					debug_info("Invalid region overlay front alpha\n");
					return BASE_RET_FAIL;	
				}
				psOverlayAttr->lFAlpha= lCmdVal;
				psOverlayAttr->eAttrType=BASE_MW_OVERLAY_DYNAMIC_ATTR;
				psOverlayAttr->eChangeType=BASE_MW_OVERLAY_CHANGE_TYPE_FGALPHA;
				break;

			case COMM_SUBCMD_OVERLAYBOTTOMALPHA:
				if(BASE_INVALID_CH == lCh)
				{
					debug_info("Invalid region overlay front alpha\n");
					return BASE_RET_FAIL;	
				}
				psOverlayAttr->lFAlpha= lCmdVal;
				psOverlayAttr->eAttrType=BASE_MW_OVERLAY_DYNAMIC_ATTR;
				psOverlayAttr->eChangeType=BASE_MW_OVERLAY_CHANGE_TYPE_BGALPHA;
				break;
				
			case COMM_SUBCMD_OVERLAYSHOWBMP:
				if(BASE_INVALID_CH == lCh)
				{
					debug_info("Invalid region overlay show bmp\n");
					return BASE_RET_FAIL;	
				}
				psOverlayAttr->lCmdType |=COMM_BIT(BASE_MW_OVERLAY_SHOWBMP);

				/* prepare string */
				pcOsdString = psOverlayAttr->cOsdString;
				memset(pcOsdString, 0, BASE_OSDMAXSTRING);
				COMM_MW_StrPool_ReadEntry(COMM_GET_STRPOOL(), pcOsdString, lCmdVal);
				
				break;

			case COMM_SUBCMD_OVERLAYCHANGEATTR:
				if(BASE_INVALID_CH == lCh)
				{
					debug_info("Invalid region overlay change attr\n");
					return BASE_RET_FAIL;	
				}
				psOverlayAttr->lCmdType  |= COMM_BIT(BASE_MW_OVERLAY_CHANGEATTR);
				break;
				
			case COMM_SUBCMD_OVERLAYSHOWTYPE:
				if(BASE_INVALID_CH == lCh)
				{
					debug_info("Invalid region overlay change attr\n");
					return BASE_RET_FAIL;	
				}
				psOverlayAttr->lShowType= lCmdVal;
				break;
				
			default:
				printf("Invalide region command!\n");
				//return BASE_RET_FAIL;
				break;
		}
		/* Move pointer position to get next paramerater ID and value */
		plPara += 2;
	}

	return BASE_MW_REGION_SUCCESS;
}


eBASE_MW_REGION_ERROR eBASE_MW_REGION_Handle(LONG lVench,sBASE_MW_REGION* psRegCov,sBASE_MW_OVERLAY_ATTR* psOverlayAttr)
{
	eBASE_MW_REGION_ERROR eRet = BASE_MW_REGION_SUCCESS;

	LONG lViCh = BASE_INVALID_CH;
	static LONG lOverlayInitFlag=0;
	LONG lRegionCh=lVench;
	sBASE_MW_REGION* psRegCovTmp = &(psRegCov[lRegionCh]);
	//sBASE_MW_OVERLAY* psOverlayTmp = &(psOverlay[lVencGrp]);
	sBASE_MW_REGION_ATTR* psRegCovAttrTmp = psRegCovTmp->psAttr;
	//sBASE_MW_OVERLAY_ATTR* psOverlayAttrTmp = psOverlayTmp->psOverlayAttr;

	
	/* get vi channel */
	debug_info("-------geModeFlag=%d, venc=%d\n", geModeFlag, lVench);
	eBASE_VI_GETVICH(&geModeFlag,lVench,&lViCh);
	if(BASE_INVALID_CH == lViCh)
	{
		debug_info("[%s:%d]:get vi ch error!\n",__FUNCTION__,
				__LINE__);
		return BASE_MW_REGION_FAIL;
	}
#if 0
	/* start region  process */
	if(COMM_BIT(BASE_MW_COVER_START) & (psRegCovAttrTmp->lCmdType))
	{
		debug_info("cover start process\n");
		psRegCovAttrTmp->lCmdType &= (~ COMM_BIT(BASE_MW_COVER_START));
		if(BASE_MW_REGION_ISSTART == psRegCovTmp->eState){			
			debug_info("[ Warning ]:region cover of current resolution have start,please modify the attribute\n");			
			return BASE_MW_REGION_FAIL;		
		}		
		psRegCovTmp->eState = BASE_MW_REGION_ISSTART;
		eRet = eBASE_MW_REGION_START(psRegCovTmp);
		if(BASE_MW_REGION_SUCCESS != eRet)
		{
			debug_info("[%s:%d]:start region vi ch[%d] error\n",__FUNCTION__,
				__LINE__,psRegCovTmp->sCoverChn.s32ChnId);
			return BASE_MW_REGION_FAIL;
		}
	}

	/* stop region  process */
	if(COMM_BIT(BASE_MW_COVER_STOP) & (psRegCovAttrTmp->lCmdType))
	{
		debug_info("cover stop process\n");
		psRegCovAttrTmp->lCmdType &= (~ COMM_BIT(BASE_MW_COVER_STOP));
		if(BASE_MW_REGION_ISSTART!=psRegCovTmp->eState)
		{
			debug_info("[ Warning ]:region cover of current resolution have not start,can not stop\n");
			return BASE_MW_REGION_FAIL;
		}
		//psRegCovTmp->eState = BASE_MW_REGION_ISUSE;
		eRet = eBASE_MW_REGION_STOP(psRegCovTmp);
		if(BASE_MW_REGION_SUCCESS != eRet)
		{
			debug_info("[%s:%d]:stop region vi ch[%d] error\n",__FUNCTION__,
				__LINE__,psRegCovTmp->sCoverChn.s32ChnId);
			return BASE_MW_REGION_FAIL;
		}
	}

	/* change region attr process */
	if(COMM_BIT(BASE_MW_COVER_CHANGEATTR) & (psRegCovAttrTmp->lCmdType))
	{
		debug_info("change region display attribute process\n");
		psRegCovAttrTmp->lCmdType &= (~ COMM_BIT(BASE_MW_COVER_CHANGEATTR));

		if(BASE_MW_REGION_ISSTART!=psRegCovTmp->eState)
		{
			debug_info("[ Warning ]:region cover of current resolution have not start,can not change attribute\n");
			return BASE_MW_REGION_FAIL;
		}
		eRet = eBASE_MW_REGION_CHANGE(psRegCovTmp);
		if(BASE_MW_REGION_SUCCESS != eRet)
		{
			debug_info("[%s:%d]:change region vi ch[%d] error\n",__FUNCTION__,
				__LINE__,psRegCovTmp->sCoverChn.s32ChnId);
			return BASE_MW_REGION_FAIL;
		}
	}

#endif
	if(COMM_BIT(BASE_MW_OVERLAY_START) & (psOverlayAttr->lCmdType))
	{
		debug_info("overlay start process\n");
		psOverlayAttr->lCmdType &= (~ COMM_BIT(BASE_MW_OVERLAY_START));

		/*init region attr*/
		if(!lOverlayInitFlag){
			eRet=eBASE_MW_OVERLAY_INIT(psOverlayAttr);
			lOverlayInitFlag=1;
		}
		
		eRet = eBASE_MW_OVERLAY_START(psOverlayAttr);
		if(BASE_MW_OVERLAY_SUCCESS != eRet)
		{
			debug_info("[%s:%d]:start region vi ch[%d] error\n",__FUNCTION__,
				__LINE__,psOverlayAttr->sOverlayChn.s32ChnId);
			return BASE_MW_OVERLAY_FAIL;
		}
	}

	/* stop region  process */
	if(COMM_BIT(BASE_MW_OVERLAY_STOP) & (psOverlayAttr->lCmdType))
	{
		debug_info("overlay stop process\n");
		psOverlayAttr->lCmdType &= (~ COMM_BIT(BASE_MW_OVERLAY_STOP));

		/*region have created?*/
		if(psOverlayAttr->sOverlayUse[psOverlayAttr->lRgnHandle].eState[psOverlayAttr->lGroupCh] != BASE_MW_OVERLAY_ISSTART) {
			printf("this region have not created, can't stop!\n");
			return BASE_MW_OVERLAY_FAIL;
		}
		
		eRet = eBASE_MW_OVERLAY_STOP(psOverlayAttr);
		if(BASE_MW_OVERLAY_SUCCESS != eRet)
		{
			debug_info("[%s:%d]:stop region vi ch[%d] error\n",__FUNCTION__,
				__LINE__,psOverlayAttr->sOverlayChn.s32ChnId);
			return BASE_MW_OVERLAY_FAIL;
		}
	}

	/* change region attr process */
	if(COMM_BIT(BASE_MW_OVERLAY_CHANGEATTR) & (psOverlayAttr->lCmdType))
	{
		debug_info("change region display attribute process\n");
		psOverlayAttr->lCmdType &= (~ COMM_BIT(BASE_MW_OVERLAY_CHANGEATTR));
		
		eRet = eBASE_MW_OVERLAY_changeAttr(psOverlayAttr);
		if(BASE_MW_OVERLAY_SUCCESS != eRet)
		{
			debug_info("[%s:%d]:change region vi ch[%ld] error\n",__FUNCTION__,
				__LINE__,psOverlayAttr->lGroupCh);
			return BASE_MW_OVERLAY_FAIL;
		}
	}

	/* show bmp */
	if(COMM_BIT(BASE_MW_OVERLAY_SHOWBMP) & (psOverlayAttr->lCmdType))
	{
		debug_info("show bmp process\n");
		psOverlayAttr->lCmdType &= (~ COMM_BIT(BASE_MW_OVERLAY_SHOWBMP));

		/*init region attr*/
		if(!lOverlayInitFlag){
			eRet=eBASE_MW_OVERLAY_INIT(psOverlayAttr);
			lOverlayInitFlag=1;
		}
		
		eRet = eBASE_MW_OVERLAY_ShowBmp(psOverlayAttr);
		if(BASE_MW_OVERLAY_SUCCESS != eRet)
		{
			debug_info("[%s:%d]:change region vi ch[%ld] error\n",__FUNCTION__,
				__LINE__,psOverlayAttr->lGroupCh);
			return BASE_MW_OVERLAY_FAIL;
		}
	}

	return BASE_MW_REGION_SUCCESS;
}

#ifdef __cplusplus
	}
#endif
