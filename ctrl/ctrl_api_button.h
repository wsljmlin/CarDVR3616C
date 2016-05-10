#ifndef __CTRL_API_BUTTON_H__
#define __CTRL_API_BUTTON_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************
	include
*********************************************************/
#include "comm_common.h"
#include "comm_dd_button.h"
#include "comm_command.h"
#include "comm_queue.h"
#include "ctrl_interface.h"
/*********************************************************
	macro / enum
*********************************************************/
#define BUTTON_DEV	"/dev/BUTTON"

typedef enum {
	CTRL_API_BUTTON_SUCCESS,
	CTRL_API_BUTTON_FAIL,
	CTRL_API_BUTTON_MALLOC,
	CTRL_API_BUTTON_THREAD,
} eCTRL_API_BUTTON_Ret;

typedef enum {	
	CTRL_API_BUTTON_Status_NotEnd,	
	CTRL_API_BUTTON_Status_IsEnd,
} eCTRL_API_BUTTON_Status;

/*********************************************************
	struct
*********************************************************/
typedef struct {	
	LONG lFd;
	eCTRL_API_BUTTON_Status* peStatus;
	sCOMM_DD_BUTTON_Info sButtonInfo;
} sCTRL_API_BUTTON_ThreadArg;

typedef struct {
	eCTRL_API_BUTTON_Status eStatus;
	sCTRL_API_BUTTON_ThreadArg* psThreadArg;
	pthread_t tid;
} sCTRL_API_BUTTON_Info;

/*********************************************************
	variable
*********************************************************/
sCTRL_API_BUTTON_Info sCTRL_Btn_Req;

/*********************************************************
	prototype
*********************************************************/
eCTRL_API_BUTTON_Ret eCTRL_API_BUTTON_Init(sCTRL_API_BUTTON_Info* psInfo);
eCTRL_API_BUTTON_Ret eCTRL_API_BUTTON_Deinit(sCTRL_API_BUTTON_Info* psInfo);
eCTRL_API_BUTTON_Ret eCTRL_API_BUTTON_StartThread(sCTRL_API_BUTTON_Info* psInfo);
eCTRL_API_BUTTON_Ret eCTRL_API_BUTTON_StopThread(sCTRL_API_BUTTON_Info* psInfo);

#ifdef __cplusplus
}
#endif

#endif
