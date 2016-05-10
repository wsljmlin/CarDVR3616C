#ifdef __cplusplus
extern "C" {
#endif

#include "comm_pool.h"
#include <assert.h>

/*********************************************************
	macro / enum
*********************************************************/
//#define COMM_POOL_DEBUG
#ifdef COMM_POOL_DEBUG
#define CPD_Debug(fmt, args...) fprintf(stdout, "[ CPD ] : <%s() : %d> "fmt, __func__, __LINE__, ##args) 
#define CPD_FuncIn() fprintf(stdout, "[ CPD ] : %s() in\n", __func__)
#define CPD_FuncOut() fprintf(stdout, "[ CPD ] : %s() out\n", __func__)
#define CPD_Val(var) fprintf(stdout, "[ CPD ] : <%s() : %d> %s < %ld > @ %p\n", __func__, __LINE__, #var, var, &var)
#define CPD_iVal(var) fprintf(stdout, "[ CPD ] : <%s() : %d> %s < %d > @ %p\n", __func__, __LINE__, #var, var, &var)
#define CPD_Err(err) fprintf(stderr, "[ CPD ] : <%s() : %d> Err <%d>\n", __func__, __LINE__, err)
#define CPD_PtVal(pvar) fprintf(stdout, "[ CPD ] : <%s() : %d> pt %s @ %p\n", __func__, __LINE__, #pvar, pvar)
#else
#define CPD_Debug(fmt, args...)
#define CPD_FuncIn()
#define CPD_FuncOut()
#define CPD_Val(var)
#define CPD_iVal(var)
#define CPD_Err(err)
#define CPD_PtVal(pvar)
#endif

typedef enum {
	COMM_POOL_FIRST,
	COMM_POOL_MIDDLE,
	COMM_POOL_LAST,
} eCOMM_POOL_BlkMode;

/* for detail debug information */
// #define COMM_POOL_DEBUG_DETAIL
#ifdef COMM_POOL_DEBUG_DETAIL
#define BLK_INFO() do { \
	printf("%-8ld  %-8d  0x%08lx  0x%08lx  %-8ld  %-8ld  %-8ld  0x%08lx  0x%08lx 0x%08lx  0x%08lx  ", \
		psBlk->lBlkOrder, \
		psBlk->eBlkStat, \
		(ULONG)psBlk->pucHeader, \
		(ULONG)psBlk->pucWp, \
		psBlk->lBlkSize, \
		psBlk->lRealBlkSize, \
		psBlk->lCurReadCnt, \
		(ULONG)psInfo->psPayloadInfo->pucHeader ,\
		(ULONG)psInfo->psPayloadInfo->pucReader ,\
		(ULONG)psInfo->psPayloadInfo->pucWriter ,\
		(ULONG)psInfo->psPayloadInfo->pucTail \
	); \
} while(0)

#define BLK_INFOW() do { \
	printf("[ WRITER ] : "); \
	BLK_INFO(); \
	printf("\n"); \
} while(0)

#define BLK_INFOR() do { \
	printf("[ READER ] : "); \
	BLK_INFO(); \
	printf("[ %s ]\n", psInfo->sConsumer[lConsumerKey].pcConsumerName); \
} while(0)
#else
#define BLK_INFO()
#define BLK_INFOW()
#define BLK_INFOR()
#endif

/*********************************************************
	struct
*********************************************************/

/*********************************************************
	prototype
*********************************************************/
sCOMM_POOL_Info* psCOMM_POOL_GetPoolFromId(LONG lId);
sCOMM_POOL_Block* psCOMM_POOL_GetFreeBlk(LONG lId);
eCOMM_POOL_Err eCOMM_POOL_GetFreePool(LONG* plId);
eCOMM_POOL_Err eCOMM_POOL_InitBlk(sCOMM_POOL_Block* psBlk, LONG lMaxBlkSize, eCOMM_POOL_BlkMode eBlkMode);
eCOMM_POOL_Err eCOMM_POOL_DeinitBlk(sCOMM_POOL_Block* psBlk, eCOMM_POOL_BlkMode eBlkMode);
eCOMM_POOL_Err eCOMM_POOL_SetAllConsumerReader(sCOMM_POOL_Info* psInfo, sCOMM_POOL_Block* psBlk, LONG lConsumerCnt);
eCOMM_POOL_Err eCOMM_POOL_SetConsumerReader(sCOMM_POOL_Info* psInfo, LONG lConsumerKey, sCOMM_POOL_Block* psBlk);
eCOMM_POOL_Err eCOMM_POOL_GetConsumerReader(sCOMM_POOL_Info* psInfo, LONG lConsumerKey, sCOMM_POOL_Block** ppsBlk);
eCOMM_POOL_Err eCOMM_POOL_GetConsumerKey(sCOMM_POOL_Info* psInfo, LONG* plConsumerKey);
eCOMM_POOL_Err eCOMM_POOL_RemoveConsumerReader(sCOMM_POOL_Info* psInfo, LONG lConsumerKey);
eCOMM_POOL_Err eCOMM_POOL_ShowAllConsumerReader(sCOMM_POOL_Info* psInfo);
eCOMM_POOL_Err eCOMM_POOL_GetConsumerStat(sCOMM_POOL_Info* psInfo, LONG lConsumerKey, eCOMM_POOL_ConsumerStat* peStat);
eCOMM_POOL_Err eCOMM_POOL_SetConsumerStat(sCOMM_POOL_Info* psInfo, LONG lConsumerKey, eCOMM_POOL_ConsumerStat eStat);
eCOMM_POOL_Err eCOMM_POOL_ResetConsumer(sCOMM_POOL_Info* psInfo);
eCOMM_POOL_Err eCOMM_POOL_ShowReadList(sCOMM_POOL_Info* psInfo);
eCOMM_POOL_Err eCOMM_POOL_SetBlkOrder(sCOMM_POOL_Info* psInfo, sCOMM_POOL_Block* psBlk);
eCOMM_POOL_Err eCOMM_POOL_ShowPoolInfo(sCOMM_POOL_Info* psInfo);
eCOMM_POOL_Err eCOMM_POOL_PreWriteHandle(sCOMM_POOL_Info* psInfo, LONG* plWriteLen, eCOMM_POOL_WriteStat eWtStat);
eCOMM_POOL_Err eCOMM_POOL_PushReader(sCOMM_POOL_Info* psInfo);
eCOMM_POOL_Err eCOMM_POOL_ShowBlock(sCOMM_POOL_Block* psBlk);
eCOMM_POOL_Err eCOMM_POOL_Update(sCOMM_POOL_Info* psInfo, sCOMM_POOL_Block* psBlk, LONG lConsumerKey);

/*********************************************************
	function
*********************************************************/

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_InitAll()
* @param		: NULL
* @retval		: eCOMM_POOL_Err
* @note		: init all the pool
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_InitAll() {
	eCOMM_POOL_Err eErr = COMM_POOL_SUCCESS;
	sCOMM_POOL_Info* psInfo = NULL;
	LONG lPoolIdTmp;

	CPD_FuncIn();

	{
		for(lPoolIdTmp = 0; lPoolIdTmp != COMM_POOL_MAXPOOL; lPoolIdTmp ++) {
			psInfo = psCOMM_POOL_GetPoolFromId(lPoolIdTmp);

			psInfo->ePoolStat = COMM_POOL_Invalid;
		}
	}

	CPD_FuncOut();

	return eErr;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_Init(LONG* plId, LONG lPayLoadSize, LONG lBlkCnt, LONG lMaxBlkSize)
* @param		: LONG* plId, LONG lPayLoadSize, LONG lBlkCnt, LONG lMaxBlkSize
* @retval		: eCOMM_POOL_Err
* @note		: get a free pool
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_Init(LONG* plId, LONG lPayLoadSize, LONG lBlkCnt, LONG lMaxBlkSize) {
	eCOMM_POOL_Err eErr = COMM_POOL_SUCCESS;
	sCOMM_POOL_Info* psInfo = NULL;
	sCOMM_POOL_Block* psBlk = NULL;
	LONG lBlkCntTmp;

	CPD_FuncIn();

	{

		/* check the valid of argument */
		if(!(lPayLoadSize && lBlkCnt && lMaxBlkSize)) {
			/* indicate one of the argument is zero, wrong */
			CPD_Debug("wrong argument lPayLoadSize <%ld>, lBlkCnt <%ld>, lMaxBlkSize <%ld>\n", lPayLoadSize, lBlkCnt, lMaxBlkSize);
			return COMM_POOL_PARAERR;
		}
		
		eErr = eCOMM_POOL_GetFreePool(plId);
		if(eErr != COMM_POOL_SUCCESS) {
			CPD_Err(eErr);
			return eErr;
		}

		psInfo = psCOMM_POOL_GetPoolFromId(*plId);
		psInfo->lConsumerCnt = 0;
		psInfo->lBlkCnt = lBlkCnt;
		psInfo->lBlkOrder = 0;
		psInfo->lBlkMax = lMaxBlkSize;
		psInfo->psWriter = NULL;
		psInfo->psReader = NULL;
		eCOMM_POOL_ResetConsumer(psInfo);
		MALLOC(lBlkCnt * sizeof(sCOMM_POOL_Block));
		MALLOC(sizeof(sCOMM_POOL_PAYLOAD_Info));
		psInfo->psBlk = (sCOMM_POOL_Block*)malloc(lBlkCnt * sizeof(sCOMM_POOL_Block));
		psInfo->psPayloadInfo = (sCOMM_POOL_PAYLOAD_Info*)malloc(sizeof(sCOMM_POOL_PAYLOAD_Info));
		eCOMM_POOL_PAYLOAD_Init(psInfo->psPayloadInfo, lPayLoadSize, lMaxBlkSize);

		/* init block */
		for(lBlkCntTmp = 0; lBlkCntTmp != lBlkCnt; lBlkCntTmp ++) {
			psBlk = psInfo->psBlk + lBlkCntTmp;
			eCOMM_POOL_InitBlk(psBlk, psInfo->lBlkMax, COMM_POOL_FIRST);
		}

		/* init mutex and condition flag */
		pthread_rwlock_init(& psInfo->rwConsumerCnt, NULL);
		pthread_mutex_init(& psInfo->mutBlkOrder, NULL);
		pthread_mutex_init(& psInfo->mutReadNotify, NULL);
		pthread_cond_init(& psInfo->condReadNotify, NULL);

		pthread_mutex_init(&psInfo->mutFinished, NULL);
		pthread_cond_init(&psInfo->condFinished, NULL);

		sem_init(&psInfo->semResourceCnt, 0, lBlkCnt);

		/* toggle the pool stat */
		psInfo->ePoolStat = COMM_POOL_Valid;

		CPD_Debug("Pool <%ld> init\n", *plId);
		
	}

	CPD_FuncOut();

	return eErr;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_Attach(LONG lId, LONG* plConsumerKey, CHAR* pcName)
* @param		: LONG lId, LONG* plConsumerKey, CHAR* pcName
* @retval		: eCOMM_POOL_Err
* @note		: attach a pool, add a consumer and return a consumerkey, the key should use in read function
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_Attach(LONG lId, LONG* plConsumerKey, CHAR* pcName) {
	eCOMM_POOL_Err eErr = COMM_POOL_SUCCESS;
	sCOMM_POOL_Info* psInfo = NULL;
	pthread_rwlock_t* pmutCnt = NULL;

	CPD_FuncIn();

	{
		psInfo = psCOMM_POOL_GetPoolFromId(lId);
		if(!psInfo) {
			CPD_Debug("error Id\n");
			return COMM_POOL_INVALIDID;
		}
		
		if(psInfo->lConsumerCnt == COMM_POOL_MAXCONSUMER) {
			/* indicate reach the max consumer */
			CPD_Debug("reach the max consumer\n");
			return COMM_POOL_REACHMAXCONSUMER;
		}
	
		pmutCnt = &psInfo->rwConsumerCnt;
		pthread_rwlock_wrlock(pmutCnt);

		/* increase the consumer count */
		eCOMM_POOL_GetConsumerKey(psInfo, plConsumerKey);
		psInfo->lConsumerCnt ++;

		/* init consumer */
		eCOMM_POOL_SetConsumerReader(psInfo, *plConsumerKey, psInfo->psReader);
		eCOMM_POOL_SetConsumerStat(psInfo, *plConsumerKey, COMM_POOL_ConsumerIsUse);
		if(pcName) {
			/* consumer name */
			psInfo->sConsumer[*plConsumerKey].pcConsumerName = strdup(pcName);
		}
		CPD_Debug("Get Consumer Key <%ld>\n", *plConsumerKey);
		CPD_PtVal(psInfo->psReader);
		
		pthread_rwlock_unlock(pmutCnt);
	}

	CPD_FuncOut();

	return eErr;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_Read(LONG lId, pvCALLBACK pvPostFunc, void* pvArg, LONG lConsumerKey)
* @param		: LONG lId, pvCALLBACK pvPostFunc, void* pvArg, LONG lConsumerKey
* @retval		: eCOMM_POOL_Err
* @note		: read from pool and do something with pvPostFunc, the lConsumerKey is the return val from attach
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_Read(LONG lId, pvCALLBACK pvPostFunc, void* pvArg, LONG lConsumerKey) {
	eCOMM_POOL_Err eErr = COMM_POOL_SUCCESS;
	sCOMM_POOL_Info* psInfo = NULL;
	sCOMM_POOL_Block* psBlk = NULL;
	sCOMM_POOL_CBFuncArg sCBFuncArg;

	CPD_FuncIn();

	{
		/* get current block */
		psInfo = psCOMM_POOL_GetPoolFromId(lId);
		if(!psInfo) {
			CPD_Debug("error Id\n");
			return COMM_POOL_INVALIDID;
		}

		/* get cosumer reader */
		eErr = eCOMM_POOL_GetConsumerReader(psInfo, lConsumerKey, &psBlk);
		if(eErr != COMM_POOL_SUCCESS) {
			CPD_Debug("wrong consumer <%ld>\n", lConsumerKey);
			return eErr;
		}

		/* should wait writer init first */
		while(!psBlk) {
			pthread_mutex_lock(&psInfo->mutReadNotify);
			pthread_cond_wait(&psInfo->condReadNotify, &psInfo->mutReadNotify);
			pthread_mutex_unlock(&psInfo->mutReadNotify);

			eCOMM_POOL_GetConsumerReader(psInfo, lConsumerKey, &psBlk);
		}
		
		/* judge whether ready to read */
		pthread_mutex_lock(&psInfo->mutReadNotify);
		while(!(psBlk->eBlkStat & COMM_POOL_BlkReadyToRead)) {
			CPD_Debug("consumer <%ld> waiting for resoure\n", lConsumerKey);
			pthread_cond_wait(&psInfo->condReadNotify, &psInfo->mutReadNotify);
		}
		pthread_mutex_unlock(&psInfo->mutReadNotify);
		CPD_Debug("consumer <%ld> get resource\n", lConsumerKey);

		/* bug fix : may be this time, system already quit */
		if(psInfo->ePoolStat == COMM_POOL_Invalid) {
			/* indicate system already quit */
			CPD_Debug("competetion occur, system already quit\n");
			return COMM_POOL_NEEDTOQUIT;
		}

		/* for test */
		BLK_INFOR();
		#if 0
		LONG lIsISlice = 0;
		eCOMM_UTIL_GetFrameType(&lIsISlice, psBlk->pucHeader + 1);
		printf("%-10s : read block <%ld> finished !! %s\n", psInfo->sConsumer[lConsumerKey].pcConsumerName, psBlk->lBlkOrder,
			lIsISlice ? "[ I Slice ]" : "");
		#endif

		if((psInfo->ePoolStat & COMM_POOL_WriterQuit) && (psBlk == psInfo->psWriter)) {
			CPD_Debug("Indicate quit!\n");
			CPD_Val(lConsumerKey);
			CPD_PtVal(psBlk);
			CPD_PtVal(psInfo->psWriter);
			eCOMM_POOL_ShowAllConsumerReader(psInfo);

			/* notify to the deinit thread */
			pthread_mutex_lock(&psInfo->mutFinished);
			pthread_cond_broadcast(&psInfo->condFinished);
			pthread_mutex_unlock(&psInfo->mutFinished);
			
			return COMM_POOL_NEEDTOQUIT;
		}

		/* prepare argument and call callback function if not NULL */
		if(pvPostFunc) {
			memset(&sCBFuncArg, 0, sizeof(sCOMM_POOL_CBFuncArg));
			sCBFuncArg.pucHeader = psBlk->pucHeader;
			sCBFuncArg.lLen = psBlk->lRealBlkSize;
			sCBFuncArg.pvArg = pvArg;
			pvPostFunc((void*)& sCBFuncArg);
		}

		/* for test */
		#if 0
		LONG lIsISlice = 0;
		eCOMM_UTIL_GetFrameType(&lIsISlice, psBlk->pucHeader + 1);
		printf("%-10s : read block <%ld> finished !! %s\n", psInfo->sConsumer[lConsumerKey].pcConsumerName, psBlk->lBlkOrder,
			lIsISlice ? "[ I Slice ]" : "");
		#endif

		/* first judge whether all the resource is read */
		while(!(psBlk->pucNextBlk)) {
			pthread_mutex_lock(&psInfo->mutReadNotify);
			pthread_cond_wait(&psInfo->condReadNotify, &psInfo->mutReadNotify);
			pthread_mutex_unlock(&psInfo->mutReadNotify);
		}

		/* update the pool */
		eErr = eCOMM_POOL_Update(psInfo, psBlk, lConsumerKey);
	}

	CPD_FuncOut();
		
	return eErr;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_Write(LONG lId, UCHAR* pucSrc, LONG lSrcLen, eCOMM_POOL_WriteStat eWtStat)
* @param		: LONG lId, UCHAR* pucSrc, LONG lSrcLen, eCOMM_POOL_WriteStat eWtStat
* @retval		: eCOMM_POOL_Err
* @note		: write to a pool
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_Write(LONG lId, UCHAR* pucSrc, LONG lSrcLen, eCOMM_POOL_WriteStat eWtStat) {
	eCOMM_POOL_Err eErr = COMM_POOL_SUCCESS;
	eCOMM_POOL_PAYLOAD_Ret ePayloadRet = COMM_POOL_PAYLOAD_SUCCESS;
	sCOMM_POOL_Info* psInfo = NULL;
	sCOMM_POOL_Block* psBlkNew = NULL;
	sCOMM_POOL_Block* psBlk = NULL;
	sCOMM_POOL_PAYLOAD_Info* psPayLoad = NULL;
	LONG lWriteLen = lSrcLen;

	CPD_FuncIn();

	{
		/* valid check */
		if(!pucSrc) {
			CPD_Debug("src data is NULL");
			return COMM_POOL_SRCNULL;
		}

		psInfo = psCOMM_POOL_GetPoolFromId(lId);
		if(!psInfo) {
			CPD_Debug("error Id\n");
			return COMM_POOL_INVALIDID;
		}
		if(!(psInfo->psWriter)) {
			/* indicate the first one */
			sem_wait(&psInfo->semResourceCnt);
			psBlk = psCOMM_POOL_GetFreeBlk (lId);
			assert(psBlk->lBlkOrder == 0);
			
			/* init the block */
			eCOMM_POOL_InitBlk(psBlk, psInfo->lBlkMax, COMM_POOL_MIDDLE);
			
			psInfo->psWriter = psBlk;
			psInfo->psReader = psBlk;
			eCOMM_POOL_SetAllConsumerReader(psInfo, psInfo->psReader, psInfo->lConsumerCnt);

			/* broadcast to the readers */
			pthread_mutex_lock(&psInfo->mutReadNotify);
			pthread_cond_broadcast(&psInfo->condReadNotify);
			pthread_mutex_unlock(&psInfo->mutReadNotify);
			
		}

		psBlk = psInfo->psWriter;
		psPayLoad = psInfo->psPayloadInfo;
		if(!psBlk->pucHeader) {
			eCOMM_POOL_PAYLOAD_GetWriter(psPayLoad, &psBlk->pucHeader);
		}

		/* prewrite handle, assure write len is ok */
		eCOMM_POOL_PreWriteHandle(psInfo, &lWriteLen, eWtStat);

		/* write */
		if (lWriteLen != 0) {
			ePayloadRet = eCOMM_POOL_PAYLOAD_Write(psPayLoad, pucSrc, lWriteLen);
		}

		/* update writer */
		eCOMM_POOL_PAYLOAD_GetWriter(psPayLoad, &psBlk->pucWp);

		/* prepare for next write block */
		if((eWtStat & COMM_POOL_NEWBLOCK) && (ePayloadRet == COMM_POOL_PAYLOAD_SUCCESS)) {
		
			/* prepare the special block */
			if(psBlk->pucWp < psBlk->pucHeader) {
				eCOMM_POOL_PAYLOAD_WriteExtraData(psPayLoad, psPayLoad->pucHeader, psPayLoad->pucWriter - psPayLoad->pucHeader);
			}

			/* test */
			BLK_INFOW();
			#if 0
			LONG lIsISlice = 0;
			eCOMM_UTIL_GetFrameType(&lIsISlice, psBlk->pucHeader + 1);
			printf("\nwriter will write block <%ld> %s [ %ld ]\n", psBlk->lBlkOrder,
				lIsISlice ? "[ I Slice ]" : "", psBlk->lRealBlkSize);
			#endif

			/* notify all the reader */
			pthread_mutex_lock(&psInfo->mutReadNotify);
			psBlk->eBlkStat |= COMM_POOL_BlkReadyToRead;
			pthread_cond_broadcast(&psInfo->condReadNotify);
			pthread_mutex_unlock(&psInfo->mutReadNotify);
			sem_wait(&psInfo->semResourceCnt);
			psBlkNew = psCOMM_POOL_GetFreeBlk(lId);
			if(!psBlkNew){
				CPD_Debug("should not enter here!\n");
			}

			/* init the block */
			eCOMM_POOL_InitBlk(psBlkNew, psInfo->lBlkMax, COMM_POOL_MIDDLE);

			/* add to link */
			psBlk->pucNextBlk = psBlkNew;
			psInfo->psWriter = psBlkNew;

		}

		/* judge whether to quit */
		if(psInfo->ePoolStat & COMM_POOL_ValidToInvalid) {

			CPD_Debug("writer ready to quit\n");

			/* judge whether to quit */
			psInfo->ePoolStat |= COMM_POOL_WriterQuit;

			/* notice : we must do this to inform the reader release the block */
			psInfo->psWriter->eBlkStat |= COMM_POOL_BlkReadyToRead;
			pthread_mutex_lock(&psInfo->mutReadNotify);
			CPD_Debug("writer send signal\n");
			pthread_cond_broadcast(&psInfo->condReadNotify);
			pthread_mutex_unlock(&psInfo->mutReadNotify);

			return COMM_POOL_NEEDTOQUIT;
		}
	}

	CPD_FuncOut();
		
	return eErr;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_Dettach(LONG lId, LONG lConsumerKey)
* @param		: LONG lId, LONG lConsumerKey
* @retval		: eCOMM_POOL_Err
* @note		: detach from the pool, delete a consumer
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_Dettach(LONG lId, LONG lConsumerKey) {
	eCOMM_POOL_Err eErr = COMM_POOL_SUCCESS;
	sCOMM_POOL_Info* pInfo = NULL;
	pthread_rwlock_t* pmutCnt = NULL;
	sCOMM_POOL_Consumer* psConsumer = NULL;

	CPD_FuncIn();

	{
		pInfo = psCOMM_POOL_GetPoolFromId(lId);
		if(!pInfo) {
			CPD_Debug("error Id\n");
			return COMM_POOL_INVALIDID;
		}

		/* check whether quit */
		if(pInfo->ePoolStat & COMM_POOL_Valid) {
			pmutCnt = &pInfo->rwConsumerCnt;
			psConsumer = &(pInfo->sConsumer[lConsumerKey]);
			pthread_rwlock_wrlock(pmutCnt);
			eCOMM_POOL_RemoveConsumerReader(pInfo, lConsumerKey);
			eCOMM_POOL_SetConsumerReader(pInfo, lConsumerKey, NULL);
			eCOMM_POOL_SetConsumerStat(pInfo, lConsumerKey, COMM_POOL_ConsumerNotUse);
			pInfo->lConsumerCnt --;
			if(psConsumer->pcConsumerName) {
				free(psConsumer->pcConsumerName);
			}
			
			pthread_rwlock_unlock(pmutCnt);
		}
	}

	CPD_FuncOut();

	return eErr;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_Deinit(LONG lId)
* @param		: LONG lId
* @retval		: eCOMM_POOL_Err
* @note		: deinit the pool
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_Deinit(LONG lId) {
	eCOMM_POOL_Err eErr = COMM_POOL_SUCCESS;
	sCOMM_POOL_Info* psInfo = NULL;
	sCOMM_POOL_Block* psBlk = NULL;
	LONG lBlkCntTmp;

	CPD_FuncIn();

	{
		psInfo = psCOMM_POOL_GetPoolFromId(lId);
		if(!psInfo) {
			/* indicate wrong id */
			return COMM_POOL_INVALIDVAL;
		}

		psInfo->ePoolStat |= COMM_POOL_ValidToInvalid;

		/* ensure all the consumer is quit readed at this time */
		pthread_mutex_lock(& psInfo->mutFinished);
		while((psInfo->psReader != psInfo->psWriter) || !(psInfo->ePoolStat & COMM_POOL_WriterQuit)) {
			CPD_Debug("waiting all the block is readed ...\n");
			pthread_cond_wait(& psInfo->condFinished, & psInfo->mutFinished);
		}
		pthread_mutex_unlock(& psInfo->mutFinished);
		CPD_Debug("all the block is readed, ready to deinit\n");
		CPD_PtVal(psInfo->psReader);
		CPD_PtVal(psInfo->psWriter);
		CPD_iVal(psInfo->ePoolStat);
		
		psInfo->ePoolStat = COMM_POOL_Invalid;
		psInfo->lConsumerCnt = -1;
		psInfo->lBlkOrder = 0;

		/* payload deinit */
		eCOMM_POOL_PAYLOAD_Deinit(psInfo->psPayloadInfo);
		free(psInfo->psPayloadInfo);

		/* deinit block */
		for(lBlkCntTmp = 0; lBlkCntTmp != psInfo->lBlkCnt; lBlkCntTmp ++) {
			psBlk = psInfo->psBlk + lBlkCntTmp;
			eCOMM_POOL_DeinitBlk(psBlk, COMM_POOL_LAST);
		}
		free(psInfo->psBlk);
		psInfo->psWriter = NULL;
		psInfo->psReader = NULL;
		psInfo->lBlkCnt = -1;
		eCOMM_POOL_ResetConsumer(psInfo);

		/* deinit condition flag and mutex */
		pthread_rwlock_destroy(&psInfo->rwConsumerCnt);
		pthread_mutex_destroy(&psInfo->mutBlkOrder);
		pthread_mutex_destroy(&psInfo->mutReadNotify);
		pthread_cond_destroy(&psInfo->condReadNotify);
		pthread_mutex_destroy(&psInfo->mutFinished);
		pthread_cond_destroy(&psInfo->condFinished);

		sem_destroy(&psInfo->semResourceCnt);
	}

	CPD_FuncOut();
		
	return eErr;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_DeinitAll()
* @param		: NULL
* @retval		: eCOMM_POOL_Err
* @note		: deinit all the pool
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_DeinitAll() {
	eCOMM_POOL_Err eErr = COMM_POOL_SUCCESS;
	LONG lPoolIdTmp;
	sCOMM_POOL_Info* psInfo = NULL;

	CPD_FuncIn();

	{
		for(lPoolIdTmp = 0; lPoolIdTmp != COMM_POOL_MAXPOOL; lPoolIdTmp ++) {
			psInfo = psCOMM_POOL_GetPoolFromId(lPoolIdTmp);

			if(psInfo->ePoolStat & COMM_POOL_Valid) {
				eCOMM_POOL_Deinit(lPoolIdTmp);
			}
		}

	}

	CPD_FuncOut();

	return eErr;
}

/*******************************************************************************
* @brief		: sCOMM_POOL_Info* psCOMM_POOL_GetPoolFromId(LONG lId)
* @param		: LONG lId
* @retval		: sCOMM_POOL_Info*
* @note		: get the pool from id
*******************************************************************************/
sCOMM_POOL_Info* psCOMM_POOL_GetPoolFromId(LONG lId) {
	sCOMM_POOL_Info* psInfo = NULL;

	CPD_FuncIn();

	{

		/* check the valid of id */
		if(lId >= 0 && lId <= COMM_POOL_MAXPOOL) {
			psInfo = sPoolInfo + lId;
		}

	}

	CPD_FuncOut();

	return psInfo;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_GetFreePool(LONG* plId)
* @param		: LONG* plId
* @retval		: eCOMM_POOL_Err
* @note		: get a free pool and return the id
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_GetFreePool(LONG* plId) {
	eCOMM_POOL_Err eErr = COMM_POOL_FULL;
	sCOMM_POOL_Info* psInfo = NULL;
	LONG lPoolIdTmp;
	*plId = COMM_POOL_INVALIDVAL;

	CPD_FuncIn();

	{
		for(lPoolIdTmp = 0; lPoolIdTmp != COMM_POOL_MAXPOOL; lPoolIdTmp ++) {
			psInfo = sPoolInfo + lPoolIdTmp;

			if(psInfo->ePoolStat & COMM_POOL_Invalid) {
				*plId = lPoolIdTmp;
				eErr = COMM_POOL_SUCCESS;
				CPD_Debug("Get Free Pool : ID <%ld>\n", lPoolIdTmp);
				break;
			}
		}
	}

	CPD_FuncOut();
	
	return eErr;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_InitBlk(sCOMM_POOL_Block* psBlk, LONG lMaxBlkSize, eCOMM_POOL_BlkMode eBlkMode)
* @param		: sCOMM_POOL_Block* psBlk, LONG lMaxBlkSize, eCOMM_POOL_BlkMode eBlkMode
* @retval		: eCOMM_POOL_Err
* @note		: init block
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_InitBlk(sCOMM_POOL_Block* psBlk, LONG lMaxBlkSize, eCOMM_POOL_BlkMode eBlkMode) {
	eCOMM_POOL_Err eErr = COMM_POOL_SUCCESS;

	CPD_FuncIn();

	{
		if(eBlkMode == COMM_POOL_FIRST) {
			pthread_mutex_init(&psBlk->mutCurReadCnt, NULL);
			psBlk->eBlkStat = COMM_POOL_BlkInvalid;
		} else {
			psBlk->eBlkStat = COMM_POOL_BlkValid;
		}
		
		psBlk->lBlkSize = 0;
		psBlk->lRealBlkSize = 0;
		psBlk->lBlkSizeRemain = lMaxBlkSize;
		psBlk->pucHeader = NULL;
		psBlk->pucWp = NULL;
		psBlk->pucNextBlk = NULL;
		psBlk->lCurReadCnt = 0;
	}

	CPD_FuncOut();

	return eErr;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_DeinitBlk(sCOMM_POOL_Block* psBlk, eCOMM_POOL_BlkMode eBlkMode)
* @param		: sCOMM_POOL_Block* psBlk, eCOMM_POOL_BlkMode eBlkMode
* @retval		: eCOMM_POOL_Err
* @note		: deinit block
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_DeinitBlk(sCOMM_POOL_Block* psBlk, eCOMM_POOL_BlkMode eBlkMode) {
	eCOMM_POOL_Err eErr = COMM_POOL_SUCCESS;

	CPD_FuncIn();

	{
		psBlk->eBlkStat = COMM_POOL_BlkValidToInvalid;
		psBlk->pucNextBlk = NULL;
		psBlk->lCurReadCnt = 0;
		psBlk->pucHeader = NULL;
		psBlk->pucWp = NULL;
		psBlk->lBlkSize = 0;
		psBlk->lRealBlkSize = 0;
		psBlk->lBlkSizeRemain = 0;
		psBlk->lBlkOrder = 0;

		if(eBlkMode == COMM_POOL_LAST) {
			pthread_mutex_destroy(&psBlk->mutCurReadCnt);
		}

		/* finally, let the block stat to be invalid */
		psBlk->eBlkStat = COMM_POOL_BlkInvalid;
	}

	CPD_FuncOut();

	return eErr;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err psCOMM_POOL_GetFreeBlk(LONG lId)
* @param		: LONG lId
* @retval		: eCOMM_POOL_Err
* @note		: get free block from the ID pool
*******************************************************************************/
sCOMM_POOL_Block* psCOMM_POOL_GetFreeBlk(LONG lId) {
	sCOMM_POOL_Info* psInfo = NULL;
	sCOMM_POOL_Block* psBlk = NULL;
	sCOMM_POOL_Block* psBlkTmp = NULL;
	LONG lBlkCntTmp;

	CPD_FuncIn();

	{
		psInfo = psCOMM_POOL_GetPoolFromId(lId);
		if(!psInfo) {
			CPD_Debug("invalid pool id\n");
			return psBlk;
		}

		for(lBlkCntTmp = 0; lBlkCntTmp != psInfo->lBlkCnt; lBlkCntTmp ++) {
			psBlkTmp = psInfo->psBlk + lBlkCntTmp;
			if(psBlkTmp->eBlkStat & COMM_POOL_BlkInvalid) {
				psBlk = psBlkTmp;
				psBlkTmp->eBlkStat &= ~COMM_POOL_BlkInvalid;
				psBlkTmp->eBlkStat |= COMM_POOL_BlkValid;
				break;
			}
		}

		/* distribute block order */
		eCOMM_POOL_SetBlkOrder(psInfo, psBlk);

	}

	CPD_FuncOut();

	return psBlk;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_SetAllConsumerReader(sCOMM_POOL_Info* psInfo, sCOMM_POOL_Block* psBlk, LONG lConsumerCnt)
* @param		: sCOMM_POOL_Info* psInfo, sCOMM_POOL_Block* psBlk, LONG lConsumerCnt
* @retval		: eCOMM_POOL_Err
* @note		: set all the consumer reader pointer of pool to a certain pointer
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_SetAllConsumerReader(sCOMM_POOL_Info* psInfo, sCOMM_POOL_Block* psBlk, LONG lConsumerCnt) {
	eCOMM_POOL_Err eErr = COMM_POOL_SUCCESS;
	LONG lConsumerCntTmp;
	sCOMM_POOL_Consumer* psConsumer = NULL;

	CPD_FuncIn();

	{
		for(lConsumerCntTmp = 0; lConsumerCntTmp != lConsumerCnt; ) {
			psConsumer = psInfo->sConsumer + lConsumerCntTmp;
			if(psConsumer->eConsumerStat == COMM_POOL_ConsumerIsUse) {
				psConsumer->psConsumerReader = psBlk;
				lConsumerCntTmp ++;
			}
		}
	}

	CPD_FuncOut();

	return eErr;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_SetConsumerReader(sCOMM_POOL_Info* psInfo, LONG lConsumerKey, sCOMM_POOL_Block* psBlk)
* @param		: sCOMM_POOL_Info* psInfo, LONG lConsumerKey, sCOMM_POOL_Block* psBlk
* @retval		: eCOMM_POOL_Err
* @note		: set the consumer reader to a certain block
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_SetConsumerReader(sCOMM_POOL_Info* psInfo, LONG lConsumerKey, sCOMM_POOL_Block* psBlk) {
	eCOMM_POOL_Err eErr = COMM_POOL_SUCCESS;
	sCOMM_POOL_Consumer* psConsumer = NULL;

	CPD_FuncIn();

	{
		if((lConsumerKey > COMM_POOL_MAXCONSUMER) || (lConsumerKey < 0)) {
			CPD_Debug("invalid consumer key\n");
			return COMM_POOL_INVALIDKEY;
		}

		psConsumer = psInfo->sConsumer + lConsumerKey;
		psConsumer->psConsumerReader = psBlk;

	}

	CPD_FuncOut();

	return eErr;
}


/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_GetConsumerReader(sCOMM_POOL_Info* psInfo, LONG lConsumerKey, sCOMM_POOL_Block** ppsBlk)
* @param		: sCOMM_POOL_Info* psInfo, LONG lConsumerKey, sCOMM_POOL_Block** ppsBlk
* @retval		: eCOMM_POOL_Err
* @note		: get the consumer reader to a certain block
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_GetConsumerReader(sCOMM_POOL_Info* psInfo, LONG lConsumerKey, sCOMM_POOL_Block** ppsBlk) {
	eCOMM_POOL_Err eErr = COMM_POOL_SUCCESS;
	sCOMM_POOL_Consumer* psConsumer = NULL;

	CPD_FuncIn();

	{
		if((lConsumerKey > COMM_POOL_MAXCONSUMER) || (lConsumerKey < 0)) {
			CPD_Debug("invalid consumer key\n");
			return COMM_POOL_INVALIDKEY;
		}

		psConsumer = psInfo->sConsumer + lConsumerKey;
		*ppsBlk = psConsumer->psConsumerReader;
	}

	CPD_FuncOut();

	return eErr;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_GetConsumerKey(sCOMM_POOL_Info* psInfo, LONG* plConsumerKey)
* @param		: sCOMM_POOL_Info* psInfo, LONG* plConsumerKey
* @retval		: eCOMM_POOL_Err
* @note		: get consumer key
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_GetConsumerKey(sCOMM_POOL_Info* psInfo, LONG* plConsumerKey) {
	eCOMM_POOL_Err eRet = COMM_POOL_REACHMAXCONSUMER;
	LONG lConsumerCntTmp;
	sCOMM_POOL_Consumer* psConsumer;

	CPD_FuncIn();

	{
		for(lConsumerCntTmp = 0; lConsumerCntTmp != COMM_POOL_MAXCONSUMER; lConsumerCntTmp ++) {
			psConsumer = psInfo->sConsumer + lConsumerCntTmp;
			if(psConsumer->eConsumerStat != COMM_POOL_ConsumerIsUse) {
				*plConsumerKey = lConsumerCntTmp;
				eRet = COMM_POOL_SUCCESS;
				break;
			}
		}
	}

	CPD_FuncOut();

	return eRet;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_RemoveConsumerReader(sCOMM_POOL_Info* psInfo, LONG lConsumerKey)
* @param		: sCOMM_POOL_Info* psInfo, LONG lConsumerKey
* @retval		: eCOMM_POOL_Err
* @note		: remove consumer reader
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_RemoveConsumerReader(sCOMM_POOL_Info* psInfo, LONG lConsumerKey) {
	eCOMM_POOL_Err eRet = COMM_POOL_SUCCESS;
	sCOMM_POOL_Block* psBlkTmp = NULL;
	sCOMM_POOL_Block* psConsumerReadBlk = NULL;

	CPD_FuncIn();

	{
		eCOMM_POOL_GetConsumerReader(psInfo, lConsumerKey, &psConsumerReadBlk);
		for(psBlkTmp = psInfo->psReader; psBlkTmp != psConsumerReadBlk; psBlkTmp = psBlkTmp->pucNextBlk) {
			pthread_mutex_lock(&psBlkTmp->mutCurReadCnt);
			psBlkTmp->lCurReadCnt --;
			pthread_mutex_unlock(&psBlkTmp->mutCurReadCnt);
		}

		if (psConsumerReadBlk == psInfo->psReader) {
			CPD_Debug("the cosumer reader to be removed is the last consumer !\n");
			eCOMM_POOL_PushReader(psInfo);
		}
	}

	CPD_FuncOut();

	return eRet;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_ShowAllConsumerReader(sCOMM_POOL_Info* psInfo)
* @param		: sCOMM_POOL_Info* psInfo
* @retval		: eCOMM_POOL_Err
* @note			: show all the cosumer reader of the pool
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_ShowAllConsumerReader(sCOMM_POOL_Info* psInfo) {
	eCOMM_POOL_Err eRet = COMM_POOL_SUCCESS;
	sCOMM_POOL_Consumer* psConsumer = NULL;
	LONG lConsumerCntTmp;
	
	CPD_FuncIn();
	
	{
		for(lConsumerCntTmp = 0; lConsumerCntTmp != COMM_POOL_MAXCONSUMER; ) {
			psConsumer = psInfo->sConsumer + lConsumerCntTmp;
			CPD_Debug("The consumer pointer <%ld> @ %p with status <%d>\n", lConsumerCntTmp, psConsumer->psConsumerReader, psConsumer->eConsumerStat);
			lConsumerCntTmp++;
		}
	}
	
	CPD_FuncOut();
	
	return eRet;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_GetConsumerStat(sCOMM_POOL_Info* psInfo, LONG lConsumerKey, eCOMM_POOL_ConsumerStat* peStat)
* @param		: sCOMM_POOL_Info* psInfo, LONG lConsumerKey, eCOMM_POOL_ConsumerStat* peStat
* @retval		: eCOMM_POOL_Err
* @note		: get consumer status
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_GetConsumerStat(sCOMM_POOL_Info* psInfo, LONG lConsumerKey, eCOMM_POOL_ConsumerStat* peStat) {
	eCOMM_POOL_Err eRet = COMM_POOL_SUCCESS;
	sCOMM_POOL_Consumer* psConsumer = NULL;

	CPD_FuncIn();

	{
		psConsumer = psInfo->sConsumer + lConsumerKey;
		*peStat = psConsumer->eConsumerStat;
	}
	
	CPD_FuncOut();

	return eRet;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_SetConsumerStat(sCOMM_POOL_Info* psInfo, LONG lConsumerKey, eCOMM_POOL_ConsumerStat eStat)
* @param		: sCOMM_POOL_Info* psInfo, LONG lConsumerKey, eCOMM_POOL_ConsumerStat eStat
* @retval		: eCOMM_POOL_Err
* @note		: set consumer status
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_SetConsumerStat(sCOMM_POOL_Info* psInfo, LONG lConsumerKey, eCOMM_POOL_ConsumerStat eStat) {
	eCOMM_POOL_Err eRet = COMM_POOL_SUCCESS;
	sCOMM_POOL_Consumer* psConsumer = NULL;

	CPD_FuncIn();

	{
		psConsumer = psInfo->sConsumer + lConsumerKey;
		psConsumer->eConsumerStat = eStat;
	}
	
	CPD_FuncOut();

	return eRet;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_ResetConsumer(sCOMM_POOL_Info* psInfo)
* @param		: sCOMM_POOL_Info* psInfo
* @retval		: eCOMM_POOL_Err
* @note		: reset all the consumers
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_ResetConsumer(sCOMM_POOL_Info* psInfo) {
	eCOMM_POOL_Err eRet = COMM_POOL_SUCCESS;
	sCOMM_POOL_Consumer* psConsumer = NULL;
	LONG lConsumerKeyTmp;

	CPD_FuncIn();

	{
		for(lConsumerKeyTmp = 0; lConsumerKeyTmp != COMM_POOL_MAXCONSUMER; lConsumerKeyTmp ++) {
			psConsumer = psInfo->sConsumer + lConsumerKeyTmp;
			psConsumer->eConsumerStat = COMM_POOL_ConsumerNotUse;
			psConsumer->psConsumerReader = NULL;
			psConsumer->pcConsumerName = NULL;
		}
	}
	
	CPD_FuncOut();

	return eRet;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_ShowReadList(sCOMM_POOL_Info* psInfo)
* @param		: sCOMM_POOL_Info* psInfo
* @retval		: eCOMM_POOL_Err
* @note		: show the read list of the pool
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_ShowReadList(sCOMM_POOL_Info* psInfo) {
	eCOMM_POOL_Err eRet = COMM_POOL_SUCCESS;
	sCOMM_POOL_Block* psBlkTmp = NULL;

	CPD_FuncIn();

	{
		CPD_Debug("\n==================================\n");
		CPD_Debug("Read Block List : \n");
		for(psBlkTmp = psInfo->psReader; psBlkTmp != psInfo->psWriter; psBlkTmp = psBlkTmp->pucNextBlk) {
			CPD_Debug("Block @ <%p> with reader cnt <%ld>, next <%p>\n", psBlkTmp, psBlkTmp->lCurReadCnt, psBlkTmp->pucNextBlk);
		}
		CPD_Debug("\n");
	}

	CPD_FuncOut();

	return eRet;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_QueryConsumerOrder(LONG lId, LONG lConsumerKey, LONG* lOrder)
* @param		: LONG lId, LONG lConsumerKey, LONG* lOrder
* @retval		: eCOMM_POOL_Err
* @note		: query consumer order
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_QueryConsumerOrder(LONG lId, LONG lConsumerKey, LONG* lOrder) {
	eCOMM_POOL_Err eRet = COMM_POOL_SUCCESS;
	sCOMM_POOL_Info* psInfo = NULL;
	sCOMM_POOL_Block* psBlk;

	CPD_FuncIn();

	{
		psInfo = psCOMM_POOL_GetPoolFromId(lId);
		if(!psInfo) {
			/* indicate wrong id */
			return COMM_POOL_INVALIDVAL;
		}

		eRet = eCOMM_POOL_GetConsumerReader(psInfo, lConsumerKey, &psBlk);
		if(eRet != COMM_POOL_SUCCESS) {
			return eRet;
		}

		*lOrder = psBlk->lBlkOrder;

	}

	CPD_FuncOut();

	return eRet;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_SetBlkOrder(sCOMM_POOL_Info* psInfo, sCOMM_POOL_Block* psBlk)
* @param		: sCOMM_POOL_Info* psInfo, sCOMM_POOL_Block* psBlk
* @retval		: eCOMM_POOL_Err
* @note		: set block order
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_SetBlkOrder(sCOMM_POOL_Info* psInfo, sCOMM_POOL_Block* psBlk) {
	eCOMM_POOL_Err eRet = COMM_POOL_SUCCESS;

	CPD_FuncIn();

	{

		if(!psInfo || !psBlk) {
			/* indicate do not need set order */
			return eRet;
		}

		/* distribute the block order */
		pthread_mutex_lock(&psInfo->mutBlkOrder);
		psBlk->lBlkOrder = psInfo->lBlkOrder ++;
		if(psInfo->lBlkOrder == COMM_POOL_MAXORDER) {
			psInfo->lBlkOrder = 0;
		}
		pthread_mutex_unlock(&psInfo->mutBlkOrder);

	}

	CPD_FuncOut();

	return eRet;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_ShowPoolInfo(sCOMM_POOL_Info* psInfo)
* @param		: sCOMM_POOL_Info* psInfo
* @retval		: eCOMM_POOL_Err
* @note		: show the whole pool information
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_ShowPoolInfo(sCOMM_POOL_Info* psInfo) {
	eCOMM_POOL_Err eRet = COMM_POOL_SUCCESS;
	sCOMM_POOL_Block* psBlk = NULL;
	sCOMM_POOL_Consumer* psConsumer = NULL;
	LONG lConsumerTmp;
	LONG lMaxBlkSize = 0;
	sCOMM_POOL_PAYLOAD_Info* psPayLoad = psInfo->psPayloadInfo;

	CPD_FuncIn();

	{

		printf("\n------------------------------------------------------------------\n");
		printf("Pool Information dump\n");
		printf("------------------------------------------------------------------\n\n");
		printf("blockcnt %ld    state %d    consumer %ld\n", psInfo->lBlkCnt, psInfo->ePoolStat, psInfo->lConsumerCnt);
		printf("payload info : header <0x%08lx> tail <0x%08lx> wt <0x%08lx> rd <0x%08lx>\n",
			(ULONG)psPayLoad->pucHeader, (ULONG)psPayLoad->pucTail, (ULONG)psPayLoad->pucWriter, (ULONG)psPayLoad->pucReader);
		printf("order     state     head        write       blksize   realblk   curcnt    consumer\n");

		for (psBlk=psInfo->psReader; psBlk; psBlk=psBlk->pucNextBlk) {

			/* get max block size */
			if (lMaxBlkSize < psBlk->lBlkSize) {
				lMaxBlkSize = psBlk->lBlkSize;
			}
			
			/* show the block info */
			printf("%-8ld  %-8d  0x%08lx  0x%08lx  %-8ld  %-8ld  %-8ld  ", 
				psBlk->lBlkOrder,
				psBlk->eBlkStat,
				(ULONG)psBlk->pucHeader,
				(ULONG)psBlk->pucWp,
				psBlk->lBlkSize,
				psBlk->lRealBlkSize,
				psBlk->lCurReadCnt);

			/* show the consumer info */
			for (lConsumerTmp = 0; lConsumerTmp != COMM_POOL_MAXCONSUMER; lConsumerTmp ++) {
				psConsumer = psInfo->sConsumer + lConsumerTmp;

				if (psConsumer->psConsumerReader == psBlk) {
					printf("%s  ", psConsumer->pcConsumerName);
				}
			}

			printf("\n");
		}

		printf("max block size : %ld\n", lMaxBlkSize);
	}

	CPD_FuncOut();

	return eRet;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_PreWriteHandle(sCOMM_POOL_Info* psInfo, LONG* plWriteLen, eCOMM_POOL_WriteStat eWtStat)
* @param		: sCOMM_POOL_Info* psInfo, LONG* plWriteLen, eCOMM_POOL_WriteStat eWtStat
* @retval		: eCOMM_POOL_Err
* @note		: pre write handle
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_PreWriteHandle(sCOMM_POOL_Info* psInfo, LONG* plWriteLen, eCOMM_POOL_WriteStat eWtStat) {
	eCOMM_POOL_Err eRet = COMM_POOL_SUCCESS;
	LONG lWriteLen = *plWriteLen;
	sCOMM_POOL_Block* psBlk = psInfo->psWriter;

	CPD_FuncIn();

	{
		psBlk->lBlkSize += lWriteLen;
		if (lWriteLen > psBlk->lBlkSizeRemain) {
			if (!(eWtStat & COMM_POOL_FORCE)) {
				lWriteLen = psBlk->lBlkSizeRemain;
				printf("[ WARNING ] block [ %ld ] : size < %ld > threshold < %ld > writelen < %ld > remainlen < %ld >\n",
					psBlk->lBlkOrder, psBlk->lBlkSize, psInfo->lBlkMax,
					lWriteLen, psBlk->lBlkSizeRemain);
			} else {
				// printf("[ FORCE WRITE ] block [ %ld ] : size < %ld > threshold < %ld > writelen < %ld > remainlen < %ld >\n",
				//	psBlk->lBlkOrder, psBlk->lBlkSize, psInfo->lBlkMax,
				//	lWriteLen, psBlk->lBlkSizeRemain);
			}
		}	

		/* update real block size */
		psBlk->lRealBlkSize += lWriteLen;
		
		psBlk->lBlkSizeRemain -= lWriteLen;
		if (psBlk->lBlkSizeRemain < 0) {
			/* regular remain size */
			psBlk->lBlkSizeRemain = 0;
		}

		/* update write len */
		*plWriteLen = lWriteLen;
	}

	CPD_FuncOut();

	return eRet;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_PushReader(sCOMM_POOL_Info* psInfo)
* @param		: sCOMM_POOL_Info* psInfo
* @retval		: eCOMM_POOL_Err
* @note		: push the pool reader, used in detach function
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_PushReader(sCOMM_POOL_Info* psInfo) {
	eCOMM_POOL_Err eRet = COMM_POOL_SUCCESS;
	sCOMM_POOL_Block* psBlk = psInfo->psReader;
	sCOMM_POOL_Block* psBlkToDelete = NULL;
	LONG lBreak = 1;

	CPD_FuncIn();

	{
		while(1) {
			/* init lBreak */
			lBreak = 1;

			/* update block */
			psBlkToDelete = psBlk;
			psBlk = psBlkToDelete->pucNextBlk;
			
			/* lock and check */
			pthread_mutex_lock(&psBlkToDelete->mutCurReadCnt);
			if (psBlkToDelete->lCurReadCnt == psInfo->lConsumerCnt - 1) {
				/* update the node */
				psInfo->psReader = psBlkToDelete->pucNextBlk;
				eCOMM_POOL_PAYLOAD_Read(psInfo->psPayloadInfo, psBlkToDelete->pucHeader, psBlkToDelete->lRealBlkSize);
				eCOMM_POOL_DeinitBlk(psBlkToDelete, COMM_POOL_MIDDLE);
				sem_post(&psInfo->semResourceCnt);

				/* go on to the next node */
				lBreak = 0;

				// printf("successful update node, new block order %ld\n", psBlk->lBlkOrder);
			}
			pthread_mutex_unlock(&psBlkToDelete->mutCurReadCnt);

			/* update */
			if (lBreak) {
				break;
			} 
		}
	}

	CPD_FuncOut();

	return eRet;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_ShowBlock(sCOMM_POOL_Block* psBlk)
* @param		: sCOMM_POOL_Block* psBlk
* @retval		: eCOMM_POOL_Err
* @note		: show the block information
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_ShowBlock(sCOMM_POOL_Block* psBlk) {
	eCOMM_POOL_Err eRet = COMM_POOL_SUCCESS;

	CPD_FuncIn();

	{

		printf("Block Info : ");
		printf("order     state     head        write       blksize   realblk   curcnt    consumer\n");

		/* show the block info */
		printf("%-8ld  %-8d  0x%08lx  0x%08lx  %-8ld  %-8ld  %-8ld	", 
			psBlk->lBlkOrder,
			psBlk->eBlkStat,
			(ULONG)psBlk->pucHeader,
			(ULONG)psBlk->pucWp,
			psBlk->lBlkSize,
			psBlk->lRealBlkSize,
			psBlk->lCurReadCnt);

		printf("\n");

	}

	CPD_FuncOut();

	return eRet;
}

/*******************************************************************************
* @brief		: eCOMM_POOL_Err eCOMM_POOL_Update(sCOMM_POOL_Info* psInfo, sCOMM_POOL_Block* psBlk, LONG lConsumerKey)
* @param		: sCOMM_POOL_Info* psInfo, sCOMM_POOL_Block* psBlk, LONG lConsumerKey
* @retval		: eCOMM_POOL_Err
* @note		: update the pool
*******************************************************************************/
eCOMM_POOL_Err eCOMM_POOL_Update(sCOMM_POOL_Info* psInfo, sCOMM_POOL_Block* psBlk, LONG lConsumerKey) {
	eCOMM_POOL_Err eRet = COMM_POOL_SUCCESS;
	LONG lIsReaderStep = 0;
	sCOMM_POOL_Block* psNextBlk = NULL;

	CPD_FuncIn();

	{
		/* update status */
		pthread_rwlock_rdlock(&psInfo->rwConsumerCnt);
		pthread_mutex_lock(&psBlk->mutCurReadCnt);
		
		psNextBlk = psBlk->pucNextBlk;
		eCOMM_POOL_SetConsumerReader(psInfo, lConsumerKey, psNextBlk);

		if(++(psBlk->lCurReadCnt) == psInfo->lConsumerCnt) {
			if (psInfo->psReader != psBlk) {
				printf("order %ld\n", psBlk->lBlkOrder);
				eCOMM_POOL_ShowPoolInfo(psInfo);
			}
			assert(psInfo->psReader == psBlk);
			lIsReaderStep = 1;
			psInfo->psReader = psInfo->psReader->pucNextBlk;

			/* for debug */
			if (psInfo->psPayloadInfo->pucReader != psBlk->pucHeader) {
				eCOMM_POOL_ShowBlock(psBlk);
				eCOMM_POOL_ShowPoolInfo(psInfo);
			}

			/* push the reader and deinit the block */
			eCOMM_POOL_PAYLOAD_Read(psInfo->psPayloadInfo, psBlk->pucHeader, psBlk->lRealBlkSize);
			eCOMM_POOL_DeinitBlk(psBlk, COMM_POOL_MIDDLE);
			sem_post(&psInfo->semResourceCnt);

		}

		/* ensure all the block is read */
		if(psInfo->ePoolStat & COMM_POOL_WriterQuit) {
			CPD_PtVal(psInfo->psWriter);
			CPD_PtVal(psNextBlk);
			CPD_Val(lIsReaderStep);
			if(psNextBlk == psInfo->psWriter) {
				if(lIsReaderStep) {
					/* notify deinit to quit */
					CPD_Debug("Consumer <%ld> ready to quit\n", lConsumerKey);
					pthread_mutex_lock(&psInfo->mutFinished);
					pthread_cond_broadcast(&psInfo->condFinished);
					pthread_mutex_unlock(&psInfo->mutFinished);
				}

				pthread_mutex_unlock(&psBlk->mutCurReadCnt);
				pthread_rwlock_unlock(&psInfo->rwConsumerCnt);
				
				return COMM_POOL_NEEDTOQUIT;
			}
		}

		pthread_mutex_unlock(&psBlk->mutCurReadCnt);
		pthread_rwlock_unlock(&psInfo->rwConsumerCnt);

	}

	CPD_FuncOut();

	return eRet;
}

#ifdef __cplusplus
}
#endif
