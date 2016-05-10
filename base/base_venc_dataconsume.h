#ifndef __BASE_VENC_DATACONSUME_H__
#define __BASE_VENC_DATACONSUME_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************
	include
*********************************************************/
#include "base_comm.h"
#include "base_api_ledctl.h"

/*********************************************************
	macro / enum
*********************************************************/
#define BASE_VENC_DATACONSUME_MAXNAME 256
#define BASE_VENC_DATACONSUME_DEFAULTTIME 0

typedef enum {
	BASE_VENC_DATACONSUME_SUCCESS,
	BASE_VENC_DATACONSUME_FAIL,
	BASE_VENC_DATACONSUME_POOL,
	BASE_VENC_DATACONSUME_NEWTHREAD,
	BASE_VENC_DATACONSUME_FILENAME,
	BASE_VENC_DATACONSUME_FILEOPEN,
	BASE_VENC_DATACONSUME_FILEWRITE,
	BASE_VENC_DATACONSUME_FILECLOSE,
} eBASE_VENC_DATACONSUME_Ret;

typedef enum {
	BASE_VENC_DATACONSUME_Invalid = 0x0,
	BASE_VENC_DATACONSUME_Valid = 0x1,
	BASE_VENC_DATACONSUME_StartM = 0x10,
	BASE_VENC_DATACONSUME_StartV = 0x20,
	BASE_VENC_DATACONSUME_Stop = 0x100,
} eBASE_VENC_DATACONSUME_Status;

typedef enum {
	BASE_VENC_DATACONSUME_CommuNoData,
	BASE_VENC_DATACONSUME_CommuHaveData,
} eBASE_VENC_DATACONSUME_CommuStatus;

/*********************************************************
	struct
*********************************************************/
typedef struct {
	pthread_mutex_t mutPayLoad;
	pthread_cond_t condPayLoad;
	LONG lPayLoad;
	eBASE_VENC_DATACONSUME_CommuStatus eCommuStatus;
} sBASE_VENC_DATACONSUME_Communicate;

typedef struct {
	eBASE_VENC_DATACONSUME_Status* peMStatus;
	sBASE_VENC_DATACONSUME_Communicate* psCommu;
	LONG* plPoolId;
	LONG lMConsumerKey;
	LONG lMFrameCntTh;
	CHAR* pcName;
	CHAR* pcDirName;
} sBASE_VENC_DATACONSUME_MArg;

typedef struct {
	eBASE_VENC_DATACONSUME_Status* peVStatus;
	sBASE_VENC_DATACONSUME_Communicate* psCommu;
	LONG* plPoolId;
	LONG lVConsumerKey;
	LONG lVFrameCntTh;
} sBASE_VENC_DATACONSUME_VArg;

typedef struct {
	pthread_t sPidM;
	pthread_t sPidV;
	eBASE_VENC_DATACONSUME_Status eStatus;
	LONG* plPoolId;
	LONG lTimeVTh;
	LONG lTimeMTh;
	CHAR cName[BASE_VENC_DATACONSUME_MAXNAME];
	sBASE_VENC_DATACONSUME_Communicate* psCommu;
	sBASE_VENC_DATACONSUME_MArg* psMArg;
	sBASE_VENC_DATACONSUME_VArg* psVArg;
} sBASE_VENC_DATACONSUME_Info;

/*********************************************************
	variable
*********************************************************/
sBASE_VENC_DATACONSUME_Info gsDataConsume;

/*********************************************************
	prototype
*********************************************************/
eBASE_VENC_DATACONSUME_Ret eBASE_VENC_DATACONSUME_Init(sBASE_VENC_DATACONSUME_Info* psInfo, LONG* plPoolId, LONG lTimeVTh, LONG lTimeMTh, CHAR* pcPathName);
eBASE_VENC_DATACONSUME_Ret eBASE_VENC_DATACONSUME_StartMConsumer(sBASE_VENC_DATACONSUME_Info* psInfo);
eBASE_VENC_DATACONSUME_Ret eBASE_VENC_DATACONSUME_StartVConsumer(sBASE_VENC_DATACONSUME_Info* psInfo);
eBASE_VENC_DATACONSUME_Ret eBASE_VENC_DATACONSUME_StopMConsumer(sBASE_VENC_DATACONSUME_Info* psInfo);
eBASE_VENC_DATACONSUME_Ret eBASE_VENC_DATACONSUME_StopVConsumer(sBASE_VENC_DATACONSUME_Info* psInfo);
eBASE_VENC_DATACONSUME_Ret eBASE_VENC_DATACONSUME_Deinit(sBASE_VENC_DATACONSUME_Info* psInfo);
eBASE_VENC_DATACONSUME_Ret eBASE_VENC_DATACONSUME_UpdateTimeThreshold(sBASE_VENC_DATACONSUME_Info* psInfo, LONG lNewMTimeTh, LONG lNewVTimeTh);

#ifdef __cplusplus
}
#endif

#endif
