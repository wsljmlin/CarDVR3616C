#ifndef __COMM_LIST_H__
#define __COMM_LIST_H__

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
	COMM_LIST_SUCCESS,
	COMM_LIST_FAIL,
	COMM_LIST_NOKEY,
	COMM_LIST_MALLOC,
	COMM_LIST_NOENTRYFUNC,
	COMM_LIST_TRAVERSALCALLBACK,
	COMM_LIST_INDEX,
	COMM_LIST_NOENTRY,
	COMM_LIST_TRAVERSALEMPTYLIST,
	COMM_LIST_RETRIEVEKEY,
} eCOMM_LIST_Ret;

typedef enum {
	COMM_LIST_RETRIEVEKEY_FIRST,
	COMM_LIST_RETRIEVEKEY_LAST,
} eCOMM_LIST_RetrieveKey;

typedef LONG(*l_pFunc_pv_pv)(void*, void*);
typedef void (*v_pFunc_pv)(void*);

/*********************************************************
	struct
*********************************************************/
typedef struct _sCOMM_NODE_Info {
	void* pvEntry;
	struct _sCOMM_NODE_Info* psNodeBefore;
	struct _sCOMM_NODE_Info* psNodeNext;
} sCOMM_NODE_Info;

typedef struct {
	l_pFunc_pv_pv lCallBackKeyOfNode;
	v_pFunc_pv vShowEntry;
	sCOMM_NODE_Info* psHead;
	sCOMM_NODE_Info* psTail;
	LONG lCount;
	LONG lSizeOfEntry;
	pthread_mutex_t* psMutex;
} sCOMM_LIST_Info;

/*********************************************************
	variable
*********************************************************/

/*********************************************************
	prototype
*********************************************************/
eCOMM_LIST_Ret eCOMM_LIST_Init(sCOMM_LIST_Info* psInfo, l_pFunc_pv_pv plCallback, v_pFunc_pv pvShowEntry, LONG lSizeOfEntry);
eCOMM_LIST_Ret eCOMM_LIST_Create(sCOMM_LIST_Info* psInfo, void* pvEntry);
eCOMM_LIST_Ret eCOMM_LIST_Retrieve(sCOMM_LIST_Info* psInfo, void* pvKey, void* pvEntry);
eCOMM_LIST_Ret eCOMM_LIST_Update(sCOMM_LIST_Info* psInfo, void* pvKey, void* pvEntry);
eCOMM_LIST_Ret eCOMM_LIST_Delete(sCOMM_LIST_Info* psInfo, void* pvKey);
eCOMM_LIST_Ret eCOMM_LIST_Reset(sCOMM_LIST_Info* psInfo);
eCOMM_LIST_Ret eCOMM_LIST_GetCount(sCOMM_LIST_Info* psInfo, LONG* plCount);
eCOMM_LIST_Ret eCOMM_LIST_Deinit(sCOMM_LIST_Info* psInfo);
eCOMM_LIST_Ret eCOMM_LIST_ShowEntry(sCOMM_LIST_Info* psInfo);
eCOMM_LIST_Ret eCOMM_LIST_Traversal(sCOMM_LIST_Info* psInfo, l_pFunc_pv_pv pfCallback, void* pvArg);
eCOMM_LIST_Ret eCOMM_LIST_RetrieveSpecial(sCOMM_LIST_Info* psInfo, eCOMM_LIST_RetrieveKey eKey, void* pvArg);

#ifdef __cplusplus
}
#endif

#endif
