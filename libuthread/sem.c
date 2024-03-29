#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "queue.h"
#include "sem.h"
#include "thread.h"

struct semaphore {
	int count;  //operate by down/up
	void *thread_list;
};

sem_t sem_create(size_t count)
{
	sem_t new;
	if (new = malloc(sizeof(struct semaphore))){
		//malloc successful
		new->count = count;
		queue_t list = queue_create();
		new->thread_list = list;
	}else
		fprintf(stderr,"malloc semaphore error");	
	

}

int sem_destroy(sem_t sem)
{
	if(sem == NULL || queue_length(sem->thread_list)!=0){
		fprintf(stderr,"semaphore destory error");
		return -1;
	}else{
		queue_destroy(sem->thread_list);
		free(sem);
		return 0;
	}
	
}

/*block if already 0
not zero,lock down now
*/
int sem_down(sem_t sem)
{
	enter_critical_section();
	if(sem == NULL) return -1;
	while(sem->count == 0){ //block self while no resources aviliable
		queue_enqueue(sem->thread_list,(void*)pthread_self());
		thread_block();
	}
	sem->count -= 1;
	exit_critical_section();
	return 0;
}

/* increament,wake up one of the waiting threads if any
 */
int sem_up(sem_t sem)
{	
	enter_critical_section();
	if(sem == NULL) return -1;
	sem->count += 1;
	//wake up first in line
	void* rtnid;
	if(queue_dequeue(sem->thread_list,&rtnid)==0)
		thread_unblock((pthread_t)rtnid);
	exit_critical_section();
	return 0;
}

int sem_getvalue(sem_t sem, int *sval)
{
	if(sem == NULL) return -1;
	if (sem->count > 0) {
		*sval = sem->count;
	}else {
		*sval = -1*queue_length(sem->thread_list);
	}
	return 0;
}

