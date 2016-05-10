#ifdef __cplusplus
extern "C"{
#endif

#include "comm_conditionFlag.h"

void vCOMM_CF_InitCF( sCOMM_CONDFLAG * pCf, LONG lTimeout, CHAR* pcName){
	/* init condition flag */
	pthread_cond_init(&(pCf->condRet), NULL);
	pthread_cond_init(&(pCf->condId), NULL);
	pthread_cond_init(&(pCf->condIdBack), NULL);

	/* init mutex */
	pthread_mutex_init(&(pCf->lockRet), NULL);
	pthread_mutex_init(&(pCf->lockId), NULL);
	pthread_mutex_init(&(pCf->lockIdBack), NULL);

	/* set the id of pCf to invalid */
	pCf->lId=COMM_COND_CFIDINVALID;
	pCf->lTimeout=lTimeout;

	/* init the count */
	pCf->lCfNotifyCnt=0;
	pCf->lCfCurNoti=0;
	pCf->lCfCurState=COMM_COND_NOTWRITE;

	/* init name */
	pCf->pcName=strdup(pcName);
}

void vCOMM_CF_RegCF( sCOMM_CONDFLAG * psCf ){

	/* update the count according to the type of input argument */
	pthread_mutex_lock(&psCf->lockIdBack);
	psCf->lCfNotifyCnt++;
	pthread_mutex_unlock(&psCf->lockIdBack);

}


void vCOMM_CF_DeregCF( sCOMM_CONDFLAG * psCf){

	/* update the count according to the type of the cf */
	pthread_mutex_lock(&psCf->lockIdBack);
	psCf->lCfNotifyCnt--;
	pthread_mutex_unlock(&psCf->lockIdBack);

}

void vCOMM_CF_RdCF( sCOMM_CONDFLAG * pCf, LONG* content){
	LONG lRet=0;
	struct timespec timeout;

	/* get the lock */
	pthread_mutex_lock (&(pCf->lockRet));

	if(COMM_COND_WRITED!=pCf->lCfCurState){
		/* get time expire */
		vCOMM_CF_GetTimeoutCF( &timeout, pCf->lTimeout);
		
		/* start to wait */
		lRet=pthread_cond_timedwait( &(pCf->condRet), &(pCf->lockRet), &timeout);
		if(lRet==0){
			/* get the feed back */
			*content=pCf->lRet;
		}else{
			/* failure */
			*content=COMM_COND_CFSYSRETFAIL;
			printf("cf <%s> : time out to read\n", pCf->pcName);
		}
	} else {
		*content=pCf->lRet;
	}

	/* release the lock */
	pthread_mutex_unlock (&(pCf->lockRet));

}

void vCOMM_CF_WtCF( sCOMM_CONDFLAG * pCf, LONG content){
	/* get lock */
	pthread_mutex_lock(&(pCf->lockRet));
	
	/* write back the feed back and toggle dirty flag */
	pCf->lRet=content;

	/* notified the read thread */
	pthread_cond_broadcast(&(pCf->condRet));

	/* update current state */
	pCf->lCfCurState=COMM_COND_WRITED;

	/* release the lock */
	pthread_mutex_unlock(&(pCf->lockRet));
}

void vCOMM_CF_SetIdCF( sCOMM_CONDFLAG * pCf, LONG lId, LONG lType ){

	/* if no block type, wait all the getters when need */
	if( COMM_COND_NOBLOCK != lType ){
		if(pCf->lCfNotifyCnt!=pCf->lCfCurNoti){
			debug_info("warning, <%s> competition to set id, but to be continue ..\n", pCf->pcName);
			pthread_mutex_lock(&(pCf->lockIdBack));
			pthread_cond_wait(&(pCf->condIdBack), &(pCf->lockIdBack));
			pthread_mutex_unlock(&(pCf->lockIdBack));
		}
	}

	/* get lock */
	pthread_mutex_lock(&(pCf->lockId));
	
	/* set pCf id */
	pCf->lId=lId;
	
	/* update the current state to not write */
	pCf->lCfCurState=COMM_COND_NOTWRITE;
	
	if( COMM_COND_NOBLOCK != lType ){
		/* block case */
		pthread_cond_broadcast(&(pCf->condId));

		/* reset current count */
		pCf->lCfCurNoti=0;

	}
	/* release the lock */
	pthread_mutex_unlock(&(pCf->lockId));

}
void vCOMM_CF_GetIdCF( sCOMM_CONDFLAG * pCf, LONG* lId, LONG lType ){
	/* get lock */
	pthread_mutex_lock(&(pCf->lockId));

	/* get pCf id */
	if( COMM_COND_NOBLOCK != lType ){
		if(pCf->lCfNotifyCnt==++pCf->lCfCurNoti){
			pthread_cond_signal(&(pCf->condIdBack));
		}
		pthread_cond_wait(&(pCf->condId), &(pCf->lockId));
	}

	/* get the id */
	*lId=pCf->lId;

	/* release the lock */
	pthread_mutex_unlock(&(pCf->lockId));
}

void vCOMM_CF_DeInitCF( sCOMM_CONDFLAG * pCf){
	/* free the name */
	free(pCf->pcName);

	/* destroy condition flag */
	pthread_cond_destroy(&(pCf->condRet)); 
	pthread_cond_destroy(&(pCf->condId));
	pthread_cond_destroy(&(pCf->condIdBack));

	/* destroy mutex */
	pthread_mutex_destroy(&(pCf->lockRet));
	pthread_mutex_destroy(&(pCf->lockId));
	pthread_mutex_destroy(&(pCf->lockIdBack));

}

void vCOMM_CF_GetTimeoutCF(struct timespec *tsp, long secs){

	struct timeval sNow;
	struct timezone tz;
	/* get the current time */
	gettimeofday(&sNow,&tz);

	/* make time out */
	tsp->tv_sec=sNow.tv_sec;
	tsp->tv_nsec=sNow.tv_usec*1000;

	/* add the offset to get timeout value */
	tsp->tv_sec += secs;

}

#ifdef __cplusplus
}
#endif
