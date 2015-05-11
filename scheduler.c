#include "gtthread.h"
#include "ThreadQueue.h"
extern Thread_Queue queue;
extern struct itimerval timeslice;
extern long millisec;
extern sigset_t signalMask;
ucontext_t common_context;
ThreadControlBlock_Ptr getCurrentControlBlock_Safe();


void scheduler(int signal){
	//printf("\n\t ***************************\n\t");
	//printf("\n\t ********* Scheduler *******\n\t");
	//printf("\n\t ***************************\n\t"); 
	//printf("\n Time period is %ld",millisec);
	sigprocmask(SIG_BLOCK,&signalMask,NULL);
	//printf("The Queue size is %d \n",getQueueSize(queue));	
	//handle what if only one node is present in the ready queue
	if(getQueueSize(queue)==1)
	{	//printf("\n\t First entry in the queue \n");
		ThreadControlBlock_Ptr CurrentControlBlock = getCurrentBlock(queue);
		if(CurrentControlBlock->isExecuted)
		{
			dequeue(queue);
			sigprocmask(SIG_UNBLOCK,&signalMask,NULL);
			exit(0);
		}

	}
	//if more nodes are present
	else if(getQueueSize(queue) > 1)
	{	//printf("\n\tmore entries in the queue \n");
		ThreadControlBlock_Ptr CurrentControlBlock = getCurrentBlock(queue);
		int flag=0;
		//printf("\n\t Current ControlBlocks id %ld : ",CurrentControlBlock->thread_id);
		if(CurrentControlBlock != NULL)
		{	//printf("\n 1\n");
			ThreadControlBlock_Ptr nextBlock;
			if(CurrentControlBlock->isExecuted)
			{	//printf("\n 1.1 \n");
				flag=1;
				dequeue(queue);
			}
			else
			{	//printf("\n 1.2 Moving to the next block in the queue \n");
				next(queue);
			}
		 	//printf("\n pavan ");	
			nextBlock = getCurrentBlock(queue);
			//printf("\n pavan here");
			//printf("\n\t NextBlocks id %ld : ",nextBlock->thread_id);
			while((nextBlock!= NULL ) && (nextBlock->isBlocked || nextBlock->isExecuted))
			{	printf("\n 2\n");
				if(nextBlock->isExecuted)
					dequeue(queue);
				else
					next(queue);
				
				nextBlock=getCurrentBlock(queue);					
			}//while ends
	        	
			if(nextBlock == NULL)
			{	//printf("\n 3 nextBlock is null\n");
				sigprocmask(SIG_UNBLOCK,&signalMask,NULL);
				exit(0);
			}
			
			if(nextBlock != CurrentControlBlock)
			{	//printf("\n4 Thread ID %ld,\n",nextBlock->thread_id);
				
				//timeslice.it_value.tv_usec = 0;
				//setitimer(ITIMER_VIRTUAL,&timeslice,NULL);
				sigprocmask(SIG_UNBLOCK,&signalMask,NULL);//The signals in this context would have been blocked hence unblock	
				if(flag)
				{	
					//printf("\n\n Current Node has completed hence setting nextblocks context \n");
					setcontext(&(nextBlock->thread_context));
					
				}
				else
				{	//printf("\n Swapping context from %ld and %ld\n",CurrentControlBlock->thread_id,nextBlock->thread_id);
					
						//printf("\n ***********Swapping context from %ld and %ld\n",CurrentControlBlock->thread_id,nextBlock->thread_id);
						swapcontext(&(CurrentControlBlock->thread_context),&(nextBlock->thread_context));
						//printf("\n Done swapping \n");
				}
			}
		 }

	}

	sigprocmask(SIG_UNBLOCK,&signalMask,NULL);
	
}


void threadCompleted(){

	ThreadControlBlock_Ptr currentNode = getCurrentControlBlock_Safe();
	BlockedThreadList_Ptr blockedThread = currentNode->blockedThreads;

	while(blockedThread != NULL)
	{
		blockedThread->thread->isBlocked =0;
		blockedThread = blockedThread->next;
	}
	
	//printf("\n Thread completed : %ld",currentNode->thread_id );
	currentNode->isExecuted=1;
	raise(SIGVTALRM);
	
}

ucontext_t getCommonContext()
{
	static contextAlreadySet=0;
	if(!contextAlreadySet)
	{
		getcontext(&common_context);
		common_context.uc_link = 0;
        	common_context.uc_stack.ss_sp = malloc(STACKSIZE);
        	common_context.uc_stack.ss_size = STACKSIZE;
        	common_context.uc_stack.ss_flags= 0;
        	makecontext( &common_context, (void (*) (void))&threadCompleted, 0);
        contextAlreadySet = 1;
	}
}



