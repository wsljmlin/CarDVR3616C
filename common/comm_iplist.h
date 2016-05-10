#ifndef __COMM_IP_LIST_H__
#define __COMM_IP_LIST_H__

#include "comm_common.h"


#ifdef __cplusplus
extern "C"{
#endif


typedef struct _sCOMM_PORT_NODE {
	/* payload, the channal of the specific IP address */
	LONG lCh;
	/* payload, the port of the specific IP address */
	LONG lPort;
	/* point to the next node */
	struct _sCOMM_PORT_NODE *psNext;
} sCOMM_PORT_NODE, *psCOMM_PORT_NODE;

typedef struct _sCOMM_IP_NODE {
	/* payload, the pcIP address of the run-time rtp connect list */
	CHAR pcIP[COMM_IP_ADDRESS_LENGTH];
	/* point to the next IP node */
	struct _sCOMM_IP_NODE *psNext;
	/* point to the head of the port list */
	struct _sCOMM_PORT_NODE *psHead;
	/* point to the tail of the port list */
	struct _sCOMM_PORT_NODE *psTail;
} sCOMM_IP_NODE, *psCOMM_IP_NODE;


typedef struct {
	/*point to the head of the IP list*/
	struct _sCOMM_IP_NODE *psHead;
	/* point to the tail of the IP list */
	struct _sCOMM_IP_NODE *psTail;
	pthread_mutex_t lockIPList;
} sCOMM_IP_LIST, *psCOMM_IP_LIST;

typedef struct {
	LONG lCh;
	LONG lPort;
	CHAR pcIP[COMM_IP_ADDRESS_LENGTH];
} sCOMM_IPLIST_ARRAY;

typedef enum {
	COMM_IPLIST_SUCCESS,
	COMM_IPLIST_NOIP,
	COMM_IPLIST_NOPORT,
	COMM_IPLIST_LISTNULL,
} eCOMM_IPLIST_RET;

/*------------------------------------------------------------------------------*/
/*!
	@brief	Init pcIP list,init head and tail point NULL
    
	@param	psCOMM_IP_LIST pIPList		[in]	Point to the run-time rtp list 


	@retval	void
*/
/* -----------------------------------------------------------------------------*/

void vCOMM_IPList_Init(psCOMM_IP_LIST pIPList);

/*------------------------------------------------------------------------------*/
/*!
	@brief	Add the node to the list of the run-time rtp
    
	@param	CHAR* pcIP					[in]	The client IP
			LONG lCh					[in]	The video channel
			LONG lPort					[in]	The client lPort
			psCOMM_IP_LIST pIPList		[out]	Point to the run-time rtp list


	@retval	LONG							'0' indicate add pcIP, lCh and lPort to pcIP list success
											'-1'indicate add pcIP, lCh and lPort to pcIP list error
*/
/* -----------------------------------------------------------------------------*/

LONG lCOMM_IPList_Add(psCOMM_IP_LIST pIPList, CHAR* pcIP, LONG lCh, LONG lPort);

/*------------------------------------------------------------------------------*/
/*!
	@brief	Delete the node of pcIP and lPort from the iplist
    
	@param	CHAR* pcIP					[in]	The client IP
			LONG lPort					[in]	The client lPort
			psCOMM_IP_LIST pIPList			[out]	Point to the run-time rtp list


	@retval	LONG						'0' indicate delete node success
										'-1'indicate delete node error
*/
/* -----------------------------------------------------------------------------*/

LONG lCOMM_IPList_Del(psCOMM_IP_LIST pIPList, CHAR* pcIP, LONG lPort);

/*------------------------------------------------------------------------------*/
/*!
	@brief	Find the specified node payload of the list of the run-time rtp
    
	@param	CHAR* pcIP					[in]	The client IP
			LONG lPort					[in]	The client lPort
			psCOMM_IP_LIST pIPList		[out]	The IP list pointer

	@retval	LONG								'0' indicate find the IP and Port
												'-1' indicate not find the IP and Port
*/
/* -----------------------------------------------------------------------------*/

LONG lCOMM_IPList_Find(psCOMM_IP_LIST pIPList, CHAR* pcIP, LONG lPort);

/*------------------------------------------------------------------------------*/
/*!
	@brief	Find the specified pcIP payload of the list of the run-time rtp
    
	@param	psCOMM_IP_NODE psHead		[in]	Point to the head of the IP node
			CHAR* pcIP					[in]	The client IP

	@retval	psCOMM_IP_NODE						The IP node for the specific IP
				  								NULL for not find
*/
/* -----------------------------------------------------------------------------*/

psCOMM_IP_NODE psCOMM_IPList_FindIP(psCOMM_IP_NODE psHead, CHAR* pcIP);

/*------------------------------------------------------------------------------*/
/*!
	@brief	Find the specified pcIP payload of the list of the run-time rtp
    
	@param	psCOMM_IP_NODE psHead		[in]	Point to the head of the IP node
			LONG lPort					[in]	The client Port

	@retval	LONG								1 indicate find the Port node
				  								0 indicate not find
*/
/* -----------------------------------------------------------------------------*/

LONG lCOMM_IPList_FindPort(psCOMM_PORT_NODE psHead, LONG lPort);

/*------------------------------------------------------------------------------*/
/*!
	@brief	change the channel of iplist
    
	@param	CHAR* pcIP						[in]	The client IP
				LONG lCh							[in]	The video channel
				LONG lPort						[in]	The client lPort
				psCOMM_IP_LIST pIPList		[out]	Point to the run-time rtp list
	@retval	eCOMM_IPLIST_RET			COMM_IPLIST_SUCCESS	success
													COMM_IPLIST_NOIP		no ip address
													COMM_IPLIST_NOPORT	no port
*/
/* -----------------------------------------------------------------------------*/
eCOMM_IPLIST_RET eCOMM_IPList_ChangeCH(psCOMM_IP_LIST pIPList, CHAR* pcIP, LONG lPort, LONG lCh,LONG lChTre);

/*------------------------------------------------------------------------------*/
/*!
	@brief	change the channel of iplist
    
	@param	sCOMM_IPLIST_ARRAY* psArray	[out]	the output array
				LONG* plLen							[out]	the output array length
				psCOMM_IP_LIST pIPList			[in]	Point to the run-time rtp list
	@retval	eCOMM_IPLIST_RET				COMM_IPLIST_SUCCESS	success
														COMM_IPLIST_NOIP		no ip address
														COMM_IPLIST_NOPORT	no port
*/
/* -----------------------------------------------------------------------------*/
eCOMM_IPLIST_RET eCOMM_IPList_ToArray(psCOMM_IP_LIST pIPList, sCOMM_IPLIST_ARRAY* psArray, LONG* plLen);

/*------------------------------------------------------------------------------*/
/*!
	@brief	change the channel of iplist
    
	@param	sCOMM_IPLIST_ARRAY* psArray	[out]	the output array
				LONG* lLen							[out]	the output array length
				psCOMM_IP_LIST pIPList			[in]		Point to the run-time rtp list
				LONG									[in]		channel to be changed to
	@retval	eCOMM_IPLIST_RET				COMM_IPLIST_SUCCESS	success
														COMM_IPLIST_NOIP		no ip address
														COMM_IPLIST_NOPORT	no port
*/
/* -----------------------------------------------------------------------------*/
eCOMM_IPLIST_RET eCOMM_IPList_ChangeCH_ByArray(psCOMM_IP_LIST pIPList, sCOMM_IPLIST_ARRAY* psArray, LONG lArrayLen, LONG lCh,LONG lChTre);

/*------------------------------------------------------------------------------*/
/*!
	@brief	Deinit the pcIP list
    
	@param	psCOMM_IP_LIST pIPList		[out]	Point to the run-time rtp list

	@retval	void		
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_IPList_DeInit(psCOMM_IP_LIST pIPList);

/*------------------------------------------------------------------------------*/
/*!
	@brief	Show the whole pcIP list
    
	@param	psCOMM_IP_LIST pIPList		[in]		Point to the run-time rtp list
	@param	CHAR* pcInfo						[out]	the ip list information	
	@retval	void
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_IPList_GetInfo(psCOMM_IP_LIST pIPList, CHAR* pcInfo, LONG* plLen);


#ifdef SHOW_INFO
/*------------------------------------------------------------------------------*/
/*!
	@brief	Show the whole pcIP list
    
	@param	psCOMM_IP_LIST pIPList		[out]	Point to the run-time rtp list

	@retval	void
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_IPList_Tranversal(psCOMM_IP_LIST pIPList);
#endif /* SHOW_INFO */



#ifdef __cplusplus
}
#endif

#endif





