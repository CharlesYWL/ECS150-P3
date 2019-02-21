#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tps.h>

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
    char b[1024] = {0};
    assert(tps_create() == 0);

    // test: normal read and write
    assert(tps_write(0, 1024, b) == 0);
    assert(tps_read(0, 1024, b) == 0);

    // test: memory protection, a segmentation fault should be thrown
    char *protectedAddr = latest_mmap_addr;
    protectedAddr[0] = 1;

    return 0;
}

int main(int argc, char **argv)
{
    pthread_t tid;
    //init
    assert(tps_init(1)==0);

    pthread_create(&tid, NULL, thread1, NULL);
    pthread_join(tid, NULL);

}