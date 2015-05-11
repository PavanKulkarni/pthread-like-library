/*
 * ThreadQueue.h
 *
 *  Created on: Jan 28, 2013
 *      Author: pavan
 */

#ifndef THREADQUEUE_H_
#define THREADQUEUE_H_
//#define STACKSIZE 1024*8
#include<ucontext.h>
#include "gtthread.h"
typedef struct Thread {

	gtthread_t thread_id;
	ucontext_t thread_context;
	int isActive;
	int isExecuted;
	int isBlocked;
	int isMain;
	struct Thread *next;
 	struct BlockedThreadList *blockedThreads;	
}ThreadControlBlock,*ThreadControlBlock_Ptr;


typedef struct ThreadWaitingList{
	ThreadControlBlock_Ptr thread;
	struct ThreadWaitingList *next;
}*ThreadWaitingList;

typedef struct ThreadQueue{
	ThreadControlBlock_Ptr head;
	ThreadControlBlock_Ptr tail;
	long count;
}*Thread_Queue;

typedef struct BlockedThreadList{
	ThreadControlBlock_Ptr thread;
	struct BlockedThreadList *next;
}*BlockedThreadList_Ptr;

typedef struct FinishedThread{
	gtthread_t thread_id;
	void **returnValue;
	struct FinishedThread *next;
}*FinishedThread_Ptr;

typedef struct FinishedControlBlockQueue{
	struct FinishedThread *thread;
	long count;
}*FinishedQueue;

ThreadControlBlock_Ptr getControlBlock_Main();
ThreadControlBlock_Ptr getControlBlock();
ThreadControlBlock_Ptr getCurrentBlockByThread(Thread_Queue,gtthread_t);
ThreadControlBlock_Ptr getCurrentBlock(Thread_Queue queue);
int getQueueSize(Thread_Queue queue);
Thread_Queue getQueue();
void freeControlBlock(ThreadControlBlock_Ptr);
int next(Thread_Queue);
int enqueueToCompletedList(FinishedQueue,FinishedThread_Ptr);
FinishedThread_Ptr getFinishedThread(FinishedQueue,gtthread_t,int);
BlockedThreadList_Ptr getBlockedThreadList();
int addToBlockedThreadList(ThreadControlBlock_Ptr,ThreadControlBlock_Ptr);
FinishedThread_Ptr getCompletedThread();
FinishedQueue getFinishedQueue();
#endif /* THREADQUEUE_H_ */

