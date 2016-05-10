#ifdef __cplusplus
extern "C"{
#endif

#include "comm_queue.h"

/***************************************************************************
  Function		: IPC_COMM_CQ_INIT( )
  Description	: init the cycle queue for thread communicate
  INput			: void
  OUTput		: void
  Notation

 History:
  No.	Date			Designed by		Comment
-----+---------------+--------------+---------------
  1		2013.07.12		CASKY			develope cycle queue, add the thread of mux
  2		2013.07.15		CASKY			improve the cycle queue to linked list, control by mutex


***************************************************************************/
void vCOMM_LQ_Init( sCOMM_QUEUE* q, LONG qmax ) {
	q->psHead =  NULL;
	q->psTail = NULL;
	pthread_mutex_init(&q->mutex,NULL);
	sem_init (&q->frm_queue_max,   0, qmax);
	sem_init (&q->frm_queue_count, 0,   0);
}

/***************************************************************************
  Function		: IPC_COMM_CQ_WRITE( )
  Description	: write the node to the cycle queue
  INput			: char* data	: the payload of the cycle queue
  INput           : LONG lLen : the payload len
  OUTput		: void
  Notation

 History:
  No.	Date			Designed by		Comment
-----+---------------+--------------+---------------
  1		2013.07.12		CASKY			develope cycle queue, add the thread of mux
  2		2013.07.15		CASKY			improve the cycle queue to linked list, control by mutex


***************************************************************************/
void vCOMM_LQ_Write(sCOMM_QUEUE *q, CHAR* pcData, LONG lLen) {
	sCOMM_LIST* pPstTmp = 
		(sCOMM_LIST*)malloc(sizeof(sCOMM_LIST));
	MALLOC(sizeof(sCOMM_LIST));
	memset(pPstTmp, 0,sizeof(sCOMM_LIST));

	/* make payload */
	if (lLen == AUTO_DETECT_STRLEN) {
		pPstTmp->pcVal=strdup(pcData);
	} else {
		pPstTmp->pcVal = calloc(lLen, sizeof(CHAR));
		memcpy(pPstTmp->pcVal, pcData, lLen);
	}

	pPstTmp->lLen = lLen;
	pPstTmp->psNext=NULL;
	
	sem_wait(&q->frm_queue_max);

	/* get lock */	
	pthread_mutex_lock(&q->mutex);

	/* insert the node */
	if( (q->psHead == NULL) ){
		q->psHead = q->psTail = pPstTmp;
	}else{
		q->psTail->psNext = pPstTmp;
		q->psTail = pPstTmp;
	}
	sem_post(&q->frm_queue_count);

	/* release the lock */
	pthread_mutex_unlock(&q->mutex);
}


/***************************************************************************
  Function		: IPC_COMM_CQ_READ( )
  Description	: read the node of the cycle queue
  INput			: char* data	: the read payload of the cycle queue
  OUTput		: void
  Notation

 History:
  No.	Date			Designed by		Comment
-----+---------------+--------------+---------------
  1		2013.07.12		CASKY			develope cycle queue, add the thread of mux
  2		2013.07.15		CASKY			improve the cycle queue to linked list, control by mutex


***************************************************************************/
void vCOMM_LQ_Read(sCOMM_QUEUE* q, CHAR** ppcData )
{
	sCOMM_LIST* pPstTmp = NULL;
	/* read one data */
	sem_wait(&q->frm_queue_count);
	
	pthread_mutex_lock(&q->mutex);
	pPstTmp  = q->psHead;
	q->psHead = q->psHead->psNext;
	sem_post(&q->frm_queue_max);
	pthread_mutex_unlock(&q->mutex);
	
	/* copy the value for read */
	if (pPstTmp->lLen == AUTO_DETECT_STRLEN) {
		strcpy(*ppcData, pPstTmp->pcVal);
	} else {
		memcpy(*ppcData, pPstTmp->pcVal, pPstTmp->lLen);
	}

	/* free the duplicata pointer */
	free(pPstTmp->pcVal);

	/* free the node */
	free(pPstTmp);
}

/***************************************************************************
  Function		: IPC_COMM_CQ_DEINIT( )
  Description	: deinit the cycle queue
  INput			: void
  OUTput		: void
  Notation

 History:
  No.	Date			Designed by		Comment
-----+---------------+--------------+---------------
  1		2013.07.12		CASKY			develope cycle queue, add the thread of mux
  2		2013.07.15		CASKY			improve the cycle queue to linked list, control by mutex


***************************************************************************/
void vCOMM_LQ_DeInit(sCOMM_QUEUE* q) {
	sCOMM_LIST* pPstTmp = q->psHead;
	while( (q->psHead  != NULL) ){
		q->psHead = q->psHead->psNext;
		
		/* free the duplicate string */
		free(pPstTmp->pcVal);

		/* free the node */
		free(pPstTmp);
		pPstTmp = q->psHead;
	}
	sem_destroy(&q->frm_queue_count);
	sem_destroy(&q->frm_queue_max);
	pthread_mutex_destroy(&q->mutex);
}

#ifdef __cplusplus
}
#endif
