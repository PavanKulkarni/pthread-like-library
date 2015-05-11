/*
 * gtthread.c
 *
 *  Created on: Jan 28, 2013
 *      Author: pavan
 */
#include <ucontext.h>
#include <malloc.h>
#include <sys/time.h>
#include "gtthread.h"
#include "ThreadQueue.h"
//#include "mutex.h"
#include<string.h>
#define null ((void *)0)
#define STACKSIZE 1024*8
#define MAX_THREADS 1

//All declarations
typedef struct {
	gtthread_t thread_id;
	ucontext_t thread_context;
	int active;
	int executed;
	int exited;
	int isMain;
	void *retvalue;
}thread;

//static ucontext_t main_context;
static int threadid;
extern ucontext_t common_context;
struct sigaction scheduler_interrupt_handler;
struct itimerval timeslice;
sigset_t signalMask;
void scheduler();
void *helper(void *(*start_routine)(void*),void *arg);
Thread_Queue queue = NULL;
FinishedQueue finishedQueue =NULL;
ThreadControlBlock_Ptr getCurrentControlBlock_Safe();
long millisec;
//The init method
void gtthread_init(long period)
{
	
	threadid=1;
	sigemptyset(&signalMask);
	sigaddset(&signalMask, SIGVTALRM);
	//intializing the context of the scheduler
	finishedQueue = getFinishedQueue();
	queue = getQueue();
	millisec=period;
	ThreadControlBlock_Ptr MainThread = getControlBlock_Main();
	//getcontext(&(MainThread->thread_context));
	//printf("in init \n");
	getCommonContext();
	MainThread->thread_context.uc_link = &common_context;
	MainThread->thread_id = threadid;
	enqueue(queue,MainThread);
	memset(&scheduler_interrupt_handler, 0, sizeof (scheduler_interrupt_handler));
	scheduler_interrupt_handler.sa_handler= &scheduler;
	sigaction(SIGVTALRM,&scheduler_interrupt_handler,NULL);
	millisec =period;
	timeslice.it_value.tv_sec = 0;
	timeslice.it_interval.tv_sec = 0;
	timeslice.it_value.tv_usec = millisec; // timer start decrementing from here to 0
	timeslice.it_interval.tv_usec = millisec; //timer after 0 resets to this value
	setitimer(ITIMER_VIRTUAL, &timeslice, NULL);
	//printf("Exiting init");
}



int gtthread_create(gtthread_t *thread,void *(*start_routine)(void *), void *arg){

long int temp;
//block everytime when a certain part needs to be done atomically
  if(queue!= NULL)
	{

  	//printf("\n in create ");
  	sigprocmask(SIG_BLOCK,&signalMask,NULL);
  	ThreadControlBlock_Ptr  threadCB= getControlBlock_Main();
        getcontext((&threadCB->thread_context));
        threadCB->thread_context.uc_stack.ss_sp=malloc(STACKSIZE);
	threadCB->thread_context.uc_stack.ss_size=STACKSIZE;
	threadCB->thread_context.uc_stack.ss_flags=0;
	threadCB->isMain=0;	
	threadCB->thread_context.uc_link = &common_context;
	//temp =rand();
	threadCB->thread_id= ++threadid;
        *thread = threadCB->thread_id;
        
	makecontext(&(threadCB->thread_context),(void (*)(void))&helper,2,start_routine,arg);
	
  	//printf("Thread is created %ld\n", *thread);
	enqueue(queue,threadCB);
	sigprocmask(SIG_UNBLOCK, &signalMask, NULL);
  	sigemptyset(&(threadCB->thread_context.uc_sigmask));	
	return 0;
	}
	printf("\n Thread not initialized: Did you miss calling gtthread_init() before create ??? \n ");
	return 0;
}


void *helper(void *(*start_routine)(void*), void *arg){

	void *returnValue;
	ThreadControlBlock_Ptr currentThread = getCurrentControlBlock_Safe();
	//printf("In Helper");
	returnValue = (*start_routine)(arg);
	sigprocmask(SIG_BLOCK,&signalMask,NULL);
	FinishedThread_Ptr finishedThread = getCompletedThread();
	if(finishedThread != NULL)
	{  
		*(finishedThread->returnValue) = returnValue;
		finishedThread->thread_id = currentThread->thread_id;
		enqueueToCompletedList(finishedQueue,finishedThread);
	}
	sigprocmask(SIG_UNBLOCK,&signalMask,NULL);
	return returnValue;
	//Todo set this value to the completed nodes return value
}	


ThreadControlBlock_Ptr getCurrentControlBlock_Safe(){

ThreadControlBlock_Ptr currentControlBlock =null;
sigprocmask(SIG_BLOCK,&signalMask,null);
currentControlBlock= getCurrentBlock(queue);
sigprocmask(SIG_UNBLOCK,&signalMask,null);
return currentControlBlock;

}

int gtthread_join(gtthread_t thread,void **status){

	//printf("\n*********Join Called*****\n");
	//block signal
	sigprocmask(SIG_BLOCK,&signalMask,NULL);
	ThreadControlBlock_Ptr callingThread = getCurrentBlock(queue);
	ThreadControlBlock_Ptr joinThread = getCurrentBlockByThread(queue,thread);
	
	//check if callingthread is blocking on itself or is null
	if(callingThread ==NULL || callingThread == joinThread)
	{       
		//printf("\n 100.1 \n");
		sigprocmask(SIG_UNBLOCK,&signalMask,NULL);	
		return -1;
	}
	if(joinThread==NULL)
	{       //printf("\n 100.2 \n");
		//The thread is finished hence can be found in finished Queue
		FinishedThread_Ptr finishedThread = getFinishedThread(finishedQueue,thread,1);
		sigprocmask(SIG_UNBLOCK,&signalMask,NULL);
		if(finishedThread)
		{  //printf("\n100.3\n");
			if(status)
			{	//printf("\n 100.4 assigning value\n");
				*status =*(finishedThread->returnValue);
			}
			free(finishedThread);
			return 0;
		}
		else
			return -1;
			
	}
		
	//printf("\n Value is %d :",(joinThread->blockedThreads==NULL));
	if(joinThread->blockedThreads==NULL)
	{
		//Todo
		//printf("\n100.5\n");
		addToBlockedThreadList(joinThread,callingThread);
		int isBlocked=callingThread->isBlocked;
		sigprocmask(SIG_UNBLOCK,&signalMask,NULL);
		while(isBlocked){
			isBlocked=callingThread->isBlocked;
		}
		sigprocmask(SIG_BLOCK,&signalMask,NULL);
		FinishedThread_Ptr finishedThread = getFinishedThread(finishedQueue,thread,1);
		sigprocmask(SIG_UNBLOCK,&signalMask,NULL);
		//printf("\n 100.51 \n");
		if(finishedThread !=NULL && status !=NULL)
		{	//printf("\n 100.6 \n");
                       if(status){
			*status=*(finishedThread->returnValue);
			}
		free(finishedThread);
		}
		return 0;
	}else{
		//printf("\n 100.7 \n");
		sigprocmask(SIG_UNBLOCK,&signalMask,NULL);
		return -1;
	      }	
}

int gtthread_equal(gtthread_t thread1,gtthread_t thread2){
	//printf("\n**********Equal called******\n");
	return thread1==thread2;
}

gtthread_t gtthread_self(){
	
	//printf("\n**********Self called******\n");
		sigprocmask(SIG_BLOCK,&signalMask,NULL);
		ThreadControlBlock_Ptr currentThread = getCurrentBlock(queue);
		if(currentThread==NULL)
		{
			sigprocmask(SIG_UNBLOCK,&signalMask,NULL);
			return -1;
		}
		sigprocmask(SIG_UNBLOCK,&signalMask,NULL);
		return currentThread->thread_id;
}

int gtthread_cancel(gtthread_t thread){

	//printf("\n**********Cancel called******\n");
	sigprocmask(SIG_BLOCK,&signalMask,NULL);
	ThreadControlBlock_Ptr currentThread = getCurrentBlock(queue);
	if((currentThread != NULL) &&(currentThread->thread_id != thread))
	{
		ThreadControlBlock_Ptr cancelThread= getCurrentBlockByThread(queue,thread);
		if(cancelThread!= NULL){
			BlockedThreadList_Ptr blockedThread = cancelThread->blockedThreads;

			while(blockedThread != NULL)
			{
				blockedThread->thread->isBlocked=0;
				blockedThread=blockedThread->next;
			}
			cancelThread->isExecuted=1;
			printf("Cancelled Thread : %ld \n",cancelThread->thread_id);
			dequeue(queue);	
			sigprocmask(SIG_UNBLOCK,&signalMask,NULL);
			return 0;
		}
		sigprocmask(SIG_UNBLOCK,&signalMask,NULL);
		return -1;
	}
	sigprocmask(SIG_UNBLOCK,&signalMask,NULL);
	return -1;
}

void gtthread_exit(void *retval){
	
	printf("\n**********Exit called******\n");
	sigprocmask(SIG_BLOCK,&signalMask,NULL);
	ThreadControlBlock_Ptr currentThread= getCurrentBlock(queue);
	FinishedThread_Ptr finishedThread = getCompletedThread();
	if(finishedThread !=NULL && currentThread != NULL)
	{
		*(finishedThread->returnValue) = retval;
		finishedThread->thread_id = currentThread->thread_id;
		enqueueToCompletedList(finishedQueue,finishedThread);
	}
	threadCompleted();
	sigprocmask(SIG_UNBLOCK,&signalMask,NULL);
	raise(SIGVTALRM);

}

void gtthread_yield(void){

	printf("\n**********Yield called******\n");
raise(SIGVTALRM);
}


