#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tps.h>
//protect test

void *latest_mmap_addr;

void *__real_mmap(void *addr, size_t len, int prot, int flags, int fildes,
                  off_t off);
void *__wrap_mmap(void *addr, size_t len, int prot, int flags, int fildes,
                  off_t off)
{
    latest_mmap_addr = __real_mmap(addr, len, prot, flags, fildes, off);
    return latest_mmap_addr;
}
void *thread1(void *arg)
{
    char tps_addr[10] = "hello\0";
    assert(tps_create() == 0);

    assert(tps_write(0, 1024, tps_addr) == 0);
    assert(tps_read(0, 1024, tps_addr) == 0);

    char * extraadder = latest_mmap_addr;
    extraadder[0] = '\0';

    return 0;
}

int main(int argc, char **argv)
{
    pthread_t tid;

    assert(tps_init(1) == 0);

    pthread_create(&tid, NULL, thread1, NULL);
    pthread_join(tid, NULL);

    return 0;
}