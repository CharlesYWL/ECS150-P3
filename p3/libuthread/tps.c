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

queue_t globalStore;

struct tps
{
	pthread_t pid;
	// size_t length;
	void *storage;
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
	return ((tps_t)target1)->storage == target2 ? 1 : 0;
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
		if (queue_iterate(globalStore, queue_equal, (void *)&target, (void *)&rtn))
			return NULL;
		return rtn;
	}
}

tps_t tps_findaddress(void* targetpage)
{
	tps_t rtn = NULL;
	if (globalStore == NULL || !targetpage)
	{
		// fprintf(stderr,"invalid queue_t or target\n");
		return NULL;
	}
	else
	{
		if (queue_iterate(globalStore, queue_equaladdress, (void *)&targetpage, (void *)&rtn))
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

int tps_init(int segv)
{
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

int tps_create(void)
{
	tps_t t = malloc(sizeof(struct tps));
	// PROT_EXEC/READ/WRITE/NONE
	// MAP_SHARED/FIXED/PRIVATE
	void *newpage =
		mmap(NULL, TPS_SIZE, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	perror("errno: ");
	fprintf(stderr, "newpage: %p has been assigned\n", newpage);
	t->pid = pthread_self();
	t->storage = newpage;
	queue_enqueue(globalStore, t);
}

int tps_destroy(void)
{
	tps_t target = tps_find(pthread_self());
	return munmap(target->storage, TPS_SIZE);
}

int tps_read(size_t offset, size_t length, char *buffer)
{
	tps_t target = tps_find(pthread_self());
	if (!target || mprotect(target->storage, TPS_SIZE, PROT_READ) == -1)
	{
		perror("tps_read: mprotect: ");
		return -1;
	}
	else
	{
		memcpy(buffer, target->storage + offset, length);
		mprotect(target->storage, TPS_SIZE, PROT_NONE);
		return 0;
	}
}

int tps_write(size_t offset, size_t length, char *buffer)
{
	tps_t target = tps_find(pthread_self());
	if (!target || mprotect(target->storage, TPS_SIZE, PROT_WRITE) == -1)
	{
		// if any failed
		perror("tps_write: mprotect: ");
		return -1;
	}
	else
	{
		memcpy(target->storage + offset, buffer, length);
		perror("wirte: mmcpy");
		mprotect(target->storage, TPS_SIZE, PROT_NONE);
		return 0;
	}
}

int tps_clone(pthread_t tid)
{
	tps_t self = tps_find(pthread_self());
	tps_t target = tps_find(tid);
	if (target == NULL || self != NULL)
		return -1;
	else
	{
		tps_create();
		self = tps_find(pthread_self());
		// tempary attain access
		mprotect(target->storage, TPS_SIZE, PROT_READ);
		mprotect(self->storage, TPS_SIZE, PROT_WRITE);
		memcpy(self->storage, target->storage, TPS_SIZE);
		mprotect(target->storage, TPS_SIZE, PROT_NONE);
		mprotect(self->storage, TPS_SIZE, PROT_NONE);
	}
}
