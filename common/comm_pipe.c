#ifdef __cplusplus
extern "C"{
#endif

#include "comm_pipe.h"

void vCOMM_Pipe_Create(sCOMM_PIPE_COMMU* pc, CHAR* cName){
	LONG lRet=0;

	lRet=pipe ((int*)pc->lFd);
	if(lRet!=0){
		debug_info ("pipe create error\n");
		exit(-1);
	}

	/* pipe name */
	pc->cName=strdup(cName);
	
}

void vCOMM_Pipe_Init(sCOMM_PIPE_COMMU* pc, LONG lFlag){
	sCOMM_PIPE_COMMU* pipec=pc;

	if(lFlag==COMM_PIPE_WRITE){
		/* close reader fd */
		close(COMM_PIPE_GET_PIPERD(pipec->lFd));
		/* set flag */
		pipec->lFlag=COMM_PIPE_WRITE;
		/* open stream */
		pipec->pStream=fdopen(COMM_PIPE_GET_PIPEWT(pipec->lFd),"w");
		debug_info ("write pipe ready to work\n");
	} else if ( lFlag== COMM_PIPE_READ ){
		close(COMM_PIPE_GET_PIPEWT(pipec->lFd));
		/* set flag */
		pipec->lFlag=COMM_PIPE_READ;
		/* open stream */
		pipec->pStream=fdopen(COMM_PIPE_GET_PIPERD(pipec->lFd),"r");
		debug_info("read pipe ready to work\n");
	} else {
		debug_info ("pipe : wrong parameter!\n");
		exit(-1);
	}
	
}

void vCOMM_Pipe_Read(sCOMM_PIPE_COMMU* pc, CHAR* content){
	CHAR* pcRet;
	FILE* pStream=pc->pStream;
	pcRet=fgets (content, COMM_PIPE_MAXCHAR, pStream);
	if(!pcRet){
		/* check the valid */
		debug_info("%s : error to pipe read\n", pc->cName);	
		return;
	}
	content[strlen(content)-1]=0;
	debug_info("%s read %s\n", pc->cName, content);
	return;
}

void vCOMM_Pipe_Write(sCOMM_PIPE_COMMU* pc, CHAR* content){
	FILE* pStream=pc->pStream;
	fprintf (pStream, "%s\n", content);
	fflush (pStream);
	debug_info("%s write %s\n", pc->cName, content);
}

void vCOMM_Pipe_DeInit(sCOMM_PIPE_COMMU* pc){
	FILE* pStream=pc->pStream;
	LONG lFlag=pc->lFlag;

	/* close the stream */
	fclose(pStream);

	/* free name */
	free(pc->cName);

	/* close file description */
	if(lFlag==COMM_PIPE_WRITE){
		close(COMM_PIPE_GET_PIPEWT(pc->lFd));
		debug_info ("write pipe close\n");
	} else if ( lFlag== COMM_PIPE_READ ){
		close(COMM_PIPE_GET_PIPERD(pc->lFd));
		debug_info("read pipe close\n");
	}
}

#ifdef SHOW_INFO
void vCOMM_Pipe_Show(sCOMM_PIPE_COMMU* pc){
	printf("\nThe info of the pipe [ %s ] :\n", pc->cName);
	printf("flag\t%ld\n", pc->lFlag);
	printf("lFd\t<%ld,%ld>\n", pc->lFd[COMM_PIPE_READ], pc->lFd[COMM_PIPE_WRITE]);
	printf("stream\t%p\n", pc->pStream);
}
#endif /* SHOW_INFO */


#ifdef __cplusplus
}
#endif


