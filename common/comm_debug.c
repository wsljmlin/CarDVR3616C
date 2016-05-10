#ifdef __cplusplus
extern "C"{
#endif

#include "comm_common.h"

/* time measure */
void vCOMM_TimeMeasure(CHAR* pcCmt){

	ULONG lDUsec;
	static struct timeval sTimeStart;
	struct timeval sTimeEnd;
	static ULONG lFst = 1;

	/* judge whether first time */
	if(lFst){
		gettimeofday(&sTimeStart, NULL);
		lFst=0;
	}

	gettimeofday(&sTimeEnd, NULL);

	lDUsec = (sTimeEnd.tv_sec * 1000000 + sTimeEnd.tv_usec) - (sTimeStart.tv_sec * 1000000 + sTimeStart.tv_usec);

	printf("%s DiffTime : <%ld>\n", pcCmt, lDUsec);
	sTimeStart=sTimeEnd;
}

void vCOMM_SetBit(sCOMM_LONGTYPE* psLongType, LONG lBit){
	if(lBit>COMM_LONGTYPE_PERMAX){
		/* indicate greater than 4B */
		psLongType->lType1|=(COMM_BIT(lBit-COMM_LONGTYPE_PERMAX));
	} else {
		/* indicate not greater than 4B */
		psLongType->lType0|=(COMM_BIT(lBit));
	}
}
void vCOMM_UnsetBit(sCOMM_LONGTYPE* psLongType, LONG lBit){
	if(lBit>COMM_LONGTYPE_PERMAX){
		/* indicate greater than 4B */
		psLongType->lType1&=(~(COMM_BIT(lBit-COMM_LONGTYPE_PERMAX)));
	} else {
		/* indicate not greater than 4B */
		psLongType->lType0&=(~(COMM_BIT(lBit)));
	}
}
LONG vCOMM_IsSetBit(sCOMM_LONGTYPE* psLongType, LONG lBit){
	LONG lRet=COMM_LONGTYPE_NOTSET;
	LONG lRes=0;
	
	if(lBit>COMM_LONGTYPE_PERMAX){
		/* indicate greater than 4B */
		lRes=psLongType->lType1&(COMM_BIT(lBit-COMM_LONGTYPE_PERMAX));
	} else {
		/* indicate not greater than 4B */
		lRes=psLongType->lType0&(COMM_BIT(lBit));
	}
	if(lRes)
		lRet=COMM_LONGTYPE_ISSET;
	return lRet;
}

/*------------------------------------------------------------------------------*/
/*!
	@brief	Print memory data
	@param	LONG lStartPhyAddr					[in]	phyadd of start 
			LONG lLength						[in]	length
			LONG *plRegData						[in]	store memory date

	@retval	none										
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_MemPrint(ULONG ulStartMemAddr,LONG lLength,UCHAR *pucMemData)
{
	ULONG lCnt = 0;
			
	UCHAR* pucMemDataTmp = pucMemData;
	ULONG* pulMemDataTmp = (ULONG*)pucMemData;

	/* modified value */
	ULONG ulMemAddrTmp;

	/* printf length */
	ULONG ulLenPrt=0;

	/* input length */
	ULONG ulLenMod = (ULONG)lLength;
	/* head length */
	ULONG ulLenHead;
	ULONG ulLenTail;


	if(!ulStartMemAddr)
	{
		/* video buffer print */
		while((lCnt++) < ulLenMod){
			printf("%02x ",*pucMemDataTmp++);
			
			if(!(lCnt%16)){
				printf("\n");
			}
		}
		
		printf("\n");		
	}
	else
	{
		/* modify the ulStartMemAddr and the lLength */
		ulMemAddrTmp = ulStartMemAddr&(~0xf);
		ulLenHead = ulStartMemAddr - ulMemAddrTmp;
		ulLenTail = ((ulLenHead+ulLenMod)&0xf) ? (16 - ((ulLenHead+ulLenMod)&0xf)):0;

		/* calculate all the length (4Bytes) */
		ulLenMod += (ulLenHead +ulLenTail);
		ulLenMod >>= 2;
		ulLenHead >>= 2;
		ulLenTail >>=2;
		ulLenPrt = ulLenMod - ulLenTail;
		
		while(lCnt < ulLenMod ){
			if(!(lCnt%4)){
				printf("\n%#08lx : ",ulMemAddrTmp);
				ulMemAddrTmp += 16;
			}
			if((lCnt >=  ulLenHead) && (lCnt < ulLenPrt)){
				printf(" %08lx",*pulMemDataTmp++);
			}
			else{
				printf(" --------");
			}
			lCnt++;
		}
		
	}

}

/*******************************************************************************
* @brief		: eCOMM_GetFileName(CHAR* pcFilename, sCOMM_FILENAME_INFO* psFileInfo)
* @param		: CHAR* pcFilename, sCOMM_FILENAME_INFO psFileInfo
* @retval		: eCOMM_RET_STAT
* @note		: get filename
*******************************************************************************/
eCOMM_RET_STAT eCOMM_GetFileName(CHAR* pcFilename, sCOMM_FILENAME_INFO* psFileInfo) {
	eCOMM_RET_STAT eRet = COMM_RET_SUCCESS;
	CHAR cSuffix[16];
	struct tm *tm_loc;
	time_t tTime;

	/* check input */
	if(psFileInfo->lWidth <= 0 || psFileInfo->lHeight <= 0 || psFileInfo->lFrameRate <= 0) {
		printf("error input width <%ld> height <%ld> framerate <%ld>\n",
			psFileInfo->lWidth, psFileInfo->lHeight, psFileInfo->lFrameRate);
		return COMM_RET_FAIL;
	}

	/* make suffix */
	memset(cSuffix, 0, sizeof(cSuffix));
	switch(psFileInfo->eFileType) {
		case COMM_FILETYPE_SNAP :
			strcpy(cSuffix, "_snap.jpg");
			break;
		case COMM_FILETYPE_REC :
			strcpy(cSuffix, "_rec.h264");
			break;
		case COMM_FILETYPE_RAW :
			strcpy(cSuffix, "_raw.yuv");
			break;
		default :
			printf("error input formate <%d>\n", psFileInfo->eFileType);
			return COMM_RET_FAIL;
	};

	/* init time */
	tTime = time(NULL);
	tm_loc = localtime(&tTime);

	/* make file name */
	sprintf(pcFilename, "%s%d_%02d_%02d_%02d_%02d_%02d_%ld_%ld@%ld%s", 
		psFileInfo->cFilePath, 1900 + tm_loc->tm_year, tm_loc->tm_mon + 1, tm_loc->tm_mday, tm_loc->tm_hour,
		tm_loc->tm_min, tm_loc->tm_sec, psFileInfo->lWidth, psFileInfo->lHeight, psFileInfo->lFrameRate,
		cSuffix);
	debug_info("File name : %s\n", pcFilename);

	return eRet;
}

/*******************************************************************************
* @brief		: vCOMM_PrintTime(CHAR* pcMsg, time_t* psTime)
* @param	: CHAR* pcMsg, time_t* psTime
* @retval		: void
* @note		: print current time
*******************************************************************************/
void vCOMM_PrintTime(CHAR* pcMsg, time_t* psTime) {
	time_t sTime;
	struct tm* psParseTime;
	CHAR cTimePrint[64];

	/* get time */
	if(!psTime) {
		sTime = time(NULL);
	} else {
		sTime = *psTime;
	}

	/* translate to parser time */
	psParseTime = localtime(&sTime);

	/* print current time */
	if(pcMsg) {
		printf("%s ", pcMsg);
	}

	strftime(cTimePrint, 64, "[ %F %T ]", psParseTime);
	printf("%s\n", cTimePrint);
}


#ifdef __cplusplus
}
#endif