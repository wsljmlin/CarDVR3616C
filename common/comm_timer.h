#ifndef __COMM_TIMER_H__
#define __COMM_TIMER_H__

#ifdef __cplusplus 
extern "C" {
#endif

/***************************************************************
	include
***************************************************************/
#include "comm_common.h"

/***************************************************************
	enum / macro
***************************************************************/
#define COMM_TIMER_MAXCNT 20

typedef enum {
	COMM_TIMER_SUCCESS,
	COMM_TIMER_FAIL,
	COMM_TIMER_TIMERREACHMAX,
	COMM_TIMER_NOTUSED,
} eCOMM_TIMER_RET;

typedef enum {
	COMM_TIMER_INVALID,
	COMM_TIMER_VALID,
	COMM_TIMER_ISUSE,
} eCOMM_TIMER_STATUS;

typedef void(* pvTimerCallBackFunc)(void*);

/***************************************************************
	struct
***************************************************************/
typedef struct _sCOMM_TIMER_INFO {
	LONG lTimeOut;
	LONG lTimerId;
	LONG lCurTime;
	eCOMM_TIMER_STATUS eStatus;
	pvTimerCallBackFunc pvCBFunc;
	void* pvCBFuncArg;
	pthread_mutex_t sMutStatus;
	struct _sCOMM_TIMER_INFO* psNextTimer;
} sCOMM_TIMER_INFO;

typedef struct {
	sCOMM_TIMER_INFO* psHeader;
	sCOMM_TIMER_INFO* psTail;
	pthread_mutex_t mutStatus;
} sCOMM_TIMER_LIST;

/***************************************************************
	variable
***************************************************************/
sCOMM_TIMER_LIST gsTimerList;
sCOMM_TIMER_INFO* psTimerNode[COMM_TIMER_MAXCNT];

/***************************************************************
	prototype
***************************************************************/
eCOMM_TIMER_RET eCOMM_TIMER_InitAll();
eCOMM_TIMER_RET eCOMM_TIMER_DeinitAll();
eCOMM_TIMER_RET eCOMM_TIMER_Init(const LONG lTimeOut, const pvTimerCallBackFunc pvCBFunc, void* pvCBFuncArg, LONG* plTimerId);
eCOMM_TIMER_RET eCOMM_TIMER_Deinit(const LONG lTimerId);

#ifdef __cplusplus
}
#endif
#endif