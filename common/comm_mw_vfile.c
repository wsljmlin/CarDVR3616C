#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************
	include
*******************************************************/
#define _GNU_SOURCE
#include "comm_mw_vfile.h"
#include <stdlib.h>
#include <fcntl.h>
#include <sys/statvfs.h>
#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>

/*******************************************************
	macro / define
*******************************************************/
//#define COMM_MW_VFILE_Debug
#ifdef COMM_MW_VFILE_Debug
#define CMV_Debug(fmt, arg...) fprintf(stdout, "[ CMV ] : %s < %d > "fmt, __func__, __LINE__, ##arg)
#else
#define CMV_Debug(fmt, arg...)
#endif

#define CMV_FuncIn() //CMV_Debug("in\n");
#define CMV_FuncOut() //CMV_Debug("out\n");
#define CMV_iVal(ival) CMV_Debug("%s <%d>\n", #ival, ival)
#define CMV_lVal(lval) CMV_Debug("%s <%ld>\n", #lval, lval)
#define CMV_ptVal(ptval) CMV_Debug("%s <%p>\n", #ptval, ptval)

#define COMM_MW_VFILE_POSITION (COMM_MW_VFILE_FIRST | COMM_MW_VFILE_SECOND)
#define COMM_MW_VFILE_BUFSIZE  (512 * 1024)
#define COMM_MW_VFILE_MEMALIGN (0x4000)
#define COMM_MW_VFILE_DEFAULT_BLKSIZE (0x10000)

/*******************************************************
	enum 
*******************************************************/

/*******************************************************
	struct
*******************************************************/

/*******************************************************
	variable
*******************************************************/

/*******************************************************
	prototype
*******************************************************/
eCOMM_MW_VFILE_Ret eCOMM_MW_VFILE_GetBlkSize(CHAR* pcSavePath, LONG* lBlkSize);
eCOMM_MW_VFILE_Ret eCOMM_MW_VFILE_Show(sCOMM_MW_VFILE_Info* psInfo);
eCOMM_MW_VFILE_Ret eCOMM_MW_VFILE_PreClose(sCOMM_MW_VFILE_Info* psInfo);
eCOMM_MW_VFILE_Ret eCOMM_MW_VFILE_Clear();

/*******************************************************
	function
*******************************************************/

/********************************************
 * func : eCOMM_MW_VFILE_Open(sCOMM_MW_VFILE_Info* psInfo, const CHAR* pcFileName)
 * arg : sCOMM_MW_VFILE_Info* psInfo, const CHAR* pcFileName
 * ret : eCOMM_MW_VFILE_Ret
 * note : open virtual file
 ********************************************/
eCOMM_MW_VFILE_Ret eCOMM_MW_VFILE_Open(sCOMM_MW_VFILE_Info* psInfo, const CHAR* pcFileName) {
	eCOMM_MW_VFILE_Ret eRet = COMM_MW_VFILE_SUCC;
	LONG lOpenFlag = 0;
	mode_t sOpenMode;

	CMV_FuncIn();

	{
		/* init variable according to flag */
#ifdef MMAP_ENABLE
		lOpenFlag = O_RDWR | O_CREAT | O_TRUNC;
#else
		lOpenFlag = O_WRONLY | O_CREAT | O_TRUNC;
#endif /* MMAP_ENABLE */
		
		if (psInfo->eFlag & COMM_MW_VFILE_NOCACHED) {
			lOpenFlag |= O_DIRECT;
		}

		if (psInfo->eFlag & COMM_MW_VFILE_WITH_SYNC) {
			lOpenFlag |= O_SYNC;
		}

		sOpenMode = S_IRWXU | S_IRWXG | S_IRWXO;
		
		/* open file */
		psInfo->lFd = open(pcFileName, lOpenFlag, sOpenMode); 
		if(psInfo->lFd == -1) {
			printf("[ CMV ] : Error can not open file %s\n", pcFileName);
			return COMM_MW_VFILE_CANNOTOPENFILE;
		}
		psInfo->eFlag |= COMM_MW_VFILE_OPENED;
#ifdef MMAP_ENABLE
		printf("************lseek start***************\n");
		lseek(psInfo->lFd, MMAP_USIZE - 1, SEEK_CUR);
		printf("************lseek end****************\n");
		snprintf(psInfo->cFileName, 128, "%s", pcFileName);
		write(psInfo->lFd, "0",  1);
		printf("************write end***************\n");
    		psInfo->ptrTmp = psInfo->ptr  = mmap(NULL, MMAP_USIZE, 
			PROT_READ |PROT_WRITE, MAP_SHARED, psInfo->lFd, 0);
		printf("************mmap end***************\n");
		if( (psInfo->ptr==MAP_FAILED) ){
			printf("[ CMV ] : mmap error");
		}
		psInfo->sum = MMAP_USIZE;
		psInfo->size= 0;
#endif /* MMAP_ENABLE */
	}

	CMV_FuncOut();

	return eRet;
}

/********************************************
 * func : eCOMM_MW_VFILE_Write(sCOMM_MW_VFILE_Info* psInfo, const UCHAR* pucHeader, const LONG lLen)
 * arg : sCOMM_MW_VFILE_Info* psInfo, const UCHAR* pucHeader, const LONG lLen
 * ret : eCOMM_MW_VFILE_Ret
 * note : write the virtual file
 ********************************************/
eCOMM_MW_VFILE_Ret eCOMM_MW_VFILE_Write(sCOMM_MW_VFILE_Info* psInfo, const UCHAR* pucHeader, const LONG lLen) {
	eCOMM_MW_VFILE_Ret eRet = COMM_MW_VFILE_SUCC;
	LONG lBufToTail;
	LONG lBufRemain;
	LONG lReadNum;
	LONG lNeedReadNum;
	LONG WriteLen = lLen;

	CMV_FuncIn();

	{

		/* prehandle, write at most psInfo->lBufLen */
		if (WriteLen > psInfo->lBufLen) {
			printf("[ Warning ] : too large to write\n");
			WriteLen = psInfo->lBufLen;
		}

		lBufToTail = psInfo->pucTail - psInfo->pucWt;
		
		/* write data to virtual buffer */
		if(WriteLen <= lBufToTail) {
			CMV_Debug("buffer is enough\n");
			memcpy(psInfo->pucWt, pucHeader, WriteLen);
			psInfo->pucWt += WriteLen;
			if( (psInfo->pucWt >= psInfo->pucTail) ){
				psInfo->pucWt =  psInfo->pucBuf;
			}
		} else {
			CMV_Debug("will wrap now\n");
			lBufRemain = WriteLen - lBufToTail;
			memcpy(psInfo->pucWt, pucHeader, lBufToTail);
			memcpy(psInfo->pucBuf, pucHeader + lBufToTail, lBufRemain);
			psInfo->pucWt = psInfo->pucBuf + lBufRemain;
		}

		/* judge whether write to file */
		if((psInfo->eFlag & COMM_MW_VFILE_FIRST) && (psInfo->pucWt >= psInfo->pucMid)) {
			CMV_Debug("the first buffer will be readed\n");
			lNeedReadNum = psInfo->pucMid - psInfo->pucRd;
#ifdef MMAP_ENABLE
			if( (MAP_FAILED != psInfo->ptr) ){
				LONG  size = psInfo->size + lNeedReadNum;
				if( (size > psInfo->sum) ){
					LONG offset = 0;
					munmap(psInfo->ptr, MMAP_USIZE);
					lseek(psInfo->lFd, MMAP_USIZE-1, SEEK_CUR);
					write(psInfo->lFd, "0",  1);
					offset = psInfo->size 
						& (~(sysconf(_SC_PAGE_SIZE) - 1));
					psInfo->ptrTmp=psInfo->ptr =mmap(NULL,  
						MMAP_USIZE + psInfo->size -offset, 
						PROT_READ | PROT_WRITE, 
						MAP_SHARED, psInfo->lFd, offset);
					psInfo->ptrTmp+=(psInfo->size-offset);
					psInfo->sum += MMAP_USIZE;
				}
				memcpy(psInfo->ptrTmp, psInfo->pucRd, lNeedReadNum);
				psInfo->ptrTmp += lNeedReadNum;
				lReadNum = lNeedReadNum;
				psInfo->size = size;
			}else{
				lReadNum = write(
					psInfo->lFd, psInfo->pucRd, lNeedReadNum);
			}	
#else
			lReadNum = write(psInfo->lFd, psInfo->pucRd, lNeedReadNum);
#endif /* MMAP_ENABLE */
			if(lReadNum != lNeedReadNum) {
				perror("write:");
				printf("[ CMV ] : error wrong write\n");
				return COMM_MW_VFILE_WRITE;
			}
			
			psInfo->eFlag &= (~ COMM_MW_VFILE_POSITION);
			psInfo->pucRd = psInfo->pucMid;
			psInfo->eFlag |= COMM_MW_VFILE_SECOND;
		}

		if((psInfo->eFlag & COMM_MW_VFILE_SECOND) && (psInfo->pucWt < psInfo->pucMid)) {
			CMV_Debug("the second buffer will be readed\n");
			lNeedReadNum = psInfo->pucTail - psInfo->pucRd;
#ifdef MMAP_ENABLE
			if( (MAP_FAILED != psInfo->ptr) ){
				LONG  size = psInfo->size + lNeedReadNum;
				if( (size > psInfo->sum) ){
					LONG offset = 0;
					munmap(psInfo->ptr, MMAP_USIZE);
					lseek(psInfo->lFd, MMAP_USIZE-1, SEEK_CUR);
					write(psInfo->lFd, "0",  1);
					offset = psInfo->size 
						& (~(sysconf(_SC_PAGE_SIZE) - 1));
					psInfo->ptrTmp=psInfo->ptr =mmap(NULL,  
						MMAP_USIZE + psInfo->size -offset, 
						PROT_READ | PROT_WRITE, 
						MAP_SHARED, psInfo->lFd, offset);
					psInfo->ptrTmp+=(psInfo->size-offset);
					psInfo->sum += MMAP_USIZE;
				}
				memcpy(psInfo->ptrTmp, psInfo->pucRd, lNeedReadNum);
				psInfo->ptrTmp += lNeedReadNum;
				lReadNum = lNeedReadNum;
				psInfo->size = size;
			}else{
				lReadNum = write(
					psInfo->lFd, psInfo->pucRd, lNeedReadNum);
			}
#else
			lReadNum = write(psInfo->lFd, psInfo->pucRd, lNeedReadNum);
#endif /* MMAP_ENABLE */
			if(lReadNum != lNeedReadNum) {
				perror("write:");
				printf("[ CMV ] : error wrong write\n");
				return COMM_MW_VFILE_WRITE;
			}
			
			psInfo->eFlag &= (~ COMM_MW_VFILE_POSITION);
			psInfo->pucRd = psInfo->pucBuf;
			psInfo->eFlag |= COMM_MW_VFILE_FIRST;
		}
	}

	CMV_FuncOut();

	return eRet;
}

/********************************************
 * func : eCOMM_MW_VFILE_Close(sCOMM_MW_VFILE_Info* psInfo)
 * arg : sCOMM_MW_VFILE_Info* psInfo
 * ret : eCOMM_MW_VFILE_Ret
 * note : close the virtual file
 ********************************************/
eCOMM_MW_VFILE_Ret eCOMM_MW_VFILE_Close(sCOMM_MW_VFILE_Info* psInfo) {
	eCOMM_MW_VFILE_Ret eRet = COMM_MW_VFILE_SUCC;

	CMV_FuncIn();

	{
		eCOMM_MW_VFILE_PreClose(psInfo);
		close(psInfo->lFd);
#ifdef MMAP_ENABLE
		printf("************munmap start***************\n");
		if( (MAP_FAILED != psInfo->ptr) ){
			munmap(psInfo->ptr, MMAP_USIZE);
		}
		printf("************munmap end***************\n");
		truncate(psInfo->cFileName, psInfo->size);
		printf("***truncate end[]%d********\n", psInfo->size);
#endif /* MMAP_ENABLE */

		/* clear the cache according to flag */
		if (psInfo->eFlag & COMM_MW_VFILE_CLEAR_BEFORE_CLOSE) {
			eCOMM_MW_VFILE_Clear();	
		}

		/* reset the vfile, discard the data not write */
		psInfo->lFd = -1;
		psInfo->eFlag &= (~ COMM_MW_VFILE_OPENED);
	}

	CMV_FuncOut();

	return eRet;
}

/********************************************
 * func : eCOMM_MW_VFILE_PreClose(sCOMM_MW_VFILE_Info* psInfo)
 * arg : sCOMM_MW_VFILE_Info* psInfo
 * ret : eCOMM_MW_VFILE_Ret
 * note : before close the virtual file
 ********************************************/
eCOMM_MW_VFILE_Ret eCOMM_MW_VFILE_PreClose(sCOMM_MW_VFILE_Info* psInfo) {
	eCOMM_MW_VFILE_Ret eRet = COMM_MW_VFILE_SUCC;
	LONG lBufRemainCnt = 0;
	LONG lBufRemainSize = 0;
	LONG lReadNum = 0;

	CMV_FuncIn();

	{
		/* judge whether need to write back, notice this algorithm assure the reader pointer never exceed the single buffer  border !! */
		lBufRemainCnt = (psInfo->pucWt - psInfo->pucRd + psInfo->lBlkSize - 1) / psInfo->lBlkSize;
		if(lBufRemainCnt == 0) {
			/* indicate writer equals reader */
			return COMM_MW_VFILE_SUCC;
		}

		/* we have remain data in buffer, write back to file, notice in order to assure alignment, we add padding data */
		lBufRemainSize = lBufRemainCnt * psInfo->lBlkSize;
#ifdef MMAP_ENABLE
		if( (NULL != psInfo->ptr) ){
			LONG  size = psInfo->size + lBufRemainSize;
			if( (size > psInfo->sum) ){
				LONG offset = 0;
				munmap(psInfo->ptr, MMAP_USIZE);
				lseek(psInfo->lFd, MMAP_USIZE-1, SEEK_SET);
				write(psInfo->lFd, "0",  1);
				offset = psInfo->size 
					& (~(sysconf(_SC_PAGE_SIZE) - 1));
				psInfo->ptrTmp=psInfo->ptr =mmap(NULL,  
					MMAP_USIZE + psInfo->size -offset, 
					PROT_READ | PROT_WRITE, 
					MAP_SHARED, psInfo->lFd, offset);
				psInfo->ptrTmp+=(psInfo->size-offset);
				psInfo->sum += MMAP_USIZE;
			}
			memcpy(psInfo->ptrTmp, psInfo->pucRd, lBufRemainSize);
			psInfo->ptrTmp += lBufRemainSize;
			lReadNum = lBufRemainSize;
			psInfo->size = size;
		}else{
			lReadNum = write(
				psInfo->lFd, psInfo->pucRd, lBufRemainSize);
		}
#else
		lReadNum = write(psInfo->lFd, psInfo->pucRd, lBufRemainSize);
#endif /* MMAP_ENABLE */
		if(lReadNum != lBufRemainSize) {
			perror("write:");
			printf("[ CMV ] : error wrong write\n");
			return COMM_MW_VFILE_WRITE;
		}

		/* update reader pt and write pt */
		psInfo->pucRd += lBufRemainSize;
		psInfo->pucWt = psInfo->pucRd;
	}

	CMV_FuncOut();

	return eRet;
}

/********************************************
 * func : eCOMM_MW_VFILE_GetBlkSize(CHAR* pcSavePath, LONG* lBlkSize)
 * arg : CHAR* pcSavePath, LONG* lBlkSize
 * ret : eCOMM_MW_VFILE_Ret
 * note : get block size
 ********************************************/
eCOMM_MW_VFILE_Ret eCOMM_MW_VFILE_GetBlkSize(CHAR* pcSavePath, LONG* lBlkSize) {
	eCOMM_MW_VFILE_Ret eRet = COMM_MW_VFILE_SUCC;
	struct statvfs sVfs;
	LONG lRet = 0;

	CMV_FuncIn();

	{
		/* get block size of save path */
		lRet = statvfs(pcSavePath, &sVfs);
		if (lRet != 0) {
			CMV_Debug("warning : cannot get block size <%ld>\n", lRet);
			*lBlkSize = COMM_MW_VFILE_DEFAULT_BLKSIZE;
			return COMM_MW_VFILE_GETBLOCKSIZE;
		}
		
		*lBlkSize = sVfs.f_bsize;
	}

	CMV_FuncOut();

	return eRet;
}

/********************************************
 * func : eCOMM_MW_VFILE_Init(sCOMM_MW_VFILE_Info* psInfo, CHAR* pcSavePath, eCOMM_MW_VFILE_SetFlag eSetFlag)
 * arg : sCOMM_MW_VFILE_Info* psInfo, CHAR* pcSavePath, eCOMM_MW_VFILE_SetFlag eSetFlag
 * ret : eCOMM_MW_VFILE_Ret
 * note : init struct of vfile
 ********************************************/
eCOMM_MW_VFILE_Ret eCOMM_MW_VFILE_Init(sCOMM_MW_VFILE_Info* psInfo, CHAR* pcSavePath, eCOMM_MW_VFILE_SetFlag eSetFlag) {
	eCOMM_MW_VFILE_Ret eRet = COMM_MW_VFILE_SUCC;
	void* pvHeader = NULL;

	CMV_FuncIn();

	{
		memset(psInfo, 0, sizeof(sCOMM_MW_VFILE_Info));
		psInfo->eFlag = COMM_MW_VFILE_VALID | COMM_MW_VFILE_FIRST;
		eCOMM_MW_VFILE_GetBlkSize(pcSavePath, &psInfo->lBlkSize);
		psInfo->lBufLen = COMM_MW_VFILE_BUFSIZE - (COMM_MW_VFILE_BUFSIZE % psInfo->lBlkSize);
		CMV_lVal(psInfo->lBlkSize);
		CMV_lVal(psInfo->lBufLen);
		
		/* malloc two buffer */
		posix_memalign(&pvHeader, COMM_MW_VFILE_MEMALIGN, 2 * psInfo->lBufLen);
		if(!pvHeader) {
			printf("[ CMV ] : error malloc\n");
			return COMM_MW_VFILE_MALLOC;
		}
		
		psInfo->pucBuf = pvHeader;
		CMV_ptVal(psInfo->pucBuf);
		CMV_lVal(psInfo->lBufLen);

		/* init writer and reader */
		psInfo->pucRd = psInfo->pucBuf;
		psInfo->pucWt = psInfo->pucBuf;
		psInfo->pucMid = psInfo->pucBuf + psInfo->lBufLen;
		psInfo->pucTail = psInfo->pucBuf + 2 * psInfo->lBufLen;

		/* init fd */
		psInfo->lFd = -1;

		/* init eflag according to setflag */
		if (eSetFlag & COMM_MW_VFILE_SETFLAG_NOCACHED) {
			psInfo->eFlag |= COMM_MW_VFILE_NOCACHED;
		}

		if (eSetFlag & COMM_MW_VFILE_SETFLAG_CLEAR_BEFORE_CLOSE) {
			psInfo->eFlag |= COMM_MW_VFILE_CLEAR_BEFORE_CLOSE;
		}

		if (eSetFlag & COMM_MW_VFILE_SETFLAG_WITH_SYNC) {
			psInfo->eFlag |= COMM_MW_VFILE_WITH_SYNC;
		}
	}

	CMV_FuncOut();

	return eRet;
}

eCOMM_MW_VFILE_Ret eCOMM_MW_VFILE_Reset(sCOMM_MW_VFILE_Info* psInfo)
{
	eCOMM_MW_VFILE_Ret eRet = COMM_MW_VFILE_SUCC;

	CMV_FuncIn();
	{
		psInfo->eFlag &= (~ COMM_MW_VFILE_POSITION);
		psInfo->eFlag |= COMM_MW_VFILE_FIRST;
		
		/* init writer and reader */
		psInfo->pucRd = psInfo->pucBuf;
		psInfo->pucWt = psInfo->pucBuf;
		psInfo->pucMid = psInfo->pucBuf + psInfo->lBufLen;
		psInfo->pucTail = psInfo->pucBuf + 2 * psInfo->lBufLen;

		if(psInfo->eFlag & COMM_MW_VFILE_OPENED) {
			close(psInfo->lFd);
			psInfo->lFd = -1;
			psInfo->eFlag &= (~ COMM_MW_VFILE_OPENED);
		}
	}
	CMV_FuncOut();
	
	return eRet;
}
	
/********************************************
 * func : eCOMM_MW_VFILE_Deinit(sCOMM_MW_VFILE_Info* psInfo)
 * arg : sCOMM_MW_VFILE_Info* psInfo
 * ret : eCOMM_MW_VFILE_Ret
 * note : deinit struct of vfile
 ********************************************/
eCOMM_MW_VFILE_Ret eCOMM_MW_VFILE_Deinit(sCOMM_MW_VFILE_Info* psInfo) {
	eCOMM_MW_VFILE_Ret eRet = COMM_MW_VFILE_SUCC;

	CMV_FuncIn();

	{
		/* if fd not closed, close it first */
		if(psInfo->eFlag & COMM_MW_VFILE_OPENED) {
			close(psInfo->lFd);
			psInfo->lFd = -1;
		}
		
		free(psInfo->pucBuf);
		psInfo->eFlag = COMM_MW_VFILE_INVALID;
		psInfo->lBufLen = 0;
		psInfo->lBlkSize = 0;
		psInfo->pucBuf = NULL;
		psInfo->pucMid = NULL;
		psInfo->pucRd = NULL;
		psInfo->pucTail = NULL;
		psInfo->pucWt = NULL;
	}

	CMV_FuncOut();

	return eRet;
}



/********************************************
 * func : eCOMM_MW_VFILE_Show(sCOMM_MW_VFILE_Info* psInfo)
 * arg : sCOMM_MW_VFILE_Info* psInfo
 * ret : eCOMM_MW_VFILE_Ret
 * note : debug the struct
 ********************************************/
eCOMM_MW_VFILE_Ret eCOMM_MW_VFILE_Show(sCOMM_MW_VFILE_Info* psInfo) {
	eCOMM_MW_VFILE_Ret eRet = COMM_MW_VFILE_SUCC;

	CMV_FuncIn();

	{
		CMV_iVal(psInfo->eFlag);
		CMV_lVal(psInfo->lBlkSize);
		CMV_lVal(psInfo->lBufLen);
		CMV_lVal(psInfo->lFd);
		CMV_ptVal(psInfo->pucBuf);
		CMV_ptVal(psInfo->pucMid);
		CMV_ptVal(psInfo->pucRd);
		CMV_ptVal(psInfo->pucTail);
		CMV_ptVal(psInfo->pucWt);
	}

	CMV_FuncOut();

	return eRet;
}

/********************************************
 * func : eCOMM_MW_VFILE_Clear()
 * arg : 
 * ret : eCOMM_MW_VFILE_Ret
 * note : clear the cache
 ********************************************/
eCOMM_MW_VFILE_Ret eCOMM_MW_VFILE_Clear() {
	eCOMM_MW_VFILE_Ret eRet = COMM_MW_VFILE_SUCC;

	CMV_FuncIn();

	{
		system("sync ; sync ; sysctl -w vm.drop_caches=3 > /dev/null");
	}

	CMV_FuncOut();

	return eRet;
}

#ifdef __cplusplus
}
#endif

