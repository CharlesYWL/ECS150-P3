#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include "queue.h"
#include "thread.h"
#include "tps.h"

queue_t globalStore;

struct tps
{
	pthread_t pid;
	//size_t length;
	void* storage;
};
typedef struct tps *tps_t;

int queue_equal(void* target1, void* target2){
	//return 1 if same, 0 not same
	return ((tps_t)target1)->pid == *(pthread_t*)target2 ? 1 : 0;
}

tps_t tps_find(pthread_t target){
	tps_t rtn ;
	if(globalStore==NULL || !target){
		//fprintf(stderr,"invalid queue_t or target\n");
		return NULL;
	}
	else{
		queue_iterate(globalStore,queue_equal,(void*)&target,(void*)&rtn);
		return rtn;
	}
		
}

int tps_init(int segv)
{
		globalStore = queue_create();
}

int tps_create(void)
{
	tps_t t = malloc(sizeof(struct tps));
	//PROT_EXEC/READ/WRITE/NONE
	//MAP_SHARED/FIXED/PRIVATE
	void * newpage = mmap(NULL,TPS_SIZE,PROT_READ|PROT_READ,MAP_ANONYMOUS,-1,0);
	perror("errno: ");
	fprintf(stderr,"newpage: %p has been assigned",newpage);
	t->pid = pthread_self();
	t->storage = newpage;
	queue_enqueue(globalStore,t);

}

int tps_destroy(void)
{
	tps_t target = tps_find(pthread_self());
	return munmap(target->storage,TPS_SIZE);
}

int tps_read(size_t offset, size_t length, char *buffer)
{
	tps_t target = tps_find(pthread_self());
	memcpy(buffer,target->storage+offset,length);
}

int tps_write(size_t offset, size_t length, char *buffer)
{
	tps_t target = tps_find(pthread_self());
	memcpy(target->storage,buffer+offset,length);
}

int tps_clone(pthread_t tid)
{
	tps_t self = tps_find(pthread_self());
	tps_t target = tps_find(tid);
	if(target == NULL|| self != NULL) 
		return -1;
	else{
		tps_create();
		self = tps_find(pthread_self());
		memcpy(self->storage,target->storage,TPS_SIZE);
	}
}


