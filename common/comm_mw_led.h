#ifndef __COMM_MW_LED_H__
#define __COMM_MW_LED_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************
	include
*********************************************************/
#include "comm_common.h"

/*********************************************************
	macro / enum
*********************************************************/
#define MAX_BUF_LEN		128
#define LED_REDOFF				1
#define LED_REDON				0
#define LED_GREENOFF			3
#define LED_GREENON			2



//#define LED_TEST

typedef enum {
	COMM_MW_LED_SUCCESS,
	COMM_MW_LED_FAIL,
} eCOMM_MW_LED_Ret;

typedef enum {
	COMM_MW_LED_Red_On = 0x1,
	COMM_MW_LED_Red_Blink = 0x2,
	COMM_MW_LED_GREEN_On = 0x10,
	COMM_MW_LED_GREEN_Blink = 0x20,
	COMM_MW_LED_COLORMASK = 0xff,
} eCOMM_MW_LED_Color;

typedef enum {
	COMM_MW_LED_VALID = 0x100,
	COMM_MW_LED_OPEN = 0x200,
	COMM_MW_LED_STATUSMASK = 0xf00,
} eCOMM_MW_LED_Status;

/*********************************************************
	struct
*********************************************************/
typedef struct {
	LONG lFd;
	LONG lStatus; 
	CHAR cDevName[MAX_BUF_LEN];
	pthread_mutex_t mutStatus;
} sCOMM_MW_LED_Info;

/*********************************************************
	variable
*********************************************************/

/*********************************************************
	prototype
*********************************************************/
eCOMM_MW_LED_Ret eCOMM_MW_LED_Init(sCOMM_MW_LED_Info* psInfo);
eCOMM_MW_LED_Ret eCOMM_MW_LED_Open(sCOMM_MW_LED_Info* psInfo);
eCOMM_MW_LED_Ret eCOMM_MW_LED_Close(sCOMM_MW_LED_Info* psInfo);
eCOMM_MW_LED_Ret eCOMM_MW_LED_Deinit(sCOMM_MW_LED_Info* psInfo);
eCOMM_MW_LED_Ret eCOMM_MW_LED_RedOn(sCOMM_MW_LED_Info* psInfo);
eCOMM_MW_LED_Ret eCOMM_MW_LED_RedOff(sCOMM_MW_LED_Info* psInfo);
eCOMM_MW_LED_Ret eCOMM_MW_LED_GreenOn(sCOMM_MW_LED_Info* psInfo);
eCOMM_MW_LED_Ret eCOMM_MW_LED_GreenOff(sCOMM_MW_LED_Info* psInfo);

#ifdef __cplusplus
}
#endif

#endif
