# Project3 SEMAPHORE & TPS

## Overview

P3 is the most chanlleging project we think so far. Debugging this project 
requires a huge amout of time and a lot of patience. Phase 1 is relatively 
simple and easy to implement. Phase 2 is a nightmare but we survived.
A huge Shout-out to an amazing Chinese IT Blog website 
[CSDN](https://www.csdn.net/) which helped us a lot for the ideas of 
implementing the page map.



## Phase 1  

The whole Phase is about how to implement a semaphore with required funcs.
The first idea came into our mind was that we should have a global queue 
for a single processor. We have to know when one thread paused, which 
thread is the next to operate. That's our `thread_list`. 
The structure of semaphore is pretty simple, a counter and a thread list. 
The functions involved are pretty straightfoward and easy to understand.
in`sem_create()`: we just malloc the size and set thread_list as queue.
in`sem_dostroy()`: we check the sem before free it, nothing special.
for the `sem_down` and `sem_up`, we need to keep them away from being 
interrupted. Thus, we wrap the core codes with critical sections.
`enter_critical_section()` and `exit_critical_section()`
inside, we will check the countnumber to decide whether the thread will be
`thread_block()`/`unblock()`,at the same time we `dequeue()`/`enqueue()`
to operate the list. For the `sem_getvalue()`, we just need to check the 
queue and the counter.

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

