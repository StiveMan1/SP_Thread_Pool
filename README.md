# Thread pool.
### Language: C.

Need to implement a thread pool. In various programs executing
many independent and easily parallelized tasks it is often quite
handy to distribute them across multiple threads. But creating a
thread for each necessity to execute something in parallel is
very expensive in time and resources. If a task is not too long,
doesn't read disk, doesn't touch network, then creation/deletion
of a thread might take more time than the task itself.

Then tasks are either not paralleled at all, or when there are
many of them, people make thread pools. It is usually a task queue
and a few so called "worker threads" which take tasks from the
queue. Thus there is always an already created thread which can
quickly pick up a task. And instead of exiting the thread simply
picks up a next task.

In big general purpose libraries often there is an out of the box
solution: in Qt it is QThreadPool class, in .NET it is ThreadPool
class, in boost it is thread_pool class. In the task you have to
implement an own similar pool.

In the files thread_pool.h and thread_pool.c you can fine
templates of functions and structures which need to be
implemented.

The thread pool is described by a struct thread_pool implemented
in thread_pool.c. A user can only have a pointer at it. Each
task/job is described with struct thread_task, which a user can
create and put into the pool's queue.

User can check task's status (waits for getting a worker; is
already being executed), can wait for its end and get its result
with thread_task_join, similar to how pthread_join works.

Since the task is to implement a library, there is no 'main'
function and no input from anywhere. You can write tests in C in a
separate file with 'main' and which will 'include' your solution.
For example, make a file main.c, add 'include "thread_pool.h"',
and in the function 'main' you do tests. It can all be built like
this:

        gcc thread_pool.c main.c
