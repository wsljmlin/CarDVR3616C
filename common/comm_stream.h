#ifndef __COMM_STREAM_H__
#define __COMM_STREAM_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************
	include
*********************************************************/
#include "comm_common.h"

/*********************************************************
	macro / enum
*********************************************************/
typedef enum {
	COMM_STREAM_SUCCESS,
	COMM_STREAM_FAIL,
} eCOMM_STREAM_Ret;

/*********************************************************
	struct
*********************************************************/

/*********************************************************
	variable
*********************************************************/

/*********************************************************
	prototype
*********************************************************/
eCOMM_STREAM_Ret eCOMM_STREAM_FindByte(UCHAR* pucStream, CHAR* pcByte, LONG lStreamLen, UCHAR** ppucPosition);
eCOMM_STREAM_Ret eCOMM_STREAM_FindNotByte(UCHAR* pucStream, CHAR* pcByte, LONG lStreamLen, UCHAR** ppucPosition);
eCOMM_STREAM_Ret eCOMM_STREAM_Split(UCHAR* pucStream, CHAR* pcDelimi, LONG lStreamLen, UCHAR* pucEntry[], LONG* plEntryLen);

#ifdef __cplusplus
}
#endif

#endif
