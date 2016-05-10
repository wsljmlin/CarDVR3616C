#ifndef __COMM_DD_IRCUT_H__
#define __COMM_DD_IRCUT_H__

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
typedef enum {
 	eCOMM_DD_IRCUT_SUCCESS,
 	eCOMM_DD_IRCUT_FAIL,
} eCOMM_DD_IRCUT_Ret;

typedef enum {
 	eCOMM_DD_IRCUT_TYPE_DAY=1,
 	eCOMM_DD_IRCUT_TYPE_NIGHT,
} eCOMM_DD_IRCUT_TYPE;

/*********************************************************
	struct
*********************************************************/
typedef struct {
	LONG lFd;
	eCOMM_DD_IRCUT_TYPE eIrcutType;
} sCOMM_DD_IRCUT_Info;

/*********************************************************
	variable
*********************************************************/

/*********************************************************
	prototype
*********************************************************/
eCOMM_DD_IRCUT_Ret eCOMM_DD_IRCUT_Init(sCOMM_DD_IRCUT_Info* psInfo, LONG lFd);
eCOMM_DD_IRCUT_Ret eCOMM_DD_IRCUT_Set(sCOMM_DD_IRCUT_Info* psInfo, eCOMM_DD_IRCUT_TYPE lType);
eCOMM_DD_IRCUT_Ret eCOMM_DD_IRCUT_DeInit(sCOMM_DD_IRCUT_Info* psInfo);

#ifdef __cplusplus
}
#endif

#endif
