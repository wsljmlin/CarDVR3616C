#ifndef __COMMON_CF_H__
#define __COMMON_CF_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "comm_common.h"

#define COMM_COND_CFIDINVALID -1

typedef enum {
	 COMM_COND_CFSYSRETSUC=0,
	 COMM_COND_CFSYSRETFAIL=-1,
	 COMM_COND_CFSYSQUIT=2,
} eCOMM_COND_CFRETSTATE;

typedef enum {
	COMM_COND_NOBLOCK,
	COMM_COND_BLOCK
} eCOMM_COND_CFBLOCKTYPE;

typedef enum {
	COMM_COND_NOTWRITE,
	COMM_COND_WRITED,	
} eCOMM_COND_CFSTATE;

typedef struct{
	
	/* name */
	CHAR* pcName;

	/* lock */
	pthread_mutex_t lockRet;
	pthread_mutex_t lockId;
	pthread_mutex_t lockIdBack;

	/* condition flag */
	pthread_cond_t condRet;
	pthread_cond_t condId;
	pthread_cond_t condIdBack;

	/* payload */
	LONG lId;
	LONG lRet;
	LONG lTimeout;

	/* type */
	LONG lCfType;

	/* communicate count */
	LONG lCfNotifyCnt;

	/* current cnt update in run time */
	volatile LONG lCfCurNoti;
	volatile LONG lCfCurState;

}sCOMM_CONDFLAG ;

/*------------------------------------------------------------------------------*/
/*!
	@brief			init the condition flag

	@param		sCOMM_CONDFLAG * cf			[in]		the pointer point to the condition flag struct
					LONG lTimeout					[in]		the time out secs of the condition flag
	@param		sCOMM_CONDFLAG * cf			[out]	the pointer point to the condition flag struct
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_CF_InitCF( sCOMM_CONDFLAG * cf, LONG lTimeout, CHAR* pcName);

/*------------------------------------------------------------------------------*/
/*!
	@brief			register the condition flag

	@param		sCOMM_CONDFLAG * psCf		[in]		the pointer point to the condition flag struct
					LONG lType						[in]		the type of the condition flag, COMM_COND_NOTIFY or COMM_COND_FEEDBACK
	@param		sCOMM_CONDFLAG * psCf		[out]	the pointer point to the condition flag struct
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_CF_RegCF( sCOMM_CONDFLAG * psCf);

/*------------------------------------------------------------------------------*/
/*!
	@brief			deregister the condition flag

	@param		sCOMM_CONDFLAG * psCf		[in]		the pointer point to the condition flag struct
	@param		sCOMM_CONDFLAG * psCf		[out]	the pointer point to the condition flag struct
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_CF_DeregCF( sCOMM_CONDFLAG * psCf);



/*------------------------------------------------------------------------------*/
/*!
	@brief			set the id of the condition flag

	@param		sCOMM_CONDFLAG * cf			[in]		the pointer point to the condition flag struct
					LONG lId							[in]		the id of the condition flag
					LONG lType						[in]		the type of the get, COMM_COND_NOBLOCK or COMM_COND_BLOCK
	@param		sCOMM_CONDFLAG * cf			[out]	the pointer point to the condition flag struct
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_CF_SetIdCF( sCOMM_CONDFLAG * cf, LONG lId, LONG lType );

/*------------------------------------------------------------------------------*/
/*!
	@brief			get the id of the condition flag

	@param		sCOMM_CONDFLAG * cf			[in]		the pointer point to the condition flag struct
					LONG lType						[in]		the type of the get, COMM_COND_NOBLOCK or COMM_COND_BLOCK
	@param		LONG lId							[out]	the id of the condition flag
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_CF_GetIdCF( sCOMM_CONDFLAG * cf, LONG* lId, LONG lType);

/*------------------------------------------------------------------------------*/
/*!
	@brief			read the content of the condition flag

	@param		sCOMM_CONDFLAG * cf			[in]		the pointer point to the condition flag struct
	@param		LONG* content					[out]	the content of the condition flag
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_CF_RdCF( sCOMM_CONDFLAG * cf, LONG* content);

/*------------------------------------------------------------------------------*/
/*!
	@brief			get the content of the condition flag

	@param		sCOMM_CONDFLAG * cf			[in]		the pointer point to the condition flag struct
					LONG* content					[in]		the content write to the condition flag
	@param		sCOMM_CONDFLAG * cf			[out]	the pointer point to the condition flag struct
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_CF_WtCF( sCOMM_CONDFLAG * cf, LONG content);

/*------------------------------------------------------------------------------*/
/*!
	@brief			deinit the condition flag

	@param		sCOMM_CONDFLAG * cf			[in]		the pointer point to the condition flag struct
	@param		sCOMM_CONDFLAG * cf			[out]	the pointer point to the condition flag struct
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_CF_DeInitCF( sCOMM_CONDFLAG * cf);

/*------------------------------------------------------------------------------*/
/*!
	@brief			get the absolute time of timeout

	@param		LONG secs							[in]		the time out time in secs
	@param		struct timespec *tsp				[out]	the absolute time of timeout
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_CF_GetTimeoutCF(struct timespec *tsp, long secs);


#ifdef __cplusplus
}
#endif

#endif
