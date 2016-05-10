#ifndef __BASE_MW_BOARDINTERFACE_H__
#define __BASE_MW_BOARDINTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************
	include
*********************************************************/
#include "base_comm.h"
#include "base_loadbmp.h"

/*********************************************************
	macro / enum
*********************************************************/
typedef enum {
	BASE_MW_BOARDINTERFACE_WIFITYPE_Connect,
	BASE_MW_BOARDINTERFACE_WIFITYPE_Disconnect,
} eBASE_MW_BOARDINTERFACE_WifiType;

/*********************************************************
	struct
*********************************************************/

/*********************************************************
	variable
*********************************************************/

/*********************************************************
	prototype
*********************************************************/
void vBASE_CH_INIT();
void vBASE_VI_INIT_CH(sBASE_VIDEO_VI* psViCh, VI_CHN ViChn, VI_CHN ViBindCh, LONG lTgtFr, eBASE_VENC_PIC_SIZE eSize, eBASE_VICH_TYPE eType);
void vBASE_VENC_INIT_CH(sBASE_VIDEO_VENC* psVencCh, VI_CHN ViChn);
LONG lBASE_VENC_START_CH(sBASE_VIDEO_VENC* psVencCh);
LONG lBASE_VI_StopChn(VI_CHN ViChn);
LONG lBASE_SYS_DefVb(HI_S32 s32VbNum1, HI_S32 s32VbNum2, eBASE_VENC_PIC_SIZE enPicSize, VB_CONF_S *pstVbConf);
LONG lBASE_SYS_Init(VB_CONF_S *pstVbConf);
LONG lBASE_VI_Init();
LONG lBASE_VI_Deinit();
LONG lBASE_VENC_StartGetStream(HI_S32 s32Cnt);
LONG lBASE_VENC_StopGetStream();
LONG lBASE_VI_STOP(sBASE_VIDEO_VI* psViCh);
LONG lBASE_VENC_STOP(sBASE_VIDEO_VENC* psVencCh);
void vBASE_SYS_Exit(void);
LONG lBASE_VENC_StreamStop(VENC_CHN VencChn,LONG lStopType);
LONG lBASE_VENC_StreamStart(VENC_CHN VencChn, PAYLOAD_TYPE_E enType,
												eBASE_VENC_PIC_SIZE enSize,HI_BOOL bVppEnable,
											eBASE_RC_E enRcMode,sBASE_VENC_PARA_ATTR* psVencAttr);
LONG lBASE_GetBmpInfo(const char *filename,     OSD_BITMAPFILEHEADER  *pBmpFileHeader
									,OSD_BITMAPINFO *pBmpInfo);
LONG lBASE_CreateSurfaceByBitMap(const HI_CHAR *pszFileName, OSD_SURFACE_S *pstSurface, HI_U8 *pu8Virt);
LONG lBASE_VPSS_INIT_CH(sBASE_VIDEO_VPSS* psVpss, VI_CHN ViCh, VPSS_CHN VpssCh, eBASE_VPSS_TYPE eType, eBASE_VENC_PIC_SIZE eSize);
LONG lBASE_VPSS_START_CH(sBASE_VIDEO_VPSS* psVpss);
LONG lBASE_VPSS_STOP_CH(sBASE_VIDEO_VPSS* psVpss);

#ifdef __cplusplus
}
#endif

#endif
