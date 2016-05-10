#ifndef __COMM_DD_BUTTON_H__
#define __COMM_DD_BUTTON_H__

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
#define COMM_DD_BUTTON_DEVNAMEMAX 		32
#define COMM_DD_BUTTON_READBUFMAX           32 

typedef enum {
	COMM_DD_BUTTON_SUCCESS,
	COMM_DD_BUTTON_FAIL,
	COMM_DD_BUTTON_OPEN,	
	COMM_DD_BUTTON_READ,	
	COMM_DD_BUTTON_CLOSE,	
} eCOMM_DD_BUTTON_Ret;

/*********************************************************
	struct
*********************************************************/
typedef struct {
	CHAR pcDev[COMM_DD_BUTTON_DEVNAMEMAX];	
	LONG lFd;
} sCOMM_DD_BUTTON_Info;

/*********************************************************
	variable
*********************************************************/

/*********************************************************
	prototype
*********************************************************/
eCOMM_DD_BUTTON_Ret eCOMM_DD_BUTTON_Init(sCOMM_DD_BUTTON_Info* psInfo, CHAR* pcDev);
eCOMM_DD_BUTTON_Ret eCOMM_DD_BUTTON_Open(sCOMM_DD_BUTTON_Info* psInfo);
eCOMM_DD_BUTTON_Ret eCOMM_DD_BUTTON_Read(sCOMM_DD_BUTTON_Info* psInfo);
eCOMM_DD_BUTTON_Ret eCOMM_DD_BUTTON_Write(sCOMM_DD_BUTTON_Info* psInfo);
eCOMM_DD_BUTTON_Ret eCOMM_DD_BUTTON_Close(sCOMM_DD_BUTTON_Info* psInfo);

#ifdef __cplusplus
}
#endif

#endif
