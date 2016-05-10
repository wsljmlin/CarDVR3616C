#ifdef __cplusplus
extern "C"{
#endif
#include "comm_iplist.h"

/********************************************************
	define / macro
********************************************************/
// #define COMM_IPLIST_DEBUG
#ifdef COMM_IPLIST_DEBUG
#define CI_Debug(fmt, arg...) fprintf(stdout, "[ IP List ] %s() <%d> "fmt, __func__, __LINE__, ##arg)
#else
#define CI_Debug(fmt, arg...)
#endif

#define CI_FuncIn() CI_Debug("in\n")
#define CI_FuncOut() CI_Debug("out\n")
#define CI_iVal(ival) CI_Debug("%s <%d> @ %p\n", #ival, ival, &ival)
#define CI_lVal(lval) CI_Debug("%s <%ld> @ %p\n", #lval, lval, &lval)
#define CI_PtVal(pval) CI_Debug("%s <%p>\n", #pval, pval)

/********************************************************
	function
********************************************************/
void vCOMM_IPList_Init(psCOMM_IP_LIST pIPList){

	CI_FuncIn();
	
	pIPList->psHead = NULL;
	pIPList->psTail = NULL;
	pthread_mutex_init(&(pIPList->lockIPList), NULL);

	CI_PtVal(pIPList->psHead);
	CI_PtVal(pIPList->psTail);
	
	CI_FuncOut();
}


LONG lCOMM_IPList_Add(psCOMM_IP_LIST pIPList, CHAR* pcIP, LONG lCh, LONG lPort){
	psCOMM_IP_NODE pIN;
	psCOMM_PORT_NODE pPN;

	/* check whether pcIP is exist */
	pthread_mutex_lock(&(pIPList->lockIPList));
	pIN = psCOMM_IPList_FindIP( pIPList->psHead, pcIP );

	if ( ! pIN ) {
		/* pcIP is not exist */
		/* 1. prepare pIN and pPN, add pPN to pIN */
		// malloc
		MALLOC( sizeof( sCOMM_IP_NODE ));
		MALLOC( sizeof( sCOMM_PORT_NODE ));
		pIN = ( psCOMM_IP_NODE ) malloc ( sizeof( sCOMM_IP_NODE ));
		pPN = ( psCOMM_PORT_NODE ) malloc ( sizeof( sCOMM_PORT_NODE ));

		// prepare pPN
		pPN->lPort= lPort;
		pPN->lCh= lCh;
		pPN->psNext = NULL;

		// prepare pIN
		strcpy(pIN->pcIP, pcIP);
		pIN->psHead = pPN;
		pIN->psTail = pPN;
		pIN->psNext = NULL;

		/* 2. add pIN to list */
		if ( pIPList->psHead ){
			/* not the first */
			pIPList->psTail->psNext = pIN;
			pIPList->psTail = pIN;
		} else {
			/* first element */
			pIPList->psHead = pIN;
			pIPList->psTail = pIN;
		}
		pthread_mutex_unlock(&(pIPList->lockIPList));
		return 0;
	}

	pthread_mutex_unlock(&(pIPList->lockIPList));

	/* check the lPort */
	if( lCOMM_IPList_FindPort( pIN->psHead, lPort )){
		/* pcIP and lPort already in use */
		CI_Debug("IP [ %s ], Port [ %ld ] is already in use !\n", pcIP, lPort);
		return -1;
	}

	/* pcIP already exist, just add the lPort to the tail */
	MALLOC( sizeof( sCOMM_PORT_NODE ));
	pPN = ( psCOMM_PORT_NODE ) malloc ( sizeof( sCOMM_PORT_NODE ));
	pPN->lPort= lPort;
	pPN->lCh= lCh;
	pPN->psNext = NULL;
	pIN->psTail->psNext = pPN;
	pIN->psTail = pPN;
	
	return 0;
}

LONG lCOMM_IPList_Del(psCOMM_IP_LIST pIPList, CHAR* pcIP, LONG lPort){
	psCOMM_IP_NODE pIN, pINTmp, pINOld;
	psCOMM_PORT_NODE pPN, pPNOld;

	CI_FuncIn();

	{

		/* check whether pcIP is exist */
		pthread_mutex_lock(&(pIPList->lockIPList));
		pIN = psCOMM_IPList_FindIP( pIPList->psHead, pcIP );
		if ( !pIN ){
			CI_Debug("pcIP %s is not use now!\n", pcIP);
			pthread_mutex_unlock(&(pIPList->lockIPList));
			return -1;
		}

		if( ! lCOMM_IPList_FindPort(pIN->psHead, lPort)){
			CI_Debug("IP %s is exist, but lPort %ld is not used!\n", pcIP, lPort);
			pthread_mutex_unlock(&(pIPList->lockIPList));
			return -1;
		}

		CI_Debug("now, we are certain both ip and port is exist\n");

		/* now, we can sure both the pcIP and lPort is exist */
		pPN = pIN->psHead;
		if( pPN->lPort== lPort ){
			CI_Debug("the port Node is in header of IP Node\n");
			/* 1.1 the node is head */
			pIN->psHead = pIN->psHead->psNext;
			free( pPN );
			if( ! pIN->psHead ){
				CI_Debug("the IP Node is empty after delete the port Node, Need to delete the IP Node\n");
				/* 1.2 need to delete pcIP node */
				pINTmp = pIPList->psHead;
				if( ! strcmp( pINTmp->pcIP, pcIP)){
					CI_Debug("the IP Node is header of ip list\n");
					/* 1.2.1 the node is head */
					pIPList->psHead = pIPList->psHead->psNext;
					free( pINTmp );

					/* update tail */
					if(pIPList->psHead==NULL){
						CI_Debug("the ip list is empty after delete the IP Node\n");
						pIPList->psTail=NULL;
					} else {
						CI_Debug("the ip is not empty after delete the IP Node\n");
					}
					pthread_mutex_unlock(&(pIPList->lockIPList));
					return 1;
				}
				CI_Debug("the IP Node is not header of ip list\n");
				while( strcmp( pINTmp->psNext->pcIP, pcIP )){
					pINTmp = pINTmp->psNext;
				}
				/* 1.2.2 delete the pcIP node*/
				pINOld = pINTmp->psNext;
				pINTmp->psNext = pINTmp->psNext->psNext;
				free( pINOld );
				if(!pINTmp->psNext){
					CI_Debug("the IP Node is tail of ip list\n");
					/* indicate last one, should update psTail ! */
					pIPList->psTail=pINTmp;
				} else {
					CI_Debug("the IP Node is not tail of ip list\n");
				}
			}
			pthread_mutex_unlock(&(pIPList->lockIPList));
			return 0;
		}

		pthread_mutex_unlock(&(pIPList->lockIPList));

		/* 2.1 find the lPort node before the specific lPort */
		CI_Debug("the port is not header of the IP Node\n");
		while( pPN->psNext->lPort!= lPort ){
			pPN = pPN->psNext;
		}
		/* delete the lPort node*/
		pPNOld = pPN->psNext;
		pPN->psNext = pPN->psNext->psNext;
		free( pPNOld );
		/* update tail pt */
		if( pPN->psNext == NULL ){
			CI_Debug("the port is the tail of the IP Node\n");
			/* indicate delete tail */
			pIN->psTail=pPN;
		} else {
			CI_Debug("the port is not the tail of the IP Node\n");
		}

	}
	
	CI_FuncOut();
	
	return 0;
}


LONG lCOMM_IPList_GetCh(psCOMM_PORT_NODE psHead, LONG lPort){
	LONG ret = -1;
	psCOMM_PORT_NODE pfind;

	/* find lPort */
	pfind = psHead;
	while( pfind ) {
		if ( pfind->lPort== lPort ){
			ret = pfind->lCh;
			break;
		}
		pfind = pfind->psNext;
	}

	return ret;
}


LONG lCOMM_IPList_Find(psCOMM_IP_LIST pIPList, CHAR* pcIP, LONG lPort){
	LONG ret = -1;
	psCOMM_IP_NODE pfind;

	/* first, find the specific pcIP */
	pthread_mutex_lock(&(pIPList->lockIPList));
	pfind = psCOMM_IPList_FindIP( pIPList->psHead, pcIP );
	if ( ! pfind ){
		return 0;
	}
	pthread_mutex_unlock(&(pIPList->lockIPList));

	/* second, find the lPort */
	if( lCOMM_IPList_FindPort( pfind->psHead, lPort ) ){
		ret = lCOMM_IPList_GetCh( pfind->psHead, lPort );
	} 
	
	return ret;
}

psCOMM_IP_NODE psCOMM_IPList_FindIP(psCOMM_IP_NODE psHead, CHAR* pcIP){
	psCOMM_IP_NODE pfind;

	/* find pcIP */
	pfind = psHead;
	while( pfind ) {
		if ( ! strcmp( pfind->pcIP, pcIP ) ){
			break;
		}
		pfind = pfind->psNext;
	}
	return pfind;
}


LONG lCOMM_IPList_FindPort(psCOMM_PORT_NODE psHead, LONG lPort){
	LONG ret = 0;
	psCOMM_PORT_NODE pfind;

	/* find lPort */
	pfind = psHead;
	while( pfind ) {
		if ( pfind->lPort== lPort ){
			ret = 1;
			break;
		}
		pfind = pfind->psNext;
	}

	return ret;
}


void vCOMM_IPList_DeInit(psCOMM_IP_LIST pIPList){
	psCOMM_IP_NODE pIN, pINOld;
	psCOMM_PORT_NODE pPN, pPNOld;
	pthread_mutex_lock(&(pIPList->lockIPList));
	pIN = pIPList->psHead;
	while( pIN ){
		/* free the pcIP node */
		pPN = pIN->psHead;
		while( pPN ){
			/* free the lPort node */
			pPNOld = pPN;
			pPN = pPN->psNext;
			free( pPNOld );
		}
		pIN->psHead = NULL;
		pIN->psTail = NULL;

		pINOld = pIN;
		pIN = pIN->psNext;
		free( pINOld );
	}

	pIPList->psHead = NULL;
	pIPList->psTail = NULL;
	pthread_mutex_unlock(&(pIPList->lockIPList));

	CI_Debug("[ Deinit ] : list done\n");
	
	pthread_mutex_destroy(&(pIPList->lockIPList));
}

void vCOMM_IPList_GetInfo(psCOMM_IP_LIST pIPList, CHAR* pcInfo, LONG* plLen){
	psCOMM_IP_NODE pIN;
	psCOMM_PORT_NODE pPN;
	CHAR* pcTmp=pcInfo;

	pthread_mutex_lock(&(pIPList->lockIPList));
	pIN = pIPList->psHead;
	pthread_mutex_unlock(&(pIPList->lockIPList));

	if( !pIN ) {
		pcTmp+=sprintf(pcTmp, "The List is empty now!\n");
		*plLen=(pcTmp-pcInfo)+1;
		return;
	}
	
	while( pIN ){
		/* tranversal the pcIP node */
		pcTmp+=sprintf(pcTmp, "[ IP ] : %s\n", pIN->pcIP);
		pPN = pIN->psHead;
		while( pPN ){
			/* tranversal the lPort node */
			pcTmp+=sprintf(pcTmp, "\tch : %ld\tport : %ld\n", pPN->lCh, pPN->lPort);
			pPN = pPN->psNext;
		}
		pcTmp+=sprintf(pcTmp, "\n");
		pIN = pIN->psNext;
	}

	/* return the length, include '\0' */
	*plLen=(pcTmp-pcInfo)+1;
	
	return;
}

#ifdef SHOW_INFO

void vCOMM_IPList_Tranversal(psCOMM_IP_LIST pIPList){
	psCOMM_IP_NODE pIN;
	psCOMM_PORT_NODE pPN;

	pthread_mutex_lock(&(pIPList->lockIPList));
	pIN = pIPList->psHead;
	pthread_mutex_unlock(&(pIPList->lockIPList));

	if( !pIN ) {
		CI_Debug("The List is empty now!\n");
		return;
	}
	
	while( pIN ){
		/* tranversal the pcIP node */
		CI_Debug("[ IP ] : %s\n", pIN->pcIP);
		pPN = pIN->psHead;
		while( pPN ){
			/* tranversal the lPort node */
			CI_Debug("\tch : %ld\tport : %ld\n", pPN->lCh, pPN->lPort);
			pPN = pPN->psNext;
		}
		CI_Debug("\n");
		pIN = pIN->psNext;
	}

	CI_Debug("done\n\n");
	
	return;
}

#endif /* SHOW_INFO */

eCOMM_IPLIST_RET eCOMM_IPList_ChangeCH(psCOMM_IP_LIST pIPList, CHAR* pcIP, LONG lPort, LONG lCh,LONG lChTre){
	eCOMM_IPLIST_RET eRet=COMM_IPLIST_NOIP;
	sCOMM_IP_NODE* psIpNode=pIPList->psHead;
	sCOMM_PORT_NODE* psPortNode=NULL;

	/* judge whether null list */
	if( !psIpNode ) {
		return COMM_IPLIST_LISTNULL;
	}
	
	/* traverse the iplist */
	while( psIpNode->psNext != NULL ){
		/* compare the target ip address */
		if(COMM_STRING_EQUAL != strcmp(psIpNode->pcIP, pcIP)){
			/* go to the next IpNode */
			psIpNode=psIpNode->psNext;
			continue;
		} else {
			/* find one, break */
			break;
		}
	}

	/* judge if find the ip */
	if( COMM_STRING_EQUAL == strcmp(psIpNode->pcIP, pcIP)){
		/* traverse the port list */
		psPortNode=psIpNode->psHead;
		eRet=COMM_IPLIST_NOPORT;
		while( psPortNode->psNext != NULL ){
			/* compare the port */
			if( psPortNode->lPort != lPort ) {
				/* go to the next port */
				psPortNode=psPortNode->psNext;
				continue;
			} else {
				/* find the target, break */
				break;
			}
		}
	}

	/* judge if find the port */
	if( psPortNode && psPortNode->lPort== lPort ){
		if(psPortNode->lCh <= lChTre){
			psPortNode->lCh=lCh;
		}
		eRet=COMM_IPLIST_SUCCESS;
	}

	return eRet;
}

eCOMM_IPLIST_RET eCOMM_IPList_ToArray(psCOMM_IP_LIST pIPList, sCOMM_IPLIST_ARRAY* psArray, LONG* plLen){
	eCOMM_IPLIST_RET eRet=COMM_IPLIST_SUCCESS;
	sCOMM_IP_NODE* psIpNode=pIPList->psHead;
	sCOMM_PORT_NODE* psPortNode=NULL;
	LONG lLen=0;

	/* check the valid of ip list */
	if( !psIpNode ){
		*plLen = lLen;
		return COMM_IPLIST_LISTNULL;
	}
	
	/* traverse the iplist */
	do {
		/* get port list */
		psPortNode=psIpNode->psHead;

		/* traverse the port list */
		while( psPortNode ) {
			/* get port node, turn to array */
			memset(&psArray[lLen], 0, sizeof(psArray[lLen]));
			strcpy(psArray[lLen].pcIP, psIpNode->pcIP);
			psArray[lLen].lPort=psPortNode->lPort;
			psArray[lLen].lCh=psPortNode->lCh;
			lLen++;

			/* update port node */
			psPortNode=psPortNode->psNext;
		}

		/* update ip node */
		psIpNode=psIpNode->psNext;
	} while( psIpNode );

	/* output the length */
	*plLen=lLen;

	return eRet;
}

eCOMM_IPLIST_RET eCOMM_IPList_ChangeCH_ByArray(psCOMM_IP_LIST pIPList, sCOMM_IPLIST_ARRAY* psArray, LONG lArrayLen, LONG lCh,LONG lChTre){
	eCOMM_IPLIST_RET eRet=COMM_IPLIST_SUCCESS;
	sCOMM_IPLIST_ARRAY* psArrayList=NULL;
	LONG lLen=0;

	/* change the channel through the list */
	for(lLen=0;lLen!=lArrayLen;lLen++){
		psArrayList=&psArray[lLen];
		eRet=eCOMM_IPList_ChangeCH(pIPList, psArrayList->pcIP, psArrayList->lPort, lCh,lChTre);
		if( eRet != COMM_IPLIST_SUCCESS ){
			CI_Debug("error in change channel by array\n");
			return eRet;
		}
		if(psArrayList->lCh <= lChTre)
		{
			psArrayList->lCh=lCh;
		}
	}

	return eRet;
}


#ifdef __cplusplus
}
#endif


