#ifndef __BASE_MW_COMM_H__
#define __BASE_MW_COMM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "base_rtp.h"

/******************************************************
	macro / enum
******************************************************/
#define BASE_INVALID_PARA_ID -1
#define BASE_INVALID_PARA 0
#define BASE_OSDMAXSTRING 128

#define BASE_VENC_CH_NUM 3
#define BASE_VI_TARGET_FRAMERATE			(( geModeFlag & 0xf ) ? 60 : 30 )
#define BASE_VENC_CTRL_VO_ENABLE
#define BASE_CFBASETIMEOUT 70

#define BASE_REG_DEBUG
#define BASE_VENC_CH_NUM 3

/* RTP defination */
#define BASE_RTP_FRE             90000.0
#define BASE_DEFCODEC			96
#define BASE_NFS					30
#define BASE_USC  0
#define BASE_SEC  10
#define BASE_LOCALPORTBASE 6510

#define BASE_CH_CNT_SHIFT 4

/* Client max num */
#define BASE_MAX_CLIENT 5

/* define vi framerate */
#ifdef BASE_SUPPORT60P
#define BASE_VI_FRAMERATE						60
#else
#define BASE_VI_FRAMERATE						30
#endif /* BASE_SUPPORT60P */

#define BASE_COMM_GETTYPEVAL(val, type) ((val>>(8* (type)))&0xff)
#define BASE_GLOBAL_NORM	    VIDEO_ENCODING_MODE_NTSC
#define BASE_SYS_ALIGN_WIDTH      16
#define BASE_PIXEL_FORMAT          PIXEL_FORMAT_YUV_SEMIPLANAR_420

/* rtp */
#define GetRtpSession(ppv,ch) (&((ppv)[(ch)].psess))
/* macro for get rtp local port */
#define GetRtpLocalPort(ppv,ch) ((ppv)[(ch)].loc_port)
#define BASE_IPLIST_CNT 10

/* I slice */
#define BASE_MW_COMM_NALTYPEMASK 0x1f
#define BASE_MW_COMM_NALTYPE_PSLICE 1

typedef enum
{
	BASE_RES_HD1080,
	BASE_RES_HD720,
	BASE_RES_SD480,
}eBASE_VENC_PIC_SIZE;

/* return */
typedef enum {
	BASE_RET_SUCCESS=0,
	BASE_RET_FAIL=-1,
	BASE_RET_SYSTEMQUIT=1,
	BASE_RET_OPENFILEFAIL=-2,
	BASE_RET_LOCKFAIL = 2,
	BASE_RET_UNLOCKFAIL = 3,
	BASE_RET_SD_GETSPACEFAIL = 4,
	BASE_RET_SD_FILESORTFAIL = 5,
} eBASE_RET_STATE;

/* pool type */
typedef enum {
	BASE_POOL_VENC,
	BASE_POOL_ALL
}eBASE_POOl_TYPE;

typedef enum {
	BASE_VENC_SYS_STOP,
	BASE_SNAP_STOP,
	BASE_VENC_VI_STOP,	
	BASE_VENC_VO_STOP,		
	BASE_VENC_VENC_STOP,
	BASE_VENC_VPSS_STOP,
	BASE_VENC_GETSTREAM,
	BASE_VENC_REGSTOP,
	BASE_VENC_OVERLAYSTOP,
	BASE_VENC_VDASTOP,
	BASE_VENC_VENCTHREAD_STOP,
	BASE_VENC_VENCHANDLE_STOP
} eBASE_VENC_STOP_CMD;

/* common variables defination */
typedef enum{
	BASE_COMMU_UID,
	BASE_COMMU_CNT=10,
}eBASE_COMMU;

/*  rtp command  */
typedef enum {
	BASE_RTP_VIDEO_START,
	BASE_RTP_VIDEO_STOP,
	BASE_RTP_AUDIO_START,
	BASE_RTP_AUDIO_STOP,
	BASE_RTP_INVALID
}eBASE_RTP_CMD;

typedef enum 
{
    BASE_RC_CBR = 0,
    BASE_RC_VBR,
    BASE_RC_FIXQP
}eBASE_RC_E;

typedef enum {
	BASE_STATIC_ATTR =1 ,
	BASE_DYNAMIC_ATTR,
	BASE_INVALID_ATTR
}eBASE_ATTR_TYPE;

typedef enum{
	BASE_VENC_VIDEO,
	BASE_VENC_AUDIO,
}eBASE_VENC_TYPE;

typedef enum
{
	BASE_VIDEO_CH0,
	BASE_VIDEO_CH1,
	BASE_VIDEO_CH2,
	BASE_AUDIO_CH,
	BASE_VIDEO_AUDIO_CH_NUM,
	BASE_INVALID_CH = -1
}sBASE_AUDIO_CH;

typedef enum {
	BASE_VICH_BASE,
	BASE_VICH_EXT,
} eBASE_VICH_TYPE;

typedef enum {
	BASE_NOTUSE,
	BASE_ISUSE,
} eBASE_CH_STATE;

typedef enum
{
    BASE_APTINA_AR0130_DC_720P_30FPS = 0,
    BASE_APTINA_9M034_DC_720P_30FPS,
    BASE_SONY_ICX692_DC_720P_30FPS,
    BASE_SONY_IMX104_DC_720P_30FPS,
    BASE_SONY_IMX138_DC_720P_30FPS,
    BASE_SONY_IMX122_DC_1080P_30FPS,
    BASE_SONY_IMX236_DC_1080P_30FPS,
    BASE_OMNI_OV9712_DC_720P_30FPS,
    BASE_APTINA_MT9P006_DC_1080P_30FPS,
    BASE_OMNI_OV2710_DC_1080P_30FPS,
    BASE_SOI_H22_DC_720P_30FPS,
    BASE_HIMAX_1375_DC_720P_30FPS,
    BASE_APTINA_AR0330_DC_1080P_30FPS,
    BASE_PIXELPLUS_3100K_DC_720P_30FPS,
    BASE_SAMPLE_VI_MODE_1_D1,
}eBASE_VI_DEV_TYPE_E;

typedef enum {
	BASE_VI_CH0,
	BASE_VI_CH1,
	BASE_VI_CH2,
	BASE_VI_CH3,
	BASE_VI_EXT_SNAP,
	BASE_VI_CHALL,
} eBASE_VI_CH;

typedef enum {
	BASE_VENC_CH0,
	BASE_VENC_CH1,
	BASE_VENC_CH2,
	BASE_VENC_CHALL,
} eBASE_VENC_CH;

typedef enum {
	BASE_VPSS_CH0,
	BASE_VPSS_CH1,
	BASE_VPSS_CH2,
	BASE_VPSS_CH3,
	BASE_VPSS_CHALL,
} eBASE_VPSS_CH;

typedef enum 
{
    BASE_VI_CHN_SET_NORMAL = 0, /* mirror, filp close */
    BASE_VI_CHN_SET_MIRROR,      /* open MIRROR */
    BASE_VI_CHN_SET_FILP		/* open filp */
}eBASE_VI_CHN_SET_E;

typedef enum {	
	BASE_CH_VO1080P=0,	
	BASE_CH_VO720P,	
	BASE_CH_VENC1080P_H=4,	
	BASE_CH_VENC720P_H,	
	BASE_CH_VENC480P_H,	
	BASE_CH_VENC1080P=8,	
	BASE_CH_VENC720P,	
	BASE_CH_VENC480P,
} eBASE_CH_CHANGE;

/* system command */
typedef enum {
	BASE_SYSTEM_EXIT,
	BASE_SYSTEM_RDREG,
	BASE_SYSTEM_WTREG,
	BASE_SYSTEM_RDMEM,
	BASE_SYSTEM_WTMEM,
	BASE_SYSTEM_RDVIDEO,
	BASE_SYSTEM_LSMSG,
	BASE_SYSTEM_MAILSEND,
	BASE_SYSTEM_GETVIRAWDATA,
	BASE_SYSTEM_SETTIME,
	BASE_SYSTEM_CLEARSUBCARD,
	BASE_SYSTEM_WIFIUSERNAME,
	BASE_SYSTEM_WIFIPASSWORD,
	BASE_SYSTEM_WIFITYPE,
	BASE_SYSTEM_LED,
	BASE_SYSTEM_THREADSTART,
	BASE_SYSTEM_INVALID
}BASE_SYSTEM_CMD;

/* rtp command type */
typedef enum {
	BASE_RTP_VIDEO=1,
	BASE_RTP_AUDIO,
	BASE_RTP_INVALIDTYPE
}eBASE_RTP_CMD_TYPE;

typedef enum {
	BASE_RECORD_VIDEO = 1,
	BASE_RECORD_AUDIO,
	BASE_RECORD_SNAP,
	BASE_RECORD_HLS,
	BASE_RECORD_SUBCARD,
	BASE_RECORD_BUFFERTIME,
	BASE_RECORD_WHOLETIME,
	BASE_RECORD_OVERLAYTYPE,
	BASE_RECORD_INVALIDTYPE
}eBASE_RECORD_CMD_TYPE;

typedef enum {
	BASE_FORCE_STOP = 1,
	BASE_SOFT_STOP
}eBASE_STOP_TYPE;

typedef enum {
	BASE_RECORD_VIDEO_START,
	BASE_RECORD_VIDEO_STOP,
	BASE_RECORD_AUDIO_START,
	BASE_RECORD_AUDIO_STOP,
	BASE_RECORD_HLS_START,
	BASE_RECORD_HLS_STOP,
	BASE_RECORD_INVALID
}eBASE_RECORD_CMD;

typedef enum {
	BASE_TABLE_TYPE_VI,
	BASE_TABLE_TYPE_VPSSGRP,
	BASE_TABLE_TYPE_VPSS,
	BASE_TABLE_TYPE_VENCGRP,
	BASE_TABLE_TYPE_VENC,
	BASE_TABLE_TYPE_MAX,
} eBASE_TABLE_TYPE;

typedef enum {
	BASE_VPSS_TYPE_BASE,
	BASE_VPSS_TYPE_EXT,
} eBASE_VPSS_TYPE;

/******************************************************
	struct
******************************************************/
/* some useful struct */
typedef struct {
	LONG lStartX;
	LONG lStartY;
	LONG lWidth;
	LONG lHeight;
} sBASE_COMM_RECT;

/* rtp parameter */
typedef struct {
	/* RTP command */
	eBASE_RTP_CMD eRtpCmd;
	/* RTP commamnd type */
	LONG lRtpCmdType;
	/* Session */
	void* psess;
	/* local rtp port */
	USHORT loc_port;
	/* remote rtp port */
	USHORT rmt_port;
	/* client ip */
	CHAR cClientIp[COMM_IP_ADDRESS_LENGTH];
}sBASE_PARA_RTP;

/* venc attr */
typedef struct{

	/* video encoder type */
	PAYLOAD_TYPE_E enType;
	/* video bitrate */
	/* video resolution */
	/* video framerate */
	LONG lFrameRate;
	/* video gop */
	LONG lGop;
	/* video control type */
	eBASE_RC_E enRcMode;
	/* video profile */
	LONG lProfile;
	/* video cover */
	/* venc picture size */
	eBASE_VENC_PIC_SIZE enSize;
	/* Attr type */
	eBASE_ATTR_TYPE eAttrType;
	/*record vedio or audio attr*/
	eBASE_VENC_TYPE eType;

}sBASE_VENC_PARA_ATTR;

/* file struct */
typedef struct{
	/* File record start or stop */
	LONG lRecordCmd;
	/* FILE record type */
	LONG lRecordCmdType;
	/* File Record Channel */
	LONG lVideoRecordCh;
	/* Audio Record Channel */
	LONG lAudioRecordCh;
	/* File name */
	CHAR pcFilename[64];
	/* uid */
	LONG lUID[4];
	/* buffer time */
	LONG lBufferTime;
	/* whole time */
	LONG lWholeTime;
	/* overlay type */
	LONG lOverLayType;
} sBASE_FILE_PARA_ATTR;

/* struct for vi vo and venc */
typedef struct {
	eBASE_VICH_TYPE eType;
	VI_DEV ViDev;
	VI_CHN ViChn;
	VI_CHN ViBindCh;
	eBASE_VI_DEV_TYPE_E  eViDevType;
	eBASE_VI_CHN_SET_E eViChnSet;
	eBASE_VENC_PIC_SIZE eSize;
	LONG lSrcFr;
	LONG lTgtFr;
	eBASE_CH_STATE eState;
} sBASE_VIDEO_VI;

typedef struct {
	VPSS_CHN VpssChn;
	VENC_CHN VencChn;
	eBASE_CH_STATE eState;
	sBASE_VENC_PARA_ATTR* psVencAttr;
} sBASE_VIDEO_VENC;

typedef struct {
	VI_CHN ViChn;
	VPSS_CHN VpssChn;
	eBASE_VPSS_TYPE eType;
	eBASE_VENC_PIC_SIZE eSize;
	eBASE_CH_STATE eState;
} sBASE_VIDEO_VPSS;

/* system struct */
typedef struct{
	/*  system type */
	LONG lSystemCmdType;
	/* reg addr */
	ULONG lRdRegStartAddr;
	/* size */
	ULONG lWtRegDateSize;
	/* write addr */
	ULONG lWtRegStartAddr;
	/* write addr */
	ULONG lWtRegVal;
	/* size */
	ULONG lRdRegDateSize;
	/* mem addr */
	ULONG lRdMemAddr;
	/* mem size*/
	ULONG lRdMemSize;
	/* write mem val */
	ULONG lWtMemVal;
	/* write mem addr */
	ULONG lWtMemAddr;
	/* mem size*/
	ULONG lWtMemSize;
	/* video channel */
	LONG lVenCh;
	/* list */
	eCOMM_LIST_MESSAGE eListMsg;
	/* vi raw channel */
	LONG lViRawCh;
	/* vi raw frame number */
	LONG lFrameCnt;
	/* time */
	CHAR cSetTime[64];
	/* wifi user name */
	CHAR cWifiUserName[16];
	/* wifi pass word */
	CHAR cWifiPassWord[16];
	/* wifi type */
	LONG lWifiType;
	/* led value */
	LONG lLedStatus;
}sBASE_SYSTEM_PARA_ATTR;

typedef struct 
{
     HI_BOOL bThreadStart;
     HI_S32  s32Cnt;
}sBASE_VENC_GETSTREAM_PARA_S;

/******************************************************
	variable
******************************************************/
LONG lCurrentChangeParamsArray[2*COMM_CMD_ARRAYSIZE]; // ParamID ParamVal ParamID ParamVal ...
LONG *plCurrentChangeParamsArray;
LONG glCommuBuf[BASE_COMMU_CNT];
sCOMM_CONDFLAG stCfVencCtrlCh[BASE_VENC_CH_NUM];
sCOMM_CONDFLAG stCfAudioCtrl;

/* pool ID */
LONG glPoolId[BASE_POOL_ALL];
sBASE_PARA_RTP gsParaRtp[4];
sBASE_FILE_PARA_ATTR gsFileAttr;
sBASE_PARA_RTP gsRtpAttr[BASE_VIDEO_AUDIO_CH_NUM];
sBASE_VENC_PARA_ATTR gsVencAttr[BASE_VENC_CH_NUM];
sBASE_SYSTEM_PARA_ATTR gsSystemAttr;

/* IP list */
sCOMM_IP_LIST gIPList;

/* global */
sBASE_VIDEO_VI gsVi[BASE_VI_CHALL];
sBASE_VIDEO_VENC gsVenc[BASE_VENC_CHALL];
sBASE_VIDEO_VPSS gsVpss[BASE_VPSS_CHALL];

/******************************************************
	prototype
******************************************************/
__inline__ LONG lBASE_GET_PARAID(LONG lcmd,LONG lopt,LONG lch);
LONG lBASE_VENC_Ctrl_VencStop(eBASE_VENC_STOP_CMD eVencStopCmd);
LONG lBASE_SYS_GetPicSize(VIDEO_NORM_E enNorm, eBASE_VENC_PIC_SIZE enPicSize, SIZE_S *pstSize);

LONG lBASE_VENC_GetSystemUpdatedParam(LONG *plPara,sBASE_SYSTEM_PARA_ATTR* psSystemAttr);
LONG lBASE_VENC_SystemParaHandle(sBASE_SYSTEM_PARA_ATTR* psSystemAttr);
LONG lBASE_VENC_GetVencUpdatedParam(LONG *plPara,sBASE_VENC_PARA_ATTR* psVencAttr);
LONG lBASE_VENC_ParaHandle(VENC_CHN VencChn,sBASE_VENC_PARA_ATTR* psVencAttr);
LONG lBASE_VENC_GetRtpUpdatedParam(eBASE_CH_CHANGE* peChangeCh,LONG *plPara,LONG* plVch,LONG* plAch,sBASE_PARA_RTP* psRtpAttr);
LONG lBASE_VENC_GetRecordUpdatedParam(LONG *plPara,LONG* plACh, LONG* plVCh, sBASE_FILE_PARA_ATTR* psFileAttr);
LONG lBASE_VENC_ParaToVencPara(VENC_CHN_ATTR_S* psChnAttr,sBASE_VENC_PARA_ATTR* psVencAttr);
LONG lBASE_VENC_RtpParaHandle(LONG lCh,LONG* pRtpCh,LONG* lParamRtpUpdateFlag,
					sBASE_PARA_RTP* psRtpAttr);
LONG lBASE_SYS_GetPicSize(VIDEO_NORM_E enNorm, eBASE_VENC_PIC_SIZE enPicSize, SIZE_S *pstSize);
LONG lBASE_MW_COMM_IsISlice(UCHAR* pucHeader, LONG lLen);

#ifdef __cplusplus
}
#endif

#endif
