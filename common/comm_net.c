#ifdef __cplusplus
extern "C"{
#endif

#include "comm_net.h"
#include <netinet/tcp.h>
#include "comm_command.h"

/* socket write lock */
static pthread_mutex_t gLockNetWt;

typedef enum {
	COMM_NET_CONNECTED,
	COMM_NET_DISCONNECTED,
} eCOMM_NET_CONNECTTYPE;

eCOMM_NET_CONNECTTYPE eCOMM_NET_ISCONNECT(LONG lAcceptFd);

void vCOMM_Net_Init(sCOMM_NET* psNet, LONG lPort, LONG lType, LONG lMaxCnt){


	/* socket address of server and client */
	struct sockaddr_in sSrvAddr;
	LONG lOn=1; 

	/* init server address */
	memset(&sSrvAddr, 0,sizeof(sSrvAddr));

	/* socket write lock init */
	pthread_mutex_init(&gLockNetWt, NULL);

	/* create socket */
	if((psNet->lSockFd = socket(AF_INET,SOCK_STREAM,0))==-1) {
		printf("socket error\n");
		exit(-1);
	}

	/* init psNet */
	psNet->lServerPort=lPort;
	psNet->lType=lType;

	if(COMM_NET_SERVER==lType){

		/* to avoid rebind error */
		if((setsockopt(psNet->lSockFd,SOL_SOCKET,SO_REUSEADDR,&lOn,sizeof(lOn)))<0) {  
			printf("setsockopt failed\n");  
			exit(-1);  
		} 

		/* prepare for server address */
		sSrvAddr.sin_family=AF_INET; 				
		sSrvAddr.sin_port=htons( lPort );

		/* bind */
		if(bind(psNet->lSockFd, (struct sockaddr *)&sSrvAddr, sizeof(struct sockaddr))==-1) {
			printf("bind error\n");
			exit(-1);
		}

		/* listen */
		if(listen(psNet->lSockFd, lMaxCnt)==-1) {
			printf("listen error\n");
			exit(-1);
		}
	} 
}


void vCOMM_Net_Connect(sCOMM_NET* psNet, CHAR* pcIp){

	/* server address */
	struct sockaddr_in sServerAddr;

	if(COMM_NET_CLIENT==psNet->lType){
		/* prepare for server address */
		memset(&sServerAddr,0,sizeof(sServerAddr));
		sServerAddr.sin_family=AF_INET;				
		sServerAddr.sin_port=htons( psNet->lServerPort );
		sServerAddr.sin_addr.s_addr = inet_addr(pcIp);
		connect (psNet->lSockFd, (struct sockaddr *)&sServerAddr, sizeof(struct sockaddr));
	} else {
		printf("warning server need not connect!\n");
	}
}

LONG lCOMM_Net_Read(LONG lAcceptFd, CHAR*  pcBuf, LONG lLen){

	LONG lRet=COMM_NET_SUCCESS;
	LONG lRecieveCnt;
	LONG i;
	CHAR *pcLastCmd = NULL;
	CHAR cCmdBufAlias[COMM_CMDSPACKETMAXCHAR];
	
	// clear socket msg buffer
	memset(cCmdBufAlias, 0, COMM_CMDSPACKETMAXCHAR);
	
	/* check the valid of the socket */
	if(eCOMM_NET_ISCONNECT(lAcceptFd) != COMM_NET_CONNECTED) {
		printf("[lCOMM_Net_Read] ERROR when check the valid of the socket \n");
		return COMM_NET_SOCKETINVALID;
	}
	
	/* try to recieve from the accept fd */
	lRecieveCnt = recv(lAcceptFd, cCmdBufAlias, lLen, 0);
	if(lRecieveCnt<=0){
		printf("[lCOMM_Net_Read] ERROR when try to recieve from the accept fd: %s  \n", pcBuf);
		return COMM_NET_FAILURE;
	}
	// if socket msg includes many commands, only return the last one command
	for( i = lRecieveCnt-2; i >=0; i--){
		if( '&' == cCmdBufAlias[i])
			break;
	}
	pcLastCmd = cCmdBufAlias + i - 1;
	// return the last one command
	strcpy(pcBuf, pcLastCmd);
	
	return lRet;
}

LONG vCOMM_Net_Write(LONG lAcceptFd, CHAR* pcBuf, LONG lLen){

	LONG lRet=COMM_NET_SUCCESS;
	LONG lSendCnt;
	pthread_mutex_lock(&gLockNetWt);

	/* check the valid of the socket */
	if(eCOMM_NET_ISCONNECT(lAcceptFd) != COMM_NET_CONNECTED) {
		pthread_mutex_unlock(&gLockNetWt);
		return COMM_NET_SOCKETINVALID;
	}
	
	/* send to the accept fd */
	lSendCnt = send(lAcceptFd, pcBuf, lLen, 0);
	if(lSendCnt<=0){
		lRet=COMM_NET_FAILURE;
	}
	pthread_mutex_unlock(&gLockNetWt);

	return lRet;
}

void vCOMM_Net_DeInit(sCOMM_NET* psNet){

	/* close the socket fd */
	close(psNet->lSockFd);

	/* socket write lock init */
	pthread_mutex_destroy(&gLockNetWt);
}

eCOMM_NET_CONNECTTYPE eCOMM_NET_ISCONNECT(LONG lAcceptFd) {
	eCOMM_NET_CONNECTTYPE eNetType = COMM_NET_CONNECTED;
	struct tcp_info sTcpInfo;
	LONG lLen = sizeof(sTcpInfo);
	LONG lSocket = lAcceptFd;

	/* check the socket first */
	if(lSocket < 0) {
		return COMM_NET_DISCONNECTED;
	}

	/* prepare for the sTcpInfo */
	memset(&sTcpInfo, 0, sizeof(sTcpInfo));
	getsockopt(lSocket, IPPROTO_TCP, TCP_INFO, &sTcpInfo, (socklen_t*)&lLen);

	if(sTcpInfo.tcpi_state != 1) {
		return COMM_NET_DISCONNECTED;
	}

	return eNetType;
}



#ifdef __cplusplus
}
#endif


