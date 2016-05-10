/* include */
#include "ctrl/ctrl_interface.h" 
#include "base/base_interface.h"
#include "common/comm_interface.h"

#ifdef __cplusplus
extern "C"{
#endif


/* user setting */
const CHAR* gpSDPath="/opt/mainCard/";

/* global two pipes */
sCOMM_PIPE_COMMU gsPipeCommand;
sCOMM_PIPE_COMMU gsPipeFeedback;

/* global share memory */
sCOMM_SHAREMEM gsShareMem;

/* global parameter list */
 sCOMM_PARA gsParas[ COMM_MAXPARACNT ];

/* share memory offset */
const LONG SM_OFFSET[]={
	0,			// string pool : 0KB -> 4KB
	4 * 1024,	// reserve
};

int main(int argc, CHAR* argv[]){
	
	/* pid */
	pid_t basepid;
	 sCOMM_PARA* pParas=gsParas;

	 debug_show("main start\n");
	
	/* create pipe */
	vCOMM_Pipe_Create(&gsPipeCommand, "commandPipe");
	vCOMM_Pipe_Create(&gsPipeFeedback, "feedbackPipe");

	/* create share memory */
	vCOMM_ShareMem_Create(&gsShareMem, "shareMem", COMM_SM_SIZE);

	/* init parameter list */
	vCOMM_PARA_InitParas( & pParas );

	/* global parameter judge */
	memset(&gsTerminalClient, 0,sizeof(gsTerminalClient));
	if(argc==3){
		if( !strcmp(*++argv,"-f")) {
			gsTerminalClient.lCfgType=CTRL_CMD_NEEDREAD;
			strcpy(gsTerminalClient.cCfgName, *++argv);
			printf("config file name : %s\n", gsTerminalClient.cCfgName);
		} else {
			printf("not support now\n");
			exit(EXIT_FAILURE);
		}
	}

	/* fork */
	basepid = fork();
	if( basepid == -1 ){
		printf("fork error!\n");
		exit(EXIT_FAILURE);
	}
	
	if (basepid == (pid_t) 0) {
		debug_show("Base Process start\n");
		/* init pipe */
		vCOMM_Pipe_Init (&gsPipeCommand, COMM_PIPE_READ);
		vCOMM_Pipe_Init (&gsPipeFeedback, COMM_PIPE_WRITE);

		/* attach share memory */
		vCOMM_ShareMem_Attach(&gsShareMem);
		COMM_MW_StrPool_Init(COMM_GET_STRPOOL(), COMM_SP_SIZE);
		
		/* base process */
		vBASE_AVMsgManagement();

		/* deinit pipe */
		vCOMM_Pipe_DeInit (&gsPipeCommand);
		vCOMM_Pipe_DeInit (&gsPipeFeedback);

		/* detach share memory */
		vCOMM_ShareMem_Detach(&gsShareMem);

		/* child process terminate */
		debug_show("Base Process terminate\n");
		_exit(EXIT_SUCCESS);

	} else {
		/* father process */
		debug_show("Control Process start\n");
		/* init pipe */
		vCOMM_Pipe_Init (&gsPipeCommand, COMM_PIPE_WRITE);
		vCOMM_Pipe_Init (&gsPipeFeedback, COMM_PIPE_READ);

		/* attach share memory */
		vCOMM_ShareMem_Attach(&gsShareMem);
		
		/* father process, i.e. control process */
		vCTRL_InteractiveCtl();

		/* deinit pipe */
		vCOMM_Pipe_DeInit (&gsPipeCommand);
		vCOMM_Pipe_DeInit (&gsPipeFeedback);

		/* detach share memory */
		vCOMM_ShareMem_Detach(&gsShareMem);
	}

	/* wait child terminate */
	waitpid(basepid, NULL, 0);
	
	/* destroy share memory */
	vCOMM_ShareMem_Destroy(&gsShareMem);

	/* write the parameter list back to the cfg file */
	vCOMM_PARA_WriteParasBack(gsParas);

	debug_show("Control Process stop\n");

	/* halt the system */
	system("sync; sync; halt");

	return 0;
}

#ifdef __cplusplus
}
#endif

