#ifndef __SHAREMEM_COM_H__
#define __SHAREMEM_COM_H__


#ifdef __cplusplus
extern "C"{
#endif

#include "comm_common.h"
#include <sys/shm.h>
#include <sys/stat.h>

/* enum */
typedef enum {
	COMM_SHAREMEM_SUCCESS,
	COMM_SHAREMEM_LENGTHTOOLARGE,
} eCOMM_SHAREMEM_RET;

/* struct */
typedef struct{
	CHAR* pcName;
	UCHAR* pHead;
	LONG lSmId;
	LONG lSmLen;
	struct shmid_ds sSmInfo;
} sCOMM_SHAREMEM;

/* funtion declare */

/*------------------------------------------------------------------------------*/
/*!
	@brief			create share memory

	@param		sCOMM_SHAREMEM* psSm		[in]		the pointer point to the share memory struct
					CHAR* cName						[in]		the name of the share memory
					LONG lSmLen						[in]		the length of the share memory
	@param		sCOMM_SHAREMEM* psSm		[out]	the pointer point to the pipe struct
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_ShareMem_Create(sCOMM_SHAREMEM* psSm, CHAR* pcName, LONG lSmLen);

/*------------------------------------------------------------------------------*/
/*!
	@brief			attach the share memory

	@param		sCOMM_SHAREMEM* psSm		[in]		the pointer point to the share memory struct
	@param		sCOMM_SHAREMEM* psSm		[out]	the pointer point to the share memory struct
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_ShareMem_Attach(sCOMM_SHAREMEM* psSm);

/*------------------------------------------------------------------------------*/
/*!
	@brief			read content from the share memory

	@param		sCOMM_SHAREMEM* psSm		[in]		the pointer point to the pipe struct
	@param		LONG lOffset						[in]		the offset to be read
	@param		CHAR* pcTgt						[out]	the content read from the share memory
	@param		LONG* plLen						[out]	the length of the output content
	@ret			eCOMM_SHAREMEM_RET				COMM_SHAREMEM_SUCCESS or COMM_SHAREMEM_LENGTHTOOLARGE
*/
/* -----------------------------------------------------------------------------*/
eCOMM_SHAREMEM_RET eCOMM_ShareMem_Read(sCOMM_SHAREMEM* psSm, LONG lOffset, UCHAR* pucTgt, LONG* plLen);

/*------------------------------------------------------------------------------*/
/*!
	@brief			get pointer and length of share memory

	@param		sCOMM_SHAREMEM* psSm		[in]		the pointer point to the pipe struct
	@param		LONG lOffset						[in]		the offset to be read
	@param		CHAR** ppcTgt					[out]	the content read from the share memory
	@param		LONG* plLen						[out]	the length of the output content
	@ret			eCOMM_SHAREMEM_RET				COMM_SHAREMEM_SUCCESS or COMM_SHAREMEM_LENGTHTOOLARGE
*/
/* -----------------------------------------------------------------------------*/
eCOMM_SHAREMEM_RET eCOMM_ShareMem_GetDate(sCOMM_SHAREMEM* psSm, LONG lOffset, UCHAR** ppucTgt, LONG* plLen);

/*------------------------------------------------------------------------------*/
/*!
	@brief			write content to the share memory

	@param		sCOMM_SHAREMEM* psSm		[in]		the pointer point to the pipe struct
	@param		LONG lOffset						[in]		the offset to be read
	@param		LONG lLen							[in]		the length to be read
	@param		CHAR* pcSrc						[out]	the source write to the share memory
	@ret			eCOMM_SHAREMEM_RET				COMM_SHAREMEM_SUCCESS or COMM_SHAREMEM_LENGTHTOOLARGE
*/
/* -----------------------------------------------------------------------------*/
eCOMM_SHAREMEM_RET eCOMM_ShareMem_Write(sCOMM_SHAREMEM* psSm, LONG lOffset, UCHAR* pucSrc, LONG lLen);

/*------------------------------------------------------------------------------*/
/*!
	@brief			detach the share memory

	@param		sCOMM_SHAREMEM* psSm		[in]		the pointer point to the share memory struct
	@param		sCOMM_SHAREMEM* psSm		[out]	the pointer point to the share memory struct
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_ShareMem_Detach(sCOMM_SHAREMEM* psSm);

/*------------------------------------------------------------------------------*/
/*!
	@brief			destroy the share memory

	@param		sCOMM_SHAREMEM* psSm		[in]		the pointer point to the share memory struct
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_ShareMem_Destroy(sCOMM_SHAREMEM* psSm);


#ifdef __cplusplus
}
#endif

#endif /* __SHAREMEM_COM_H__ */