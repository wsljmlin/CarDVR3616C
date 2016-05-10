#ifndef __CTRL_INTERFACE_H__
#define __CTRL_INTERFACE_H__

#include "comm_interface.h"
#include "ctrl_api_button.h"
#ifdef __cplusplus
extern "C"{
#endif

/* include */
#include "ctrl_uart_getCommand.h"
#include "ctrl_uartReq.h"

#define CTRL_REQUEST_LISTMAX 10
#define CTRL_COMMAND_LISTMAX 10
#define CTRL_FEEDBACK_LISTMAX 10
#define CTRL_CFGFILENAMECHAR 50				// the max char of config file name

typedef enum {
	CTRL_CMD_NONEEDREAD,					// no need read
	CTRL_CMD_NEEDREAD						// need read
} eCTRL_CMD_CFGSTATE;

typedef struct {
	eCOMM_UI eUi;				// UI ID
	LONG lSocket;				// accept socket
	sCOMM_CONDFLAG sCf;	// condition flag, communicate between gui request and his thread
	LONG lCh;						// request channel
	CHAR cIp[COMM_IP_ADDRESS_LENGTH];		// client ip address
	LONG lRtpPort;					// port
	LONG lValid;					// valid, 1 for available, 0 for use
} sCTRL_GUI_CLIENT;

typedef struct {
	LONG lCfgType;											// config file type
	CHAR cCfgName[CTRL_CFGFILENAMECHAR];			// config file name
} sCTRL_TERMINAL_CLIENT;

/* global define */
sCTRL_GUI_CLIENT gsGuiClient[COMM_UI_MAXGUICNT];
sCTRL_TERMINAL_CLIENT gsTerminalClient;

/* request list */
sCOMM_QUEUE sReqList;

/* get command list */
sCOMM_QUEUE sCmdList;

/* feedback list */
sCOMM_QUEUE sFeedbackList;

/* condition flag, ctrl return for terminal */
sCOMM_CONDFLAG  stCfCtlRetTerminal;

/* condition flag, ctrl return for gui control */
sCOMM_CONDFLAG  stCfCtlRetGui;

/* condition flag, ctrl return for uart control */
sCOMM_CONDFLAG stCfCtlRetUart;

/* end flag */
LONG glEndFlag;

/* uart request */
sCTRL_UARTREQ_Info sUartReq;

/* get command */
sCTRL_UART_GETCOMMAND_Info sGetCommand;

/*------------------------------------------------------------------------------*/
/*!
    @brief			interactiveCtl, get request communicate with base process
*/
/* -----------------------------------------------------------------------------*/
void vCTRL_InteractiveCtl();


#ifdef __cplusplus
}
#endif

#endif /* End of #ifndef __INTERFACE_CTRL_H__*/

