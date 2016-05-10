#ifndef __BASE_MW_FILESORT_H__
#define __BASE_MW_FILESORT_H__

#ifdef __cplusplus
extern "C"{
#endif

/***************************************************
	include
***************************************************/
#include "comm_common.h"
#include "base_mw_sd.h"
#include <sys/stat.h>

/***************************************************
	macro / enum
***************************************************/
typedef enum {
	BASE_MW_FILESORT_SUCCESS,
	BASE_MW_FILESORT_FAIL,
	BASE_MW_SCANDIR_FAIL,
	BASE_MW_NFTW_FAIL,
	BASE_MW_CHECK_FAIL,
} eBASE_MW_FILESORT_RET;

typedef enum {
	BASE_MW_FILESORT_Invalid = 0x0,
	BASE_MW_FILESORT_Valid = 0x1,		
} eBASE_MW_FILESORT_Status;

typedef enum {
	BASE_MW_FILESORT_IsEnough = 0x0,
	BASE_MW_FILESORT_IsNoenough= 0x1,		
} eBASE_MW_FILESORT_SpaceStatus;

/***************************************************
	struct
***************************************************/
typedef struct {
	sCOMM_QUEUE* psFileList;
	CHAR* pcFilePath;
	LONG lState;
	LONG lFileLen;
	LONG lFileMaxLen;
	LONG lUsedSize;
	LONG lUsableSize;
	LONG lIsEnough;
} sBASE_MW_FILESORT_Info;

/***************************************************
	variable
***************************************************/

/***************************************************
	prototype
***************************************************/
eBASE_MW_FILESORT_RET eBASE_MW_FILESORT_Init(sBASE_MW_FILESORT_Info* psInfo, const LONG lMaxLen, const CHAR* pcFilePath);
eBASE_MW_FILESORT_RET eBASE_MW_FILESORT_Write(sBASE_MW_FILESORT_Info* psInfo, const CHAR* pcFileName ) ;
eBASE_MW_FILESORT_RET eBASE_MW_FILESORT_Read(sBASE_MW_FILESORT_Info* psInfo, CHAR* pcFileName );
eBASE_MW_FILESORT_RET eBASE_MW_FILESORT_CheckSpace(sBASE_MW_FILESORT_Info* psInfo, LONG lTotalSpaceSize, double dRatio, LONG lMaxFileSize);
eBASE_MW_FILESORT_RET eBASE_MW_FILESORT_DeInit(sBASE_MW_FILESORT_Info* psInfo);

#ifdef __cplusplus
}
#endif

#endif
