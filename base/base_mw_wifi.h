#ifndef __BASE_MW_WIFI_H__
#define __BASE_MW_WIFI_H__

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
#define WIFI_ENABLE	
#define BASE_MW_WIFI_NAMEMAX 64
#define BASE_MW_WIFI_PASSWDMAX 64
#define BASE_MW_WIFI_SCRIPTMAX 64

typedef enum {
	BASE_MW_WIFI_SUCCESS = 0x0,
	BASE_MW_WIFI_FAIL = 0x1,
	BASE_MW_WIFI_POWERON = 0x10,
	BASE_MW_WIFI_POWEROFF = 0x20,
	BASE_MW_WIFI_GENERAL_OPENSCRIPT = 0x100,
	BASE_MW_WIFI_EXECUTE_OPENSCRIPT = 0x200,
	BASE_MW_WIFI_GENERAL_CLOSESCRIPT = 0x400,
	BASE_MW_WIFI_EXECUTE_CLOSESCRIPT = 0x800,	
	BASE_MW_WIFI_CHECK_FAIL = 0x1000,
} eBASE_MW_WIFI_Ret;

typedef enum {
	BASE_MW_WIFI_STATUS_INVALID = 0x0,
	BASE_MW_WIFI_STATUS_DISCONNECT = 0x1,
	BASE_MW_WIFI_STATUS_CONNECT = 0x2,
	BASE_MW_WIFI_STATUS_CONNECTSCRIPT = 0x10,
	BASE_MW_WIFI_STATUS_DISCONNECTSCRIPT = 0x20,
} eBASE_MW_WIFI_Status;

/*********************************************************
	struct
*********************************************************/
typedef struct {
	LONG fd;
	LONG lStatus;
	CHAR cUserName[BASE_MW_WIFI_NAMEMAX];
	CHAR cPassWord[BASE_MW_WIFI_PASSWDMAX];
	CHAR cConnectScript[BASE_MW_WIFI_SCRIPTMAX];
	CHAR cDisConnectScript[BASE_MW_WIFI_SCRIPTMAX];
} sBASE_MW_WIFI_Info;

/*********************************************************
	variable
*********************************************************/
sBASE_MW_WIFI_Info gsWifiInfo;

/*********************************************************
	prototype
*********************************************************/
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_Init(sBASE_MW_WIFI_Info* psInfo);
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_Update(sBASE_MW_WIFI_Info* psInfo, CHAR* pcUserName, CHAR* pcPassWord);
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_Connect(sBASE_MW_WIFI_Info* psInfo);
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_DisConnect(sBASE_MW_WIFI_Info* psInfo);
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_Deinit(sBASE_MW_WIFI_Info* psInfo);

#ifdef __cplusplus
}
#endif

#endif
