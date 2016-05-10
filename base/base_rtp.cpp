/**********************************************************************
Copyright (C) 2013 CASKY eTech Co., Ltd .
------------------------------------------------------------
Project    : IPC
************************************************************************
   File Name	: ipc_rtp.cpp
   Module Name	: ipc rtp common function
   Description	: 
   History:
  No.		Ver.		Date		Designed by		Comment
-----+----------+-----------------+--------------+-----------------
    1      0.0.1      2013.06.03	CASKY			new create
    2      0.0.2      2013.07.19	CASKY			new created

************************************************************************/


#include <semaphore.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>

#ifndef WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif // WIN32
#include <iostream>
#include <memory.h>

#include "rtpdefines.h"
#include "rtpsession.h"
#include "rtppacket.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"
#include "rtpsourcedata.h"

#include "base_rtp.h"

#include "comm_interface.h"



#define error_check(fmt,args...) \
	fprintf(stderr, "%s:%d,%s: "fmt, \
	__FILE__, __LINE__, __func__, ##args)

#define MAX_SESS_BND		20000000.0

#define RTP_PACKET_SIZE \
	(RTP_DEFAULTPACKETSIZE-12)
/*
#define IPC_PRT(fmt...)   \
    do {\
        printf("[%s]-%d: ", __FUNCTION__, __LINE__);\
        printf(fmt);\
       }while(0)
*/
#define IPC_PRT(fmt...)

#define FNRI_TYPE		0x1C
#define SER_START		0x80
#define SER_MIDLLE		0x00
#define SER_END			0x40
#define TYPE_MSK		0x1F
#define FNRI_SER_MSK		0xE0
#define NIDR_TYPE		0x01
#define IDR_TYPE		0x05

typedef struct _RtpPack{
	unsigned char fui;
	unsigned char fuh;
	unsigned char data[RTP_PACKET_SIZE-2];
}RtpPack;

/***************************************************************************
  Function	: RTPSession_delport( )
  Description: to delete the port from specific session
  INput		: void* vp						: point to the rtp session
			  char* remote_ip4addr			: the remote ip address
			  unsigned short int destport	: the remote destinate port
  OUTput	: int							: 0 for success, -1 for failure
  Notation	:

 History:
  No.	Date			Designed by		Comment
-----+---------------+--------------+---------------
  1		2013.07.19		CASKY			new created

***************************************************************************/
int RTPSession_delport(void* vp, char *remote_ip4addr,
			unsigned short int destport)
{
	int lRet = 0;
	unsigned int destip;
	destip = inet_addr(remote_ip4addr);

	if( (destip == INADDR_NONE) ){
		error_check("Bad IP address specified\n");
		/* invalid address, return -1 */
		lRet = -1;
		return lRet;
	}
	
	/* malloc RTPSession */
	RTPSession* psess;
	psess = ( RTPSession* ) vp;
	destip = ntohl(destip);

	/* Add remote address and port*/
	lRet = psess -> DeleteDestination(RTPIPv4Address(destip,  destport));
	if( (lRet < 0) ){
		error_check("add remote address and port failed %d\n",lRet);
	} else {
		IPC_PRT("[ Del ] : ip, %s port, %d\n", remote_ip4addr, destport);
	}

	return lRet;
}


/***************************************************************************
  Function		: RTPSession_addport( )
  Description	: to add the port from specific session
  INput			: void* vp						: point to the rtp session
				  char* remote_ip4addr			: the remote ip address
				  unsigned short int destport	: the remote destinate port
  OUTput		: int							: 0 for success, -1 for failure
  Notation	:

 History:
  No.	Date			Designed by		Comment
-----+---------------+--------------+---------------
  1		2013.07.19		CASKY			new created

***************************************************************************/
int RTPSession_addport(void* vp, char *remote_ip4addr,
			unsigned short int destport)
{
	int lRet = 0;
	unsigned int destip;
	destip = inet_addr(remote_ip4addr);
	
	if( (destip == INADDR_NONE) ){
		error_check("Bad IP address specified\n");
		/* invalid address, return -1 */
		lRet = -1;
		return lRet;
	}
	
	/* malloc RTPSession */
	RTPSession* psess;
	psess = ( RTPSession* ) vp;
	destip = ntohl(destip);

	/* Add remote address and port*/
	lRet = psess -> AddDestination(RTPIPv4Address(destip,  destport));
	if( (lRet < 0) ){
		error_check("add remote address and port failed %d\n",lRet);
	} else {
		debug_info("[ Add ] : ip, %s port, %d\n", remote_ip4addr, destport);
	}

	return lRet;
}

/***************************************************************************
  Function		: RTPSession_serverinit( )
  Description	: init the server session of rtp
  INput			: void** vp						: point to the point of rtp session
				  unsigned short int portbase	: the remote destinate port
				  double dHz					: the Hz of the session
  OUTput		: int							: 0 for success, -1 for failure
  Notation		:

 History:
  No.	Date			Designed by   Comment
-----+---------------+--------------+---------------
  1		2013.07.19		CASKY			new created

***************************************************************************/
int RTPSession_serverinit(void** vp,
			unsigned short int portbase,
			double dHz)
{
	int lRet = 0;
	bool acceptflg = true;

	/* initial the return value */
	*vp = NULL;
	
	RTPSessionParams sessparams;
	RTPUDPv4TransmissionParams transparams;

	/* malloc RTPSession */
	RTPSession* psess;
	psess = new RTPSession;

	/* Setting time stamp unit */
	sessparams.SetOwnTimestampUnit(1.0/dHz);
	
	/* Setting accept flag */
	sessparams.SetAcceptOwnPackets(acceptflg);
	/* Setting rtp band width*/
	//sessparams.SetSessionBandwidth(MAX_SESS_BND);
	
	/* Setting local port */
	transparams.SetPortbase(portbase);
	//IPC_PRT("Begin to careat\n");  
	/* Create session */
	lRet = psess -> Create(sessparams, &transparams);
	//IPC_PRT("Creat finish\n"); 
	if( (lRet < 0) ){
		error_check("create rtp session failed %d\n",lRet);
		delete psess;
	}else{
		/* create ok */
		*vp = ( void* ) psess;
	}
	return lRet;
}

/***************************************************************************
  Function		: RTPSession_clientinit( )
  Description	: init the client session of rtp
  INput			: void** vp						: point to the point of rtp session
				  unsigned short int destport	: the remote destinate port
				  double dHz					: the Hz of the session
  OUTput		: int							: 0 for success, -1 for failure
  Notation	:

 History:
  No.	Date			Designed by   Comment
-----+---------------+--------------+---------------
  1		2013.07.19		CASKY			new created

***************************************************************************/
int RTPSession_clientinit(void** vp, unsigned short int destport, double dHz)
{
	int lRet = 0;
	RTPSessionParams sessparams;
	RTPUDPv4TransmissionParams transparams;

	/* initial the vp */
	*vp = NULL;
	
	/* Setting time stamp unit */
	sessparams.SetOwnTimestampUnit(1.0/dHz);
	lRet = sessparams.SetUsePollThread(true);
	if( (lRet < 0) ){
		error_check("set rtp poll thread failed, %d\n", lRet);
	}else{	
		/* Construct RTPSession */
		RTPSession* psess;
		psess = new RTPSession;
		
		sessparams.SetSessionBandwidth(MAX_SESS_BND);			
		/* Setting local port */
		transparams.SetPortbase(destport);
		
		/* Setting local port */
		//lRet = psess->Create(sessparams, &transparams);
		lRet = psess -> Create(sessparams, &transparams);
		if( (lRet < 0) ){
			error_check("create rtp session failed %d\n",lRet);
			delete psess;
		} else {
			*vp = (void*) psess;
		}
	}
	return lRet;
}

/***************************************************************************
  Function		: RTPSession_setplayloadtype( )
  Description	: set the play load type of the specific session
  INput			: void* vp				: point to the rtp session
				  int playloadtype		: the play load type
  OUTput		: int					: 0 for success, -1 for failure
  Notation	:

 History:
  No.	Date			Designed by   Comment
-----+---------------+--------------+---------------
  1		2013.07.19		CASKY			new created

***************************************************************************/
int RTPSession_setplayloadtype(void* vp, int playloadtype)
{
	int lRet = 0;
	/* initial RTPSession */
	RTPSession* psess;
	psess = ( RTPSession* ) vp;
	debug_info("playloadtype p %p\n", vp);
	/* Setting playload type */
	//int lRet = psess->SetDefaultPayloadType(playloadtype);
	lRet = psess -> SetDefaultPayloadType(playloadtype);
	if( (lRet < 0) ){
		error_check("set playload type failed,%d\n",lRet);
	}
	return lRet;
}

/***************************************************************************
  Function		: RTPSession_destroy( )
  Description	: destory the specific session
  INput			: void* vp		: point to the rtp session
				  int sec		: to construct rtptime
				  int usec		: to construct rtptime
  OUTput		: void
  Notation		:

  History:
  No.	Date			Designed by   Comment
-----+---------------+--------------+---------------
  1		2013.07.19		CASKY			new created

***************************************************************************/
void RTPSession_destroy(void* vp, unsigned int sec, unsigned int usec)
{
	RTPSession* psess;
	psess = ( RTPSession* ) vp;
	
	//psess->BYEDestroy(time, 0, 0);
	psess -> BYEDestroy(RTPTime(sec, usec), 0, 0);
	delete psess;
}

/***************************************************************************
  Function		: RTPSession_send( )
  Description	: send rtp packet of the specific session
  INput			: void* vp			: point to the rtp session
				  int timestamp		: time stamp
				  void *pBuffer		: the head of the memery to be send
				  int length		: the length of the memery to be send
  OUTput		: int				: -1 for failure
  Notation	:

 History:
  No.	Date			Designed by   Comment
-----+---------------+--------------+---------------
  1		2013.07.19		CASKY			new created

***************************************************************************/
int RTPSession_send(void* vp, int timpstamp,void *pBuffer,int length)
{
	long lRet;
	bool mark = false;
	unsigned char *pucData = 
		(unsigned char *)pBuffer;

	/* initial RTPSession */
	RTPSession* psess;
	psess = ( RTPSession* ) vp;

	//debug_info("length=%d\n",length);
	/* Remove sync word */
	while( (*pucData++ == 0x00) ){
		length--;
	};
	length--;
	//debug_info("length=%d\n",length);
	
	/* Set playload type */
	//lRet = psess->SetDefaultMark(mark);
	lRet = psess -> SetDefaultMark(mark);

	if( (lRet < 0) ){
		error_check("set default mark failed,%ld\n",lRet);
	}else{
		/* Seding packet */
		if( (length <= RTP_PACKET_SIZE) ){
			//lRet = psess->SetDefaultTimestampIncrement(timpstamp);
			lRet = psess -> SetDefaultTimestampIncrement(timpstamp);
			if( (lRet < 0) ){
				error_check("set time stamp failed,%ld\n", lRet);
			}else{				
				//lRet = psess->SendPacket(pucData,length);
				lRet = psess -> SendPacket(pucData,length);
				if( (lRet < 0) ){
					error_check("rtp send packet failed,%ld\n",lRet);
				}
			}
		}else{
			//lRet = psess->SetDefaultTimestampIncrement(0);
			lRet = psess -> SetDefaultTimestampIncrement(0);
			if( (lRet < 0) ){
				error_check("set time stamp failed,%ld\n", lRet);
			}else{
				RtpPack pakt;
				pakt.fui = ((pucData[0] & FNRI_SER_MSK) | FNRI_TYPE);
				pakt.fuh = ((pucData[0] & TYPE_MSK) | SER_START);
				memcpy(pakt.data,pucData+1,(RTP_PACKET_SIZE-2));		
			
				//lRet = psess->SendPacket((void*)&pakt,RTP_PACKET_SIZE);
				lRet = psess -> SendPacket((void*)&pakt,RTP_PACKET_SIZE);
				if( (lRet < 0) ){
					error_check("rtp send packet failed,%ld\n",lRet);
				}else{
					pucData += (RTP_PACKET_SIZE-1);
					length  -= (RTP_PACKET_SIZE-1);
					
					for(;length > (int)(RTP_PACKET_SIZE-2);)
					{
						pakt.fuh = (pakt.fuh & TYPE_MSK);
						memcpy(pakt.data,pucData,(RTP_PACKET_SIZE-2));
						//lRet = psess->SendPacket((void*)&pakt,RTP_PACKET_SIZE);
						lRet = psess -> SendPacket((void*)&pakt,RTP_PACKET_SIZE);
						if( (lRet < 0) ){
							error_check("rtp send packet failed,%ld\n",lRet);
							break;
						}
						pucData += (RTP_PACKET_SIZE-2);
						length  -= (RTP_PACKET_SIZE-2);
					}

					/* Send last package and update time stamp */
					if( lRet < 0 ){
					}else{
						//lRet = psess->SetDefaultTimestampIncrement(timpstamp);
						lRet = psess -> SetDefaultTimestampIncrement(timpstamp);
						if( (lRet < 0) ){
							error_check("set time stamp failed,%ld\n", lRet);
						}else{
							pakt.fuh = ( (pakt.fuh & TYPE_MSK) | SER_END);
							memcpy(pakt.data,pucData,length);
							//lRet = psess->SendPacket((void*)&pakt,length+2);
							lRet = psess -> SendPacket((void*)&pakt,length+2);
							//IPC_PRT("Send This\n");
							if( (lRet < 0) ){
								error_check("rtp send packet failed,%ld\n",lRet);
							}
						}
					}
				}
			}
		}
	}
	return lRet;
}

int RTPSession_Audio_send(void* vp, int timpstamp,void *pBuffer,int length)
{
	long lRet;
	bool mark = false;
	unsigned short *pucData = 
		(unsigned short *)pBuffer;

	/* initial RTPSession */
	RTPSession* psess;
	psess = ( RTPSession* ) vp;

	length=(pucData[1]&0x00ff)*sizeof(unsigned short)+4;

	//debug_info("length=%d\n",length);
	/* Remove sync word */
	//while( (*pucData++ == 0x00) ){
	//	length--;
//	};
//	length--;
	//debug_info("length=%d\n",length);

	/* Set playload type */
	//lRet = psess->SetDefaultMark(mark);
	lRet = psess -> SetDefaultMark(mark);

	if( (lRet < 0) ){
		error_check("set default mark failed,%ld\n",lRet);
	}else{
		/* Seding packet */
		if( (length <= RTP_PACKET_SIZE) ){
			//lRet = psess->SetDefaultTimestampIncrement(timpstamp);
			lRet = psess -> SetDefaultTimestampIncrement(timpstamp);
			if( (lRet < 0) ){
				error_check("set time stamp failed,%ld\n", lRet);
			}else{				
				//lRet = psess->SendPacket(pucData,length);
				lRet = psess -> SendPacket(pucData,length);
				if( (lRet < 0) ){
					error_check("rtp send packet failed,%ld\n",lRet);
				}
			}
		}else{
			//lRet = psess->SetDefaultTimestampIncrement(0);
			lRet = psess -> SetDefaultTimestampIncrement(0);
			if( (lRet < 0) ){
				error_check("set time stamp failed,%ld\n", lRet);
			}else{
				RtpPack pakt;
				pakt.fui = ((/*pucData[0] &*/ FNRI_SER_MSK) | FNRI_TYPE);
				pakt.fuh = ((/*pucData[0] & */TYPE_MSK) | SER_START);
				memcpy(pakt.data,pucData,(RTP_PACKET_SIZE-2));		

				//lRet = psess->SendPacket((void*)&pakt,RTP_PACKET_SIZE);
				lRet = psess -> SendPacket((void*)&pakt,RTP_PACKET_SIZE);
				if( (lRet < 0) ){
					error_check("rtp send packet failed,%ld\n",lRet);
				}else{
					pucData += (RTP_PACKET_SIZE-2);
					length  -= (RTP_PACKET_SIZE-2);

					for(;length > (int)(RTP_PACKET_SIZE-2);)
					{
						pakt.fuh = (pakt.fuh & TYPE_MSK);
						memcpy(pakt.data,pucData,(RTP_PACKET_SIZE-2));
						//lRet = psess->SendPacket((void*)&pakt,RTP_PACKET_SIZE);
						lRet = psess -> SendPacket((void*)&pakt,RTP_PACKET_SIZE);
						if( (lRet < 0) ){
							error_check("rtp send packet failed,%ld\n",lRet);
							break;
						}
						pucData += (RTP_PACKET_SIZE-2);
						length  -= (RTP_PACKET_SIZE-2);
					}

					/* Send last package and update time stamp */
					if( lRet < 0 ){
					}else{
						//lRet = psess->SetDefaultTimestampIncrement(timpstamp);
						lRet = psess -> SetDefaultTimestampIncrement(timpstamp);
						if( (lRet < 0) ){
							error_check("set time stamp failed,%ld\n", lRet);
						}else{
							pakt.fuh = ( (pakt.fuh & TYPE_MSK) | SER_END);
							memcpy(pakt.data,pucData,length);
							//lRet = psess->SendPacket((void*)&pakt,length+2);
							lRet = psess -> SendPacket((void*)&pakt,length+2);
							//IPC_PRT("Send This\n");
							if( (lRet < 0) ){
								error_check("rtp send packet failed,%ld\n",lRet);
							}
						}
					}
				}
			}
		}
	}
	return lRet;
}