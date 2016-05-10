#ifndef __PIPE_COM_H__
#define __PIPE_COM_H__


#ifdef __cplusplus
extern "C"{
#endif

#include "comm_common.h"

/* macro */
enum {
	COMM_PIPE_READ,
	COMM_PIPE_WRITE,
} eCOMM_PIPE_TYPE;

#define COMM_PIPE_GET_PIPERD(lFd) ((lFd)[COMM_PIPE_READ])
#define COMM_PIPE_GET_PIPEWT(lFd) ((lFd)[COMM_PIPE_WRITE])
#define COMM_PIPE_MAXCHAR 100

/* struct */
typedef struct{
	CHAR* cName;
	LONG lFd[2];
	LONG lFlag;
	FILE* pStream;
} sCOMM_PIPE_COMMU;

/* funtion declare */

/*------------------------------------------------------------------------------*/
/*!
	@brief			create the pipe

	@param		sCOMM_PIPE_COMMU* pc		[in]		the pointer point to the pipe struct
					CHAR* cName						[in]		the name of the pipe
	@param		sCOMM_PIPE_COMMU* pc		[out]	the pointer point to the pipe struct
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_Pipe_Create(sCOMM_PIPE_COMMU* pc, CHAR* cName);

/*------------------------------------------------------------------------------*/
/*!
	@brief			init the pipe according to the pipe type, open the stream

	@param		sCOMM_PIPE_COMMU* pc		[in]		the pointer point to the pipe struct
					LONG lFlag						[in]		the type of the pipe, COMM_PIPE_READ or COMM_PIPE_WRITE
	@param		sCOMM_PIPE_COMMU* pc		[out]	the pointer point to the pipe struct
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_Pipe_Init(sCOMM_PIPE_COMMU* pc, LONG lFlag);

/*------------------------------------------------------------------------------*/
/*!
	@brief			read content from the pipe

	@param		sCOMM_PIPE_COMMU* pc		[in]		the pointer point to the pipe struct
	@param		CHAR* content					[out]	the content read from the pipe
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_Pipe_Read(sCOMM_PIPE_COMMU* pc, CHAR* content);

/*------------------------------------------------------------------------------*/
/*!
	@brief			write content to the pipe

	@param		sCOMM_PIPE_COMMU* pc		[in]		the pointer point to the pipe struct
	@param		CHAR* content					[in]		the content write to the pipe
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_Pipe_Write(sCOMM_PIPE_COMMU* pc, CHAR* content);

/*------------------------------------------------------------------------------*/
/*!
	@brief			deinit the pipe

	@param		sCOMM_PIPE_COMMU* pc		[in]		the pointer point to the pipe struct
	@param		sCOMM_PIPE_COMMU* pc		[out]	the pointer point to the pipe struct
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_Pipe_DeInit(sCOMM_PIPE_COMMU* pc);
#ifdef SHOW_INFO
/*------------------------------------------------------------------------------*/
/*!
	@brief			show the information of the pipe

	@param		sCOMM_PIPE_COMMU* pc		[in]		the pointer point to the pipe struct
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_Pipe_Show(sCOMM_PIPE_COMMU* pc);
#endif /* SHOW_INFO */


#ifdef __cplusplus
}
#endif

#endif /* __PIPE_COM_H__ */