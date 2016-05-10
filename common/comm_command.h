#ifndef __COMMON_CMD_H__
#define __COMMON_CMD_H__

#ifdef __cplusplus
extern "C"{
#endif


#include "comm_common.h"

/* global */
LONG glReqId;
pthread_mutex_t sMutReqId;

/* macro */
/* bytes constrain */
#define COMM_CMDSYSMAXCNT 50
#define COMM_CMDMAXREQID 20
#define COMM_CMDMAXCHAREACH 32																				// max CHAR for each command
#define COMM_CMDMAXOPTCNT 10																					// max option count
#define COMM_CMDSMAXCHAR ( COMM_CMDMAXCHAREACH * COMM_CMDMAXOPTCNT )					// max commands [ command * optionCount ]
#define COMM_CMDSPACKETMAXCHAR ( COMM_CMDMAXCHAREACH * ( COMM_CMDMAXOPTCNT + 2 ) )	// include head and tail

/* setter and getter */
#define COMM_CMDGETPARAID(pcmd, lCh) ((((pcmd->lCmd == COMM_CMD_VENC0 || pcmd->lCmd == COMM_CMD_ISP0 )?(pcmd->lCmd+COMM_CMDCHSTEP*lCh):(pcmd->lCmd)) << COMM_CMDMAXSUBSHIFT) + pcmd->lOpt)
#define COMM_CMDGETPARAVAL(pcmd) (pcmd->lVal)

/* construct reqId */
#define COMM_REQID 0
#define COMM_REQCH 1
#define COMM_REQUI 2
#define COMM_GETREQINFO(reqInfo, content) ((reqInfo>>(8*content)) & 0xff)
#define COMM_SETREQINFO(reqInfo, content) ((reqInfo & 0xff)<<(8*content))

/* channel id */
typedef enum {
	COMM_REQ_CH0,
	COMM_REQ_CH1,
	COMM_REQ_CH2,
} eCOMM_REQ_CH;

/* ui id, one termianl and at most 5 gui */
typedef enum {
	COMM_UI_GUI1,
	COMM_UI_GUI2,
	COMM_UI_MAXGUICNT,
	COMM_UI_TERMINAL,
	COMM_UI_UART,
	COMM_UI_BUTTON,
} eCOMM_UI;

/* error check */
typedef enum {
	COMM_CMD_VALID=0,
	COMM_CMD_INVALID=-1,
	COMM_CMD_INVALIDCMD=-2,
	COMM_CMD_INVALIDOPT=-3,
	COMM_CMD_INVALIDVAL=-4,
	COMM_CMD_INVALIDRESERVE=-5,
} eCOMM_CMD_STATE;

/* command array size used in vBASE_VENC_Ctrl_Thread and Venc thread */
#define COMM_CMD_ARRAYSIZE 16

/* default value */
#define COMM_CMDVAL_DEFAULT -1		// default value, some command do not need value


/* function declare */
/*------------------------------------------------------------------------------*/
/*!
	@brief				init the command

	@param			LONG lCmd				[in]			the command
						LONG lOption				[in]			the option
						LONG lValue				[in]			the value
	@param			sCOMM_CMD* pstCmd	[out]		the pointer point to the command struct
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_CMD_InitCmd(sCOMM_CMD* pstCmd, LONG lCmd, LONG lOption, LONG lValue);

/*------------------------------------------------------------------------------*/
/*!
	@brief				convert the command struct into command char

	@param			sCOMM_CMD* pstCmd	[in]			the pointer point to the command struct
	@param			CHAR** ppcCmd			[out]		the pointer point to the command char
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_CMD_CmdToChar(sCOMM_CMD* pstCmd, CHAR** ppcCmd);

/*------------------------------------------------------------------------------*/
/*!
	@brief				convert the command char into command struct

	@param			CHAR* pcCmd				[in]		the pointer point to the command char
	@param			sCOMM_CMD* pstCmd	[out]	the pointer point to the command struct
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_CMD_CharToCmd(sCOMM_CMD* pstCmd, CHAR* pcCmd);

/*------------------------------------------------------------------------------*/
/*!
	@brief				check the valid of the command

	@param			sCOMM_CMD* pstCmd	[in]				the pointer point to the command struct
	@retval			COMM_CMD_VALID						the command is valid
						COMM_CMD_INVALID					the command is invalid
						COMM_CMD_INVALIDCMD				the command is invalid
						COMM_CMD_INVALIDOPT				the option is invalid
						COMM_CMD_INVALIDVAL				the value is invalid
						COMM_CMD_INVALIDRESERVE		the command is reserve
*/
/* -----------------------------------------------------------------------------*/
LONG lCOMM_CMD_CheckCmd(sCOMM_CMD* pstCmd);

/*------------------------------------------------------------------------------*/
/*!
	@brief				check the valid of the commands

	@param			sCOMM_CMD* pstCmds	[in]				the pointer point to the multiple command char
	@retval			COMM_CMD_VALID						the command is valid
						COMM_CMD_INVALID					the command is invalid
						COMM_CMD_INVALIDCMD				the command is invalid
						COMM_CMD_INVALIDOPT				the option is invalid
						COMM_CMD_INVALIDVAL				the value is invalid
						COMM_CMD_INVALIDRESERVE		the command is reserve
*/
/* -----------------------------------------------------------------------------*/
LONG lCOMM_CMD_CheckCmds(CHAR* pcCmds);

/*------------------------------------------------------------------------------*/
/*!
	@brief				check the valid of the eth sub command

	@param			sCOMM_CMD* pstCmd	[in]				the pointer point to the comamnd
	@retval			COMM_CMD_VALID						the command is valid
						COMM_CMD_INVALID					the command is invalid
						COMM_CMD_INVALIDCMD				the command is invalid
						COMM_CMD_INVALIDOPT				the option is invalid
						COMM_CMD_INVALIDVAL				the value is invalid
						COMM_CMD_INVALIDRESERVE		the command is reserve
*/
/* -----------------------------------------------------------------------------*/
LONG lCOMM_CMD_CheckCmdVenc(sCOMM_CMD* pstCmd);

/*------------------------------------------------------------------------------*/
/*!
	@brief				check the valid of the rtp sub command

	@param			sCOMM_CMD* pstCmd	[in]				the pointer point to the comamnd
	@retval			COMM_CMD_VALID						the command is valid
						COMM_CMD_INVALID					the command is invalid
						COMM_CMD_INVALIDCMD				the command is invalid
						COMM_CMD_INVALIDOPT				the option is invalid
						COMM_CMD_INVALIDVAL				the value is invalid
						COMM_CMD_INVALIDRESERVE		the command is reserve
*/
/* -----------------------------------------------------------------------------*/
LONG lCOMM_CMD_CheckCmdEth(sCOMM_CMD* pstCmd);

/*------------------------------------------------------------------------------*/
/*!
	@brief				check the valid of the hdmi sub command

	@param			sCOMM_CMD* pstCmd	[in]				the pointer point to the comamnd
	@retval			COMM_CMD_VALID						the command is valid
						COMM_CMD_INVALID					the command is invalid
						COMM_CMD_INVALIDCMD				the command is invalid
						COMM_CMD_INVALIDOPT				the option is invalid
						COMM_CMD_INVALIDVAL				the value is invalid
						COMM_CMD_INVALIDRESERVE		the command is reserve
*/
/* -----------------------------------------------------------------------------*/
LONG lCOMM_CMD_CheckCmdHdmi(sCOMM_CMD* pstCmd);
/*------------------------------------------------------------------------------*/
/*!
	@brief				check the valid of the hdmi sub command

	@param			sCOMM_CMD* pstCmd	[in]				the pointer point to the comamnd
	@retval			COMM_CMD_VALID						the command is valid
						COMM_CMD_INVALID					the command is invalid
						COMM_CMD_INVALIDCMD				the command is invalid
						COMM_CMD_INVALIDOPT				the option is invalid
						COMM_CMD_INVALIDVAL				the value is invalid
						COMM_CMD_INVALIDRESERVE		the command is reserve
*/
/* -----------------------------------------------------------------------------*/
LONG lCOMM_CMD_CheckCmdVda(sCOMM_CMD* pstCmd);

/*------------------------------------------------------------------------------*/
/*!
	@brief				check the valid of the system sub command

	@param			sCOMM_CMD* pstCmd	[in]				the pointer point to the comamnd
	@retval			COMM_CMD_VALID						the command is valid
						COMM_CMD_INVALID					the command is invalid
						COMM_CMD_INVALIDCMD				the command is invalid
						COMM_CMD_INVALIDOPT				the option is invalid
						COMM_CMD_INVALIDVAL				the value is invalid
						COMM_CMD_INVALIDRESERVE		the command is reserve
*/
/* -----------------------------------------------------------------------------*/
LONG lCOMM_CMD_CheckCmdSystem(sCOMM_CMD* pstCmd);

/*------------------------------------------------------------------------------*/
/*!
	@brief				check the valid of the isp sub command

	@param			sCOMM_CMD* pstCmd	[in]				the pointer point to the comamnd
	@retval			COMM_CMD_VALID						the command is valid
						COMM_CMD_INVALID					the command is invalid
						COMM_CMD_INVALIDCMD				the command is invalid
						COMM_CMD_INVALIDOPT				the option is invalid
						COMM_CMD_INVALIDVAL				the value is invalid
						COMM_CMD_INVALIDRESERVE		the command is reserve
*/
/* -----------------------------------------------------------------------------*/
LONG lCOMM_CMD_CheckCmdIsp(sCOMM_CMD* pstCmd);

/*------------------------------------------------------------------------------*/
/*!
	@brief				check the valid of the rec sub command

	@param			sCOMM_CMD* pstCmd	[in]				the pointer point to the comamnd
	@retval			COMM_CMD_VALID						the command is valid
						COMM_CMD_INVALID					the command is invalid
						COMM_CMD_INVALIDCMD				the command is invalid
						COMM_CMD_INVALIDOPT				the option is invalid
						COMM_CMD_INVALIDVAL				the value is invalid
						COMM_CMD_INVALIDRESERVE		the command is reserve
*/
/* -----------------------------------------------------------------------------*/
LONG lCOMM_CMD_CheckCmdRec(sCOMM_CMD* pstCmd);

/*------------------------------------------------------------------------------*/
/*!
	@brief				check the valid of the eth sub command

	@param			LONG lReqId				[in]				the request info of the request
						CHAR** ppcCmds			[in]				the input commands, without head and tail
	@param			CHAR** ppcCmds			[out]			the output commands, with head and tail
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_CMD_AddCmdHeadTail(CHAR** ppcCmds, LONG lReqId);

#ifdef SHOW_INFO
/*------------------------------------------------------------------------------*/
/*!
	@brief				show the infomation of the command

	@param			sCOMM_CMD* pstCmd				[in]				the pointer point to the command
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_CMD_ShowCmd(sCOMM_CMD* pstCmd);
#endif /* SHOW_INFO */
/*------------------------------------------------------------------------------*/
/*!
	@brief				split the commands

	@param			CHAR *pcCmd						[in]				the pointer point to the commands to be split
															[in]				the split delimiter
	@param			CHAR **ppcCharArray			[out]			the split array
															[out]			the size of the split array
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_CMD_Split(CHAR *pcCmd,CHAR *pcDelim, CHAR **ppcCharArray, LONG *plArraySize);

void vCOMM_CMD_AddReqInfo(CHAR* pcReq, eCOMM_UI eUi);

#ifdef __cplusplus
}
#endif

#endif
