/* include */
#include "ctrl_interface.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef __cplusplus
extern "C"{
#endif

/* extern condition flag */
extern sCOMM_CONDFLAG stCfCtlRetTerminal;
extern sCOMM_CONDFLAG stCfCtlRetGui;
extern sCOMM_CONDFLAG stCfCtlRetUart;

/* extern function */
extern void* pvCTRL_TerminalReq(void* arg);
extern void* pvCTRL_Gui_Req(void* arg);
extern LONG lCTRL_RequestResponse(CHAR* pcReq);

/* define */
//#define CTRL_INTERACTIVECTL_DEBUG
#ifdef CTRL_INTERACTIVECTL_DEBUG
#define CI_Debug(fmt, arg...) fprintf(stdout, "[ CI ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CI_Debug(fmt, arg...)
#endif

#define CI_FuncIn() CI_Debug("in\n")
#define CI_FuncOut() CI_Debug("out\n")
#define CI_iVal(iVal) CI_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CI_lVal(lVal) CI_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CI_PtVal(ptVal) CI_Debug("pt %s @ %p\n", #ptVal, ptVal)

/* enum */
typedef enum {
	CTRL_POST_FEEDBACK_START,
	CTRL_POST_FEEDBACK_STOP,
} eCTRL_POST_FEEDBACK_STAT;

/* init global condition flag */
LONG lFlag=0;

typedef struct {
	LONG fd;									
	volatile LONG lEndFlag;
} sCTRL_BTN_PARAM;

/*------------------------------------------------------------------------------*/
/*!
    @brief			get request from request list, communicate with base process
*/
/* -----------------------------------------------------------------------------*/
void vCTRL_InteractiveCtl(){
	LONG lRet=0;
	
#ifndef CLOSE_TERM_REQ
	pthread_t terReq_pid;
#endif /* CLOSE_TERM_REQ */

#ifndef CLOSE_GUI_REQ
#ifndef DEBUG_TERMINAL_REQ
	pthread_t guiReq_pid;
#endif
#endif /* CLOSE_GUI_REQ */


	sCTRL_API_BUTTON_Info sButtontReq;

	volatile LONG lEndFlag=0;

	/* init the global end flag */
	glEndFlag=0;

	/* request */
	CHAR cReq[COMM_CMDSMAXCHAR];
	CHAR* pcReq=cReq;

	debug_show("interactive control start\n");

	/* first create lReq list */
	vCOMM_LQ_Init(&sReqList, CTRL_REQUEST_LISTMAX);

	/* create lCmd list */
	vCOMM_LQ_Init(&sCmdList, CTRL_COMMAND_LISTMAX);

	/* create feedback list */
	vCOMM_LQ_Init(&sFeedbackList, CTRL_FEEDBACK_LISTMAX);

	/* init mutex */
	pthread_mutex_init(&sMutReqId, NULL);

	/* init condition flag */
	vCOMM_CF_InitCF(&stCfCtlRetTerminal, COMM_CFCTRLTIMEOUT, "reqToterminal");
	vCOMM_CF_InitCF(&stCfCtlRetGui, COMM_CFCTRLGUITIMEOUT, "reqToGuiControl");
	vCOMM_CF_InitCF(&stCfCtlRetUart, COMM_CFCTRLUARTTIMEOUT, "reqToUartControl");

	/* init get command info */
	lRet = eCTRL_UART_getCommand_Init(&sGetCommand, &sCmdList, &sFeedbackList);
	if(lRet != 0) {
		debug_info("[ UART ] : error code %ld\n", lRet);
		exit(EXIT_FAILURE);
	}
	lRet = eCTRL_UART_getCommand_StartThread(&sGetCommand);
	if(lRet != 0) {
		debug_info("[ UART ] : error code %ld\n", lRet);
		exit(EXIT_FAILURE);
	}

	/* init and start uart info */
	lRet = eCTRL_UARTREQ_Init(&sUartReq, &sCmdList, &sFeedbackList, &sReqList);
	if(lRet != 0) {
		debug_info("[ UARTREQ ] : error code %ld\n", lRet);
		exit(EXIT_FAILURE);
	}
	lRet = eCTRL_UARTREQ_StartThread(&sUartReq);
	if(lRet != 0) {
		debug_info("[ UARTREQ ] : error code %ld\n", lRet);
		exit(EXIT_FAILURE);
	}

	/* init and start button info */
	lRet = eCTRL_API_BUTTON_Init(&sButtontReq);
	if(lRet != 0) {
		debug_info("[ BUTTON ] : error code %ld\n", lRet);
		exit(EXIT_FAILURE);
	}	
	lRet = eCTRL_API_BUTTON_StartThread(&sButtontReq);
	if(lRet != 0) {
		debug_info("[ BUTTON ] : error code %ld\n", lRet);
		exit(EXIT_FAILURE);
	}	

#ifndef CLOSE_TERM_REQ	
	/* create terminal request thread */
	lRet=pthread_create(&terReq_pid, NULL, pvCTRL_TerminalReq, (void*) NULL);
	if(lRet){
		debug_info("[ Thread ] : terminal thread create error!\n");
		exit(-1);
	}
#endif /* CLOSE_TERM_REQ */

#ifndef CLOSE_GUI_REQ
	/* create gui request thread */
#ifndef DEBUG_TERMINAL_REQ
	lRet=pthread_create(&guiReq_pid, NULL, pvCTRL_Gui_Req, (void*) (&lEndFlag));
	if(lRet){
		debug_info("[ Thread ] : terminal thread create error!\n");
		exit(-1);
	}
#endif /* DEBUG_TERMINAL_REQ */
#endif

	while(1){
		/* get request from request list */
		vCOMM_LQ_Read(&sReqList, &pcReq);
		
		/* request response */
		lRet=lCTRL_RequestResponse(pcReq);

		/* how to quit the system ?*/
		if( lRet != COMM_RET_SYSQUIT ){
			/* system not quit */
			continue;
		}else{
			/* indicate system quit toggle the end flag, stop the gui thread */
			CI_Debug("system quit command\n");
			lEndFlag=1;
			break;
		}
	}
	
#ifndef CLOSE_TERM_REQ
	pthread_cancel(terReq_pid);
	pthread_join(terReq_pid, (void**) NULL);
	CI_Debug("terminal quit\n");
#endif /* CLOSE_TERM_REQ */

#ifndef CLOSE_GUI_REQ
#ifndef DEBUG_TERMINAL_REQ
	pthread_cancel(guiReq_pid);
	pthread_join(guiReq_pid, (void**) NULL);
	CI_Debug("gui quit\n");
#endif
#endif

	/* deinit and stop button info */	
	lRet = eCTRL_API_BUTTON_StopThread(&sButtontReq);
	if(lRet != 0) {
		debug_info("[ BUTTON ] : error code %ld\n", lRet);
		exit(EXIT_FAILURE);
	}
	
	lRet = eCTRL_API_BUTTON_Deinit(&sButtontReq);
	if(lRet != 0) {
		debug_info("[ BUTTON ] : error code %ld\n", lRet);
		exit(EXIT_FAILURE);
	}
	CI_Debug("botton thread quit\n");
	
	/* deinit get command */
	lRet = eCTRL_UART_getCommand_StopThread(&sGetCommand);
	if(lRet != 0) {
		debug_info("[ UART ] : error code %ld\n", lRet);
		exit(EXIT_FAILURE);
	}
	lRet = eCTRL_UART_getCommand_Deinit(&sGetCommand);
	if(lRet != 0) {
		debug_info("[ UART ] : error code %ld\n", lRet);
		exit(EXIT_FAILURE);
	}
	
	CI_Debug("get command thread quit\n");

	/* deinit uartreq */
	lRet = eCTRL_UARTREQ_StopThread(&sUartReq);
	if(lRet != 0) {
		debug_info("[ UARTREQ ] : error code %ld\n", lRet);
		exit(EXIT_FAILURE);
	}
	lRet = eCTRL_UARTREQ_Deinit(&sUartReq);
	if(lRet != 0) {
		debug_info("[ UARTREQ ] : error code %ld\n", lRet);
		exit(EXIT_FAILURE);
	}

	CI_Debug("uart request thread quit\n");

	/* deinit mutex */
	pthread_mutex_destroy(&sMutReqId);

	/* Deinit Feedback list */
	vCOMM_LQ_DeInit(&sFeedbackList);

	/* Deinit lCmd list */
	vCOMM_LQ_DeInit(&sCmdList);

	/* Deinit lReq list */
	vCOMM_LQ_DeInit(&sReqList);

	/* Deinit condition flag */
	vCOMM_CF_DeInitCF(&stCfCtlRetTerminal);
	vCOMM_CF_DeInitCF(&stCfCtlRetGui);
	vCOMM_CF_DeInitCF(&stCfCtlRetUart);

	debug_show("interactive control stop\n");
}

#ifdef __cplusplus
}
#endif

