#ifndef __COMM_MW_VFILE_H__
#define __COMM_MW_VFILE_H__

#ifdef __cplusplus
extern "C" {
#endif

//#define MMAP_ENABLE
/*******************************************************
	include
*******************************************************/
#include "comm_common.h"

/*******************************************************
	macro / define
*******************************************************/
#ifdef MMAP_ENABLE
#define MMAP_USIZE		(4*1024*1024)
#endif  /* MMAP_ENABLE */

/*******************************************************
	enum
*******************************************************/
typedef enum {
	COMM_MW_VFILE_SUCC,
	COMM_MW_VFILE_FAIL,
	COMM_MW_VFILE_CANNOTOPENFILE,
	COMM_MW_VFILE_WRITE,
	COMM_MW_VFILE_MALLOC,
	COMM_MW_VFILE_GETBLOCKSIZE,
} eCOMM_MW_VFILE_Ret;

typedef enum {
	COMM_MW_VFILE_INVALID = 0x0,
	COMM_MW_VFILE_FIRST = 0x01,
	COMM_MW_VFILE_SECOND = 0x02,
	COMM_MW_VFILE_VALID = 0x10,
	COMM_MW_VFILE_OPENED = 0x20,
	COMM_MW_VFILE_NOCACHED = 0x100,
	COMM_MW_VFILE_CLEAR_BEFORE_CLOSE = 0x200,
	COMM_MW_VFILE_WITH_SYNC = 0x400,
} eCOMM_MW_VFILE_Flag;

typedef enum {
	COMM_MW_VFILE_SETFLAG_NONE = 0x0,
	COMM_MW_VFILE_SETFLAG_NOCACHED = 0x1,
	COMM_MW_VFILE_SETFLAG_CLEAR_BEFORE_CLOSE = 0x2,
	COMM_MW_VFILE_SETFLAG_WITH_SYNC = 0x4,
} eCOMM_MW_VFILE_SetFlag;

/*******************************************************
	struct
*******************************************************/
typedef struct {
	UCHAR* pucBuf;
	UCHAR* pucWt;
	UCHAR* pucRd;
	UCHAR* pucTail;
	UCHAR* pucMid;
	LONG lBlkSize;
	LONG lBufLen;
	LONG lFd;
#ifdef MMAP_ENABLE
	char *ptrTmp;
	LONG sum;
	char *ptr;
	LONG size;
	char cFileName[128];
#endif /* MMAP_ENABLE */
	eCOMM_MW_VFILE_Flag eFlag;
} sCOMM_MW_VFILE_Info;

/*******************************************************
	variable
*******************************************************/

/*******************************************************
	prototype
*******************************************************/
eCOMM_MW_VFILE_Ret eCOMM_MW_VFILE_Init(sCOMM_MW_VFILE_Info* psInfo, CHAR* pcSavePath, eCOMM_MW_VFILE_SetFlag eSetFlag);
eCOMM_MW_VFILE_Ret eCOMM_MW_VFILE_Open(sCOMM_MW_VFILE_Info* psInfo, const CHAR* pcFileName);
eCOMM_MW_VFILE_Ret eCOMM_MW_VFILE_Write(sCOMM_MW_VFILE_Info* psInfo, const UCHAR* pucHeader, const LONG lLen);
eCOMM_MW_VFILE_Ret eCOMM_MW_VFILE_Close(sCOMM_MW_VFILE_Info* psInfo);
eCOMM_MW_VFILE_Ret eCOMM_MW_VFILE_Reset(sCOMM_MW_VFILE_Info* psInfo);
eCOMM_MW_VFILE_Ret eCOMM_MW_VFILE_Deinit(sCOMM_MW_VFILE_Info* psInfo);


#ifdef __cplusplus
}
#endif

#endif
