#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
	include
****************************************************************/
#include "base_mw_snap.h"
#include "base_mw_sd.h"
#include "base_mw_modechange.h"
#include "base_mw_boardinterface.h"
#include <errno.h>

/****************************************************************
	macro / enum
****************************************************************/
// #define BASE_MW_SNAP_ALLOWCACHE
//#define USE_SNAP_MW_LOG
#ifdef USE_SNAP_MW_LOG
#define BSNAP_Debug(fmt, arg...) fprintf(stdout, "[ BASE Snap ] : %s < %d > "fmt, __func__, __LINE__, ##arg)
#define BSNAP_FuncIn() fprintf(stdout, "in %s\n", __func__)
#define BSNAP_FunOut() fprintf(stdout, "out %s\n", __func__)
#define BSNAP_iVal(para) fprintf(stdout, "%s : 0x%x(%d) @ %p\n", #para, (para), (para), &(para))
#define BSNAP_lVal(para) fprintf(stdout, "%s : <%ld> @ %p\n", #para, (para), &(para))
#define BSNAP_PtVal(para) fprintf(stdout, "%s : %p\n", #para, (para))
#define BSNAP_ShowErr(err) fprintf(stderr, "%s <%d> : Error <%d>\n", __func__, __LINE__, err)
#else
#define BSNAP_Debug(fmt, args...)
#define BSNAP_FuncIn()
#define BSNAP_FunOut()
#define BSNAP_iVal(para)
#define BSNAP_lVal(para)
#define BSNAP_PtVal(para)
#define BSNAP_ShowErr(err)
#endif
/**********************************************************************************/


/****************************************************************
	variable
****************************************************************/
const CHAR RECSNAPPATH[64] = "/opt/mainCard/snapshot/";

/****************************************************************
	prototype
****************************************************************/
void vBASE_SNAP_INIT(sBASE_VIDEO_SNAP* psSnap,LONG lViChn, LONG lVencChn, eBASE_VENC_PIC_SIZE eSize);
LONG lBASE_VENC_SnapHandle_Init();
LONG lBASE_VENC_SnapHandle_Start(sBASE_VIDEO_SNAP* psSnap);
LONG lBASE_VENC_SaveSnap(VENC_STREAM_S *pstStream);
LONG lBASE_VENC_SnapStart(sBASE_VIDEO_SNAP* psSnap);
LONG lBASE_VENC_SnapHandle_Process(sBASE_VIDEO_SNAP* psSnap);

/****************************************************************
	function
****************************************************************/


/*******************************************************************************
* @brief		: vBASE_SNAP_INIT(sBASE_VIDEO_SNAP* psSnap,LONG lViChn, LONG lVencChn, eBASE_VENC_PIC_SIZE eSize)
* @param		: sBASE_VIDEO_SNAP* psSnap,LONG lViChn, LONG lVencChn, eBASE_VENC_PIC_SIZE eSize
* @retval		: void
* @note 		: init snap attribute
*******************************************************************************/
void vBASE_SNAP_INIT(sBASE_VIDEO_SNAP* psSnap,LONG lViChn, LONG lVencChn, eBASE_VENC_PIC_SIZE eSize) {

	BSNAP_FuncIn();

	/*****************************************/
	{
	
		/* init snap parameter */
		psSnap->VencChn_Snap = lVencChn;
		psSnap->ViChn_SnapExt = lViChn;
		psSnap->eSize = eSize;
		psSnap->eState = BASE_MW_SNAP_NOTUSE;

	}
	/*****************************************/

	BSNAP_FunOut();
	
}

/*******************************************************************************
* @brief		: lBASE_VENC_SaveJPEG(FILE *fpJpegFile, VENC_STREAM_S *pstStream)
* @param		: FILE *fpJpegFile, VENC_STREAM_S *pstStream
* @retval		: LONG
* @note 		: save JPEG
*******************************************************************************/
HI_S32 lBASE_VENC_SaveJPEG(FILE *fpJpegFile, VENC_STREAM_S *pstStream)
{
	VENC_PACK_S*  pstData;
	HI_U32 i;
	HI_S32 eRet = HI_SUCCESS;
	BSNAP_FuncIn();

	/*****************************************/
	{
	    for (i = 0; i < pstStream->u32PackCount; i++)
	    {
	        pstData = &pstStream->pstPack[i];
	        eRet = fwrite(pstData->pu8Addr[0], 1,  pstData->u32Len[0],fpJpegFile);
		 if (eRet != pstData->u32Len[0]) {
		 	printf("[%d]%s\n", errno, strerror(errno));
		 	eRet = HI_FAILURE;
		 	return eRet;
		 }	
	        eRet = fwrite(pstData->pu8Addr[1], 1, pstData->u32Len[1],fpJpegFile);
		 if (eRet != pstData->u32Len[1]) {
		 	printf("[%d]%s\n", errno, strerror(errno));
		 	eRet = HI_FAILURE;
		 	return eRet;		 	
		 }
	    }

	}
	/*****************************************/
	BSNAP_FunOut();

	return HI_SUCCESS;
}

/*******************************************************************************
* @brief		:  lBASE_VENC_SaveSnap(VENC_STREAM_S *pstStream)
* @param		: VENC_STREAM_S *pstStream
* @retval		: LONG
* @note 		: save snap
*******************************************************************************/
LONG lBASE_VENC_SaveSnap(VENC_STREAM_S *pstStream)
{
	LONG s32Ret = HI_SUCCESS;
  	CHAR cFile[128]  = {0};
	CHAR* pcFile = cFile;
	CHAR cFileName1[128];
  	FILE *pFile = NULL;
	SIZE_S stSize;
	//LONG lRet;
	sCOMM_FILENAME_INFO sFileInfo;
	eBASE_MW_SD_ERROR eSdRet = BASE_MW_SD_SUCCESS;
	eBASE_MW_FILESORT_RET eFileSortRet = BASE_MW_FILESORT_SUCCESS;
	sBASE_MW_FILESORT_Info *psFilesortInfo = &(gsSnap.sFileSortInfo);
	sBASE_MW_SD_STAT* psSDStat = &gsSDStat;
	sBASE_API_LEDCTL_Info* psLedCtlInfo = &gsLedCtlInfo;
	eBASE_API_LEDCTL_LedNewStatus eStatus = BASE_API_LEDCTL_NORMAL;
	
	BSNAP_FuncIn();

	/*****************************************/
	{
	
		/* get size */
		lBASE_SYS_GetPicSize(BASE_GLOBAL_NORM, gsSnap.eSize, &stSize);

		/* prepare file info struct */
		strcpy(sFileInfo.cFilePath,BASE_MW_SD_SNAPPATH);
		sFileInfo.eFileType = COMM_FILETYPE_SNAP;
		sFileInfo.lFrameRate = 30;
		sFileInfo.lWidth = stSize.u32Width;
		sFileInfo.lHeight= stSize.u32Height;

		/* get file name */
		s32Ret = eCOMM_GetFileName(pcFile, &sFileInfo);
		
		
#ifdef SD_CHECK_DEBUG		
		/* check whether enough space */
		BSNAP_Debug("mutex lock enter !\n");
		pthread_mutex_lock(&psSDStat->sMutStatus);
		
		if (BASE_MW_SD_SUCCESS != psSDStat->lSdState) 
		{
			eSdRet = eBASE_SD_MW_GETSPACE(psSDStat, TFCARD, ".jpg", 1);
			if(eSdRet != BASE_MW_SD_SUCCESS) {
				/* SD card fail */
				eStatus = BASE_API_LEDCTL_SDFAIL;
				eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);	
				
				psSDStat->lSdState = BASE_MW_SD_INVALID;
				psSDStat->eSnapStat= BASE_MW_SNAP_NOREC;				
				BSNAP_Debug("get space error <%d>\n", eSdRet);
				pthread_mutex_unlock(&psSDStat->sMutStatus);	
				
				return HI_FAILURE;					
			} else {
				psSDStat->lSdState = BASE_MW_SD_SUCCESS;
				psSDStat->eSnapStat= BASE_MW_SNAP_REC;			
			}
		}
		
		//Init  the file list of current audio directory
		if (BASE_MW_SD_SUCCESS == psSDStat->lSdState && BASE_MW_FILESORT_Valid != psFilesortInfo->lState) {
			eFileSortRet = eBASE_MW_FILESORT_Init(psFilesortInfo, psFilesortInfo->lFileMaxLen, RECSNAPPATH);
			if (eFileSortRet == BASE_MW_FILESORT_SUCCESS) {
				//printf("--- Snap:eBASE_MW_FILESORT_Init() OK!---\n");
			} else {
				//printf("--- Snap:eBASE_MW_FILESORT_Init() Failed!---\n");
				pthread_mutex_unlock(&psSDStat->sMutStatus);
				
				return HI_FAILURE;
			}		
		}

		eFileSortRet = eBASE_MW_FILESORT_CheckSpace(
			psFilesortInfo, psSDStat->lTotalKB, BASE_MW_SNAPCARD_RATIO, BASE_MW_SNAP_FILE_SIZEKB);		
		if(eFileSortRet != BASE_MW_FILESORT_SUCCESS) {
			//printf("--- eBASE_MW_FILESORT_CheckSpace() Failed -- \n");
			pthread_mutex_unlock(&psSDStat->sMutStatus);
			
			return HI_FAILURE;
		}	
		/*
		while(((psSDStat->lFreeKB - BASE_MW_SD_FILE_TEST) <= BASE_MW_SNAP_FILE_SIZEKB) 
				|| (psFilesortInfo->lFileLen >= TFCARD_MAX_FILE_NUM)) 
		*/
		while ((BASE_MW_FILESORT_IsNoenough == psFilesortInfo->lIsEnough) || (psFilesortInfo->lFileLen >= TFCARD_MAX_FILE_NUM))
		{
			/* remove the oldest file */
			if (psFilesortInfo->lFileLen == 0) {
				pthread_mutex_unlock(&psSDStat->sMutStatus);
				BSNAP_FunOut();
				
				return HI_FAILURE;				
			}
			//printf("Before remove the FileNum : %ld!\n", psFilesortInfo->lFileLen);
			memset(cFileName1,0,sizeof(cFileName1));
			eFileSortRet = eBASE_MW_FILESORT_Read(psFilesortInfo,cFileName1);
			if(eFileSortRet != BASE_MW_FILESORT_SUCCESS) {
				printf("--- eBASE_MW_FILESORT_Read() %s Failed -- \n",cFileName1);
				pthread_mutex_unlock(&psSDStat->sMutStatus);
				BSNAP_FunOut();
				
				return HI_FAILURE;
			}
			//printf("After remove the FileNum : %ld!\n", psFilesortInfo->lFileLen);
			eFileSortRet = remove(cFileName1);
		  	if(eFileSortRet)
		  	{
				BSNAP_Debug("Remove the FileName : %s failed!\n", cFileName1);
				//perror("Error to remove file");
				eFileSortRet = unlink(cFileName1);
				if(eFileSortRet) {
					BSNAP_Debug("Unlink the FileName : %s failed!\n", cFileName1);
			  		//perror("Error to remove file");	
					if (BASE_MW_SD_SUCCESS == psSDStat->lSdState)
					{
						printf("Can't find the FileName : %s \n", cFileName1);
						continue;
					} else {
						pthread_mutex_unlock(&psSDStat->sMutStatus);
						BSNAP_FunOut();
						
						return HI_FAILURE;					
					}

				}
		  	}
			//printf("Remove the FileName : %s successful!\n", cFileName1);
			
			/* check the space again */
			eSdRet = eBASE_SD_MW_CHECKSTATUS(psSDStat);
			if(eSdRet != BASE_MW_SD_SUCCESS) {
				psSDStat->lSdState = BASE_MW_SD_INVALID;
				psSDStat->eSnapStat= BASE_MW_SNAP_NOREC;						
				BSNAP_Debug("get space error <%d>\n", eSdRet);
				pthread_mutex_unlock(&psSDStat->sMutStatus);
				
				eStatus = BASE_API_LEDCTL_SDFAIL;
				eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);		
				BSNAP_FunOut();
				
				return HI_FAILURE;
			} else {
				psSDStat->lSdState = BASE_MW_SD_SUCCESS;					
			}	

			/* check the free space */
			eFileSortRet = eBASE_MW_FILESORT_CheckSpace(
				psFilesortInfo, psSDStat->lTotalKB, BASE_MW_SNAPCARD_RATIO, BASE_MW_SNAP_FILE_SIZEKB);
			if(eFileSortRet != BASE_MW_FILESORT_SUCCESS) {
				//printf("--- eBASE_MW_FILESORT_CheckSpace() Failed -- \n");
				pthread_mutex_unlock(&psSDStat->sMutStatus);
				
				return HI_FAILURE;
			}			
		}
		pthread_mutex_unlock(&psSDStat->sMutStatus);	
#endif	

	  	pFile = fopen(pcFile, "wb");
	   	if (pFile == NULL)
	    	{
	    	    	//printf("open file err\n");
			BSNAP_FunOut();
	      		return HI_FAILURE;
	   	}
		//printf("Open the FileName : %s successful!\n", pcFile);
		
#ifdef SD_CHECK_DEBUG
		eFileSortRet = eBASE_MW_FILESORT_Write(psFilesortInfo,pcFile);
		if (eFileSortRet != BASE_MW_FILESORT_SUCCESS) {
			printf("--- eBASE_MW_FILESORT_Write() %s Failed -- \n",pcFile);
			BSNAP_FunOut();
			
			return HI_FAILURE;	
		} 		
		//printf("Add the FileName : %s successful!\n", pcFile);
#endif		

	    	s32Ret = lBASE_VENC_SaveJPEG(pFile, pstStream);
	    	if (HI_SUCCESS != s32Ret)
	   		{
				printf("%s: save snap picture failed!\n", __FUNCTION__);
				fclose(pFile);
				eStatus = BASE_API_LEDCTL_SDFAIL;
				eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);	
				BSNAP_FunOut();
				
				return HI_FAILURE;
	    	}	
	    	fclose(pFile);
	}
	
	/*****************************************/
	BSNAP_FunOut();
	return HI_SUCCESS;
}

/*******************************************************************************
* @brief		: lBASE_VENC_SnapStart(sBASE_VIDEO_SNAP* psSnap)
* @param		: sBASE_VIDEO_SNAP* psSnap
* @retval		: LONG
* @note 		: start snap
*******************************************************************************/
LONG lBASE_VENC_SnapStart(sBASE_VIDEO_SNAP* psSnap)
{
	HI_S32 s32Ret;
	VENC_CHN VencChn=psSnap->VencChn_Snap;
	eBASE_VENC_PIC_SIZE enSize=psSnap->eSize;
	VENC_GRP VencGrp = VencChn;
	VENC_CHN_ATTR_S stVencChnAttr;
	SIZE_S stPicSize;
	VENC_ATTR_JPEG_S stJpegAttr;
	 
	BSNAP_FuncIn();

	/*****************************************/
	{

		s32Ret = lBASE_SYS_GetPicSize(BASE_GLOBAL_NORM, enSize, &stPicSize);
		if (HI_SUCCESS != s32Ret)
		{
		    printf("%s: Get picture size failed!\n", __FUNCTION__);
		    return HI_FAILURE;        
		}
		
		 /* Greate Venc Group*/
		s32Ret = HI_MPI_VENC_CreateGroup(VencGrp);
		if (HI_SUCCESS != s32Ret)
		{
		    printf("%s: HI_MPI_VENC_CreateGroup[%d] failed with %#x!\n",\
		             __FUNCTION__, VencGrp, s32Ret);
		    return HI_FAILURE;
		}
		
		 /* Create Venc Channel*/
		stVencChnAttr.stVeAttr.enType = PT_JPEG;
		stJpegAttr.u32MaxPicWidth  = stPicSize.u32Width;
		stJpegAttr.u32MaxPicHeight = stPicSize.u32Height;
		stJpegAttr.u32PicWidth  = stPicSize.u32Width;
		stJpegAttr.u32PicHeight = stPicSize.u32Height;
		stJpegAttr.u32BufSize = stPicSize.u32Width * stPicSize.u32Height * 2;
		stJpegAttr.bByFrame = HI_TRUE;/*get stream mode is field mode  or frame mode*/
		stJpegAttr.bVIField = HI_FALSE;/*the sign of the VI picture is field or frame?*/
		stJpegAttr.u32Priority = 0;/*channels precedence level*/
		memcpy(&stVencChnAttr.stVeAttr.stAttrJpeg, &stJpegAttr, sizeof(VENC_ATTR_JPEG_S));

		s32Ret = HI_MPI_VENC_CreateChn(VencChn, &stVencChnAttr);
		if (HI_SUCCESS != s32Ret)
		{
		    printf("%s: HI_MPI_VENC_CreateChn [%d] faild with %#x!\n",\
		            __FUNCTION__, VencChn, s32Ret);
		    return s32Ret;
		}

	}
	/*****************************************/

	BSNAP_FunOut();

	return HI_SUCCESS;
}

/*******************************************************************************
* @brief		:  lBASE_VENC_SnapHandle_Stop(sBASE_VIDEO_SNAP* psSnap)
* @param		: sBASE_VIDEO_SNAP* psSnap
* @retval		: LONG
* @note 		: stop snap
*******************************************************************************/
LONG lBASE_VENC_SnapHandle_Stop(sBASE_VIDEO_SNAP* psSnap)
{
	HI_S32 s32Ret;
	VENC_GRP VencGrp = psSnap->VencChn_Snap;

	BSNAP_FuncIn();

	/*****************************************/
	{

		if(psSnap->eState != BASE_MW_SNAP_START) {
			debug_info("snap not start!\n");
			return HI_SUCCESS;
		}

		/* Distroy Venc Channel*/
		s32Ret = HI_MPI_VENC_DestroyChn(psSnap->VencChn_Snap);
		if (HI_SUCCESS != s32Ret)
		{
		    printf("%s: HI_MPI_VENC_DestroyChn vechn[%d] failed with %#x!\n", __FUNCTION__,\
		           psSnap->VencChn_Snap, s32Ret);
		    return HI_FAILURE;
		}

		/* Distroy Venc Group*/
		s32Ret = HI_MPI_VENC_DestroyGroup(VencGrp);
		if (HI_SUCCESS != s32Ret)
		{
		    printf("%s: HI_MPI_VENC_DestroyGroup group[%d] failed with %#x!\n",\
		           __FUNCTION__, VencGrp, s32Ret);
		    return HI_FAILURE;
		}
		psSnap->eState = BASE_MW_SNAP_STOP;

	}
	/*****************************************/

	BSNAP_FunOut();

	return HI_SUCCESS;
}

/*******************************************************************************
* @brief		:  lBASE_VENC_SnapHandle_Init()
* @param		: 
* @retval		: LONG
* @note 		: init snap handle
*******************************************************************************/
LONG lBASE_VENC_SnapHandle_Init() {
	LONG lRet=BASE_RET_SUCCESS;

	BSNAP_FuncIn();

	/*****************************************/
	{
	
		vBASE_SNAP_INIT(&gsSnap,  BASE_VI_CH0, BASE_VENC_CH1, BASE_RES_HD1080);

	}
	/*****************************************/

	BSNAP_FunOut();
	
	return lRet;
}

/*******************************************************************************
* @brief		: lBASE_VENC_SnapHandle_Start(sBASE_VIDEO_SNAP* psSnap)
* @param		: sBASE_VIDEO_SNAP* psSnap
* @retval		: LONG
* @note 		: start snap vi chn and create snap venc chn
*******************************************************************************/
LONG lBASE_VENC_SnapHandle_Start(sBASE_VIDEO_SNAP* psSnap) {
	LONG lRet=BASE_RET_SUCCESS;

	BSNAP_FuncIn();

	/*****************************************/
	{
		lRet=lBASE_VENC_SnapStart(psSnap);
		if(lRet!=BASE_RET_SUCCESS) {
			return lRet;
		}

		psSnap->eState=BASE_MW_SNAP_START;

	}
	/*****************************************/

	BSNAP_FunOut();
	
	return lRet;
}

/*******************************************************************************
* @brief		: lBASE_VENC_SnapHandle(sBASE_VIDEO_SNAP* psSnap)
* @param		: sBASE_VIDEO_SNAP* psSnap
* @retval		: LONG
* @note 		: snap handle
*******************************************************************************/
LONG lBASE_VENC_SnapHandle(sBASE_VIDEO_SNAP* psSnap) {
	LONG lRet=BASE_RET_SUCCESS;

	BSNAP_FuncIn();

	/*****************************************/
	{
	
		/*snap init handle*/
		lBASE_VENC_SnapHandle_Init();

		/*snap start handle*/
		lRet=lBASE_VENC_SnapHandle_Start(psSnap);
		if(lRet != BASE_RET_SUCCESS) {
			debug_info("snap start handle error!\n");
			return lRet;
		}
		
		/*snap process*/
		lRet=lBASE_VENC_SnapHandle_Process(psSnap);
		if(BASE_RET_SUCCESS!=lRet)
		{
			debug_info("sanp process fail\n");
			// return lRet;
			/* in this case, should stop the snap handle as well */
		}

		/*snap stop*/
		lRet=lBASE_VENC_SnapHandle_Stop(psSnap);
		if(lRet != BASE_RET_SUCCESS) {
			debug_info("snap stop error!\n");
			return lRet;
		}
	
	}
	/*****************************************/

	BSNAP_FunOut();
	
	return lRet;
}

/*******************************************************************************
* @brief		: lBASE_VENC_SnapHandle_Process(sBASE_VIDEO_SNAP* psSnap)
* @param		: sBASE_VIDEO_SNAP* psSnap
* @retval		: LONG
* @note 		: start snap venc chn to get picture
*******************************************************************************/
LONG lBASE_VENC_SnapHandle_Process(sBASE_VIDEO_SNAP* psSnap)
{
	VENC_CHN SnapChn=psSnap->VencChn_Snap;
	VI_CHN ViChn=psSnap->ViChn_SnapExt;
	MPP_CHN_S stSrcChn, stDestChn;
	VENC_GRP VencGrp;
	struct timeval TimeoutVal;
	fd_set read_fds;
	HI_S32 s32VencFd;
	VENC_CHN_STAT_S stStat;
	VENC_STREAM_S stStream;
	HI_S32 s32Ret = HI_SUCCESS;
	sBASE_API_LEDCTL_Info* psLedCtlInfo = &gsLedCtlInfo;
	sBASE_MW_SD_STAT* psSDStat = &gsSDStat;
	sBASE_API_LED_CTL_Signal* psSignal = psLedCtlInfo->psRecSignal;
	eBASE_API_LEDCTL_LedNewStatus eStatus = BASE_API_LEDCTL_NORMAL;
	eBASE_MW_FILESORT_RET eFileSortRet = BASE_MW_FILESORT_SUCCESS;
	LONG lIsError = 0;
	
	BSNAP_FuncIn();

	/*****************************************/
	{

		/******************************************
		 step 1:  Regist Venc Channel to VencGrp
		******************************************/
		VencGrp = SnapChn;
		s32Ret = HI_MPI_VENC_RegisterChn(VencGrp, SnapChn);
		if (HI_SUCCESS != s32Ret)
		{
		    printf("%s: HI_MPI_VENC_RegisterChn faild with %#x!\n", __FUNCTION__, s32Ret);
		    return HI_FAILURE;
		}

		/******************************************
		 step 2:  Venc Group bind Vi Channel
		******************************************/
		stSrcChn.enModId = HI_ID_VIU;
		stSrcChn.s32DevId = 0;
		stSrcChn.s32ChnId = ViChn;

		stDestChn.enModId = HI_ID_GROUP;
		stDestChn.s32DevId = VencGrp;
		stDestChn.s32ChnId = VencGrp;

		s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
		if (HI_SUCCESS != s32Ret)
		{
		    printf("%s: HI_MPI_SYS_Bind failed with %#x!\n", __FUNCTION__, s32Ret);
		    return HI_FAILURE;
		}

		/******************************************
		 step 3:  Start Recv Venc Pictures
		******************************************/
		s32Ret = HI_MPI_VENC_StartRecvPic(SnapChn);
		if (HI_SUCCESS != s32Ret)
		{
		    printf("%s: HI_MPI_VENC_StartRecvPic faild with%#x!\n", __FUNCTION__, s32Ret);
		    return HI_FAILURE;
		}

		s32VencFd = HI_MPI_VENC_GetFd(SnapChn);
		if (s32VencFd < 0)
		{
			 printf("%s: HI_MPI_VENC_GetFd faild with%#x!\n", __FUNCTION__, s32VencFd);
		    return HI_FAILURE;
		}

		FD_ZERO(&read_fds);
		FD_SET(s32VencFd, &read_fds);

		TimeoutVal.tv_sec  = 2;
		TimeoutVal.tv_usec = 0;
		s32Ret = select(s32VencFd+1, &read_fds, NULL, NULL, &TimeoutVal);
		if (s32Ret < 0) 
		{
		    printf("%s: snap select failed!\n", __FUNCTION__);
		    return HI_FAILURE;
		}
		else if (0 == s32Ret) 
		{
		    printf("%s: snap time out!\n", __FUNCTION__);
		    return HI_FAILURE;
		}
		else
		{
		    if (FD_ISSET(s32VencFd, &read_fds))
		    {
		        s32Ret = HI_MPI_VENC_Query(SnapChn, &stStat);
		        if (s32Ret != HI_SUCCESS)
		        {
		            printf("%s: HI_MPI_VENC_Query failed with %#x!\n", __FUNCTION__, s32Ret);
		            return HI_FAILURE;
		        }
				MALLOC(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
		        stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
		        if (NULL == stStream.pstPack)
		        {
		            printf("%s: malloc memory failed!\n", __FUNCTION__);
		            return HI_FAILURE;
		        }

		        stStream.u32PackCount = stStat.u32CurPacks;
		        s32Ret = HI_MPI_VENC_GetStream(SnapChn, &stStream, HI_TRUE);
		        if (HI_SUCCESS != s32Ret)
		        {
		            printf("%s: HI_MPI_VENC_GetStream failed with %#x!\n", __FUNCTION__, s32Ret);
				HI_MPI_VENC_StopRecvPic(SnapChn);
				HI_MPI_SYS_UnBind(NULL, &stDestChn);
				HI_MPI_VENC_UnRegisterChn(SnapChn);
				lBASE_VENC_SnapHandle_Stop(psSnap);
		            free(stStream.pstPack);
		            stStream.pstPack = NULL;
		            return HI_FAILURE;
		        }

			/* snap event start*/
			//printf("=== Enter into Snap process ===\n");
			
#ifdef SD_CHECK_DEBUG		
			// SD is not init, need to update the SD space parammeter
			pthread_mutex_lock(&psSDStat->sMutStatus);
			
			if (BASE_MW_SD_SUCCESS == psSDStat->lSdState ) 
			{
				eStatus = BASE_API_LEDCTL_SNAP;
				/* new status should signal thread */
				{
					/* toggle the signal status */
					pthread_mutex_lock(&psSignal->mutStatus);
					
					if((psLedCtlInfo->psLedInfo->lStatus & COMM_MW_LED_COLORMASK) != BASE_API_LEDCTL_NOFEEDBACK) {
						psSignal->eStatus = BASE_API_LEDCTL_HAVESIG;
						//pthread_mutex_unlock(&psSignal->mutStatus);

						/* notified the handle thread */
						BSNAP_Debug("SNAP Sending signal\n");
						pthread_cond_signal(&psSignal->condStatus);
					}		
					
					pthread_mutex_unlock(&psSignal->mutStatus);
				}		
				eBASE_API_LEDCTL_ChangeLed(psLedCtlInfo->psArg,eStatus);			
			} 

			pthread_mutex_unlock(&psSDStat->sMutStatus);
#endif				

		        s32Ret = lBASE_VENC_SaveSnap(&stStream);
		        if (HI_SUCCESS != s32Ret)
		        {
				printf("%s: lBASE_VENC_SaveSnap failed with %#x!\n", __FUNCTION__, s32Ret);
#ifdef SD_CHECK_DEBUG
				eFileSortRet = eBASE_MW_FILESORT_DeInit(&gsSnap.sFileSortInfo);
				if (eFileSortRet == BASE_MW_FILESORT_SUCCESS) {
					printf("mainCard vfile error: deinit the filesort resource!\n");
					printf("--- Snap Card:eBASE_MW_FILESORT_DeInit() OK!---\n");
				} else {
					printf("mainCard vfile error: deinit the filesort resource!\n");
					printf("--- Main Card:eBASE_MW_FILESORT_DeInit() Fail!---\n");
				}		
#endif			
				/* record the error and do release the resource */
				lIsError = 1;
		            // return HI_FAILURE;
		        }
		
		        s32Ret = HI_MPI_VENC_ReleaseStream(SnapChn, &stStream);
		        if (s32Ret)
		        {
		            printf("%s: HI_MPI_VENC_ReleaseStream failed with %#x!\n", __FUNCTION__, s32Ret);
		            free(stStream.pstPack);
		            stStream.pstPack = NULL;
		            return HI_FAILURE;
		        }

		        free(stStream.pstPack);
		        stStream.pstPack = NULL;
		}
		}

		s32Ret = HI_MPI_VENC_StopRecvPic(SnapChn);
		if (s32Ret != HI_SUCCESS)
		{
		    printf("%s: HI_MPI_VENC_StopRecvPic failed with %#x!\n", __FUNCTION__, s32Ret);
		    return HI_FAILURE;
		}

		/* unbind vi and venc group */
		stSrcChn.enModId = HI_ID_VIU;
		stSrcChn.s32DevId = 0;
		stSrcChn.s32ChnId = ViChn;

		stDestChn.enModId = HI_ID_GROUP;
		stDestChn.s32DevId = VencGrp;
		stDestChn.s32ChnId = VencGrp;
		s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
		if (HI_SUCCESS != s32Ret)
		{
		    printf("%s: HI_MPI_SYS_UnBind[%d] failed with %#x!\n", __FUNCTION__,
		        VencGrp, s32Ret);
		    return HI_FAILURE;
		}

		s32Ret = HI_MPI_VENC_UnRegisterChn(SnapChn);
		if (s32Ret != HI_SUCCESS)
		{
		    printf("%s: HI_MPI_VENC_UnRegisterChn failed with %#x!\n", __FUNCTION__, s32Ret);
		    return HI_FAILURE;
		}

		stDestChn.enModId = HI_ID_GROUP;
		stDestChn.s32ChnId = VencGrp;

		/* any error happen */
		if (lIsError) {
			BSNAP_Debug("error happen !\n");
			return HI_FAILURE;
		}

	}
	/*****************************************/

	BSNAP_FunOut();
	
    return HI_SUCCESS;
}

#ifdef __cplusplus
}
#endif
