/*
 * gtthread.h
 *
 *  Created on: Jan 28, 2013
 *      Author: pavan
 */

#ifndef GTTHREAD_H_
#define GTTHREAD_H_

#include<stdio.h>
typedef unsigned long int gtthread_t;
#define STACKSIZE 1024*8
#define MAXTHREADS 20
#include <ucontext.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <malloc.h>
//#include "mutex.h"
//#include "ThreadQueue.h"
//#include "scheduler.c"
void gtthread_init(long period);
int  gtthread_create(gtthread_t *thread,
                     void *(*start_routine)(void *),
                     void *arg);
int  gtthread_join(gtthread_t thread, void **status);
void gtthread_exit(void *retval);
void gtthread_yield(void);
int  gtthread_equal(gtthread_t t1, gtthread_t t2);
int  gtthread_cancel(gtthread_t thread);
gtthread_t gtthread_self(void);



typedef struct{
        int lock;
        int count;
        volatile gtthread_t owner;
}gtthread_mutex_t;

int  gtthread_mutex_init(gtthread_mutex_t *mutex);
int  gtthread_mutex_lock(gtthread_mutex_t *mutex);
int  gtthread_mutex_unlock(gtthread_mutex_t *mutex);
#endif /* GTTHREAD_H_ */

