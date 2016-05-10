#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "comm_list.h"

/***************************************************
	macro / enum
***************************************************/
//#define COMM_LIST_DEBUG
#ifdef COMM_LIST_DEBUG
#define CL_Debug(fmt, arg...) fprintf(stdout, "[ CL ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CL_Debug(fmt, arg...)
#endif

#define CL_FuncIn() //CL_Debug("in\n")
#define CL_FuncOut() //CL_Debug("out\n")
#define CL_iVal(iVal) CL_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CL_lVal(lVal) CL_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CL_PtVal(ptVal) CL_Debug("pt %s @ %p\n", #ptVal, ptVal)


/***************************************************
	variable
***************************************************/

/***************************************************
	prototype
***************************************************/
eCOMM_LIST_Ret eCOMM_LIST_Retrive_Node(sCOMM_LIST_Info* psInfo, void* pvKey, sCOMM_NODE_Info** ppvNode);
eCOMM_LIST_Ret eCOMM_LIST_Show_Node(sCOMM_NODE_Info* psNode);
eCOMM_LIST_Ret eCOMM_LIST_Show_List(sCOMM_LIST_Info* psInfo);

/***************************************************
	function
***************************************************/

/*********************************************
* func : eCOMM_LIST_Init(sCOMM_LIST_Info* psInfo, l_pFunc_pv_pv plCallback, v_pFunc_pv pvShowEntry, LONG lSizeOfEntry)
* arg  : sCOMM_LIST_Info* psInfo, l_pFunc_pv_pv plCallback, v_pFunc_pv pvShowEntry, LONG lSizeOfEntry
* ret  : eCOMM_LIST_Ret
* note : init the list
*********************************************/
eCOMM_LIST_Ret eCOMM_LIST_Init(sCOMM_LIST_Info* psInfo, l_pFunc_pv_pv plCallback, v_pFunc_pv pvShowEntry, LONG lSizeOfEntry) {
	eCOMM_LIST_Ret eRet = COMM_LIST_SUCCESS;

	CL_FuncIn();

	{
		/* precondition */
		assert(plCallback != NULL);
		assert(lSizeOfEntry > 0);
		
		psInfo->psHead = NULL;
		psInfo->psTail = NULL;
		psInfo->lCount = 0;
		psInfo->lCallBackKeyOfNode = plCallback;
		psInfo->vShowEntry = pvShowEntry;
		psInfo->lSizeOfEntry = lSizeOfEntry;

		/* init mutex */
		psInfo->psMutex = malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(psInfo->psMutex, NULL);
	}

	CL_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_LIST_Create(sCOMM_LIST_Info* psInfo, void* pvEntry)
* arg  : sCOMM_LIST_Info* psInfo, void* pvEntry
* ret  : eCOMM_LIST_Ret
* note : create the Node with input entry
*********************************************/
eCOMM_LIST_Ret eCOMM_LIST_Create(sCOMM_LIST_Info* psInfo, void* pvEntry) {
	eCOMM_LIST_Ret eRet = COMM_LIST_SUCCESS;
	sCOMM_NODE_Info* psNode;

	CL_FuncIn();

	{
		pthread_mutex_lock(psInfo->psMutex);
		psNode = malloc(sizeof(sCOMM_NODE_Info));
		if (!psNode) {
			CL_Debug("not enough space\n");
			pthread_mutex_unlock(psInfo->psMutex);
			return COMM_LIST_MALLOC;
		}

		/* fill Node */
		psNode->psNodeBefore = psInfo->psTail;
		psNode->psNodeNext = NULL;
		psNode->pvEntry = malloc(psInfo->lSizeOfEntry);
		memcpy(psNode->pvEntry, pvEntry, psInfo->lSizeOfEntry);

		/* add the node */
		/* judge whether the first node */
		if (!psInfo->psHead) {
			/* first entry should update header */
			psInfo->psHead = psNode;
			psInfo->psTail = psNode;
		} else {
			psInfo->psTail->psNodeNext = psNode;
			psInfo->psTail = psNode;
		}
		
		psInfo->lCount++;
		pthread_mutex_unlock(psInfo->psMutex);
	}

	CL_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_LIST_Retrieve(sCOMM_LIST_Info* psInfo, void* pvKey, void* pvEntry)
* arg  : sCOMM_LIST_Info* psInfo, void* pvKey, void* pvEntry
* ret  : eCOMM_LIST_Ret
* note : retrive the entry from the list
*********************************************/
eCOMM_LIST_Ret eCOMM_LIST_Retrieve(sCOMM_LIST_Info* psInfo, void* pvKey, void* pvEntry) {
	eCOMM_LIST_Ret eRet = COMM_LIST_NOENTRY;
	sCOMM_NODE_Info* psNode;

	CL_FuncIn();

	{
		pthread_mutex_lock(psInfo->psMutex);
		eCOMM_LIST_Retrive_Node(psInfo, pvKey, &psNode);	

		/* return the entry */
		if (psNode) {
			memcpy(pvEntry, psNode->pvEntry, psInfo->lSizeOfEntry);
			eRet = COMM_LIST_SUCCESS;
		}
		
		pthread_mutex_unlock(psInfo->psMutex);
	}

	CL_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_LIST_Retrive_Node(sCOMM_LIST_Info* psInfo, void* pvKey, sCOMM_NODE_Info** ppvNode)
* arg  : sCOMM_LIST_Info* psInfo, void* pvKey, sCOMM_NODE_Info** ppvNode
* ret  : eCOMM_LIST_Ret
* note : retrive node with out mutex
*********************************************/
eCOMM_LIST_Ret eCOMM_LIST_Retrive_Node(sCOMM_LIST_Info* psInfo, void* pvKey, sCOMM_NODE_Info** ppvNode) {
	eCOMM_LIST_Ret eRet = COMM_LIST_SUCCESS;
	sCOMM_NODE_Info* psNodeInfo;
	*ppvNode = NULL;

	CL_FuncIn();

	{	
		psNodeInfo = psInfo->psHead;
		while (psNodeInfo) {
			/* show the node */
			// eCOMM_LIST_Show_Node(psNodeInfo);
			
			/* judge whether is the target node */
			if (psInfo->lCallBackKeyOfNode(pvKey, psNodeInfo->pvEntry)) {
				*ppvNode = (void*) psNodeInfo;
				break;
			}

			/* toggle to the next node */
			psNodeInfo = psNodeInfo->psNodeNext;
		}
	}

	CL_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_LIST_Update(sCOMM_LIST_Info* psInfo, void* pvKey, void* pvEntry)
* arg  : sCOMM_LIST_Info* psInfo, void* pvKey, void* pvEntry
* ret  : eCOMM_LIST_Ret
* note : update the node with key, entry
*********************************************/
eCOMM_LIST_Ret eCOMM_LIST_Update(sCOMM_LIST_Info* psInfo, void* pvKey, void* pvEntry) {
	eCOMM_LIST_Ret eRet = COMM_LIST_SUCCESS;
	sCOMM_NODE_Info* psNodeInfo;

	CL_FuncIn();

	{
		pthread_mutex_lock(psInfo->psMutex);
		
		/* find the target node */
		eCOMM_LIST_Retrive_Node(psInfo, pvKey, &psNodeInfo);
		if (!psNodeInfo) {
			/* indicate can not find the key */
			CL_Debug("the key can not be find\n");
			return COMM_LIST_NOKEY;
		}

		/* update */
		memcpy(psNodeInfo->pvEntry, pvEntry, psInfo->lSizeOfEntry);

		pthread_mutex_unlock(psInfo->psMutex);
	}

	CL_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_LIST_Delete(sCOMM_LIST_Info* psInfo, void* pvKey)
* arg  : sCOMM_LIST_Info* psInfo, void* pvKey
* ret  : eCOMM_LIST_Ret
* note : delete the node of key
*********************************************/
eCOMM_LIST_Ret eCOMM_LIST_Delete(sCOMM_LIST_Info* psInfo, void* pvKey) {
	eCOMM_LIST_Ret eRet = COMM_LIST_SUCCESS;
	sCOMM_NODE_Info* psNodeInfo;
	sCOMM_NODE_Info* psNodeInfoBefore;
	sCOMM_NODE_Info* psNodeInfoNext;

	CL_FuncIn();

	{
		pthread_mutex_lock(psInfo->psMutex);
		
		/* find the target node */
		eCOMM_LIST_Retrive_Node(psInfo, pvKey, &psNodeInfo);
		if (!psNodeInfo) {
			/* indicate can not find the key */
			CL_Debug("the key can not be find\n");
			return COMM_LIST_NOKEY;
		}

		/* get before and next entry */
		psNodeInfoBefore = psNodeInfo->psNodeBefore;
		psNodeInfoNext = psNodeInfo->psNodeNext;

		/* whether the last entry */
		if (psInfo->psTail == psNodeInfo && psInfo->psHead == psNodeInfo) {
			/* indicate the only one */
			psInfo->psHead = NULL;
			psInfo->psTail = NULL;
		} else if (psInfo->psTail == psNodeInfo) {
			/* indicate the last one */
			psNodeInfoBefore->psNodeNext = NULL;
			psInfo->psTail = psNodeInfoBefore;
		} else if (psInfo->psHead == psNodeInfo) {
			/* indicate the first one */
			psNodeInfoNext->psNodeBefore = NULL;
			psInfo->psHead = psNodeInfoNext;
		} else {
			/* indicate the middle entry */
			psNodeInfoBefore->psNodeNext = psNodeInfoNext;
			psNodeInfoNext->psNodeBefore = psNodeInfoBefore;
		}

		/* remove the entry, update count */
		free(psNodeInfo->pvEntry);
		free(psNodeInfo);
		psInfo->lCount--;

		pthread_mutex_unlock(psInfo->psMutex);

	}

	CL_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_LIST_Reset(sCOMM_LIST_Info* psInfo)
* arg  : sCOMM_LIST_Info* psInfo
* ret  : eCOMM_LIST_Ret
* note : reset the list
*********************************************/
eCOMM_LIST_Ret eCOMM_LIST_Reset(sCOMM_LIST_Info* psInfo) {
	eCOMM_LIST_Ret eRet = COMM_LIST_SUCCESS;
	sCOMM_NODE_Info* psNodeInfo;
	sCOMM_NODE_Info* psNodeDelete;

	CL_FuncIn();

	{
		pthread_mutex_lock(psInfo->psMutex);

		if (psInfo->lCount == 0) {
			CL_Debug("reset empty list\n");
			pthread_mutex_unlock(psInfo->psMutex);
			return eRet;
		}
		
		psNodeInfo = psInfo->psHead;
		while (psNodeInfo) {

			/* back up the delete node, move to the next node */
			psNodeDelete = psNodeInfo;
			psNodeInfo = psNodeDelete->psNodeNext;
			
			/* judge whether is the target node */
			free(psNodeDelete->pvEntry);
			free(psNodeDelete);
			
		}

		psInfo->psHead = NULL;
		psInfo->psTail = NULL;
		psInfo->lCount = 0;
		
		pthread_mutex_unlock(psInfo->psMutex);
	}

	CL_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_LIST_GetCount(sCOMM_LIST_Info* psInfo, LONG* plCount)
* arg  : sCOMM_LIST_Info* psInfo, LONG* plCount
* ret  : eCOMM_LIST_Ret
* note : get the count of the list
*********************************************/
eCOMM_LIST_Ret eCOMM_LIST_GetCount(sCOMM_LIST_Info* psInfo, LONG* plCount) {
	eCOMM_LIST_Ret eRet = COMM_LIST_SUCCESS;
	LONG lRetCount = 0;

	CL_FuncIn();

	{
		pthread_mutex_lock(psInfo->psMutex);
		lRetCount = psInfo->lCount;
		pthread_mutex_unlock(psInfo->psMutex);

		*plCount = lRetCount;
		//printf("list count=%ld\n",*plCount);
	}

	CL_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_LIST_Deinit(sCOMM_LIST_Info* psInfo)
* arg  : sCOMM_LIST_Info* psInfo
* ret  : eCOMM_LIST_Ret
* note : deinit the list
*********************************************/
eCOMM_LIST_Ret eCOMM_LIST_Deinit(sCOMM_LIST_Info* psInfo) {
	eCOMM_LIST_Ret eRet = COMM_LIST_SUCCESS;

	CL_FuncIn();

	{
		if (NULL == psInfo) {
			return eRet;
		}
		/* reset */
		eCOMM_LIST_Reset(psInfo);

		/* release other resource */
		psInfo->lCallBackKeyOfNode = NULL;
		psInfo->lSizeOfEntry = 0;
		pthread_mutex_destroy(psInfo->psMutex);
		free(psInfo->psMutex);
		psInfo->psMutex = NULL;
	}

	CL_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_LIST_Show_Node(sCOMM_NODE_Info* psNode)
* arg  : sCOMM_NODE_Info* psNode
* ret  : eCOMM_LIST_Ret
* note : show the node
*********************************************/
eCOMM_LIST_Ret eCOMM_LIST_Show_Node(sCOMM_NODE_Info* psNode) {
	eCOMM_LIST_Ret eRet = COMM_LIST_SUCCESS;

	CL_FuncIn();

	{
		CL_Debug("node : %p, node before : %p, node after : %p, entry : %p\n", 
			psNode, psNode->psNodeBefore, psNode->psNodeNext, psNode->pvEntry);
	}

	CL_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_LIST_Ret eCOMM_LIST_Show_List(sCOMM_LIST_Info* psInfo)
* arg  : sCOMM_LIST_Info* psInfo
* ret  : eCOMM_LIST_Ret
* note : show the list
*********************************************/
eCOMM_LIST_Ret eCOMM_LIST_Show_List(sCOMM_LIST_Info* psInfo) {
	eCOMM_LIST_Ret eRet = COMM_LIST_SUCCESS;
	sCOMM_NODE_Info* psNodeTmp = NULL;

	CL_FuncIn();

	{
		pthread_mutex_lock(psInfo->psMutex);
		CL_Debug("List Info : header %p tail %p count %ld\n", psInfo->psHead, psInfo->psTail, psInfo->lCount);
		for (psNodeTmp = psInfo->psHead; psNodeTmp ; psNodeTmp = psNodeTmp->psNodeNext) {
			eCOMM_LIST_Show_Node(psNodeTmp);
		}
		pthread_mutex_unlock(psInfo->psMutex);
	}

	CL_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_LIST_Ret eCOMM_LIST_ShowEntry(sCOMM_LIST_Info* psInfo)
* arg  : sCOMM_LIST_Info* psInfo
* ret  : eCOMM_LIST_Ret
* note : show the list entry
*********************************************/
eCOMM_LIST_Ret eCOMM_LIST_ShowEntry(sCOMM_LIST_Info* psInfo) {
	eCOMM_LIST_Ret eRet = COMM_LIST_SUCCESS;
	sCOMM_NODE_Info* psNodeTmp = NULL;

	CL_FuncIn();

	{
		pthread_mutex_lock(psInfo->psMutex);
		if (!psInfo->vShowEntry) {
			CL_Debug("no show entry function register\n");
			pthread_mutex_unlock(psInfo->psMutex);
			return COMM_LIST_NOENTRYFUNC;
		}

		CL_Debug("List Info : header %p tail %p count %ld\n", psInfo->psHead, psInfo->psTail, psInfo->lCount);
		for (psNodeTmp = psInfo->psHead; psNodeTmp ; psNodeTmp = psNodeTmp->psNodeNext) {
			psInfo->vShowEntry(psNodeTmp->pvEntry);
		}
		pthread_mutex_unlock(psInfo->psMutex);
	}

	CL_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_LIST_Ret eCOMM_LIST_Traversal(sCOMM_LIST_Info* psInfo, l_pFunc_pv_pv pfCallback, void* pvArg)
* arg  : sCOMM_LIST_Info* psInfo, l_pFunc_pv_pv pfCallback, void* pvArg
* ret  : eCOMM_LIST_Ret
* note : traversal the list
*********************************************/
eCOMM_LIST_Ret eCOMM_LIST_Traversal(sCOMM_LIST_Info* psInfo, l_pFunc_pv_pv pfCallback, void* pvArg) {
	eCOMM_LIST_Ret eRet = COMM_LIST_SUCCESS;
	sCOMM_NODE_Info* psNodeTmp = NULL;

	CL_FuncIn();

	{
		pthread_mutex_lock(psInfo->psMutex);	
		if (!pfCallback) {
			CL_Debug("no traversal call back function\n");
			pthread_mutex_unlock(psInfo->psMutex);
			return COMM_LIST_TRAVERSALCALLBACK;
		}

		/* check whether the list is null */
		if (psInfo->lCount == 0) {
			CL_Debug("traversal empty list\n");
			pthread_mutex_unlock(psInfo->psMutex);
			return COMM_LIST_TRAVERSALEMPTYLIST;
		}
		
		/* traversal the list */
		for (psNodeTmp = psInfo->psHead; psNodeTmp; psNodeTmp = psNodeTmp->psNodeNext) {
			if (pfCallback(psNodeTmp->pvEntry, pvArg)) {
				break;
			}
		}

		pthread_mutex_unlock(psInfo->psMutex);
	}

	CL_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_LIST_Ret eCOMM_LIST_RetrieveSpecial(sCOMM_LIST_Info* psInfo, eCOMM_LIST_RetrieveKey eKey, void* pvArg)
* arg  : sCOMM_LIST_Info* psInfo, eCOMM_LIST_RetrieveKey eKey, void* pvArg
* ret  : eCOMM_LIST_Ret
* note : retrieve special node
*********************************************/
eCOMM_LIST_Ret eCOMM_LIST_RetrieveSpecial(sCOMM_LIST_Info* psInfo, eCOMM_LIST_RetrieveKey eKey, void* pvArg) {
	eCOMM_LIST_Ret eRet = COMM_LIST_SUCCESS;

	CL_FuncIn();

	{
		pthread_mutex_lock(psInfo->psMutex);

		if (!psInfo->lCount) {
			CL_Debug("the list is empty, special node not exist\n");
			pthread_mutex_unlock(psInfo->psMutex);
			return COMM_LIST_NOENTRY;
		}

		switch (eKey) {
			case COMM_LIST_RETRIEVEKEY_FIRST:
				CL_Debug("retrieve first entry\n");
				memcpy(pvArg, psInfo->psHead->pvEntry, psInfo->lSizeOfEntry);
				break;
			case COMM_LIST_RETRIEVEKEY_LAST:
				CL_Debug("retrieve last entry\n");
				memcpy(pvArg, psInfo->psTail->pvEntry, psInfo->lSizeOfEntry);
				break;
			default:
				eRet = COMM_LIST_RETRIEVEKEY;
		}
		
		pthread_mutex_unlock(psInfo->psMutex);
	}

	CL_FuncOut();

	return eRet;
}

#ifdef __cplusplus
}
#endif
