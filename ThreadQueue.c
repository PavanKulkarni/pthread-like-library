/*
 * ThreadQueue.c
 *
 *  Created on: Jan 28, 2013
 *      Author: pavan
 */
#ifndef NULL
#define NULL   ((void *) 0)
#endif
#include<ucontext.h>
#include "ThreadQueue.h"


ThreadControlBlock_Ptr getControlBlock_Main(){
	ThreadControlBlock_Ptr ControlBlock = (ThreadControlBlock_Ptr)malloc(sizeof(ThreadControlBlock));
	ControlBlock->thread_context.uc_stack.ss_flags = 0;
	ControlBlock->thread_context.uc_link =0;
    ControlBlock->isActive =0;
    ControlBlock->isBlocked =0;
    ControlBlock->isExecuted =0;
    ControlBlock->isMain =1 ;
    ControlBlock->next = NULL ;

    return ControlBlock;

}

ThreadControlBlock_Ptr getControlBlock(){
	ThreadControlBlock_Ptr ControlBlock = (ThreadControlBlock_Ptr)malloc(sizeof(ThreadControlBlock_Ptr));
	ControlBlock->thread_context.uc_stack.ss_sp = malloc(STACKSIZE);
	ControlBlock->thread_context.uc_stack.ss_size = STACKSIZE;
	ControlBlock->thread_context.uc_stack.ss_flags = 0;
		ControlBlock->thread_context.uc_link =0;
	    ControlBlock->isActive =0;
	    ControlBlock->isBlocked =0;
	    ControlBlock->isExecuted =0;
	    ControlBlock->isMain =0 ;
	    ControlBlock->next = NULL ;

	    return ControlBlock;

}

int enqueue(Thread_Queue queue,ThreadControlBlock_Ptr tcb){

	//check if queue or tcb is null 
	printf("Enqueing the thread\n");
	 	
		if(queue->head == NULL)
		{
			//this is the first node 
			printf("\nThis is first node\n");
			tcb->next= tcb;
			queue->head =tcb;
			queue->tail=tcb;
		}
		else
		{	printf("else here\n");
			tcb->next =queue->head; //inserts tcb behinf the head in a circular queue
			queue->tail->next= tcb; //the existing tail should point to this tcb
			queue->tail =tcb; //the tail is the new tcb hence update it
		}
		queue ->count ++;

	

	return 0;
}


int dequeue(Thread_Queue queue){

	if(queue == NULL)
		return -1;
	else
	{	//printf("\ndequeing blocks");
		ThreadControlBlock_Ptr head,tail,temp;
		head = queue -> head;
		tail = queue -> tail;
		

		if(head != NULL)
		{
			temp = queue->head->next; //removing the head hence storing next block address in temp
			if(queue ->count ==1)
			{	
				queue->head = queue->tail= NULL; 
			}
			else
			{	//printf("\n queue has more than 1 elements hence dequeing");
				queue->head=temp; //temp is next block which is new head
				tail->next=queue->head;  //tail next block is new head
			}
		freeControlBlock(head); //free the old head
		//printf("\nFreed a block on queue");
		queue->count--;
		}
		else
		{
			return 0;
		}

	}
	return 0;
}

void freeControlBlock(ThreadControlBlock_Ptr controlBlock){
	if(!(controlBlock->isMain)){
		free(controlBlock->thread_context.uc_stack.ss_sp);
	}
	free(controlBlock);
}

int next(Thread_Queue queue){

	if(queue!= NULL)
	{       printf("Stuck here\n");
		ThreadControlBlock_Ptr current = queue -> head;
		if(current != NULL)
		{
		 queue->tail = current;
		 queue->head=current->next;
		 }
	}
printf("\n Returning fromm next");
return 0;
}

ThreadControlBlock_Ptr getCurrentBlock(Thread_Queue queue){

	if(queue !=NULL && queue->head != NULL)
	{	//printf("\n Returning CurrentBlock\n");
		return queue->head;
	}
	return NULL;
}

ThreadControlBlock_Ptr getCurrentBlockByThread(Thread_Queue queue,gtthread_t threadid){
	ThreadControlBlock_Ptr headBlock = getCurrentBlock(queue);
	//if this is the required node
	if(headBlock!=NULL && headBlock->thread_id == threadid)
	{
		return headBlock;
	}
	ThreadControlBlock_Ptr dummyThread=NULL;
	if(headBlock!=NULL)
	{
		dummyThread = headBlock->next;
	}
	
	while((headBlock != dummyThread))
	{
		if(dummyThread ->thread_id == threadid)
			return dummyThread;
		
		dummyThread = dummyThread->next;
	}
	return NULL;
}

int getQueueSize(Thread_Queue queue){
	
	return queue->count;
}

Thread_Queue getQueue(){

	Thread_Queue queue = (Thread_Queue)malloc(sizeof(struct ThreadQueue));
	queue->count=0;
	queue->head=queue->tail= NULL;
	return queue;
}

int enqueueToCompletedList(FinishedQueue queue,FinishedThread_Ptr finishedThread ){
	if(queue != NULL && finishedThread !=NULL)
	{
		finishedThread->next=queue->thread;
		queue->thread = finishedThread;
	}
	return 0;
}


FinishedThread_Ptr getFinishedThread(FinishedQueue queue,gtthread_t thread_id,int flag){

	if(queue!=NULL)
	{

		FinishedThread_Ptr thread= queue->thread;
		FinishedThread_Ptr previous_thread = NULL;
		while((thread!=NULL)&& (thread->thread_id!=thread_id))
		{
			previous_thread =thread;
			thread = thread ->next;
		}
		if(flag && thread!=NULL)
		{
			if(previous_thread == NULL)
				queue->thread  = thread->next;
			else
				previous_thread->next = thread->next;
		}
		return thread;
	}

	return NULL;
}

BlockedThreadList_Ptr getBlockedThreadList(){

BlockedThreadList_Ptr newList = (BlockedThreadList_Ptr)malloc(sizeof(struct BlockedThreadList));
	if(newList!=NULL){
		newList->thread=NULL;
		newList->next=NULL;
	}
	return newList;
}

int addToBlockedThreadList(ThreadControlBlock_Ptr fromNode,ThreadControlBlock_Ptr toNode ){
	
	BlockedThreadList_Ptr list =getBlockedThreadList();
	if(fromNode != NULL)
	{
		list->thread = toNode;
		list->next = fromNode->blockedThreads;
		fromNode->blockedThreads = list;
		toNode->isBlocked=1;
	}
	return 0;
}

FinishedThread_Ptr getCompletedThread(){
	FinishedThread_Ptr finishedThread = (FinishedThread_Ptr)malloc(sizeof(struct FinishedThread));
	if(finishedThread == NULL)
	{
		return NULL;
	}
	finishedThread->returnValue=(void**)malloc(sizeof(void*));
	if(finishedThread->returnValue ==NULL)
	{
		free(finishedThread);
		return NULL;
	}
	finishedThread->thread_id= -1;
	*(finishedThread->returnValue)= NULL;
	finishedThread->next =NULL;
	return finishedThread;
}


FinishedQueue getFinishedQueue(){
	FinishedQueue finishedQueue = (FinishedQueue)malloc(sizeof(struct FinishedControlBlockQueue));
	finishedQueue->thread = NULL;
	finishedQueue->count = 0;
	return finishedQueue;
}
