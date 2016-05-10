#ifndef __BASE_INTERFACE_H__
#define __BASE_INTERFACE_H__

#include "comm_interface.h"
#ifdef __cplusplus
extern "C"{
#endif


/* condition struct */
sCOMM_CONDFLAG stCfBaseRet;
/* extern two pipes */
extern sCOMM_PIPE_COMMU gsPipeCommand;
extern sCOMM_PIPE_COMMU gsPipeFeedback;
/* extern parameter list */
extern sCOMM_PARA gsParas[ ];
/* extern sharememory */
extern sCOMM_SHAREMEM gsShareMem;
extern const LONG SM_OFFSET[];
/* SD path */
const CHAR* gpSDPath;

/*------------------------------------------------------------------------------*/
/*!
    @brief			AV message management
*/
/* -----------------------------------------------------------------------------*/
void vBASE_AVMsgManagement();

#ifdef __cplusplus
}
#endif

#endif /* End of #ifndef __INTERFACE_BASE_H__*/
