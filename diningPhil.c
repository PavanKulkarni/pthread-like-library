#include <gtthread.h>
#include "mutex.h"

#define N 5
#define THINKING 0
#define HUNGRY 1
#define EATING 2
#define LEFT (i+N-1)%N
#define RIGHT (i+1)%N
int state[N];
gtthread_mutex_t mutex;
int semaphore[N];
int philosopher[N] ={0,1,2,3,4};
int chopsticks[N];
int i,k;
void init();
void getSticks(int );
void releaseSticks(int);
void test(int);


void init(){
	gtthread_init(1000L);
	gtthread_mutex_init(&mutex);
	for(i=0;i<N;i++)
	{
		chopsticks[i]=0;
		semaphore[i]=0;
	}
return;
}

void think(int i){
	
	for(k=0;k<1000000000;k++); // wait
	printf("\n\tPhilosopher %d is THINKING.",i+1);
	return;
}

void up(int *sem){
	volatile int temp = *sem;
	if(temp <= 0)
	{	
		(*sem)++;
	}
	else
	{
		while(1){ printf("\n Spinning in UP\n"); if(temp<=0){ break;}}
	}
	return;
}

void down(int *sem){
	volatile int temp = *sem;
	if(temp>0)
	{	
		(*sem)--;
	}
	else
	{
		while(1){ printf("\n Spinning in Down");if(temp >0){break;}}
	}
	return;
}

void getSticks(int i)
{	printf("\n\tPhilosopher %d is getting sticks ",i+1);
	gtthread_mutex_lock(&mutex);
	state[i]=HUNGRY;
	printf("\n\tPhilosopher %d is HUNGRY ",i+1);
	test(i);
	gtthread_mutex_unlock(&mutex);
	down(&semaphore[i]);
	return;

}
	
void releaseSticks(int i)
{
	printf("\n\tPhilosopher %d is releasing Sticks ",i+1);
	gtthread_mutex_lock(&mutex);
	state[i]= THINKING;
	test(LEFT);
	test(RIGHT);
	gtthread_mutex_unlock(&mutex);
	return;
}

void test(int i)
{	//printf("\nPhilosospher %d ndState[i] =%d , state[LEFT] = %d, state[RIGHT]= %d",i+1,state[i],state[LEFT],state[RIGHT]);
	if(state[i]==HUNGRY && state[LEFT]!= EATING && state[RIGHT]!=EATING)
	{
		state[i]=EATING;
		printf("\n\nPhilosopher %d is EATING ",i+1);
		up(&semaphore[i]);
	}
	return;
}
/*
void eat(int i)
{

printf("\n\t Philosopher %d is EATING : ",i+1);
return;
}
*/
void *worker(void *phil){
	int* pointer = phil;
	int temp = *pointer;	
	while(1)
	{
		think(temp);
		getSticks(temp);
		//eat(temp);
		releaseSticks(temp);
	}
}

int main()
{
    int i;
	init();
	gtthread_t philosophers_threads[N];
	for(i=0;i<N;i++)
    {
	printf("\n\tPhilosopher number %d is thinking\n",i+1);
        gtthread_create(&philosophers_threads[i],worker,&philosopher[i]);
    }
    //gtthread_exit(NULL);
    for(;;);
}
	
