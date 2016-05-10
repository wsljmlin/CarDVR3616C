#ifdef __cplusplus
extern "C"{
#endif

#include "comm_sharemem.h"


void vCOMM_ShareMem_Create(sCOMM_SHAREMEM* psSm, CHAR* pcName, LONG lSmLen){
	/* modify the length */
	lSmLen&=(~(getpagesize()-1));

	/* init share memory */
	psSm->pcName=strdup(pcName);
	psSm->lSmLen=lSmLen;
	psSm->pHead=NULL;
	
	/* shget */
	psSm->lSmId=shmget(IPC_PRIVATE, lSmLen, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
}

void vCOMM_ShareMem_Attach(sCOMM_SHAREMEM* psSm){
	/* get address */
	psSm->pHead=(UCHAR*)shmat(psSm->lSmId, 0, 0);

	/* get info */
	shmctl(psSm->lSmId, IPC_STAT, &psSm->sSmInfo);
}

eCOMM_SHAREMEM_RET eCOMM_ShareMem_Read(sCOMM_SHAREMEM* psSm, LONG lOffset, UCHAR* pucTgt, LONG* plLen){
	UCHAR* pcTmp;

	/* check input length */
	if(lOffset>psSm->lSmLen){
		return COMM_SHAREMEM_LENGTHTOOLARGE;
	}

	/* first, get length */
	pcTmp=psSm->pHead+lOffset;
	*plLen=*(LONG*)pcTmp;

	/* second, get content */
	pcTmp+=4;
	memcpy(pucTgt, pcTmp, *plLen);

	return COMM_SHAREMEM_SUCCESS;
}

eCOMM_SHAREMEM_RET eCOMM_ShareMem_GetDate(sCOMM_SHAREMEM* psSm, LONG lOffset, UCHAR** ppucTgt, LONG* plLen){
	UCHAR* pcTmp;

	/* check input length */
	if(lOffset>psSm->lSmLen){
		return COMM_SHAREMEM_LENGTHTOOLARGE;
	}

	/* first, get length */
	pcTmp=psSm->pHead+lOffset;
	*plLen=*(LONG*)pcTmp;

	/* second, get pointer */
	pcTmp+=4;
	*ppucTgt=pcTmp;

	return COMM_SHAREMEM_SUCCESS;
}

eCOMM_SHAREMEM_RET eCOMM_ShareMem_Write(sCOMM_SHAREMEM* psSm, LONG lOffset, UCHAR* pucSrc, LONG lLen){
	UCHAR* pcTmp;

	/* check input length */
	if((lOffset+lLen)>psSm->lSmLen){
		return COMM_SHAREMEM_LENGTHTOOLARGE;
	}

	/* first, write the length */
	pcTmp=psSm->pHead+lOffset;
	*(LONG*)pcTmp=lLen;

	/* second, write the content */
	pcTmp+=4;
	memcpy(pcTmp, pucSrc, lLen);
	
	return COMM_SHAREMEM_SUCCESS;
}

void vCOMM_ShareMem_Detach(sCOMM_SHAREMEM* psSm){
	if(psSm->pHead){
		shmdt(psSm->pHead);
	}
}

void vCOMM_ShareMem_Destroy(sCOMM_SHAREMEM* psSm){
	/* free the name */
	free(psSm->pcName);

	/* destroy the memory */
	shmctl(psSm->lSmId, IPC_RMID, 0);
}



#ifdef __cplusplus
}
#endif


