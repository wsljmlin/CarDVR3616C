#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "CTRL_UARTREQ_PARSECOMMAND_WIFI.h"
#include "comm_command.h"
#include "comm_mw_strpool.h"
#include "comm_sharemem.h"
#include <assert.h>

/***************************************************
	macro / enum
***************************************************/
//#define CTRL_UARTREQ_PARSECOMMAND_WIFI_DEBUG
#ifdef CTRL_UARTREQ_PARSECOMMAND_WIFI_DEBUG
#define CUPW_Debug(fmt, arg...) fprintf(stdout, "[ CUPW ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CUPW_Debug(fmt, arg...)
#endif

#define CUPW_FuncIn() CUPW_Debug("in\n")
#define CUPW_FuncOut() CUPW_Debug("out\n")
#define CUPW_iVal(iVal) CUPW_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CUPW_lVal(lVal) CUPW_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CUPW_PtVal(ptVal) CUPW_Debug("pt %s @ %p\n", #ptVal, ptVal)

#define CTRL_UARTREQ_PARSECOMMAND_WIFI_COMMANDLEN 17
#define CTRL_UARTREQ_PARSECOMMAND_WIFI_USERNAMEMAX 16
#define CTRL_UARTREQ_PARSECOMMAND_WIFI_PASSWORDMAX 16

typedef enum {
	CTRL_UARTREQ_PARSECOMMAND_WIFI_CommandContent_UserName = 0,
	CTRL_UARTREQ_PARSECOMMAND_WIFI_CommandContent_PassWord = 8,
	CTRL_UARTREQ_PARSECOMMAND_WIFI_CommandContent_Type = 16,
} eCTRL_UARTREQ_PARSECOMMAND_WIFI_CommandContent;

typedef enum {
	CTRL_UARTREQ_PARSECOMMAND_WIFI_CommandLen_UserName = 8,
	CTRL_UARTREQ_PARSECOMMAND_WIFI_CommandLen_PassWord = 8,
	CTRL_UARTREQ_PARSECOMMAND_WIFI_CommandLen_Type = 1,
} eCTRL_UARTREQ_PARSECOMMAND_WIFI_CommandLen;

/***************************************************
	variable
***************************************************/
extern sCOMM_SHAREMEM gsShareMem;
extern const LONG SM_OFFSET[];

/***************************************************
	prototype
***************************************************/
void vCTRL_UDARTREQ_PARSECOMMAND_WIFI_WriteUserName(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg, CHAR* pcReq);
void vCTRL_UDARTREQ_PARSECOMMAND_WIFI_WritePassWord(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg, CHAR* pcReq);
void vCTRL_UDARTREQ_PARSECOMMAND_WIFI_WriteWifiType(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg, CHAR* pcReq);

/***************************************************
	function
***************************************************/

/*********************************************
* func : pvCTRL_UARTREQ_PARSECOMMAND_WIFI(void* pvArg)
* arg : void* pvArg
* ret : void
* note : parse wifi system command
*********************************************/
void pvCTRL_UARTREQ_PARSECOMMAND_WIFI(void* pvArg) {
	sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg = (sCTRL_UARTREQ_PARSECOMMAND_ThreadArg*) pvArg;
	CHAR pcRequest[CTRL_UARTREQ_PARSECOMMAND_REQMAX];
	CHAR* pcReq = pcRequest;

	CUPW_FuncIn();

	{
		/* check whether config yet */
		if(!(psThreadArg->eStatus & CTRL_UARTREQ_PARSECOMMAND_STATUS_Configed)) {
			CUPW_Debug("not config yet !\n");
			psThreadArg->eRet = CTRL_UARTREQ_PARSECOMMAND_Function_NotConfigYet;
			return;
		}
		
		/* parse the inter command */
		if (psThreadArg->lCommandLen != CTRL_UARTREQ_PARSECOMMAND_WIFI_COMMANDLEN) {
			psThreadArg->eRet = CTRL_UARTREQ_PARSECOMMAND_Function_Wifi;
			CUPW_Debug("error wifi command len <%d, %ld>\n", 
				CTRL_UARTREQ_PARSECOMMAND_WIFI_COMMANDLEN, 
				psThreadArg->lCommandLen);
			return;
		}

		/* clear the request buffer */
		memset(pcRequest, 0, sizeof(pcRequest));
		
		/* construct the request */
		vCTRL_UDARTREQ_PARSECOMMAND_WIFI_WriteUserName(psThreadArg, pcReq);

		pcReq += strlen(pcReq);
		*pcReq ++ = ',';
		vCTRL_UDARTREQ_PARSECOMMAND_WIFI_WritePassWord(psThreadArg, pcReq);

		pcReq += strlen(pcReq);
		*pcReq ++ = ',';
		vCTRL_UDARTREQ_PARSECOMMAND_WIFI_WriteWifiType(psThreadArg, pcReq);

		/* construct command */
		vCOMM_CMD_AddReqInfo(pcRequest, COMM_UI_UART);

		/* write to the request saver */
		CUPW_Debug("request : %s\n", pcRequest);
		eCTRL_UARTREQ_SAVEREQ_WriteReq(psThreadArg->psReqSaver, pcRequest);
		
	}
	
	CUPW_FuncOut();

	return;
}


/*********************************************
* func : vCTRL_UDARTREQ_PARSECOMMAND_WIFI_WriteUserName(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg, CHAR* pcReq)
* arg : sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg, CHAR* pcReq
* ret : void
* note : write username request
*********************************************/
void vCTRL_UDARTREQ_PARSECOMMAND_WIFI_WriteUserName(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg, CHAR* pcReq) {

	sCOMM_CMD sCommand;
	CHAR cUserName[CTRL_UARTREQ_PARSECOMMAND_WIFI_USERNAMEMAX];
	CHAR* pcUserName = cUserName;
	LONG lStrPoolKey;
	
	CUPW_FuncIn();

	{

		/* get user name */
		vGetCharContent(psThreadArg->pucCommandFirst,
			CTRL_UARTREQ_PARSECOMMAND_WIFI_CommandContent_UserName,
			CTRL_UARTREQ_PARSECOMMAND_WIFI_CommandLen_UserName,
			cUserName);

		/* skip the prefix '0' */
		while(*pcUserName == '0') {
			pcUserName ++;
		}

		CUPW_Debug("user name : <%s> -> <%s>\n", cUserName, pcUserName);

		/* get time string strpool key */
		COMM_MW_StrPool_WriteEntry(COMM_GET_STRPOOL(), pcUserName, &lStrPoolKey, &mutStringPoolWt);
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_SYS, COMM_SUBCMD_WIFIUSERNAME, lStrPoolKey); 
		vCOMM_CMD_CmdToChar(&sCommand, &pcReq);

	}

	CUPW_FuncOut();

	return;
}

/*********************************************
* func : vCTRL_UDARTREQ_PARSECOMMAND_WIFI_WritePassWord(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg, CHAR* pcReq)
* arg : sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg, CHAR* pcReq
* ret : void
* note : write username request
*********************************************/
void vCTRL_UDARTREQ_PARSECOMMAND_WIFI_WritePassWord(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg, CHAR* pcReq) {

	sCOMM_CMD sCommand;
	CHAR cPassWord[CTRL_UARTREQ_PARSECOMMAND_WIFI_PASSWORDMAX];
	CHAR* pcPassWord = cPassWord;
	LONG lStrPoolKey;
	
	CUPW_FuncIn();

	{

		/* get user name */
		vGetCharContent(psThreadArg->pucCommandFirst,
			CTRL_UARTREQ_PARSECOMMAND_WIFI_CommandContent_PassWord,
			CTRL_UARTREQ_PARSECOMMAND_WIFI_CommandLen_PassWord,
			cPassWord);

		/* skip the prefix '0' */
		while(*pcPassWord == '0') {
			pcPassWord ++;
		}

		CUPW_Debug("pass word : <%s> -> <%s>\n", cPassWord, pcPassWord);

		/* get time string strpool key */
		COMM_MW_StrPool_WriteEntry(COMM_GET_STRPOOL(), pcPassWord, &lStrPoolKey, &mutStringPoolWt);
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_SYS, COMM_SUBCMD_WIFIPASSWORD, lStrPoolKey); 
		vCOMM_CMD_CmdToChar(&sCommand, &pcReq);

	}

	CUPW_FuncOut();

	return;
}

/*********************************************
* func : vCTRL_UDARTREQ_PARSECOMMAND_WIFI_WriteWifiType(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg, CHAR* pcReq)
* arg : sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg, CHAR* pcReq
* ret : void
* note : write username request
*********************************************/
void vCTRL_UDARTREQ_PARSECOMMAND_WIFI_WriteWifiType(sCTRL_UARTREQ_PARSECOMMAND_ThreadArg* psThreadArg, CHAR* pcReq) {

	sCOMM_CMD sCommand;
	LONG lWifiType;
	
	CUPW_FuncIn();

	{

		/* get user name */
		lWifiType = lNUCharToLong(psThreadArg->pucCommandFirst,
			CTRL_UARTREQ_PARSECOMMAND_WIFI_CommandContent_Type,
			CTRL_UARTREQ_PARSECOMMAND_WIFI_CommandLen_Type);

		/* get time string strpool key */
		vCOMM_CMD_InitCmd(&sCommand, COMM_CMD_SYS, COMM_SUBCMD_WIFITYPE, lWifiType); 
		vCOMM_CMD_CmdToChar(&sCommand, &pcReq);

	}

	CUPW_FuncOut();

	return;
}

#ifdef __cplusplus
}
#endif
