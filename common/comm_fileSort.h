#ifndef __COMM_FILESORT_H__
#define __COMM_FILESORT_H__

#ifdef __cplusplus
extern "C"{
#endif

/***************************************************
	include
***************************************************/
#include "comm_list.h"
#include "comm_common.h"
#include <sys/stat.h>
/***************************************************
	macro / enum
***************************************************/
#define COMM_FILESORT_FILENAME_MAX 128

typedef enum {
	COMM_FILESORT_SUCCESS,
	COMM_FILESORT_FAIL,
	COMM_SCANDIR_FAIL,
	COMM_FILESORT_CREATE,
	COMM_FILESORT_READ,
	COMM_FILESORT_WRITE,
	COMM_FILESORT_RESET,
	COMM_FILESORT_DELETE,
} eCOMM_FILESORT_Ret;

typedef enum {
	COMM_FILESORT_Invalid = 0x0,
	COMM_FILESORT_Valid = 0x1,		
} eCOMM_FILESORT_Status;

/***************************************************
	struct
***************************************************/
typedef struct 
{
	CHAR cFileName[COMM_FILESORT_FILENAME_MAX];
}  sCOMM_FILESORT_UserEntry;

typedef struct {
	sCOMM_LIST_Info* psFileNameList;
	CHAR* pcFilePath;
	LONG lState;
	LONG lFileLen;
} sCOMM_FILESORT_Info;

/***************************************************
	variable
***************************************************/

/***************************************************
	prototype
***************************************************/
eCOMM_FILESORT_Ret eCOMM_FILESORT_Init(sCOMM_FILESORT_Info* psInfo);
eCOMM_FILESORT_Ret eCOMM_FILESORT_Creat(sCOMM_FILESORT_Info* psInfo, const CHAR* pcFilePath);
eCOMM_FILESORT_Ret eCOMM_FILESORT_Write(sCOMM_FILESORT_Info* psInfo, const CHAR* pcFileName ) ;
eCOMM_FILESORT_Ret eCOMM_FILESORT_Read(sCOMM_FILESORT_Info* psInfo, eCOMM_LIST_RetrieveKey eKey, CHAR* pcFileName);
eCOMM_FILESORT_Ret eCOMM_FILESORT_Delete(sCOMM_FILESORT_Info* psInfo, CHAR* pcFileName );
eCOMM_FILESORT_Ret eCOMM_FILESORT_GetCount(sCOMM_FILESORT_Info* psInfo, LONG* plCount);
eCOMM_FILESORT_Ret eCOMM_FILESORT_Reset(sCOMM_FILESORT_Info* psInfo);
eCOMM_FILESORT_Ret eCOMM_FILESORT_DeInit(sCOMM_FILESORT_Info* psInfo);

#ifdef __cplusplus
}
#endif

#endif
