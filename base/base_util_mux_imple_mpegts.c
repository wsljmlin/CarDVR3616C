#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "base_util_mux_interface.h"

/***************************************************
	macro / enum
***************************************************/
//#define BASE_UTIL_MUX_INTERFACE_DEBUG
#ifdef BASE_UTIL_MUX_INTERFACE_DEBUG
#define BUMI_Debug(fmt, arg...) fprintf(stdout, "[ BUMI ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define BUMI_Debug(fmt, arg...)
#endif

#define BUMI_FuncIn() BUMI_Debug("in\n")
#define BUMI_FuncOut() BUMI_Debug("out\n")
#define BUMI_iVal(iVal) BUMI_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define BUMI_lVal(lVal) BUMI_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define BUMI_PtVal(ptVal) BUMI_Debug("pt %s @ %p\n", #ptVal, ptVal)

#define BASE_UTIL_MUX_IMPLE_MPEGTS_QUEUEMAX 20
#define BASE_UTIL_MUX_IMPLE_MPEGTS_FILENAMEMAX 256
#define BASE_UTIL_MUX_IMPLE_MPEGTS_COMMANDMAX 1024

/***************************************************
	variable
***************************************************/

/***************************************************
	prototype
***************************************************/
eBASE_UTIL_MUX_INTERFACE_Ret eBASE_UTIL_MUX_IMPLE_MPEGTS_Construct(sBASE_UTIL_MUX_INTERFACE_Info* psInfo);
eBASE_UTIL_MUX_INTERFACE_Ret eBASE_UTIL_MUX_IMPLE_MPEGTS_Init(sBASE_UTIL_MUX_INTERFACE_Info* psInfo, void* pvPrivate);
eBASE_UTIL_MUX_INTERFACE_Ret eBASE_UTIL_MUX_IMPLE_MPEGTS_Mux(sBASE_UTIL_MUX_INTERFACE_Info* psInfo, CHAR* pcSrcFile);
eBASE_UTIL_MUX_INTERFACE_Ret eBASE_UTIL_MUX_IMPLE_MPEGTS_DeInit(sBASE_UTIL_MUX_INTERFACE_Info* psInfo);
eBASE_UTIL_MUX_INTERFACE_Ret eBASE_UTIL_MUX_IMPLE_MPEGTS_Mux_PreHandle(sBASE_UTIL_MUX_INTERFACE_Info* psInfo, CHAR* pcSrcFile);
eBASE_UTIL_MUX_INTERFACE_Ret eBASE_UTIL_MUX_IMPLE_MPEGTS_Mux_Handle(sBASE_UTIL_MUX_INTERFACE_Info* psInfo, CHAR* pcSrcFile);
eBASE_UTIL_MUX_INTERFACE_Ret eBASE_UTIL_MUX_IMPLE_MPEGTS_MuxRequest(sBASE_UTIL_MUX_INTERFACE_Info* psInfo, CHAR* pcSrcFile);
eBASE_UTIL_MUX_INTERFACE_Ret eBASE_UTIL_MUX_IMPLE_MPEGTS_StartThread(sBASE_UTIL_MUX_INTERFACE_Info* psInfo);
eBASE_UTIL_MUX_INTERFACE_Ret eBASE_UTIL_MUX_IMPLE_MPEGTS_StopThread(sBASE_UTIL_MUX_INTERFACE_Info* psInfo);
void* pvBASE_UTIL_MUX_IMPLE_MPEGTS_MainHandle(void* pvArg);

/***************************************************
	function
***************************************************/

/*********************************************
* func : eBASE_UTIL_MUX_IMPLE_MPEGTS_Construct(sBASE_UTIL_MUX_INTERFACE_Info* psInfo)
* arg  : sBASE_UTIL_MUX_INTERFACE_Info* psInfo
* ret  : eBASE_UTIL_MUX_INTERFACE_Ret
* note : construct the mux interface, export function
*********************************************/
eBASE_UTIL_MUX_INTERFACE_Ret eBASE_UTIL_MUX_IMPLE_MPEGTS_Construct(sBASE_UTIL_MUX_INTERFACE_Info* psInfo) {

	eBASE_UTIL_MUX_INTERFACE_Ret eRet = BASE_UTIL_MUX_INTERFACE_SUCCESS;

	BUMI_FuncIn();

	{
		psInfo->eStatus = BASE_UTIL_MUX_INTERFACE_STATUS_INVALID;
		psInfo->eBASE_UTIL_MUX_INTERFACE_Init = eBASE_UTIL_MUX_IMPLE_MPEGTS_Init;
		psInfo->eBASE_UTIL_MUX_INTERFACE_MuxRequest = eBASE_UTIL_MUX_IMPLE_MPEGTS_MuxRequest;
		psInfo->eBASE_UTIL_MUX_INTERFACE_StartThread = eBASE_UTIL_MUX_IMPLE_MPEGTS_StartThread;
		psInfo->eBASE_UTIL_MUX_INTERFACE_StopThread = eBASE_UTIL_MUX_IMPLE_MPEGTS_StopThread;
		psInfo->eBASE_UTIL_MUX_INTERFACE_DeInit = eBASE_UTIL_MUX_IMPLE_MPEGTS_DeInit;
		psInfo->pcName = "mpegts";
		psInfo->pvPrivate = NULL;

		/* init queue */
		psInfo->psQueue = malloc(sizeof(sCOMM_QUEUE));
		if (!psInfo->psQueue) {
			BUMI_Debug("error in malloc\n");
			return BASE_UTIL_MUX_INTERFACE_MALLOC;
		}

		vCOMM_LQ_Init(psInfo->psQueue, BASE_UTIL_MUX_IMPLE_MPEGTS_QUEUEMAX);
		
		psInfo->eStatus = BASE_UTIL_MUX_INTERFACE_STATUS_IMPLE;
	}

	BUMI_FuncOut();
	
	return eRet;
}


/*********************************************
* func : eBASE_UTIL_MUX_IMPLE_MPEGTS_Init(sBASE_UTIL_MUX_INTERFACE_Info* psInfo, void* pvPrivate)
* arg  : sBASE_UTIL_MUX_INTERFACE_Info* psInfo, void* pvPrivate
* ret  : eBASE_UTIL_MUX_INTERFACE_Ret
* note : the init function of the interface
*********************************************/
eBASE_UTIL_MUX_INTERFACE_Ret eBASE_UTIL_MUX_IMPLE_MPEGTS_Init(sBASE_UTIL_MUX_INTERFACE_Info* psInfo, void* pvPrivate) {

	eBASE_UTIL_MUX_INTERFACE_Ret eRet = BASE_UTIL_MUX_INTERFACE_SUCCESS;

	assert(psInfo->eStatus == BASE_UTIL_MUX_INTERFACE_STATUS_IMPLE);

	BUMI_FuncIn();

	{
		/* mpeg ts does not need any special parameter to init */
		psInfo->eStatus = BASE_UTIL_MUX_INTERFACE_STATUS_VALID;

	}

	BUMI_FuncOut();
	
	return eRet;
}

/*********************************************
* func : eBASE_UTIL_MUX_IMPLE_MPEGTS_Mux(sBASE_UTIL_MUX_INTERFACE_Info* psInfo, CHAR* pcSrcFile)
* arg  : sBASE_UTIL_MUX_INTERFACE_Info* psInfo, CHAR* pcSrcFile
* ret  : eBASE_UTIL_MUX_INTERFACE_Ret
* note : the mux function of the interface
*********************************************/
eBASE_UTIL_MUX_INTERFACE_Ret eBASE_UTIL_MUX_IMPLE_MPEGTS_Mux(sBASE_UTIL_MUX_INTERFACE_Info* psInfo, CHAR* pcSrcFile) {

	eBASE_UTIL_MUX_INTERFACE_Ret eRet = BASE_UTIL_MUX_INTERFACE_SUCCESS;

	assert(psInfo->eStatus & BASE_UTIL_MUX_INTERFACE_STATUS_VALID);

	BUMI_FuncIn();

	{
		/* prehandle */
		eRet = eBASE_UTIL_MUX_IMPLE_MPEGTS_Mux_PreHandle(psInfo, pcSrcFile);
		if (eRet != BASE_UTIL_MUX_INTERFACE_SUCCESS) {
			BUMI_Debug("error in prehandle\n");
			return eRet;
		}

		/* mux */
		eRet = eBASE_UTIL_MUX_IMPLE_MPEGTS_Mux_Handle(psInfo, pcSrcFile);
		if (eRet != BASE_UTIL_MUX_INTERFACE_SUCCESS) {
			BUMI_Debug("error in handle\n");
			return eRet;
		}
	}

	BUMI_FuncOut();
	
	return eRet;
}

/*********************************************
* func : eBASE_UTIL_MUX_IMPLE_MPEGTS_DeInit(sBASE_UTIL_MUX_INTERFACE_Info* psInfo)
* arg  : sBASE_UTIL_MUX_INTERFACE_Info* psInfo
* ret  : eBASE_UTIL_MUX_INTERFACE_Ret
* note : the deinit function of the interface
*********************************************/
eBASE_UTIL_MUX_INTERFACE_Ret eBASE_UTIL_MUX_IMPLE_MPEGTS_DeInit(sBASE_UTIL_MUX_INTERFACE_Info* psInfo) {

	eBASE_UTIL_MUX_INTERFACE_Ret eRet = BASE_UTIL_MUX_INTERFACE_SUCCESS;

	assert(psInfo->eStatus & BASE_UTIL_MUX_INTERFACE_STATUS_VALID);

	BUMI_FuncIn();

	{
		/* mpeg ts does not need any special parameter to deinit */
		psInfo->eStatus = BASE_UTIL_MUX_INTERFACE_STATUS_INVALID;

		/* deinit the queue */
		vCOMM_LQ_DeInit(psInfo->psQueue);
		free(psInfo->psQueue);
		psInfo->psQueue = NULL;
		
		psInfo->eBASE_UTIL_MUX_INTERFACE_Init = NULL;
		psInfo->eBASE_UTIL_MUX_INTERFACE_MuxRequest = NULL;
		psInfo->eBASE_UTIL_MUX_INTERFACE_StartThread = NULL;
		psInfo->eBASE_UTIL_MUX_INTERFACE_StopThread = NULL;
		psInfo->eBASE_UTIL_MUX_INTERFACE_DeInit = NULL;
		psInfo->pcName = NULL;
		psInfo->pvPrivate = NULL;

	}

	BUMI_FuncOut();
	
	return eRet;
}

/*********************************************
* func : eBASE_UTIL_MUX_IMPLE_MPEGTS_Mux_PreHandle(sBASE_UTIL_MUX_INTERFACE_Info* psInfo, CHAR* pcSrcFile)
* arg  : sBASE_UTIL_MUX_INTERFACE_Info* psInfo, CHAR* pcSrcFile
* ret  : eBASE_UTIL_MUX_INTERFACE_Ret
* note : the prehandle of mux, check the existable of input file
*********************************************/
eBASE_UTIL_MUX_INTERFACE_Ret eBASE_UTIL_MUX_IMPLE_MPEGTS_Mux_PreHandle(sBASE_UTIL_MUX_INTERFACE_Info* psInfo, CHAR* pcSrcFile) {

	eBASE_UTIL_MUX_INTERFACE_Ret eRet = BASE_UTIL_MUX_INTERFACE_SUCCESS;
	int iRet = 0;

	assert(psInfo->eStatus & BASE_UTIL_MUX_INTERFACE_STATUS_VALID);

	BUMI_FuncIn();

	{
		/* check the source file */
		iRet = access(pcSrcFile, R_OK);
		if (iRet != 0) {
			BUMI_Debug("the src file <%s> is not exist or can not be read\n", pcSrcFile);
			return BASE_UTIL_MUX_INTERFACE_SRCFILE;
		}
	}

	BUMI_FuncOut();
	
	return eRet;
}

/*********************************************
* func : eBASE_UTIL_MUX_IMPLE_MPEGTS_Mux_Handle(sBASE_UTIL_MUX_INTERFACE_Info* psInfo, CHAR* pcSrcFile)
* arg  : sBASE_UTIL_MUX_INTERFACE_Info* psInfo, CHAR* pcSrcFile
* ret  : eBASE_UTIL_MUX_INTERFACE_Ret
* note : the main mux process, currently based on ffmpeg
*********************************************/
eBASE_UTIL_MUX_INTERFACE_Ret eBASE_UTIL_MUX_IMPLE_MPEGTS_Mux_Handle(sBASE_UTIL_MUX_INTERFACE_Info* psInfo, CHAR* pcSrcFile) {

	eBASE_UTIL_MUX_INTERFACE_Ret eRet = BASE_UTIL_MUX_INTERFACE_SUCCESS;
	int iRet = 0;
	static CHAR cCmd[BASE_UTIL_MUX_IMPLE_MPEGTS_COMMANDMAX];

	assert(psInfo->eStatus & BASE_UTIL_MUX_INTERFACE_STATUS_VALID);
	assert(strlen(pcSrcFile) + 4 < BASE_UTIL_MUX_IMPLE_MPEGTS_FILENAMEMAX);

	BUMI_FuncIn();

	{
		/* construct the command */
		sprintf(cCmd, "ffmpeg -y -i %s -vcodec copy %s.ts && rm -f %s", 
			pcSrcFile, pcSrcFile, pcSrcFile);

		BUMI_Debug("muxing file %s\n", pcSrcFile);

		/* execute the command to mux and remove tmp file */
		iRet = system(cCmd);
		if (iRet != 0) {
			BUMI_Debug("execute command %s error\n", cCmd);
			return BASE_UTIL_MUX_INTERFACE_SYSTEM;
		}

		BUMI_Debug("done\n");
	}

	BUMI_FuncOut();
	
	return eRet;
}

/*********************************************
* func : eBASE_UTIL_MUX_IMPLE_MPEGTS_MuxRequest(sBASE_UTIL_MUX_INTERFACE_Info* psInfo, CHAR* pcSrcFile)
* arg  : sBASE_UTIL_MUX_INTERFACE_Info* psInfo, CHAR* pcSrcFile
* ret  : eBASE_UTIL_MUX_INTERFACE_Ret
* note : to request max a file
*********************************************/
eBASE_UTIL_MUX_INTERFACE_Ret eBASE_UTIL_MUX_IMPLE_MPEGTS_MuxRequest(sBASE_UTIL_MUX_INTERFACE_Info* psInfo, CHAR* pcSrcFile) {

	eBASE_UTIL_MUX_INTERFACE_Ret eRet = BASE_UTIL_MUX_INTERFACE_SUCCESS;

	assert(psInfo->eStatus & BASE_UTIL_MUX_INTERFACE_STATUS_VALID);

	BUMI_FuncIn();

	{
		/* push the request file to queue */
		vCOMM_LQ_Write(psInfo->psQueue, pcSrcFile, AUTO_DETECT_STRLEN);
		BUMI_Debug("push the file %s to the queue\n", pcSrcFile);
	}

	BUMI_FuncOut();
	
	return eRet;
}

/*********************************************
* func : pvBASE_UTIL_MUX_IMPLE_MPEGTS_MainHandle(void* pvArg)
* arg  : void* pvArg
* ret  : void*
* note : the main thread of mux
*********************************************/
void* pvBASE_UTIL_MUX_IMPLE_MPEGTS_MainHandle(void* pvArg) {
	eBASE_UTIL_MUX_INTERFACE_Ret eRet = BASE_UTIL_MUX_INTERFACE_SUCCESS;
	sBASE_UTIL_MUX_INTERFACE_Info* psInfo = (sBASE_UTIL_MUX_INTERFACE_Info*) pvArg;
	CHAR cSrcFile[BASE_UTIL_MUX_IMPLE_MPEGTS_FILENAMEMAX];
	CHAR* pcSrcFile = cSrcFile;
	
	BUMI_FuncIn();

	{
		while (psInfo->eStatus & BASE_UTIL_MUX_INTERFACE_STATUS_THREADSTART) {

			/* read request file in the list */
			vCOMM_LQ_Read(psInfo->psQueue, &pcSrcFile);

			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

			/* mux the file */
			eRet = eBASE_UTIL_MUX_IMPLE_MPEGTS_Mux(psInfo, pcSrcFile);
			if (eRet != BASE_UTIL_MUX_INTERFACE_SUCCESS) {
				BUMI_Debug("error in mux\n");
				continue;
			}

			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		}
	}

	BUMI_FuncOut();

	return NULL;
}

/*********************************************
* func : eBASE_UTIL_MUX_IMPLE_MPEGTS_StartThread(sBASE_UTIL_MUX_INTERFACE_Info* psInfo)
* arg  : sBASE_UTIL_MUX_INTERFACE_Info* psInfo
* ret  : eBASE_UTIL_MUX_INTERFACE_Ret
* note : start the mux thread
*********************************************/
eBASE_UTIL_MUX_INTERFACE_Ret eBASE_UTIL_MUX_IMPLE_MPEGTS_StartThread(sBASE_UTIL_MUX_INTERFACE_Info* psInfo) {

	eBASE_UTIL_MUX_INTERFACE_Ret eRet = BASE_UTIL_MUX_INTERFACE_SUCCESS;

	assert(psInfo->eStatus & BASE_UTIL_MUX_INTERFACE_STATUS_VALID);

	BUMI_FuncIn();

	{
		psInfo->eStatus |= BASE_UTIL_MUX_INTERFACE_STATUS_THREADSTART;
		pthread_create(&psInfo->pid, NULL, pvBASE_UTIL_MUX_IMPLE_MPEGTS_MainHandle, (void*) psInfo);
	}

	BUMI_FuncOut();
	
	return eRet;
}

/*********************************************
* func : eBASE_UTIL_MUX_IMPLE_MPEGTS_StopThread(sBASE_UTIL_MUX_INTERFACE_Info* psInfo)
* arg  : sBASE_UTIL_MUX_INTERFACE_Info* psInfo
* ret  : eBASE_UTIL_MUX_INTERFACE_Ret
* note : stop the mux thread
*********************************************/
eBASE_UTIL_MUX_INTERFACE_Ret eBASE_UTIL_MUX_IMPLE_MPEGTS_StopThread(sBASE_UTIL_MUX_INTERFACE_Info* psInfo) {

	eBASE_UTIL_MUX_INTERFACE_Ret eRet = BASE_UTIL_MUX_INTERFACE_SUCCESS;

	assert(psInfo->eStatus & BASE_UTIL_MUX_INTERFACE_STATUS_VALID);

	BUMI_FuncIn();

	{
		psInfo->eStatus &= ~ BASE_UTIL_MUX_INTERFACE_STATUS_THREADSTART;
		pthread_cancel(psInfo->pid);
		pthread_join(psInfo->pid, NULL);
	}

	BUMI_FuncOut();
	
	return eRet;
}

#ifdef __cplusplus
}
#endif
