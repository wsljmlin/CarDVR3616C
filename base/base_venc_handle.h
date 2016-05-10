#ifndef __BASE_VENC_HANDLE_H__
#define __BASE_VENC_HANDLE_H__
#endif

#ifdef __cplusplus
extern "C" 
{
#endif

/*********************************************************
	include 
*********************************************************/
#include "base_comm.h"

/*********************************************************
	macro 
*********************************************************/
#define BASE_VENC_HANDLE_VENCBUFMAX (512 * 1024)
#define BASE_VENC_HANDLE_VENCBUFCNT	(512)
#define BASE_VENC_HANDLE_VENCBUFSIZEALL (45 * 1024 * 1024)

/*********************************************************
	enum
*********************************************************/
typedef enum {
	BASE_VENC_HANDLE_SUCCESS,
	BASE_VENC_HANDLE_ERROR,
	BASE_VENC_HANDLE_RTPSENDERR,
	BASE_VENC_HANDLE_VFILE,
}eBASE_VENC_HANDLE_ERR;

typedef enum {
	BASE_VENC_HANDLE_THREADEXIT,
 	BASE_VENC_HANDLE_THREADRUNNING	
}eBASE_VENC_HANDLE_THREADSTAT;


/*********************************************************
	struct
*********************************************************/

/** High handle **/
/* rtp send para */
typedef struct {
	void* pvSession;
	LONG lFrameRate;
	LONG lVencCh;
	LONG lRtpCh;
	eBASE_VENC_HANDLE_ERR eErr;
}sBASE_VENC_HIGHHANDLE_RTPSendArgv;

/* real time handle arguments */
typedef struct {
	LONG lPoolID;
	eBASE_VENC_HANDLE_THREADSTAT eThreadStat;
	sBASE_VENC_HIGHHANDLE_RTPSendArgv sRtpArgv;
}sBASE_VENC_HIGHHANDLE_HandleArgv;

/* rtp argument of start realtime handle thread  */
typedef struct {
	sBASE_PARA_RTP* psRtpPara;
	sCOMM_IP_LIST* psIplist;
	LONG lRtpCh;
}sBASE_VENC_HIGHHANDLE_ThreadRtpArgv;

/* arguments of start realtime handle thread*/
typedef struct {
	pthread_t PidHighHandle; 
	sBASE_VENC_HIGHHANDLE_ThreadRtpArgv sRtpThreadArgv;
	sBASE_VENC_HIGHHANDLE_HandleArgv sHandleArgv;
}sBASE_VENC_HIGHHANDLE_ThreadArgv;

/*********************************************************
	variable
*********************************************************/
sBASE_VENC_HIGHHANDLE_ThreadArgv gsHighThreadArgv;

/*********************************************************
	prototype
*********************************************************/
eBASE_VENC_HANDLE_ERR eBASE_VENC_HIGHHANDLE_StartThread(sBASE_VENC_HIGHHANDLE_ThreadArgv* psRealTimeThreadArgv);
eBASE_VENC_HANDLE_ERR eBASE_VENC_HIGHHANDLE_StopThread();
eBASE_VENC_HANDLE_ERR eBASE_VENC_HANDLE_ResDeInit();
eBASE_VENC_HANDLE_ERR eBASE_VENC_HANDLE_ResInit();

#ifdef __cplusplus
}
#endif
