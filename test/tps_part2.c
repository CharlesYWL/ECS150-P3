#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tps.h>
//test for all error handler
void *thread1(void *arg)
{
    char str[1000] = {0};

    //without init space
    assert(tps_write(0, 1000, str) == -1);
    assert(tps_read(0, 1000, str) == -1);

    //init
    assert(tps_create() == 0);
    //double init
    assert(tps_create() == -1);
    //destory
    assert(tps_destroy() == 0);
    //double destory
    assert(tps_destroy() == -1);
    //create
    assert(tps_create() == 0);


    //read & write out of bound
    assert(tps_write(5000, 1000, str) == -1);
    assert(tps_read(5000, 1000, str) == -1);

    //vaild op
    assert(tps_write(0, 1000, str) == 0);
    str[0] = 1;
    assert(tps_write(1, 1000, str) == 0);
    assert(tps_read(0, 1000, str) == 0);
    assert(str[0] == 0);
    assert(str[1] == 1);

    fprintf(stdout, "all error test passed\n");

    return 0;
}

int main(int argc, char **argv)
{
    pthread_t tid;

    /* Init TPS API */
    assert(tps_init(1) == 0);

    // test: double init
    assert(tps_init(0) == -1);

    /* Create thread 1 and wait */
    pthread_create(&tid, NULL, thread1, NULL);
    pthread_join(tid, NULL);

    return 0;
}