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

In Phase2, we need a table for all tps, so we have `globalStore` to store
all the tps. The structure is very simple, we put pid and pages in it. 
To initialize tps, we only initialize one time and don't allow a second 
initialization.

While create them through `init()`, we have two modes: The first mode is
for the initialization for the first time. And the second mode is clone 
from the others. That's why we have `tps_create()` and 
`tps_create_withP(page_t page)`. The second one works as inner func 
of first one, first one will have `page_init()` to put a brand new page
into `tps_create_withP()`.

While allocating the memory, we use `mmap()` let system automatic 
allocate memory for us.
In order to find target pts, we use `queue_iterator` to find the pts
with matched pid.
After that, we can do the major function`write` and `read`.
In write and read, the core func we used is `memcpy` which copies the 
contents and `mprotect` which opens the door to it and closes later.
In `mmap()`, we already set the memnory not executable. Than we
use `mprotect()` to change the mode so that we can temporarily access it.
A big trick in `write()` is that we have to deal with the case that two 
tps hold the shared memory, so we create new memory area for it and then
operate on that.
`destory()` is very strightforward. It checks all kinds of invaild
situation before free them.

### Test for TPS
`tps.c`: it's provided by professor and basically checks everything we
used as API.
`tps_part2.c` is our own program that checks all error return.
`tps_part3.c` is the checker for protected pts, we operate memory 
outside, and check whether our segv handler can get that. 


## Challenges
In Phase1, we encounterd a problem about the logic of block/unblock
and it took us a while to figure it out.
In Pahse2, one big thing jamed us was the `mmap()`things. At first,
we used `MAP_PRIVATE` only but it return `NULL` as a result. It took us 
hours of hours searching, and finally we use `errno` to 
figure out the problem: No Devices, problems happened when we put
-1 in fd which should combined with `MAP_ANONYMOUS` that points to virtual
memory. Then I tried `MAP_ANONYMOUS` only, this time `errno` showed
Invaild Agrument and `MAP_ANONYMOUS` showed as undecleared. After 
another a few hours effort, I relalized that we need both 
`MAP_ANONYMOUS` and `MAP_PRIVATE` to make it work.
After that, our process went really smooth expect some accidents like
figuring out the pointers on the `queue_iterator`.

