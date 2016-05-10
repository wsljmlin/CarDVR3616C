#ifndef __COMM_POOL_H__
#define __COMM_POOL_H__
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "comm_common.h"
#include "comm_pool_payload.h"

/*********************************************************
	macro / enum
*********************************************************/
#define COMM_POOL_MAXPOOL 5
#define COMM_POOL_MAXCONSUMER 10
#define COMM_POOL_INVALIDVAL -1
#define COMM_POOL_MAXORDER (1 << 30)
typedef void(*pvCALLBACK)(void*);

typedef enum {
	COMM_POOL_SUCCESS,
	COMM_POOL_FAILURE,
	COMM_POOL_INVALIDID,
	COMM_POOL_FULL,
	COMM_POOL_BLKFULL,
	COMM_POOL_SRCNULL,
	COMM_POOL_PARAERR,
	COMM_POOL_REACHMAXCONSUMER,
	COMM_POOL_INVALIDKEY,
	COMM_POOL_NEEDTOQUIT,
} eCOMM_POOL_Err;

typedef enum {
	COMM_POOL_REMAINBLOCK = 0x1,
	COMM_POOL_NEWBLOCK = 0x2,
	COMM_POOL_FORCE = 0x4,
} eCOMM_POOL_WriteStat;

typedef enum {
	COMM_POOL_Invalid = 0x1,
	COMM_POOL_Valid = 0x2,
	COMM_POOL_Full = 0x4,
	COMM_POOL_ValidToInvalid = 0x8,
	COMM_POOL_WriterQuit = 0x10,
} eCOMM_POOL_Stat;

typedef enum {
	COMM_POOL_BlkInvalid = 0x1,
	COMM_POOL_BlkValid = 0x2,
	COMM_POOL_BlkValidToInvalid = 0x4,
	COMM_POOL_BlkFull = 0x8,
	COMM_POOL_BlkReadyToRead = 0x10,
} eCOMM_POOL_BlkStat;

typedef enum {
	COMM_POOL_ConsumerNotUse,
	COMM_POOL_ConsumerIsUse,
} eCOMM_POOL_ConsumerStat;

/*********************************************************
	struct
*********************************************************/
typedef struct _sCOMM_POOL_Block {
	LONG lBlkSize;
	LONG lRealBlkSize;
	LONG lBlkSizeRemain;
	LONG lCurReadCnt;
	LONG lBlkOrder;
	UCHAR* pucHeader;
	UCHAR* pucWp;
	struct _sCOMM_POOL_Block* pucNextBlk;
	pthread_mutex_t mutCurReadCnt;
	eCOMM_POOL_BlkStat eBlkStat;
} sCOMM_POOL_Block;

typedef struct _sCOMM_POOL_Consumer {
	sCOMM_POOL_Block* psConsumerReader;
	eCOMM_POOL_ConsumerStat eConsumerStat;
	CHAR* pcConsumerName;
} sCOMM_POOL_Consumer;

typedef struct _sCOMM_POOL_Info {
	LONG lBlkCnt;
	LONG lBlkOrder;
	LONG lBlkMax;
	LONG lConsumerCnt;
	eCOMM_POOL_Stat ePoolStat;
	sem_t semResourceCnt;
	pthread_rwlock_t rwConsumerCnt;
	pthread_mutex_t mutBlkOrder;
	pthread_mutex_t mutReadNotify;
	pthread_cond_t condReadNotify;
	pthread_mutex_t mutFinished;
	pthread_cond_t condFinished;
	sCOMM_POOL_Block* psBlk;
	sCOMM_POOL_Block* psWriter;
	sCOMM_POOL_Block* psReader;
	sCOMM_POOL_PAYLOAD_Info* psPayloadInfo;
	sCOMM_POOL_Consumer sConsumer[COMM_POOL_MAXCONSUMER];
} sCOMM_POOL_Info;

typedef struct _sCOMM_POOL_CBFuncArg {
	UCHAR* pucHeader;
	LONG lLen;
	void* pvArg;
} sCOMM_POOL_CBFuncArg;
/*********************************************************
	variable
*********************************************************/
sCOMM_POOL_Info sPoolInfo[COMM_POOL_MAXPOOL];

/*********************************************************
	prototype
*********************************************************/
eCOMM_POOL_Err eCOMM_POOL_InitAll();
eCOMM_POOL_Err eCOMM_POOL_Init(LONG* plId, LONG lPayLoadSize, LONG lBlkCnt, LONG lMaxBlkSize);
eCOMM_POOL_Err eCOMM_POOL_Attach(LONG lId, LONG* plConsumerKey, CHAR* pcName);
eCOMM_POOL_Err eCOMM_POOL_Read(LONG lId, pvCALLBACK pvPostFunc, void* pvArg, LONG lConsumerKey);
eCOMM_POOL_Err eCOMM_POOL_Write(LONG lId, UCHAR* pucSrc, LONG lSrcLen, eCOMM_POOL_WriteStat eWtStat);
eCOMM_POOL_Err eCOMM_POOL_Dettach(LONG lId, LONG lConsumerKey);
eCOMM_POOL_Err eCOMM_POOL_Deinit(LONG lId);
eCOMM_POOL_Err eCOMM_POOL_DeinitAll();
eCOMM_POOL_Err eCOMM_POOL_QueryConsumerOrder(LONG lId, LONG lConsumerKey, LONG* lOrder);

#ifdef __cplusplus
}
#endif