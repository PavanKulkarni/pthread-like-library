//#include "mutex.h"
#include "gtthread.h"
#include "ThreadQueue.h"

sigset_t signalMask;
extern Thread_Queue queue;


int  gtthread_mutex_init(gtthread_mutex_t *mutex){
	printf("\n mutex init \n");
	mutex->lock=0;
	mutex->owner =0;
	mutex->count=1;
}



int  gtthread_mutex_lock(gtthread_mutex_t *mutex){
	//printf("\n mutex lock called \n");
	sigemptyset(&signalMask);
    	sigaddset(&signalMask, SIGVTALRM);
    	sigprocmask(SIG_BLOCK,&signalMask, NULL);
	ThreadControlBlock_Ptr currentBlock = getCurrentBlock(queue);
	sigprocmask(SIG_UNBLOCK,&signalMask,NULL);
	if(mutex->owner ==0 && (mutex->owner != currentBlock->thread_id) && mutex->lock==0)
	{
		
		while(mutex->count<=0);
    		sigprocmask(SIG_BLOCK,&signalMask, NULL);
		mutex->count--;
		mutex->lock=1;
		mutex->owner = currentBlock->thread_id;
    		sigprocmask(SIG_UNBLOCK,&signalMask, NULL);
		return 0;		
	}
	else
	{	//sigprocmask(SIG_BLOCK,&signalMask,NULL);
		while(1)
		{	//printf("\n Spinning \n");
			if(mutex->owner==0)
				break;
		}
		//sigprocmask(SIG_UNBLOCK,&signalMask,NULL);
	}
	return 0;	
}




int  gtthread_mutex_unlock(gtthread_mutex_t *mutex){
	//printf("\n mutex unlock called \n");
	sigprocmask(SIG_BLOCK,&signalMask,NULL);
	ThreadControlBlock_Ptr currentThread = getCurrentBlock(queue);
		if(mutex->owner == currentThread->thread_id)
		{
			mutex->count++;
			mutex->lock=0;
			mutex->owner =0;
		}

	sigprocmask(SIG_UNBLOCK,&signalMask, NULL);
}
