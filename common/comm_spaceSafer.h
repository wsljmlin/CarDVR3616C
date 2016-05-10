#ifndef __COMM_SPACESAFER_H__
#define __COMM_SPACESAFER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************
	include
*********************************************************/
#include "comm_common.h"
#include "comm_fileSystem.h"
#include "comm_fileSort.h"

/*********************************************************
	macro / enum
*********************************************************/
#define COMM_SPACESAFER_MAX			128
#define COMM_SPACESAFER_MAKEUP_MAX	64

#define COMM_SPACESAFER_MOUNT_PATH "/opt/mainCard/"
#define COMM_SPACESAFER_NAND_PATH "/opt/subCard/"
#define COMM_SPACESAFER_MUXPATH "/opt/mainCard/rec/"
#define COMM_SPACESAFER_SNAPPATH "/opt/mainCard/snapshot/"
#define COMM_SPACESAFER_RECAUDIOPATH "/nfsroot/"

#define COMM_SPACESAFER_B2KB(size) ((size)>>10)
#define COMM_SPACESAFER_KB2MB(size) ((size)>>10)
#define COMM_SPACESAFER_MB2GB(size) ((size)>>10)

#define COMM_SPACESAFER_KB_SIZE (1<<10)
#define COMM_SPACESAFER_MB_SIZE (1<<20)
#define COMM_SPACESAFER_GB_SIZE (1<<30)

#define COMM_SPACESAFER_SNAP_FILE_SIZE  10	//10M
#define COMM_SPACESAFER_MUX_FILE_SIZE  100	//100M

#define COMM_NAND_FILE_SIZE (100 * COMM_SPACESAFER_MB_SIZE)
#define COMM_NAND_FILE_SIZEKB (COMM_SPACESAFER_B2KB(COMM_NAND_FILE_SIZE))

#define COMM_SPACESAFER_FILE_TEST (0 * 1024 * 1024)

typedef enum {
	eCOMM_SPACESAFER_Overlay,
	eCOMM_SPACESAFER_NotOverlay,
} eCOMM_SPACESAFER_ForceOverlay;

typedef enum {
	COMM_SPACESAFER_SUCCESS,
	COMM_SPACESAFER_FAIL,
	COMM_SPACESAFER_MALLOC,
	COMM_SPACESAFER_CREATE,
	COMM_SPACESAFER_NOSPACE,
	COMM_SPACESAFER_NOFILE,
	COMM_SPACESAFER_COPMPARE,
} eCOMM_SPACESAFER_Ret;

typedef enum {
	COMM_SPACESAFER_STATUS_INVALID,
	COMM_SPACESAFER_STATUS_VALID,
} eCOMM_SPACESAFER_Status;

typedef enum {
	COMM_SPACESAFER_OLDEST,
	COMM_SPACESAFER_LASTEST,
} sCOMM_SPACESAFER_Time;

typedef enum {
	COMM_SPACESAFER_HAVE_SPACE,
	COMM_SPACESAFER_NOTHAVE_SPACE,
	COMM_SPACESAFER_INVALID_SPACE,
} sCOMM_SPACESAFER_SpaceStatus;

/*********************************************************
	struct
*********************************************************/
typedef struct {
	CHAR cCurrentPath[COMM_SPACESAFER_MAX];
	CHAR cMakeUp[COMM_SPACESAFER_MAX];
	CHAR cMountPath[COMM_SPACESAFER_MAX];
	CHAR cTargetPath[COMM_SPACESAFER_MAX];
	LONG lFileSizeThreshold;
	LONG lIsForceOverlay;
	eCOMM_SPACESAFER_Status eStatus;
	sCOMM_FILESORT_Info* psFileSortInfo;
	sCOMM_FILESYSTEM_Info* psFileSystemInfo;	
} sCOMM_SPACESAFER_Info;

/*********************************************************
	variable
*********************************************************/

/*********************************************************
	prototype
*********************************************************/
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_Init(sCOMM_SPACESAFER_Info* psInfo, 
														CHAR* pcTargetPath, 
														CHAR* pcMountPath, 
														LONG lFileSizeTHreshold, 
														LONG lIsForceOverlay,
														double dRatio);
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_DeInit(sCOMM_SPACESAFER_Info* psInfo);
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_QuerySpace(sCOMM_SPACESAFER_Info* psInfo);
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_RegisterFile(sCOMM_SPACESAFER_Info* psInfo, 
																CHAR* pcFileName);
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_Compare(sCOMM_SPACESAFER_Info* psFirtInfo,
															sCOMM_SPACESAFER_Info* psSecInfo,
															sCOMM_SPACESAFER_Time sTimeStatus,
															CHAR *pcPathName,
															LONG *plIndex);
eCOMM_SPACESAFER_Ret eCOMM_SPACESAFER_CheckSpace(sCOMM_SPACESAFER_Info* psInfo,
																LONG lFileSizeTHreshold);

#ifdef __cplusplus
}
#endif

#endif
