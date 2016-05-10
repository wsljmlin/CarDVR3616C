#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "base_mw_boardinterface.h"
#include "base_mw_modechange.h"
#include <assert.h>

/***************************************************
	macro / enum
***************************************************/
//#define BASE_MW_BOARDINTERFACE_DEBUG
#ifdef BASE_MW_BOARDINTERFACE_DEBUG
#define BMB_Debug(fmt, arg...) fprintf(stdout, "[ BMB ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define BMB_Debug(fmt, arg...)
#endif

#define BMB_FuncIn() BMB_Debug("in\n")
#define BMB_FuncOut() BMB_Debug("out\n")
#define BMB_iVal(iVal) BMB_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define BMB_lVal(lVal) BMB_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define BMB_PtVal(ptVal) BMB_Debug("pt %s @ %p\n", #ptVal, ptVal)

#define GET_VENC_GROUP_FROM_CH(venc_ch) (venc_ch)
#define _BIT(ch) (1<<(ch))

/***************************************************
	variable
***************************************************/
static pthread_t gVencPid;
static SAMPLE_VENC_GETSTREAM_PARA_S gsGetStreamPara;

/* bind table */
static LONG glBindTable[][BASE_TABLE_TYPE_MAX] = {
	/* vi ch,	vpss grp,	vpss ch,	venc grp,	venc ch */
	{0,			0,			0,			0,			0},			// BASE_RES_HD1080
	{0,			0,			1,			0,			1},			// BASE_RES_HD720
	{0,			0,			1,			0,			2},			// BASE_RES_SD480
};

static eBASE_VENC_PIC_SIZE geVencResolution[] = {
	BASE_RES_HD1080,		// venc ch0
	BASE_RES_HD720,		// venc ch1
	BASE_RES_SD480,		// venc ch2
};

static LONG lVpssChannelCurrent = 0;

/* gamma table */
/******************************************
    table 0 is the default gamma,with lowest 
    contrast and best detail in dark area.
    ******************************************/
    static const HI_U16 au16GammaTable0[GAMMA_NODE_NUMBER] = 
        { 0 ,120 ,220 ,310 ,390 ,470 ,540 ,610 ,670 ,730 ,786 ,842 ,894 ,944 ,994 ,1050,    
        1096,1138,1178,1218,1254,1280,1314,1346,1378,1408,1438,1467,1493,1519,1543,1568,    
        1592,1615,1638,1661,1683,1705,1726,1748,1769,1789,1810,1830,1849,1869,1888,1907,    
        1926,1945,1963,1981,1999,2017,2034,2052,2069,2086,2102,2119,2136,2152,2168,2184,    
        2200,2216,2231,2247,2262,2277,2292,2307,2322,2337,2351,2366,2380,2394,2408,2422,    
        2436,2450,2464,2477,2491,2504,2518,2531,2544,2557,2570,2583,2596,2609,2621,2634,    
        2646,2659,2671,2683,2696,2708,2720,2732,2744,2756,2767,2779,2791,2802,2814,2825,    
        2837,2848,2859,2871,2882,2893,2904,2915,2926,2937,2948,2959,2969,2980,2991,3001,    
        3012,3023,3033,3043,3054,3064,3074,3085,3095,3105,3115,3125,3135,3145,3155,3165,    
        3175,3185,3194,3204,3214,3224,3233,3243,3252,3262,3271,3281,3290,3300,3309,3318,    
        3327,3337,3346,3355,3364,3373,3382,3391,3400,3409,3418,3427,3436,3445,3454,3463,    
        3471,3480,3489,3498,3506,3515,3523,3532,3540,3549,3557,3566,3574,3583,3591,3600,    
        3608,3616,3624,3633,3641,3649,3657,3665,3674,3682,3690,3698,3706,3714,3722,3730,    
        3738,3746,3754,3762,3769,3777,3785,3793,3801,3808,3816,3824,3832,3839,3847,3855,    
        3862,3870,3877,3885,3892,3900,3907,3915,3922,3930,3937,3945,3952,3959,3967,3974,    
        3981,3989,3996,4003,4010,4018,4025,4032,4039,4046,4054,4061,4068,4075,4082,4089,4095};

    /******************************************
    table 1 has higher contrast than table 0
    ******************************************/
    static const HI_U16 au16GammaTable1[GAMMA_NODE_NUMBER] = 
        {0,54,106,158,209,259, 308, 356, 403, 450, 495, 540, 584, 628, 670, 713, 754, 795,
        835, 874, 913, 951, 989,1026,1062,1098,1133,1168,1203,1236,1270,1303, 1335,1367,
        1398,1429,1460,1490,1520,1549,1578,1607,1635,1663,1690,1717,1744,1770,1796,1822,
        1848,1873,1897,1922,1946,1970,1993,2017,2040,2062,2085,2107,2129,2150, 2172,2193,
        2214,2235,2255,2275,2295,2315,2335,2354,2373,2392,2411,2429,2447,2465, 2483,2501,
        2519,2536,2553,2570,2587,2603,2620,2636,2652,2668,2684,2700,2715,2731, 2746,2761,
        2776,2790,2805,2819,2834,2848,2862,2876,2890,2903,2917,2930,2944,2957, 2970,2983,
        2996,3008,3021,3033,3046,3058,3070,3082,3094,3106,3118,3129,3141,3152, 3164,3175,
        3186,3197,3208,3219,3230,3240,3251,3262,3272,3282,3293,3303,3313,3323, 3333,3343,
        3352,3362,3372,3381,3391,3400,3410,3419,3428,3437,3446,3455,3464,3473, 3482,3490,
        3499,3508,3516,3525,3533,3541,3550,3558,3566,3574,3582,3590,3598,3606, 3614,3621,
        3629,3637,3644,3652,3660,3667,3674,3682,3689,3696,3703,3711,3718,3725, 3732,3739,
        3746,3752,3759,3766,3773,3779,3786,3793,3799,3806,3812,3819,3825,3831, 3838,3844,
        3850,3856,3863,3869,3875,3881,3887,3893,3899,3905,3910,3916,3922,3928, 3933,3939,
        3945,3950,3956,3962,3967,3973,3978,3983,3989,3994,3999,4005,4010,4015, 4020,4026,
        4031,4036,4041,4046,4051,4056,4061,4066,4071,4076,4081,4085,4090,4095, 4095};

    /******************************************
    table 2 has very high contrast
    ******************************************/
    static const HI_U16 au16GammaTable2[GAMMA_NODE_NUMBER] = 
        {  0,  27,  60, 100, 140, 178,  216, 242, 276, 312, 346,  380, 412, 444, 476, 508,
         540, 572, 604, 636, 667, 698, 729,  760, 791, 822, 853, 884, 915, 945, 975, 1005,
        1035,1065,1095,1125,1155,1185,1215,1245,1275,1305,1335,1365,1395,1425,1455,1485,
        1515,1544,1573,1602,1631,1660,1689,1718,1746,1774,1802,1830,1858,1886,1914,1942,
        1970,1998,2026,2054,2082,2110,2136,2162,2186,2220,2244,2268,2292,2316,2340,2362,
        2384,2406,2428,2448,2468,2488,2508,2528,2548,2568,2588,2608,2628,2648,2668,2688,
        2708,2728,2748,2768,2788,2808,2828,2846,2862,2876,2890,2903,2917,2930,2944,2957,
        2970,2983,2996,3008,3021,3033,3046,3058,3070,3082,3094,3106,3118,3129,3141,3152,
        3164,3175,3186,3197,3208,3219,3230,3240,3251,3262,3272,3282,3293,3303,3313,3323,
        3333,3343,3352,3362,3372,3381,3391,3400,3410,3419,3428,3437,3446,3455,3464,3473,
        3482,3490,3499,3508,3516,3525,3533,3541,3550,3558,3566,3574,3582,3590,3598,3606,
        3614,3621,3629,3637,3644,3652,3660,3667,3674,3682,3689,3696,3703,3711,3718,3725,
        3732,3739,3746,3752,3759,3766,3773,3779,3786,3793,3799,3806,3812,3819,3825,3831,
        3838,3844,3850,3856,3863,3869,3875,3881,3887,3893,3899,3905,3910,3916,3922,3928,
        3933,3939,3945,3950,3956,3962,3967,3973,3978,3983,3989,3994,3999,4005,4010,4015,
        4020,4026,4031,4036,4041,4046,4051,4056,4061,4066,4071,4076,4081,4085,4090,4095,4095};

/***************************************************
	prototype
***************************************************/
void* pvBASE_VENC_GetVencStreamProc(void *pvArg);
LONG lBASE_VENC_HANDLE_Write(LONG lPollId,LONG lVenCh,PAYLOAD_TYPE_E enType,VENC_STREAM_S *pstStream);
LONG lBASE_COMM_VENC_Start(VENC_GRP VencGrp,VENC_CHN VencChn, PAYLOAD_TYPE_E enType, VIDEO_NORM_E enNorm, PIC_SIZE_E enSize, SAMPLE_RC_E enRcMode, sBASE_VENC_PARA_ATTR* psVencAttr);
LONG lBASE_SYS_CalcPicVbBlkSize(VIDEO_NORM_E enNorm, eBASE_VENC_PIC_SIZE enPicSize, PIXEL_FORMAT_E enPixFmt, HI_U32 u32AlignWidth);
LONG lBASE_COMM_VENC_BindVpss(VENC_GRP GrpChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn);
LONG lBASE_COMM_VENC_UnBindVpss(VENC_GRP GrpChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn);
LONG lBASE_COMM_START_VPSSGROUP();
LONG lBASE_COMM_STOP_VPSSGROUP();
LONG lBASE_COMM_START_VPSS_BASE(sBASE_VIDEO_VPSS* psVpss);
LONG lBASE_COMM_START_VPSS_EXT(sBASE_VIDEO_VPSS* psVpss);
LONG lBASE_COMM_JUDGE_ISLICE(VENC_STREAM_S *pstStream);
LONG lBASE_COMM_VENC_ADJUSTISP();
LONG lBASE_COMM_VENC_ADJUSTISP_DRC();
LONG lBASE_COMM_VENC_ADJUSTISP_LDC();
LONG lBASE_COMM_VENC_ADJUSTISP_AE_BASIC();
LONG lBASE_COMM_VENC_ADJUSTISP_AE_STATIC();
LONG lBASE_COMM_VENC_ADJUSTISP_AE_EX();
LONG lBASE_COMM_VENC_ADJUSTISP_GAMMA();
LONG lBASE_COMM_VENC_ADJUSTISP_ANTICOLOR();
LONG lBASE_COMM_VENC_ADJUSTISP_CSC();

#if 1
void vBASE_COMM_VENC_ISPGetModuleControl() ;
void vBASE_COMM_VENC_ISPGetAEPara() ;
void vBASE_COMM_VENC_ISPGetAWBPara();
void  vISPGetAntiFlicker();
void vBASE_COMM_VENC_ISPGetDRC() ;
void vBASE_COMM_VENC_ISPGetAntiFog();
void vBASE_COMM_VENC_ISPGetDIS() ;
void vBASE_COMM_VENC_ISPGetBlackLevel();
void vBASE_COMM_VENC_ISPSetDRC();
void vBASE_COMM_VENC_ISPSetAntiFlicker() ;
void vBASE_COMM_VENC_ISPSetAEAttr();
void vBASE_COMM_VENC_ISPSetAWBAttr();
void vBASE_COMM_VENC_ISPSetDenoise();
void vBASE_COMM_VENC_ISPSetAntiFog();


#endif

/***************************************************
	function
***************************************************/
void vBASE_CH_INIT() {
	memset(gsVenc, 0, sizeof(gsVenc));
	memset(gsVpss, 0, sizeof(gsVpss));
}

void vBASE_VI_INIT_CH(sBASE_VIDEO_VI* psViCh, VI_CHN ViChn, VI_CHN ViBindCh, LONG lTgtFr, eBASE_VENC_PIC_SIZE eSize, eBASE_VICH_TYPE eType) {
	/* init vi parameter */
	psViCh->eType=eType;
	psViCh->eViChnSet=BASE_VI_CHN_SET_NORMAL;
	psViCh->eViDevType=SENSOR_TYPE;
	psViCh->lSrcFr=BASE_VI_FRAMERATE;
	psViCh->lTgtFr=lTgtFr;
	psViCh->eSize=eSize;
	psViCh->ViBindCh=ViBindCh;
	psViCh->ViChn=ViChn;
	psViCh->ViDev=0;
	psViCh->eState=BASE_ISUSE;
}

void vBASE_VENC_INIT_CH(sBASE_VIDEO_VENC* psVencCh, VENC_CHN VencChn) {
	
	/* init venc parameter */
	psVencCh->VencChn=VencChn;
	psVencCh->psVencAttr=&gsVencAttr[VencChn];
	psVencCh->eState=BASE_ISUSE;
	
}

LONG lBASE_VENC_START_CH(sBASE_VIDEO_VENC* psVencCh){
	LONG lRet=BASE_RET_SUCCESS;
	sBASE_VENC_PARA_ATTR* psVencAttr = psVencCh->psVencAttr;
	VENC_CHN VencChn = psVencCh->VencChn;

	lRet = lBASE_VENC_StreamStart(VencChn, psVencAttr->enType,psVencAttr->enSize,
							HI_TRUE,psVencAttr->enRcMode,psVencAttr);
	if (BASE_RET_SUCCESS != lRet)
	{
		/* close venc channel */
		lBASE_VENC_Ctrl_VencStop(BASE_VENC_VENC_STOP);
		return lRet;
	}

	/* toggle the state */
	psVencCh->eState = BASE_ISUSE;
			
	return lRet;

}

LONG lBASE_VI_StopChn(VI_CHN ViChn) {
	LONG lRet=BASE_RET_SUCCESS;
			
	return lRet;
}
LONG lBASE_SYS_DefVb(HI_S32 s32VbNum1, HI_S32 s32VbNum2, eBASE_VENC_PIC_SIZE enPicSize, VB_CONF_S *pstVbConf) {
	LONG lRet=BASE_RET_SUCCESS;
	LONG lVencCh = s32VbNum1 / 2;
	LONG lBlockSize;
	struct hiVB_CPOOL_S * psVbPool;

	lBlockSize = lBASE_SYS_CalcPicVbBlkSize(BASE_GLOBAL_NORM, enPicSize, BASE_PIXEL_FORMAT, BASE_SYS_ALIGN_WIDTH);
	psVbPool = &(pstVbConf->astCommPool[ lVencCh ]);
	psVbPool->u32BlkSize = lBlockSize;

	switch(enPicSize) {
		case BASE_RES_HD1080 :
			psVbPool->u32BlkCnt = 10;
			break;
		case BASE_RES_HD720 :
		case BASE_RES_SD480 :
			psVbPool->u32BlkCnt = 6;
			break;
		default :
			assert(0);
			break;
	}
	
	return lRet;
}

LONG lBASE_SYS_Init(VB_CONF_S *pstVbConf) {
	LONG lRet=BASE_RET_SUCCESS;
	HI_S32 s32Ret = HI_SUCCESS;

	s32Ret = SAMPLE_COMM_SYS_Init(pstVbConf);
	if(s32Ret != HI_SUCCESS) {
		return BASE_RET_FAIL;
	}
	
	return lRet;
}

LONG lBASE_VI_Init() {
	LONG lRet=BASE_RET_SUCCESS;
	
	/* start vi */
	SAMPLE_VI_CONFIG_S stViConfig;
	stViConfig.enViMode = SENSOR_TYPE;
	stViConfig.enRotate = ROTATE_NONE;
	stViConfig.enNorm = VIDEO_ENCODING_MODE_AUTO;
	stViConfig.enViChnSet = VI_CHN_SET_NORMAL;

	lRet = SAMPLE_COMM_VI_StartVi(&stViConfig);
	if (lRet != 0) {
		printf("error with SAMPLE_COMM_VI_StartVi\n");
		return lRet;
	}

	/* adjust isp */
	lBASE_COMM_VENC_ADJUSTISP();
			
	return lRet;
}

LONG lBASE_VI_Deinit() {
	LONG lRet=BASE_RET_SUCCESS;

	SAMPLE_VI_CONFIG_S stViConfig;
	stViConfig.enViMode = SENSOR_TYPE;
	stViConfig.enRotate = ROTATE_NONE;
	stViConfig.enNorm = VIDEO_ENCODING_MODE_AUTO;
	stViConfig.enViChnSet = VI_CHN_SET_NORMAL;

	lRet = SAMPLE_COMM_VI_StopVi(&stViConfig);
			
	return lRet;
}

LONG lBASE_VENC_StartGetStream(HI_S32 s32Cnt) {
	LONG lRet=BASE_RET_SUCCESS;

	gsGetStreamPara.bThreadStart = HI_TRUE;
	gsGetStreamPara.s32Cnt = s32Cnt;

    lRet = pthread_create(&gVencPid, NULL, pvBASE_VENC_GetVencStreamProc, (HI_VOID*)&gsGetStreamPara);

	return lRet;
}

LONG lBASE_VENC_StopGetStream() {
	LONG lRet=BASE_RET_SUCCESS;
	
	if (HI_TRUE == gsGetStreamPara.bThreadStart)
	{
		gsGetStreamPara.bThreadStart = HI_FALSE;       
	}
	
	pthread_join(gVencPid, 0);

	return lRet;
}

LONG lBASE_VI_STOP(sBASE_VIDEO_VI* psViCh) {
	LONG lRet=BASE_RET_SUCCESS;
			
	return lRet;
}

LONG lBASE_VENC_STOP(sBASE_VIDEO_VENC* psVencCh) {

	LONG lRet = BASE_RET_SUCCESS;

	LONG lChcnt;
	sBASE_VIDEO_VENC* psVencChTmp = NULL;

	for(lChcnt = BASE_VENC_CH0; lChcnt < BASE_VENC_CHALL; lChcnt++)
	{
		psVencChTmp = psVencCh + lChcnt;
		if(BASE_NOTUSE != psVencChTmp->eState)
		{
			lRet = lBASE_VENC_StreamStop(psVencChTmp->VencChn,BASE_FORCE_STOP);
			if(BASE_RET_SUCCESS != lRet)
			{
				printf("[%s:%d]:stop venc ch[%ld] error,ret=%#lx\n",\
					__FUNCTION__,__LINE__,(LONG)(psVencChTmp->VencChn),lRet);
				return BASE_RET_FAIL;
			}
			
			psVencChTmp->eState = BASE_NOTUSE;
		}
		
		psVencChTmp++;
	}
	
	return lRet;

}

void vBASE_SYS_Exit(void) {
	SAMPLE_COMM_SYS_Exit();
	return ;
}

LONG lBASE_VENC_StreamStop(VENC_CHN VencChn,LONG lStopType) {
	LONG lRet=BASE_RET_SUCCESS;
	VENC_GRP VencGrp;
	eBASE_VENC_PIC_SIZE ePicSize;
	LONG* plChannel;

	ePicSize = geVencResolution[ VencChn ];
	plChannel = glBindTable[ ePicSize ];
	
	VencGrp = plChannel[BASE_TABLE_TYPE_VPSSGRP];
	lBASE_COMM_VENC_UnBindVpss(
		plChannel[BASE_TABLE_TYPE_VENCGRP],
		plChannel[BASE_TABLE_TYPE_VPSSGRP],
		plChannel[BASE_TABLE_TYPE_VPSS]
	);
	
	SAMPLE_COMM_VENC_Stop(VencGrp, VencChn);
	
	return lRet;
}

LONG lBASE_VENC_StreamStart(VENC_CHN VencChn, PAYLOAD_TYPE_E enType,
												eBASE_VENC_PIC_SIZE enSize,HI_BOOL bVppEnable,
											eBASE_RC_E enRcMode,sBASE_VENC_PARA_ATTR* psVencAttr) {
	LONG lRet=BASE_RET_SUCCESS;
	VENC_GRP VencGrp;
	LONG* plChannel;

	plChannel = glBindTable[psVencAttr->enSize];
	
	VencGrp = plChannel[BASE_TABLE_TYPE_VPSSGRP];
	lRet = lBASE_COMM_VENC_Start(VencGrp, VencChn, enType, BASE_GLOBAL_NORM, enSize, enRcMode, psVencAttr);
	lRet = lBASE_COMM_VENC_BindVpss(plChannel[BASE_TABLE_TYPE_VENCGRP],
				plChannel[BASE_TABLE_TYPE_VPSSGRP],
				plChannel[BASE_TABLE_TYPE_VPSS]);

	return lRet;
}

LONG lBASE_GetBmpInfo(const char *filename,     OSD_BITMAPFILEHEADER  *pBmpFileHeader
,OSD_BITMAPINFO *pBmpInfo) {
	LONG lRet=BASE_RET_SUCCESS;
	HI_S32 s32Ret = HI_SUCCESS;

	s32Ret = GetBmpInfo(filename, pBmpFileHeader, pBmpInfo);
	if(s32Ret != HI_SUCCESS) {
		return BASE_RET_FAIL;
	} 

	return lRet;
}

LONG lBASE_CreateSurfaceByBitMap(const HI_CHAR *pszFileName, OSD_SURFACE_S *pstSurface, HI_U8 *pu8Virt) {
	LONG lRet=BASE_RET_SUCCESS;
	HI_S32 s32Ret = HI_SUCCESS;

	s32Ret = CreateSurfaceByBitMap(pszFileName, pstSurface, pu8Virt);
	if(s32Ret != HI_SUCCESS) {
		return BASE_RET_FAIL;
	} 

	return lRet;
}

void* pvBASE_VENC_GetVencStreamProc(void *pvArg)
{
	LONG lRet;
	
	LONG s32ChnTotal;
	sBASE_VENC_GETSTREAM_PARA_S *pstPara;
	VENC_CHN_STAT_S stStat;
	VENC_STREAM_S stStream;
	
	LONG maxfd = 0;
	struct timeval TimeoutVal;
	fd_set read_fds;
	LONG VencFd[VENC_MAX_CHN_NUM]; 
	VENC_CHN VencChn;
	
	pstPara = (sBASE_VENC_GETSTREAM_PARA_S*)pvArg;
	s32ChnTotal = pstPara->s32Cnt;
	
	eBASE_MODEFLAG eModeFlag = geModeFlag;
	
	/* channel count check */
	if (s32ChnTotal > 5)
	{
		printf("%s: input count invaild\n", __FUNCTION__);
		return NULL;
	}
	
	/* get venc fd */
	for (VencChn = BASE_VIDEO_CH0; VencChn < s32ChnTotal; VencChn++)
	{
		if(!(BASE_COMM_GETTYPEVAL(eModeFlag,BASE_MODEFLAG_VENC)&(1<<VencChn)))
		{
			continue;
		}
		/* Set Venc Fd. */
		VencFd[VencChn] = HI_MPI_VENC_GetFd(VencChn);
		if (VencFd[VencChn] < 0)
		{
			printf("%s: HI_MPI_VENC_GetFd failed with %ld!\n", __FUNCTION__, VencFd[VencChn]);
			return NULL;
		}
		if (maxfd <= VencFd[VencChn])
		{
			maxfd = VencFd[VencChn];
		}
	}
	
	/* getstream running */
	while (HI_TRUE == pstPara->bThreadStart)
	{
		/* put venc fd to fdsets */
		FD_ZERO(&read_fds);
		for (VencChn = BASE_VIDEO_CH0; VencChn < s32ChnTotal; VencChn++)
		{
			if(!(BASE_COMM_GETTYPEVAL(eModeFlag,BASE_MODEFLAG_VENC)&(1<<VencChn)))
			{
				continue;
			}
			FD_SET(VencFd[VencChn], &read_fds);
		}
		
		TimeoutVal.tv_sec  = 2;
		TimeoutVal.tv_usec = 0;

		lRet = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
		
		if (lRet < 0)
		{
			printf("%s: select failed!\n", __FUNCTION__);
			break;
		}
		else if (lRet == 0)
		{
			printf("%s: get venc stream time out, exit thread\n", __FUNCTION__);
			break;
		}
		else
		{
			/* get venc stream */
			for (VencChn = BASE_VIDEO_CH0; VencChn < s32ChnTotal; VencChn++)
			{ 
				if(!(BASE_COMM_GETTYPEVAL(eModeFlag,BASE_MODEFLAG_VENC)&(1<<VencChn)))
				{
					continue;
				}
				
				if (FD_ISSET(VencFd[VencChn], &read_fds))
				{
					/*******************************************************
					query how many packs in one-frame stream
					*******************************************************/
					memset(&stStream, 0, sizeof(stStream));
					lRet = HI_MPI_VENC_Query(VencChn, &stStat);
					if (HI_SUCCESS != lRet)
					{
						printf("%s: HI_MPI_VENC_Query chn[%d] failed with %ld!\n", __FUNCTION__, VencChn, lRet);
						break;
					}
					/*******************************************************
					malloc corresponding number of pack nodes.
					*******************************************************/
					//MALLOC(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
					stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
					if (NULL == stStream.pstPack)
					{
						printf("%s: malloc stream pack failed!\n", __FUNCTION__);
						break;
					}
					
					/*******************************************************
					call mpi to get one-frame stream
					*******************************************************/
					stStream.u32PackCount = stStat.u32CurPacks;
					lRet = HI_MPI_VENC_GetStream(VencChn, &stStream, HI_TRUE);
					if (BASE_RET_SUCCESS != lRet)
					{
						free(stStream.pstPack);
						stStream.pstPack = NULL;
						printf("%s: HI_MPI_VENC_GetStream failed with %ld!\n", __FUNCTION__, lRet);
						break;
					}
					
					/*******************************************************
					write data to pool
					*******************************************************/
					lRet = lBASE_VENC_HANDLE_Write(glPoolId[BASE_POOL_VENC], VencChn, gsVencAttr[VencChn].enType, &stStream);
					if (COMM_POOL_NEEDTOQUIT == lRet)
					{
						free(stStream.pstPack);
						stStream.pstPack = NULL;
						printf("%s thread need to exit!\n", __FUNCTION__);
						gsGetStreamPara.bThreadStart = HI_FALSE;
						break;
					}
					else if(BASE_RET_SUCCESS != lRet)
					{
						free(stStream.pstPack);
						stStream.pstPack = NULL;
						printf("<%s:%d> lBASE_VENC_HANDLE_Write error!\n", __FUNCTION__,__LINE__);
					} 
					/*******************************************************
					release stream
					*******************************************************/
					lRet = HI_MPI_VENC_ReleaseStream(VencChn, &stStream);
					if (BASE_RET_SUCCESS != lRet)
					{
						free(stStream.pstPack);
						stStream.pstPack = NULL;
						break;
					}
					
					/*******************************************************
					free pack nodes
					*******************************************************/
					free(stStream.pstPack);
					stStream.pstPack = NULL;
				}
			}
		}
	}
	debug_info("%s thread  exit!\n",__FUNCTION__);
	return NULL;
}

LONG lBASE_VENC_HANDLE_Write(LONG lPollId,LONG lVenCh,PAYLOAD_TYPE_E enType,VENC_STREAM_S *pstStream)
{
	LONG lRet = BASE_RET_SUCCESS;
	UCHAR ucCh = (UCHAR)lVenCh;
	LONG lCnt = pstStream->u32PackCount;
	LONG i,j;
	LONG lWriteRemainStat = COMM_POOL_REMAINBLOCK;
	LONG lWriteNewStat = COMM_POOL_NEWBLOCK;

	/* judge whether I Slice */
	if (lBASE_COMM_JUDGE_ISLICE(pstStream)) {
		lWriteRemainStat |= COMM_POOL_FORCE;
		lWriteNewStat |= COMM_POOL_FORCE;
	}
	
	if (PT_H264 == enType)
	{
		lRet = eCOMM_POOL_Write(lPollId, &ucCh, 1, lWriteRemainStat);
		if(BASE_RET_SUCCESS != lRet)
		{
			debug_info("%s: eCOMM_POOL_Write(1st) return  %ld \n", __FUNCTION__, lRet); // return  9
			return lRet;
		}

		for(i = 0;i < lCnt;i++)
		{
			for(j = 0;j < 2;j++)
			{
				if(pstStream->pstPack[i].u32Len[j]>0){
					lRet = eCOMM_POOL_Write(lPollId,pstStream->pstPack[i].pu8Addr[j], pstStream->pstPack[i].u32Len[j], lWriteRemainStat);
					if(BASE_RET_SUCCESS != lRet)
					{
						debug_info("%s: eCOMM_POOL_Write(2nd) return  %ld \n", __FUNCTION__, lRet);
						return lRet;
					}
				}
			}
		}
		
		lRet = eCOMM_POOL_Write(lPollId, pstStream->pstPack[0].pu8Addr[0], 0, lWriteNewStat);
		if(BASE_RET_SUCCESS != lRet)
		{
			debug_info("<%s:%d> eCOMM_POOL_Write(3rd) return  : %ld\n", __FUNCTION__,__LINE__,lRet);
			return lRet;
		}
	}
	
	return lRet;
}

LONG lBASE_COMM_VENC_Start(VENC_GRP VencGrp,VENC_CHN VencChn, PAYLOAD_TYPE_E enType, VIDEO_NORM_E enNorm, PIC_SIZE_E enSize, SAMPLE_RC_E enRcMode, sBASE_VENC_PARA_ATTR* psVencAttr)
{
    HI_S32 s32Ret;
    VENC_CHN_ATTR_S stVencChnAttr;
    VENC_ATTR_H264_S stH264Attr;
    VENC_ATTR_H264_CBR_S    stH264Cbr;
    VENC_ATTR_H264_VBR_S    stH264Vbr;
    VENC_ATTR_H264_FIXQP_S  stH264FixQp;
    VENC_ATTR_MJPEG_S stMjpegAttr;
    VENC_ATTR_MJPEG_FIXQP_S stMjpegeFixQp;
    VENC_ATTR_JPEG_S stJpegAttr;
    SIZE_S stPicSize;

    s32Ret = lBASE_SYS_GetPicSize(enNorm, enSize, &stPicSize);
     if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Get picture size failed!\n");
        return HI_FAILURE;
    }
    /******************************************
     step 1: Greate Venc Group
    ******************************************/
    s32Ret = HI_MPI_VENC_CreateGroup(VencGrp);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_CreateGroup[%d] failed with %#x!\n",\
                 VencGrp, s32Ret);
        return HI_FAILURE;
    }

    /******************************************
     step 2:  Create Venc Channel
    ******************************************/
    stVencChnAttr.stVeAttr.enType = enType;
    switch(enType)
    {
        case PT_H264:
        {
            stH264Attr.u32MaxPicWidth = stPicSize.u32Width;
            stH264Attr.u32MaxPicHeight = stPicSize.u32Height;
            stH264Attr.u32PicWidth = stPicSize.u32Width;/*the picture width*/
            stH264Attr.u32PicHeight = stPicSize.u32Height;/*the picture height*/
            stH264Attr.u32BufSize  = stPicSize.u32Width * stPicSize.u32Height * 2;/*stream buffer size*/
            stH264Attr.u32Profile  = 1;/*0: baseline; 1:MP; 2:HP   ? */
            stH264Attr.bByFrame = HI_TRUE;/*get stream mode is slice mode or frame mode?*/
            stH264Attr.bField = HI_FALSE;  /* surpport frame code only for hi3516, bfield = HI_FALSE */
            stH264Attr.bMainStream = HI_TRUE; /* surpport main stream only for hi3516, bMainStream = HI_TRUE */
            stH264Attr.u32Priority = 0; /*channels precedence level. invalidate for hi3516*/
            stH264Attr.bVIField = HI_FALSE;/*the sign of the VI picture is field or frame. Invalidate for hi3516*/
            memcpy(&stVencChnAttr.stVeAttr.stAttrH264e, &stH264Attr, sizeof(VENC_ATTR_H264_S));

            if(SAMPLE_RC_CBR == enRcMode)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
                stH264Cbr.u32Gop            = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264Cbr.u32StatTime       = 1; /* stream rate statics time(s) */
                stH264Cbr.u32ViFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;/* input (vi) frame rate */
                stH264Cbr.fr32TargetFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;/* target frame rate */

			switch(enSize)
			{
				case BASE_RES_HD1080:
					stH264Cbr.u32BitRate = 1024*16; 
					break;
				case BASE_RES_HD720:
					stH264Cbr.u32BitRate = 1024*3;
					break;
				case BASE_RES_SD480:
					stH264Cbr.u32BitRate = 1024*2;
					break;
				default:
					break;
			}
                
                stH264Cbr.u32FluctuateLevel = 0; /* average bit rate */
                memcpy(&stVencChnAttr.stRcAttr.stAttrH264Cbr, &stH264Cbr, sizeof(VENC_ATTR_H264_CBR_S));
            }
            else if (SAMPLE_RC_FIXQP == enRcMode) 
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
                stH264FixQp.u32Gop = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264FixQp.u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264FixQp.fr32TargetFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264FixQp.u32IQp = 20;
                stH264FixQp.u32PQp = 23;
                memcpy(&stVencChnAttr.stRcAttr.stAttrH264FixQp, &stH264FixQp,sizeof(VENC_ATTR_H264_FIXQP_S));
            }
            else if (SAMPLE_RC_VBR == enRcMode) 
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
                stH264Vbr.u32Gop = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264Vbr.u32StatTime = 1;
                stH264Vbr.u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264Vbr.fr32TargetFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264Vbr.u32MinQp = 0;
                stH264Vbr.u32MaxQp = 51;
			
               	switch(enSize)
			{
				case BASE_RES_HD1080:
					stH264Vbr.u32MaxBitRate = 1024 * 8; 
					break;
				case BASE_RES_HD720:
					stH264Vbr.u32MaxBitRate = 1024 * 5;
					break;
				case BASE_RES_SD480:
					stH264Vbr.u32MaxBitRate = 1024 * 2;
					break;
				default:
					break;
			}

                memcpy(&stVencChnAttr.stRcAttr.stAttrH264Vbr, &stH264Vbr, sizeof(VENC_ATTR_H264_VBR_S));
            }
            else
            {
                return HI_FAILURE;
            }
        }
        break;
        
        case PT_MJPEG:
        {
            stMjpegAttr.u32MaxPicWidth = stPicSize.u32Width;
            stMjpegAttr.u32MaxPicHeight = stPicSize.u32Height;
            stMjpegAttr.u32PicWidth = stPicSize.u32Width;
            stMjpegAttr.u32PicHeight = stPicSize.u32Height;
            stMjpegAttr.u32BufSize = stPicSize.u32Width * stPicSize.u32Height * 2;
            stMjpegAttr.bByFrame = HI_TRUE;  /*get stream mode is field mode  or frame mode*/
            stMjpegAttr.bMainStream = HI_TRUE;  /*main stream or minor stream types?*/
            stMjpegAttr.bVIField = HI_FALSE;  /*the sign of the VI picture is field or frame?*/
            stMjpegAttr.u32Priority = 0;/*channels precedence level*/
            memcpy(&stVencChnAttr.stVeAttr.stAttrMjpeg, &stMjpegAttr, sizeof(VENC_ATTR_MJPEG_S));

            if(SAMPLE_RC_FIXQP == enRcMode)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGFIXQP;
                stMjpegeFixQp.u32Qfactor        = 90;
                stMjpegeFixQp.u32ViFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stMjpegeFixQp.fr32TargetFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                memcpy(&stVencChnAttr.stRcAttr.stAttrMjpegeFixQp, &stMjpegeFixQp,
                       sizeof(VENC_ATTR_MJPEG_FIXQP_S));
            }
            else if (SAMPLE_RC_CBR == enRcMode)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGCBR;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32StatTime       = 1;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32ViFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.fr32TargetFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32FluctuateLevel = 0;
			
                	switch(enSize)
			{
				case BASE_RES_HD1080:
					stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*16; 
					break;
				case BASE_RES_HD720:
					stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*3;
					break;
				case BASE_RES_SD480:
					stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*2;
					break;
				default:
					break;
			}
		
            }
            else if (SAMPLE_RC_VBR == enRcMode) 
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGVBR;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32StatTime = 1;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == enNorm)?25:30;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.fr32TargetFrmRate = 5;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MinQfactor = 50;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxQfactor = 95;
			
                	switch(enSize)
			{
				case BASE_RES_HD1080:
					stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*16; 
					break;
				case BASE_RES_HD720:
					stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*3;
					break;
				case BASE_RES_SD480:
					stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*2;
					break;
				default:
					break;
			}
			
            }
            else 
            {
                SAMPLE_PRT("cann't support other mode in this version!\n");

                return HI_FAILURE;
            }
        }
        break;
            
        case PT_JPEG:
            stJpegAttr.u32PicWidth  = stPicSize.u32Width;
            stJpegAttr.u32PicHeight = stPicSize.u32Height;
            stJpegAttr.u32BufSize = stPicSize.u32Width * stPicSize.u32Height * 2;
            stJpegAttr.bByFrame = HI_TRUE;/*get stream mode is field mode  or frame mode*/
            stJpegAttr.bVIField = HI_FALSE;/*the sign of the VI picture is field or frame?*/
            stJpegAttr.u32Priority = 0;/*channels precedence level*/
            memcpy(&stVencChnAttr.stVeAttr.stAttrMjpeg, &stMjpegAttr, sizeof(VENC_ATTR_MJPEG_S));
            break;
        default:
            return HI_ERR_VENC_NOT_SUPPORT;
    }

    s32Ret = HI_MPI_VENC_CreateChn(VencChn, &stVencChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_CreateChn [%d] faild with %#x!\n",\
                VencChn, s32Ret);
        return s32Ret;
    }

	/******************************************
	step 2.1:  set the vui parameter to es stream
	******************************************/
	VENC_PARAM_H264_VUI_S stVui;
	VENC_PARAM_H264_VUI_S* pstVui = &stVui;

	/* get current vui parameters */
	s32Ret = HI_MPI_VENC_GetH264Vui(VencChn, pstVui);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("HI_MPI_VENC_GetH264Vui faild with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	/* set new value of vui parameters */
	pstVui->timing_info_present_flag = 1;
	pstVui->num_units_in_tick = 1001;
	pstVui->time_scale = 60000;
	pstVui->fixed_frame_rate_flag = 1;

	/* set the vui paramter to es stream */
	s32Ret = HI_MPI_VENC_SetH264Vui(VencChn, pstVui);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("HI_MPI_VENC_SetH264Vui faild with %#x!\n", s32Ret);
		return HI_FAILURE;
	}

	printf("[ Channal %d ] : successfully set vui parameters !\n", VencChn);

    /******************************************
     step 3:  Regist Venc Channel to VencGrp
    ******************************************/
    s32Ret = HI_MPI_VENC_RegisterChn(VencGrp, VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_RegisterChn faild with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    /******************************************
     step 4:  Start Recv Venc Pictures
    ******************************************/
    s32Ret = HI_MPI_VENC_StartRecvPic(VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VENC_StartRecvPic faild with%#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;

}

LONG lBASE_SYS_CalcPicVbBlkSize(VIDEO_NORM_E enNorm, eBASE_VENC_PIC_SIZE enPicSize, PIXEL_FORMAT_E enPixFmt, HI_U32 u32AlignWidth)
{
    HI_S32 s32Ret = HI_FAILURE;
    SIZE_S stSize;

    s32Ret = lBASE_SYS_GetPicSize(enNorm, enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: get picture size[%d] failed!\n", __FUNCTION__, enPicSize);
            return HI_FAILURE;
    }

    if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 != enPixFmt && PIXEL_FORMAT_YUV_SEMIPLANAR_420 != enPixFmt)
    {
        printf("%s: pixel format[%d] input failed!\n", __FUNCTION__, enPixFmt);
            return HI_FAILURE;
    }

    if (16!=u32AlignWidth && 32!=u32AlignWidth && 64!=u32AlignWidth)
    {
        printf("%s: system align width[%d] input failed!\n",\
               __FUNCTION__, u32AlignWidth);
            return HI_FAILURE;
    }

    return (CEILING_2_POWER(stSize.u32Width, u32AlignWidth) * \
            CEILING_2_POWER(stSize.u32Height,u32AlignWidth) * \
           ((PIXEL_FORMAT_YUV_SEMIPLANAR_422 == enPixFmt)?2:1.5));
}

LONG lBASE_COMM_VENC_BindVpss(VENC_GRP GrpChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn) {
    HI_S32 s32Ret = HI_FAILURE;

    s32Ret = SAMPLE_COMM_VENC_BindVpss(GrpChn, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret)
    {
		printf("%s : venc bind vpss fail\n", __FUNCTION__);
		return HI_FAILURE;
    }

	return HI_SUCCESS;
}

LONG lBASE_COMM_VENC_UnBindVpss(VENC_GRP GrpChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn) {
    HI_S32 s32Ret = HI_FAILURE;

    s32Ret = SAMPLE_COMM_VENC_UnBindVpss(GrpChn, VpssGrp, VpssChn);
    if (HI_SUCCESS != s32Ret)
    {
		printf("%s : venc unbind vpss fail\n", __FUNCTION__);
		return HI_FAILURE;
    }

	return HI_SUCCESS;
}

LONG lBASE_COMM_START_VPSSGROUP() {
	HI_S32 s32Ret = HI_SUCCESS;
	VPSS_GRP VpssGrp;
	VPSS_GRP_ATTR_S stVpssGrpAttr;

	VpssGrp = 0;
#ifdef TEST_CHANNEL0_720P
	stVpssGrpAttr.u32MaxW = 1280;
	stVpssGrpAttr.u32MaxH = 720;
#else
	stVpssGrpAttr.u32MaxW = 1920;
	stVpssGrpAttr.u32MaxH = 1080;
#endif

	stVpssGrpAttr.bDrEn = HI_FALSE;
	stVpssGrpAttr.bDbEn = HI_FALSE;
	stVpssGrpAttr.bIeEn = HI_TRUE;
	stVpssGrpAttr.bNrEn = HI_TRUE;
	stVpssGrpAttr.bHistEn = HI_TRUE;
	stVpssGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
	stVpssGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
	s32Ret = SAMPLE_COMM_VPSS_StartGroup(VpssGrp, &stVpssGrpAttr);
	if (HI_SUCCESS != s32Ret)
	{
	    SAMPLE_PRT("Start Vpss failed!\n");
	    return HI_FAILURE;
	}

	s32Ret = SAMPLE_COMM_VI_BindVpss(SENSOR_TYPE);
	if (HI_SUCCESS != s32Ret)
	{
	    SAMPLE_PRT("Vi bind Vpss failed!\n");
	    return HI_FAILURE;
	}

	/* changle vpss attribute */
	VPSS_GRP_PARAM_S sVpssParam;
	HI_MPI_VPSS_GetGrpParam(0, &sVpssParam);
	sVpssParam.u32TfStrength = 8;
	sVpssParam.u32SfStrength = 32;
	sVpssParam.u32ChromaRange = 8;
	HI_MPI_VPSS_SetGrpParam(0, &sVpssParam);

	return s32Ret;
}

LONG lBASE_COMM_STOP_VPSSGROUP() {
	HI_S32 s32Ret = HI_SUCCESS;

	s32Ret = SAMPLE_COMM_VI_UnBindVpss(SENSOR_TYPE);
	if (HI_SUCCESS != s32Ret)
	{
	    SAMPLE_PRT("Vi unbind Vpss failed!\n");
	    return HI_FAILURE;
	}
	
	s32Ret = SAMPLE_COMM_VPSS_StopGroup(0);
	if (HI_SUCCESS != s32Ret)
	{
	    SAMPLE_PRT("vpss stop group failed!\n");
	    return HI_FAILURE;
	}

	return s32Ret;
}

LONG lBASE_VPSS_INIT_CH(sBASE_VIDEO_VPSS* psVpss, VI_CHN ViCh, VPSS_CHN VpssCh, eBASE_VPSS_TYPE eType, eBASE_VENC_PIC_SIZE eSize) {
	LONG lRet = BASE_RET_SUCCESS;

	psVpss->ViChn = ViCh;
	psVpss->VpssChn = VpssCh;
	psVpss->eType = eType;
	psVpss->eSize = eSize;
	psVpss->eState = BASE_ISUSE;
	
	return lRet;
}
LONG lBASE_VPSS_START_CH(sBASE_VIDEO_VPSS* psVpss) {
	LONG lRet = BASE_RET_SUCCESS;

	assert(lVpssChannelCurrent >= 0);
	
	/* judge whether to init vpss group */
	if(lVpssChannelCurrent == 0) {
		lRet = lBASE_COMM_START_VPSSGROUP();
		assert(lRet == BASE_RET_SUCCESS);
	}

	/* enable vpss channel */
	if(!(_BIT(psVpss->VpssChn) & lVpssChannelCurrent)) {
		/* indicate the vpss channel is not start yet */
		switch(psVpss->eType) {
			case BASE_VPSS_TYPE_BASE :
				lRet = lBASE_COMM_START_VPSS_BASE(psVpss);
				if(lRet != BASE_RET_SUCCESS) {
					printf("start vpss base error\n");
					assert(0);
				}
				break;
			case BASE_VPSS_TYPE_EXT :
				lRet = lBASE_COMM_START_VPSS_EXT(psVpss);
				if(lRet != BASE_RET_SUCCESS) {
					printf("start vpss ext error\n");
					assert(0);
				}
				break;
			default :
				assert(0);
				break;
		}

		/* regist this vpss channel */
		lVpssChannelCurrent |= _BIT(psVpss->VpssChn);
	}
	
	return lRet;
	
}

LONG lBASE_VPSS_STOP_CH(sBASE_VIDEO_VPSS* psVpss) {
	LONG lRet = BASE_RET_SUCCESS;
	eBASE_VENC_PIC_SIZE ePicSize;
	LONG* plChannel;

	assert(lVpssChannelCurrent >= 0);

	if(psVpss->eState == BASE_NOTUSE) {
		printf("no need to stop\n");
		return BASE_RET_SUCCESS;
	}

	if(_BIT(psVpss->VpssChn) & lVpssChannelCurrent) {
		/* indicate the vpss channel is start now, stop it */
		ePicSize = psVpss->eSize;
		plChannel = glBindTable[ ePicSize ];
		lRet = SAMPLE_COMM_VPSS_DisableChn(
			plChannel[BASE_TABLE_TYPE_VPSSGRP],
			plChannel[BASE_TABLE_TYPE_VPSS]
		);
		if(lRet != BASE_RET_SUCCESS) {
			printf("%s : <%d>  error\n", __func__, __LINE__);
			assert(0);
			return lRet;
		}
		
		/* unregist this vpss channel */
		lVpssChannelCurrent &= ~(_BIT(psVpss->VpssChn));
		psVpss->eState = BASE_NOTUSE;
	}

	if(lVpssChannelCurrent == 0) {
		/* indicate all the vpss channel is closed */
		lRet = lBASE_COMM_STOP_VPSSGROUP();
		if(lRet != BASE_RET_SUCCESS) {
			printf("%s : <%d>  error\n", __func__, __LINE__);
			assert(0);
			return lRet;
		}
	}

	return lRet;
}

LONG lBASE_COMM_START_VPSS_BASE(sBASE_VIDEO_VPSS* psVpss) {
	LONG lRet = BASE_RET_SUCCESS;

	HI_S32 s32Ret = HI_SUCCESS;
	VPSS_CHN VpssChn;
	VPSS_GRP VpssGrp;
	VPSS_CHN_ATTR_S stVpssChnAttr;
	VPSS_CHN_MODE_S stVpssChnMode;

	eBASE_VENC_PIC_SIZE enPicSize;
	SIZE_S stSize;

	VpssChn = psVpss->VpssChn;

	/* vpss channel attribute */
	memset(&stVpssChnAttr, 0, sizeof(stVpssChnAttr));
	stVpssChnAttr.bFrameEn = HI_FALSE;
	stVpssChnAttr.bSpEn    = HI_TRUE;    

	/* vpss channel mode */
	stVpssChnMode.enChnMode     = VPSS_CHN_MODE_USER;
	stVpssChnMode.bDouble       = HI_FALSE;
	stVpssChnMode.enPixelFormat = SAMPLE_PIXEL_FORMAT;

	enPicSize = psVpss->eSize;
	lBASE_SYS_GetPicSize(BASE_GLOBAL_NORM, enPicSize, &stSize);
	stVpssChnMode.u32Width      = stSize.u32Width;
	stVpssChnMode.u32Height     = stSize.u32Height;
	VpssGrp = glBindTable[enPicSize][BASE_TABLE_TYPE_VPSSGRP];
	
	s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, &stVpssChnAttr, &stVpssChnMode, HI_NULL);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Enable vpss chn failed!\n");
		assert(0);
		return BASE_RET_FAIL;
	}

	return lRet;

}

LONG lBASE_COMM_START_VPSS_EXT(sBASE_VIDEO_VPSS* psVpss) {
	LONG lRet = BASE_RET_SUCCESS;

	HI_S32 s32Ret = HI_SUCCESS;
	VPSS_GRP VpssGrp;
	VPSS_CHN VpssChn;
	VPSS_EXT_CHN_ATTR_S stVpssExtChnAttr;

	eBASE_VENC_PIC_SIZE enPicSize;
	SIZE_S stSize;

	VpssChn = psVpss->VpssChn;
	stVpssExtChnAttr.s32BindChn = 1;
	stVpssExtChnAttr.s32SrcFrameRate = 30;
	stVpssExtChnAttr.s32DstFrameRate = 30;
	stVpssExtChnAttr.enPixelFormat	 = SAMPLE_PIXEL_FORMAT;

	enPicSize = psVpss->eSize;
	stVpssExtChnAttr.u32Width		 = stSize.u32Width;
	stVpssExtChnAttr.u32Height		 = stSize.u32Height;
	VpssGrp = glBindTable[enPicSize][BASE_TABLE_TYPE_VPSSGRP];
	
	s32Ret = SAMPLE_COMM_VPSS_EnableChn(VpssGrp, VpssChn, HI_NULL, HI_NULL, &stVpssExtChnAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Enable vpss chn failed!\n");
		assert(0);
		return BASE_RET_FAIL;
	}

	return lRet;
}

LONG lBASE_COMM_JUDGE_ISLICE(VENC_STREAM_S *pstStream) {
	LONG lIsISlice = 0;
	LONG lCount = pstStream->u32PackCount;
	LONG lPackCntTmp;
	VENC_PACK_S* psPacket = NULL;
	H264E_NALU_TYPE_E eSliceType;

	for (lPackCntTmp = 0; lPackCntTmp != lCount; lPackCntTmp++) {
		psPacket = pstStream->pstPack + lPackCntTmp;
		eSliceType = psPacket->DataType.enH264EType;
		if (eSliceType == H264E_NALU_ISLICE) {
			// BMB_Debug("find I Slice\n");
			lIsISlice = 1;
			break;
		}
	}

	return lIsISlice;
}

LONG lBASE_COMM_VENC_ADJUSTISP() {
	LONG lRet = BASE_RET_SUCCESS;

	/* isp setting */

#if 1
	//vBASE_COMM_VENC_ISPSetAntiFlicker();
	vBASE_COMM_VENC_ISPSetAEAttr();
	//vBASE_COMM_VENC_ISPSetDRC();
	vBASE_COMM_VENC_ISPSetAWBAttr();
	vBASE_COMM_VENC_ISPSetDenoise();
	vBASE_COMM_VENC_ISPSetAntiFog();
	lBASE_COMM_VENC_ADJUSTISP_LDC();
	//lBASE_COMM_VENC_ADJUSTISP_CSC();
#endif

#if 0
	lBASE_COMM_VENC_ADJUSTISP_DRC();
	lBASE_COMM_VENC_ADJUSTISP_LDC();
	// lBASE_COMM_VENC_ADJUSTISP_AE_BASIC();
	lBASE_COMM_VENC_ADJUSTISP_AE_STATIC();
	lBASE_COMM_VENC_ADJUSTISP_AE_EX();
	lBASE_COMM_VENC_ADJUSTISP_GAMMA();
	// lBASE_COMM_VENC_ADJUSTISP_ANTICOLOR();
	lBASE_COMM_VENC_ADJUSTISP_CSC();
#endif

	return lRet;
}

#if 1
void vBASE_COMM_VENC_ISPGetModuleControl() {

	/* get module contrl flag */
	HI_U32 u32ModFlag;
	HI_MPI_ISP_GetModuleControl(&u32ModFlag);
	printf("\n-------calling function HI_MPI_ISP_GetAEAttr-------\n");
	printf("module contrl flag\t: %#x\n", u32ModFlag);
}

void vBASE_COMM_VENC_ISPGetAEPara() {
	/* get AE exposure type */
	ISP_OP_TYPE_E enExpType;
	HI_MPI_ISP_GetExposureType(&enExpType);
	printf("\n-------calling HI_MPI_ISP_GetExposureType-------\n");
	if(enExpType==OP_TYPE_AUTO) {
		printf("Exptype\t:Auto Expose\n");

		/* get Ae attr */
		 ISP_AE_ATTR_S stAEAttr;
		HI_MPI_ISP_GetAEAttr(&stAEAttr);
		printf("\n-------calling function HI_MPI_ISP_GetAEAttr-------\n");
		printf("AEMode\t:%s\n", ((stAEAttr.enAEMode==AE_MODE_LOW_NOISE) ? "AE_MODE_LOW_NOISE" : "AE_MODE_FRAME_RATE"));
		printf("ExpTimeMax\t:%d\n", stAEAttr.u16ExpTimeMax);
		printf("ExpTimeMin\t:%d\n", stAEAttr.u16ExpTimeMin);
		printf("DGainMax\t:%d\n", stAEAttr.u16DGainMax);
		printf("DGainMin\t:%d\n", stAEAttr.u16DGainMin);
		printf("AGainMax\t:%d\n", stAEAttr.u16AGainMax);
		printf("AGainMin\t:%d\n", stAEAttr.u16AGainMin);
		printf("ExpStep\t:%d\n", stAEAttr.u8ExpStep);
		printf("Tolerance\t:%d\n", stAEAttr.s16ExpTolerance);
		printf("ExpCompensation\t:%d\n", stAEAttr.u8ExpCompensation);
		printf("frameendupdatemode\t:%d\n", stAEAttr.enFrameEndUpdateMode);
		printf("bypassAE\t:%d\n", stAEAttr.bByPassAE);

		/* get Ae ex attr */
		ISP_AE_ATTR_EX_S stAEAttrEx;
		HI_MPI_ISP_GetAEAttrEx(&stAEAttrEx);
		printf("\n-------calling function HI_MPI_ISP_GetAEAttrEx-------\n");
		printf("AEMode\t:%s\n", ((stAEAttrEx.enAEMode==AE_MODE_LOW_NOISE) ? "AE_MODE_LOW_NOISE" : "AE_MODE_FRAME_RATE"));
		printf("ExpTimeMax\t:%d\n", stAEAttrEx.u32ExpTimeMax);
		printf("ExpTimeMin\t:%d\n", stAEAttrEx.u32ExpTimeMin);
		printf("DGainMax\t:%d\n", stAEAttrEx.u32DGainMax);
		printf("DGainMin\t:%d\n", stAEAttrEx.u32DGainMin);
		printf("AGainMax\t:%d\n", stAEAttrEx.u32AGainMax);
		printf("AGainMin\t:%d\n", stAEAttrEx.u32AGainMin);
		printf("ISPGDainMax\t:%d\n", stAEAttrEx.u32ISPDGainMax);
		printf("SystemGainMax\t:%d\n", stAEAttrEx.u32SystemGainMax);
		printf("ExpStep\t:%d\n", stAEAttrEx.u8ExpStep);
		printf("Tolerance\t:%d\n", stAEAttrEx.s16ExpTolerance);
		printf("ExpCompensation\t:%d\n", stAEAttrEx.u8ExpCompensation);
		printf("frameendupdatemode\t:%d\n", stAEAttrEx.enFrameEndUpdateMode);
		printf("bypassAE\t:%d\n", stAEAttrEx.bByPassAE);
	} else {
		printf("Exptype\t:Manual Expose\n");

		/* get Me attr */
	}
	
}

void vBASE_COMM_VENC_ISPGetAWBPara() {

	/* get wb type */
	ISP_OP_TYPE_E enWBType;
	HI_MPI_ISP_GetWBType(&enWBType);
	printf("\n-------calling function HI_MPI_ISP_GetWBType-------\n");
	if(enWBType== OP_TYPE_AUTO) {
		printf("WB type\t:AutoWB\n");

		/* get awb attr */
		ISP_AWB_ATTR_S stAWBAttr;
		HI_MPI_ISP_GetAWBAttr(&stAWBAttr);
		printf("\n-------calling function HI_MPI_ISP_GetAWBAttr-------\n");
		ISP_AWB_CALIBRATION_S* pstAWBCalibration = &stAWBAttr.stAWBCalibration;
		printf("CurvePara[0-5]\t:");
		int i = 0;
		for(i = 0; i < 6; i++) {
			printf("%#x\t", pstAWBCalibration->as32CurvePara[i]);
		}

		printf("\nStaticWB[0-3]\t:");
		for(i = 0; i < 4; i++) {
			printf("%#x\t", pstAWBCalibration->au16StaticWB[i]);
		}
		printf("\nRefTemp\t:%d\n",pstAWBCalibration->u16RefTemp );

		printf("Speed\t:%d\n", stAWBAttr.u16Speed);
		printf("RGStrength\t:%d\n", stAWBAttr.u8RGStrength);
		printf("BGStrength\t:%d\n", stAWBAttr.u8BGStrength);
		printf("ZoneSel\t:%d\n", stAWBAttr.u8ZoneSel);
		printf("HighColorTemp\t:%d\n", stAWBAttr.u8HighColorTemp);
		printf("LowColorTemp\t:%d\n", stAWBAttr.u8LowColorTemp);

		/* get awb alg type */
		ISP_AWB_ALG_TYPE_E enALGType;
		HI_MPI_ISP_GetAWBAlgType(&enALGType);
		printf("\n-------calling function HI_MPI_ISP_GetAWBAlgType-------\n");
		printf("AWB alg type\t:%s\n", ((enALGType==AWB_ALG_DEFAULT)? "AWB_ALG_DEFAULT": "AWB_ALG_ADVANCE"));

		if(enALGType==AWB_ALG_ADVANCE) {
			/* get adv attr */
			ISP_ADV_AWB_ATTR_S  stAdvAWBAttr;
			HI_MPI_ISP_GetAdvAWBAttr(&stAdvAWBAttr);
			printf("\n-------calling function HI_MPI_ISP_GetAdvAWBAttr-------\n");
			printf("AccuPrior\t:%d\n", stAdvAWBAttr.bAccuPrior);
			printf("Tolerance\t:%d\n", stAdvAWBAttr.u8Tolerance);
			printf("CurveLLimit\t:%d\n", stAdvAWBAttr.u16CurveLLimit);
			printf("CurveRLimit\t:%d\n", stAdvAWBAttr.u16CurveRLimit);
		}

		/* get light attr */
		ISP_AWB_ADD_LIGHTSOURCE_S stLightSource;
		HI_MPI_ISP_GetLightSource(&stLightSource);
		printf("\n-------calling function HI_MPI_ISP_GetLightSource-------\n");
		printf("LightEnable\t:%s\n", ((stLightSource.bLightEnable == 1) ? "true": "false"));
	} else {
		printf("WB type\t:ManualWB\n");
	}

}

void vBASE_COMM_VENC_ISPGetAntiFakeColor() {
	ISP_ANTI_FALSECOLOR_S stAntiFC;
	HI_MPI_ISP_GetAntiFalseColorAttr(&stAntiFC);
	printf("\n-------calling function HI_MPI_ISP_GetAntiFalseColorAttr-------\n");
	printf("flase color strength\t:%d\n", stAntiFC.u8Strength);
}

void vBASE_COMM_VENC_ISPGetAntiFog() {
	ISP_ANTIFOG_S stAntiFog;
	HI_MPI_ISP_GetAntiFogAttr(&stAntiFog);
	printf("\n-------calling function HI_MPI_ISP_GetAntiFogAttr-------\n");
	printf("antifog enable\t:%d\n", stAntiFog.bEnable);
	printf("antifog strength\t:%d\n", stAntiFog.u8Strength);
}

void vBASE_COMM_VENC_ISPGetDIS() {
	ISP_DIS_ATTR_S stDISAttr;
	HI_MPI_ISP_GetDISAttr(&stDISAttr);
	printf("\n-------calling function HI_MPI_ISP_GetDISAttr-------\n");
	printf("dis enable\t:%d\n", stDISAttr.bEnable);
}

void vBASE_COMM_VENC_ISPGetBlackLevel() {
	ISP_BLACK_LEVEL_S stBlackLevel;
	HI_MPI_ISP_GetBlackLevelAttr(&stBlackLevel);
	printf("\n-------calling function HI_MPI_ISP_GetDISAttr-------\n");
	int i = 0;
	printf("blacklevel\t:\n");
	for(i = 0; i < 4; i++) {
		printf(" %#x", stBlackLevel.au16BlackLevel[i]);
	}
	printf("\n");

}

void vBASE_COMM_VENC_ISPGetDRC() {

	/* get drc attr */
	ISP_DRC_ATTR_S stDRCAttr;
	HI_MPI_ISP_GetDRCAttr(&stDRCAttr);

	printf("\n-------calling function HI_MPI_ISP_GetAntiFlickerAttr-------\n");
	printf("DRC Enable\t:%s\n", ((stDRCAttr.bDRCEnable==1)?"true":"false"));
	printf("DRCManualEnable\t:%s\n", ((stDRCAttr.bDRCManualEnable==1)?"true":"false"));
	printf("u32StrengthTarget\t:%d\n", stDRCAttr.u32StrengthTarget);
	printf("u32SlopeMax\t:%d\n", stDRCAttr.u32SlopeMax);
	printf("u32SlopeMin\t:%d\n", stDRCAttr.u32SlopeMin);
	printf("u32WhiteLevel\t:%d\n", stDRCAttr.u32WhiteLevel);
	printf("u32BlackLevel\t:%d\n", stDRCAttr.u32BlackLevel);
	printf("u32VarianceSpace\t:%d\n", stDRCAttr.u32VarianceSpace);
	printf("u32VarianceIntensity\t:%d\n", stDRCAttr.u32VarianceIntensity);
}

void vBASE_COMM_VENC_ISPSetDRC() {
	/* get drc attr */
	ISP_DRC_ATTR_S stDRCAttr;
	
	HI_MPI_ISP_GetDRCAttr(&stDRCAttr);
	stDRCAttr.bDRCEnable = HI_TRUE;
	HI_MPI_ISP_SetDRCAttr(&stDRCAttr);
}

void vBASE_COMM_VENC_ISPSetAntiFlicker() {
	ISP_ANTIFLICKER_S sAntiflicker;

	HI_MPI_ISP_GetAntiFlickerAttr(&sAntiflicker);
	
	sAntiflicker.bEnable = HI_TRUE;
	sAntiflicker.u8Frequency = 50;
	sAntiflicker.enMode = ISP_ANTIFLICKER_MODE_0;
	
	HI_MPI_ISP_SetAntiFlickerAttr(&sAntiflicker);
}

void vBASE_COMM_VENC_ISPSetAEAttr() {
	ISP_AE_ATTR_EX_S stAEAttrEx;
	HI_MPI_ISP_GetAEAttrEx(&stAEAttrEx);

	stAEAttrEx.enAEMode = 
		AE_MODE_LOW_NOISE;
	stAEAttrEx.u8ExpStep = 32;
	stAEAttrEx.u32ExpTimeMax = 19200;
	stAEAttrEx.u32ExpTimeMin = 32;
	stAEAttrEx.u8ExpCompensation = 64;

	HI_MPI_ISP_SetAEAttrEx(&stAEAttrEx);
}

void vBASE_COMM_VENC_ISPSetAWBAttr() {
	ISP_AWB_ATTR_S stAWBAttr;
	
	HI_MPI_ISP_GetAWBAttr(&stAWBAttr);
	stAWBAttr.u8LowColorTemp = 32;
	HI_MPI_ISP_SetAWBAttr(&stAWBAttr);
	
	HI_MPI_ISP_SetWBType(OP_TYPE_AUTO);
}

void vBASE_COMM_VENC_ISPSetDenoise() {
	ISP_DENOISE_ATTR_S stDenoiseAttr;
	HI_MPI_ISP_GetDenoiseAttr(&stDenoiseAttr);
	stDenoiseAttr.bEnable = HI_TRUE;
	stDenoiseAttr.bManualEnable = HI_FALSE;
	HI_MPI_ISP_SetDenoiseAttr(&stDenoiseAttr);
}

void vBASE_COMM_VENC_ISPSetAntiFog() {
	ISP_ANTIFOG_S stAntiFog;
	HI_MPI_ISP_GetAntiFogAttr(&stAntiFog);
	stAntiFog.bEnable = HI_TRUE;
	stAntiFog.u8Strength = 0x80;
	HI_MPI_ISP_SetAntiFogAttr(&stAntiFog);
}
#endif

LONG lBASE_COMM_VENC_ADJUSTISP_DRC() {
	LONG lRet = BASE_RET_SUCCESS;
	
	/* enable drc */
	ISP_DRC_ATTR_S sDRCAttr;
	HI_MPI_ISP_GetDRCAttr(&sDRCAttr);
	sDRCAttr.bDRCEnable = HI_TRUE;
	sDRCAttr.u32StrengthTarget = 128;
	HI_MPI_ISP_SetDRCAttr(&sDRCAttr);

	return lRet;
}
LONG lBASE_COMM_VENC_ADJUSTISP_LDC() {
	LONG lRet = BASE_RET_SUCCESS;

	/* LDC */
	VI_LDC_ATTR_S stLDCAttr;
	VI_CHN ViChn = 0;
	stLDCAttr.bEnable = HI_TRUE;
	stLDCAttr.stAttr.enViewType = LDC_VIEW_TYPE_ALL;
	stLDCAttr.stAttr.s32CenterXOffset = 0;
	stLDCAttr.stAttr.s32CenterYOffset = 0;
	stLDCAttr.stAttr.s32Ratio = 255;

	lRet = HI_MPI_VI_SetLDCAttr(ViChn, &stLDCAttr);
	if (lRet != 0) {
		printf("error with HI_MPI_VI_SetLDCAttr\n");
		return lRet;
	}
	return lRet;
}

LONG lBASE_COMM_VENC_ADJUSTISP_AE_BASIC() {
	LONG lRet = BASE_RET_SUCCESS;

	/* AE */
	ISP_AE_ATTR_S stAEAttr;
	HI_MPI_ISP_GetAEAttr(&stAEAttr);
	stAEAttr.u16ExpTimeMax = 0x500;
	stAEAttr.u16ExpTimeMin = 2;
	stAEAttr.u16AGainMax = 28;
	stAEAttr.u16AGainMin = 0;
	stAEAttr.u16DGainMax = 38;
	stAEAttr.u16DGainMin = 0;
	stAEAttr.u8ExpStep = 8;
	stAEAttr.s16ExpTolerance = 4;
	stAEAttr.u8ExpCompensation = 0x20;
	HI_MPI_ISP_SetAEAttr(&stAEAttr);

	return lRet;
}

LONG lBASE_COMM_VENC_ADJUSTISP_AE_STATIC() {
	LONG lRet = BASE_RET_SUCCESS;

	/* AE Static Info */
	ISP_EXP_STA_INFO_S stExpStatistic;
	HI_MPI_ISP_GetExpStaInfo(&stExpStatistic);
	/* threshold */
	stExpStatistic.u8ExpHistThresh[0] = 0x28;
	stExpStatistic.u8ExpHistThresh[1] = 0x60;
	stExpStatistic.u8ExpHistThresh[2] = 0xa0;
	stExpStatistic.u8ExpHistThresh[3] = 0xc0;

	/* target */
	stExpStatistic.u8ExpHistTarget[0] = 0x60;
	stExpStatistic.u8ExpHistTarget[1] = 0x50;
	stExpStatistic.u8ExpHistTarget[2] = 0x28;
	stExpStatistic.u8ExpHistTarget[3] = 0x18;
	stExpStatistic.u8ExpHistTarget[4] = 0x10;
	HI_MPI_ISP_SetExpStaInfo(&stExpStatistic);

	return lRet;
}

LONG lBASE_COMM_VENC_ADJUSTISP_AE_EX() {
	LONG lRet = BASE_RET_SUCCESS;

	/* AE EX */
	ISP_AE_ATTR_EX_S sAEAttrEx;
	HI_MPI_ISP_GetAEAttrEx(&sAEAttrEx);
	sAEAttrEx.enAEMode = AE_MODE_FRAME_RATE;
	sAEAttrEx.u32ExpTimeMax = 0x500;
	sAEAttrEx.u32SystemGainMax = 0x800;
	sAEAttrEx.u8ExpStep = 20;
	sAEAttrEx.u8ExpCompensation = 0x40;
	HI_MPI_ISP_SetAEAttrEx(&sAEAttrEx);

	return lRet;
}

LONG lBASE_COMM_VENC_ADJUSTISP_GAMMA() {
	LONG lRet = BASE_RET_SUCCESS;

	/* gamma */
	ISP_GAMMA_TABLE_S sGammaTable;
	HI_MPI_ISP_GetGammaTable(&sGammaTable);
	sGammaTable.enGammaCurve = ISP_GAMMA_CURVE_USER_DEFINE;
	memcpy(sGammaTable.u16Gamma, au16GammaTable0, sizeof(au16GammaTable0));
	HI_MPI_ISP_SetGammaTable(&sGammaTable);
	return lRet;
}

LONG lBASE_COMM_VENC_ADJUSTISP_ANTICOLOR() {
	LONG lRet = BASE_RET_SUCCESS;

	/* AntiFalseColor*/
	ISP_ANTI_FALSECOLOR_S stAntiFC;
	HI_MPI_ISP_GetAntiFalseColorAttr(&stAntiFC);
	stAntiFC.u8Strength = 16;
	HI_MPI_ISP_SetAntiFalseColorAttr(&stAntiFC);

	return lRet;
}

LONG lBASE_COMM_VENC_ADJUSTISP_CSC() {
	LONG lRet = BASE_RET_SUCCESS;

	/* CSC */
	VI_CSC_ATTR_S sCSCAttr;
	HI_MPI_VI_GetCSCAttr(0, &sCSCAttr);
	sCSCAttr.u32ContrVal = 50;
	sCSCAttr.u32LumaVal = 50;
	sCSCAttr.u32HueVal = 50;
	sCSCAttr.u32SatuVal = 50;
	HI_MPI_VI_SetCSCAttr(0, &sCSCAttr);

	return lRet;
}

#ifdef __cplusplus
}
#endif
