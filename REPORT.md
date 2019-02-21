# SEMAPHORE & TPS

## Overview

Spmaphore does lock job, it can lock and unlock on a processor so that thread
can whether access to it or not.
TPS is privite storage that only the thread that creates it can use it.

## Phase 1

The whole Phase is about how to achieve the function of semaphore.
The first idea came with is that we should have a global queue for 
a single processor so that we know when one thread paused, which is 
the next to operate. That's our `thread_list`. 
the structure of semaphroe is pretty simple, one count and a thread list. 
all the functions are pretty strightfoward.
in`sem_create()`: we just malloc the size and set thread_list as queue.
in`sem_dostroy()`: checked before free, nothing special.
for the `sem_down` and `sem_up`, we need them away from interupted so wrap
the core codes with `enter_critical_section()` and `exit_critical_section()`
inside, we will check the countnumber to decide whether the thread will be
`thread_block()`/`unblock()`,at the same time we `dequeue()`/`enqueue()`
to operate the list. For the `sem_getvalue()`, it's simple so we only 
need to checked the queue and count.

## Phase 2

in Phase2, we need a table of all pts, so we have `globalStore` to store
all the tps. the structure is very simple, including pid and pages. 
To init tps, we only initiate one time and dont allow sencond init.

While create them by `init()`, we have two modes: one create first time 
and another is clone from others. That's why we have `tps_create()` and 
`tps_create_withP(page_t page)`. the second one works as inner func 
of first one, first one will have `page_init()` to put a brand new page
into `tps_create_withP()`.

While allocate memory,we use `mmap()` let system auto alloc memory for us
in order to find target pts, we use `queue_iterator` to find the pts
with matched pid
than we can do the major function`write` and `read`.
in write and read, the core func we used is `memcpy` which copy contents 
and `mprotect` which open the door to it and close later.
In `mmap()`, we already assigned the memnory not executable. Than we
use `mprotect()` to change the mode so that we can temperoly access it.
A big trick in `write()` is we have to deal the cases that two tps hold
the shared memory, so we create new memory area for it and than operate
on that.
`destory()` of everything is strightforward. check all kinds of invaild
situation before free them.

### Test for TPS
`tps.c`: it's provided by professor and basically checks everything we
used as API.
`tps_part2.c` is our own program that checks all error return.
`tps_part3.c` is the checker for protected pts, we operate memory 
outside, and checked whether our segv handler can get that. 

## Challenges
In Phase1, we encounterd a problem about the logic of block/unblock
and it took us a while to make that understood
In Pahse2, one big thing jamed us was the `mmap()`things. First time
we used `MAP_PRIVATE` only but it return `NULL` as result. It took us 
hours searching about the question and finally we use `errno` to 
figure out the problem: No Devices, problem happened when we put
-1 in fd which should combined with `MAP_ANONYMOUS` that points to virtual
memory. Than I tried `MAP_ANONYMOUS` only, this time `errno` showed
Invaild Agrument and `MAP_ANONYMOUS` showed as undecleared. After 
another a few hours research, under the help of my friend, I relalized
that we need both `MAP_ANONYMOUS` and `MAP_PRIVATE` to make it work.
After that, our process went really well expect some accident like
figuring out the pointers on the `queue_iterator`.

