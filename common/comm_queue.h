#ifndef __COMMON_QUEUE_H__
#define __COMMON_QUEUE_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "comm_common.h"

#define AUTO_DETECT_STRLEN 0

typedef struct _sCOMM_LIST {
	struct _sCOMM_LIST *psNext;
	CHAR* pcVal;
	LONG lLen;
}sCOMM_LIST;

typedef struct {
	/* current count */
	sem_t frm_queue_count;
	/* max count */
	sem_t frm_queue_max;
	/* head pointer */
	sCOMM_LIST *psHead;
	/* tail pointer */
	sCOMM_LIST *psTail;
	/* mutex */
	pthread_mutex_t mutex;
}sCOMM_QUEUE;

/*------------------------------------------------------------------------------*/
/*!
	@brief		init the list queue

	@param	sCOMM_QUEUE* q		[in]			the pointer point to the queue struct
				LONG qmax				[in]			the max container in the queue
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_LQ_Init( sCOMM_QUEUE* q, LONG qmax );

/*------------------------------------------------------------------------------*/
/*!
	@brief		add a new queue node to the queue, with the content of the input data

	@param	sCOMM_QUEUE* q		[in]			the pointer point to the queue struct
				CHAR* pcData				[in]			the content write to the queue node
	@param	sCOMM_QUEUE* q		[out]		the pointer point to the queue struct, add the write node
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_LQ_Write(sCOMM_QUEUE *q, CHAR* pcData, LONG lLen) ;


/*------------------------------------------------------------------------------*/
/*!
	@brief		read the content of the queue, and delete the node

	@param	sCOMM_QUEUE* q		[in]			the pointer point to the queue struct
	@param	CHAR** pcData			[out]		the content read from the queue node
				sCOMM_QUEUE* q		[out]		the pointer point to the queue struct, add the write node
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_LQ_Read(sCOMM_QUEUE* q, CHAR** ppcData );

/*------------------------------------------------------------------------------*/
/*!
	@brief		delete the queue

	@param	sCOMM_QUEUE* q		[in]			the pointer point to the queue struct
	@param	sCOMM_QUEUE* q		[out]		the pointer point to the queue struct, delete the node
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_LQ_DeInit(sCOMM_QUEUE* q) ;

#ifdef __cplusplus
}
#endif

#endif
