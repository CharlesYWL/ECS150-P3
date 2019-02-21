#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "queue.h"
#include "thread.h"
#include "tps.h"

queue_t globalStore = NULL;

struct page
{
	int refcounter;
	void *address;
};
typedef struct page *page_t;

struct tps
{
	pthread_t pid;
	// size_t length;
	page_t storage;
};
typedef struct tps *tps_t;

int queue_equal(void *target1, void *target2)
{
	// return 1 if same, 0 not same
	return ((tps_t)target1)->pid == *(pthread_t *)target2 ? 1 : 0;
}
int queue_equaladdress(void *target1, void *target2)
{
	// return 1 if same, 0 not same
	return ((tps_t)target1)->storage->address == target2 ? 1 : 0;
}

tps_t tps_find(pthread_t target)
{
	tps_t rtn = NULL;
	if (globalStore == NULL || !target)
	{
		// fprintf(stderr,"invalid queue_t or target\n");
		return NULL;
	}
	else
	{
		if (queue_iterate(globalStore, queue_equal, (void *)&target, (void **)&rtn))
			return NULL;
		return rtn;
	}
}

tps_t tps_findaddress(void* targetpage) //specify for segv handler
{
	tps_t rtn = NULL;
	if (globalStore == NULL || !targetpage)
	{
		// fprintf(stderr,"invalid queue_t or target\n");
		return NULL;
	}
	else{
		if (queue_iterate(globalStore, queue_equaladdress,targetpage, (void **)&rtn))
			return NULL;
		return rtn;
	}
}

static void segv_handler(int sig, siginfo_t *si, void *context)
{
	/*
   * Get the address corresponding to the beginning of the page where the
   * fault occurred
   */
	void *p_fault = (void *)((uintptr_t)si->si_addr & ~(TPS_SIZE - 1));


   //Iterate through all the TPS areas and find if p_fault matches one of them
	tps_t target = tps_findaddress(p_fault);
	if (target != NULL)
		/* Printf the following error message */
		fprintf(stderr, "TPS protection error!\n");

	/* In any case, restore the default signal handlers */
	signal(SIGSEGV, SIG_DFL);
	signal(SIGBUS, SIG_DFL);
	/* And transmit the signal again in order to cause the program to crash */
	raise(sig);
}

page_t page_init()
{
	page_t mpage = malloc(sizeof(struct page));
	mpage->address = mmap(NULL, TPS_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	mpage->refcounter = 1;
	return mpage;
}
int page_destory(page_t target)
{
	if(target->refcounter >1)
		return -1;//cannot be destroy
	if( munmap(target->address, TPS_SIZE))
		return -1; //return -1 if fail
	free(target);
	return 0;
}

int tps_init(int segv)
{
	if (globalStore != NULL) {
		return -1;
	}

	globalStore = queue_create();
	if (segv)
	{
		struct sigaction sa;

		sigemptyset(&sa.sa_mask);
		sa.sa_flags = SA_SIGINFO;
		sa.sa_sigaction = segv_handler;
		sigaction(SIGBUS, &sa, NULL);
		sigaction(SIGSEGV, &sa, NULL);
	}
	return 0;
}
int tps_create_withP();
int tps_create(void)
{
	tps_create_withP(page_init());
}
int tps_create_withP(page_t page)
{
	tps_t t = NULL;
	t = tps_find(pthread_self()); //get tid
	if (t != NULL)
	{ //t should not exist
		return -1;
	}

	t = malloc(sizeof(struct tps));
	t->pid = pthread_self();
	t->storage = page; // use exist page
	queue_enqueue(globalStore, t);
	return 0;
}

int tps_destroy(void)
{
	tps_t target = tps_find(pthread_self());
	if(target == NULL)
		return -1;
	if(target->storage->refcounter > 1)
		target->storage->refcounter -= 1;
	else if (page_destory(target->storage))
		//return -1 if failed
		return -1;
	free(target);
	return 0;
}

int tps_read(size_t offset, size_t length, char *buffer)
{
	tps_t target = tps_find(pthread_self());
	if (!target || mprotect(target->storage->address, TPS_SIZE, PROT_READ) == -1 || buffer == NULL || offset < 0 || length + offset > TPS_SIZE || length < 0)
	{
		//perror("tps_read: mprotect: "); test only
		return -1;
	}
	else
	{
		memcpy(buffer, target->storage->address + offset, length);
		mprotect(target->storage->address, TPS_SIZE, PROT_NONE);
		return 0;
	}
}

int tps_write(size_t offset, size_t length, char *buffer)
{
	tps_t target = tps_find(pthread_self());
	if (!target || mprotect(target->storage->address, TPS_SIZE, PROT_WRITE) == -1 || buffer == NULL || offset < 0 || length+offset > TPS_SIZE || length < 0)
	{
		// if any failed
		//perror("tps_write: mprotect: "); test only
		return -1;
	}
	else
	{
		if(target->storage->refcounter > 1){
			page_t newpage = page_init(); // copy and write on newpage
			mprotect(target->storage->address, TPS_SIZE, PROT_READ);
			mprotect(newpage->address, TPS_SIZE, PROT_WRITE);
			memcpy(newpage->address, target->storage->address, TPS_SIZE);
			mprotect(target->storage->address, TPS_SIZE, PROT_NONE);
			mprotect(newpage->address, TPS_SIZE, PROT_NONE);
			target->storage->refcounter -= 1;
			target->storage = newpage;
		}
		mprotect(target->storage->address, TPS_SIZE, PROT_WRITE);

		memcpy(target->storage->address + offset, buffer, length);
		mprotect(target->storage->address, TPS_SIZE, PROT_NONE);
		return 0;
	}
}

int tps_clone(pthread_t tid)
{
	tps_t self = tps_find(pthread_self());
	tps_t target = tps_find(tid);
	//target should exist and self should be first time created
	if (target == NULL || self != NULL)
		return -1;
	else
	{
		tps_create_withP(target->storage);
		self = tps_find(pthread_self());
		self->storage->refcounter += 1;
		return 0;
	}
}
