#ifndef __BASE_UTIL_MUX_INTERFACE_H__
#define __BASE_UTIL_MUX_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************
	include
*********************************************************/
#include "comm_common.h"
#include "comm_queue.h"

/*********************************************************
	macro / enum
*********************************************************/
typedef enum {
	BASE_UTIL_MUX_INTERFACE_SUCCESS,
	BASE_UTIL_MUX_INTERFACE_SRCFILE,
	BASE_UTIL_MUX_INTERFACE_MALLOC,
	BASE_UTIL_MUX_INTERFACE_SYSTEM,
	BASE_UTIL_MUX_INTERFACE_FAIL,
} eBASE_UTIL_MUX_INTERFACE_Ret;

typedef enum {
	BASE_UTIL_MUX_INTERFACE_STATUS_INVALID = 0,
	BASE_UTIL_MUX_INTERFACE_STATUS_IMPLE = 1,
	BASE_UTIL_MUX_INTERFACE_STATUS_VALID = 2,
	BASE_UTIL_MUX_INTERFACE_STATUS_THREADSTART = 4,
} eBASE_UTIL_MUX_INTERFACE_Status;

/*********************************************************
	struct
*********************************************************/
typedef struct _sBASE_UTIL_MUX_INTERFACE_Info {
	eBASE_UTIL_MUX_INTERFACE_Ret (*eBASE_UTIL_MUX_INTERFACE_Init)(struct _sBASE_UTIL_MUX_INTERFACE_Info*, void*);
	eBASE_UTIL_MUX_INTERFACE_Ret (*eBASE_UTIL_MUX_INTERFACE_MuxRequest)(struct _sBASE_UTIL_MUX_INTERFACE_Info*, CHAR*);
	eBASE_UTIL_MUX_INTERFACE_Ret (*eBASE_UTIL_MUX_INTERFACE_StartThread)(struct _sBASE_UTIL_MUX_INTERFACE_Info*);
	eBASE_UTIL_MUX_INTERFACE_Ret (*eBASE_UTIL_MUX_INTERFACE_StopThread)(struct _sBASE_UTIL_MUX_INTERFACE_Info*);
	eBASE_UTIL_MUX_INTERFACE_Ret (*eBASE_UTIL_MUX_INTERFACE_DeInit)(struct _sBASE_UTIL_MUX_INTERFACE_Info*);
	void* pvPrivate;
	eBASE_UTIL_MUX_INTERFACE_Status eStatus;
	CHAR* pcName;
	sCOMM_QUEUE* psQueue;
	pthread_t pid;
} sBASE_UTIL_MUX_INTERFACE_Info;

/*********************************************************
	variable
*********************************************************/
sBASE_UTIL_MUX_INTERFACE_Info gsMuxInterface;

/*********************************************************
	prototype
*********************************************************/

#ifdef __cplusplus
}
#endif

#endif
