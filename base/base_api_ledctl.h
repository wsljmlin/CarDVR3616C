#ifndef __BASE_API_LEDCTL_H__
#define __BASE_API_LEDCTL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************
	include
*********************************************************/
#include "comm_common.h"
#include "comm_mw_led.h"

/*********************************************************
	macro / enum
*********************************************************/
#define LED_DEV "/dev/LED"

typedef enum {
	BASE_API_LEDCTL_SUCCESS,
	BASE_API_LEDCTL_FAIL,
	BASE_API_LEDCTL_MALLOC,
	BASE_API_LEDCTL_CREATETHREAD,
	BASE_API_LEDCTL_LEDINIT,
	BASE_API_LEDCTL_TIMER,
} eBASE_API_LEDCTL_Ret;

typedef enum {
	BASE_API_LEDCTL_ISEND,
	BASE_API_LEDCTL_NOTEND,
} eBASE_API_LEDCTL_ThreadStatus;

typedef enum {
	BASE_API_LEDCTL_NORMAL = COMM_MW_LED_GREEN_On,									/* green on */
	BASE_API_LEDCTL_SDFAIL = COMM_MW_LED_Red_On,									/* red on */
	BASE_API_LEDCTL_REC =  COMM_MW_LED_GREEN_Blink,									/* green blink */
	BASE_API_LEDCTL_SNAP = (COMM_MW_LED_Red_On | COMM_MW_LED_GREEN_On),				/* red and green on */
	BASE_API_LEDCTL_NOFEEDBACK = (COMM_MW_LED_Red_Blink | COMM_MW_LED_GREEN_Blink),	/* red and green blink */
	BASE_API_LEDCTL_INVALID = 0,
} eBASE_API_LEDCTL_LedNewStatus;

typedef enum {
	BASE_API_LEDCTL_NOTHAVESIG,
	BASE_API_LEDCTL_HAVESIG,
} eBASE_API_LEDCTL_SignalStatus;

typedef enum {
	BASE_API_LEDCTL_NOTHAVETIMER,
	BASE_API_LEDCTL_HAVETIMER,
} eBASE_API_LEDCTL_TimerStatus;

/*********************************************************
	struct
*********************************************************/
typedef struct {
	LONG lFirFd;
	LONG lSecFd;
	LONG lThirdFd;
	LONG lFourthFd;
	LONG lStatus;
	LONG lEvent;
} sBASE_API_LED_CTL_Timer;

typedef struct {
	pthread_cond_t condStatus;
	pthread_mutex_t mutStatus;
	eBASE_API_LEDCTL_SignalStatus eStatus;
} sBASE_API_LED_CTL_Signal;

typedef struct 
{
	sCOMM_MW_LED_Info* psLedInfo;
	sBASE_API_LED_CTL_Signal* psRecSignal;
} sBASE_API_LEDCTL_Param;

typedef struct 
{
	eBASE_API_LEDCTL_ThreadStatus  peThreadStatus;
	sCOMM_MW_LED_Info* psLedInfo;
	sBASE_API_LED_CTL_Signal* psSignal;
	sBASE_API_LED_CTL_Signal* psRecSignal;
} sBASE_API_LEDCTL_ThreadArg;

typedef struct 
{
	pthread_t tid;
	eBASE_API_LEDCTL_ThreadStatus eThreadStatus;
	sCOMM_MW_LED_Info* psLedInfo;
	sBASE_API_LEDCTL_ThreadArg* psArg;
	sBASE_API_LED_CTL_Signal* psSignal;
	sBASE_API_LED_CTL_Signal* psRecSignal;
} sBASE_API_LEDCTL_Info;

/*********************************************************
	variable
*********************************************************/
sBASE_API_LEDCTL_Info gsLedCtlInfo;
sBASE_API_LED_CTL_Timer sLedTimer;
	
/*********************************************************
	prototype
*********************************************************/
eBASE_API_LEDCTL_Ret eBASE_API_LEDCTL_Init(sBASE_API_LEDCTL_Info* psInfo);
eBASE_API_LEDCTL_Ret eBASE_API_LEDCTL_Deinit(sBASE_API_LEDCTL_Info* psInfo);
eBASE_API_LEDCTL_Ret eBASE_API_LEDCTL_StartThread(sBASE_API_LEDCTL_Info* psInfo);
eBASE_API_LEDCTL_Ret eBASE_API_LEDCTL_StopThread(sBASE_API_LEDCTL_Info* psInfo);
eBASE_API_LEDCTL_Ret eBASE_API_LEDCTL_ChangeLed(sBASE_API_LEDCTL_ThreadArg* psInfo, eBASE_API_LEDCTL_LedNewStatus eStatus);

#ifdef __cplusplus
}
#endif


#endif
