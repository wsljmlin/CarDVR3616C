#ifndef __COMM_MW_STRPOOL_H__
#define __COMM_MW_STRPOOL_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************
	include
*****************************************************/
#include "comm_common.h"

/*****************************************************
	define / enum
*****************************************************/
typedef enum {
	COMM_MW_StrPoolSuccess,
	COMM_MW_StrPoolFull,
	COMM_MW_StrPoolTooLarge,
	COMM_MW_StrPoolFail,
} eCOMM_MW_StrPoolRet;

/*****************************************************
	struct
*****************************************************/

/*****************************************************
	variable
*****************************************************/

/*****************************************************
	prototype
*****************************************************/
eCOMM_MW_StrPoolRet COMM_MW_StrPool_Init(UCHAR* pcStrPool, LONG lPoolSize);
eCOMM_MW_StrPoolRet COMM_MW_StrPool_WriteEntry(UCHAR* pcStrPool, CHAR* pcWriteEntry, LONG* plPosition, pthread_mutex_t* psMutWt);
eCOMM_MW_StrPoolRet COMM_MW_StrPool_ReadEntry(UCHAR* pcStrPool, CHAR* pcReadEntry, LONG lPosition);


#ifdef __cplusplus
}
#endif

#endif
