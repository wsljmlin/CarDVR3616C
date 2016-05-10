/**********************************************************************
Copyright (C) 2013 CASKY eTech Co., Ltd .
------------------------------------------------------------
 Project    : IPC
************************************************************************
   File Name	: ipc_rtp.h
   Module Name	: ipc rtp common function
   Description  : 
   History:
  No.		Ver.		Date		Designed by		Comment
-----+----------+-----------------+--------------+-----------------
  1		0.0.1		2013.06.03		CASKY			new create
  2		0.0.2		2013.06.04		CASKY			add rtp
  3		0.0.3		2013.07.11		CASKY			add global flag
  4		0.0.4		2013.07.14		CASKY			add global flag
  5		0.0.5		2013.07.15		CASKY			add global flag


************************************************************************/
#ifndef __PANA_IPC_BASE_RTP_H__
#define __PANA_IPC_BASE_RTP_H__
//end add
#ifdef __cplusplus
extern "C"
{
#endif
extern int RTPSession_delport(void* vp, char *remote_ip4addr,
			unsigned short int destport);

extern int RTPSession_addport(void* vp, char *remote_ip4addr,
			unsigned short int destport);

extern int RTPSession_serverinit(void** vp, unsigned short int portbase,
			double dHz);

extern int RTPSession_clientinit(void** vp, unsigned short int destport, 
			double dHz);
extern int RTPSession_setplayloadtype(void* vp, int playloadtype);

extern void RTPSession_destroy(void* vp, unsigned int sec, unsigned int usec);

extern int RTPSession_send(void* vp, int timpstamp,void *pBuffer,
			int length);
extern int RTPSession_Audio_send(void* vp, int timpstamp,void *pBuffer,
			int length);
#ifdef __cplusplus
}
#endif
#endif /* End of __PANA_IPC_BASE_RTP_H__ */

