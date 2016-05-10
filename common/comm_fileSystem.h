#ifndef __COMM_FILESYSTEM_H__
#define __COMM_FILESYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************
	include
*********************************************************/
//#define _BSD_SOURCE
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif
#include <ftw.h>
#include <sys/stat.h>
#include "comm_common.h"
/*********************************************************
	macro / enum
*********************************************************/
#define TESTFILE		"test_makeup"
#define COMM_FILESYSTEM_MAKEUP_MAX	128

#define COMM_SYSTEM_B2KB(size) ((size)>>10)
#define COMM_SYSTEM_KB2MB(size) ((size)>>10)
#define COMM_SYSTEM_MB2GB(size) ((size)>>10)

#define COMM_SYSTEM_KB_SIZE (1<<10)
#define COMM_SYSTEM_MB_SIZE (1<<20)
#define COMM_SYSTEM_GB_SIZE (1<<30)

typedef enum {
	COMM_FILESYSTEM_SUCCESS,
	COMM_FILESYSTEM_FAIL,
	COMM_FILESYSTEM_PATHNOEXIT,
	COMM_FILESYSTEM_DELETE,
	COMM_FILESYSTEM_MAKEUP,
	COMM_FILESYSTEM_MOUNT,
	COMM_FILESYSTEM_GETSPACE,
	COMM_FILESYSTEM_CHECKSPACE,
	COMM_FILESYSTEM_GETFILESIZE,
} eCOMM_FILESYSTEM_Ret;

/*********************************************************
	struct
*********************************************************/
typedef struct {
	CHAR cMakeUp[COMM_FILESYSTEM_MAKEUP_MAX];
	LONG lFreeSize;									/* total free size */		
	LONG lAvailableSize;								/* total available space size */
	LONG lTotalSize;									/* total space size */
	double dRatio;
} sCOMM_FILESYSTEM_Info;

/*********************************************************
	variable
*********************************************************/

/*********************************************************
	prototype
*********************************************************/
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_Init();
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_DeInit();
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_DeleteFile(CHAR* pcFileName);
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_GetFileSize(CHAR* pcFileName, LONG* plFileSize);
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_CheckSpace(CHAR* pcPathName, LONG lSpaceThreshold);
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_GetAvailableSize(CHAR* pcPathName, LONG* plAvailableSize);
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_GetFreeSpaceSize(CHAR* pcPathName, LONG* plAvailableSize,LONG* plFreeSize);
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_GetToalSpaceSize(CHAR* pcPathName,LONG* plTotalSpaceSize);
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_GetPathInfo(CHAR* pcPathName, sCOMM_FILESYSTEM_Info* psPathInfo);
eCOMM_FILESYSTEM_Ret eCOMM_FILESYSTEM_JudgeMakeUp(CHAR* pcOldMakeUp);

#ifdef __cplusplus
}
#endif

#endif
