# Thread Pool Implementation in C
## Overview
This project aims to implement a thread pool in C, providing a mechanism to efficiently manage and execute multiple tasks across a fixed number of worker threads. Thread pools are beneficial in scenarios where the creation and destruction of threads are costly relative to the tasks they perform, enabling parallel execution without the overhead of frequent thread management.

## Structure
The thread pool is designed around a set of core functionalities encapsulated in `thread_pool.h` and `thread_pool.c`. The thread_pool structure manages a queue of tasks (`thread_task`) and a fixed number of worker threads that fetch and execute these tasks.

## Key Components
* `thread_pool struct`: Represents the thread pool and holds necessary information about worker threads and task queue.
* `thread_task struct`: Describes an individual task that can be added to the pool's queue.
* `Functions`: Includes functions for initializing and destroying the pool, adding tasks to the pool, checking task status, waiting for task completion (`thread_task_join`), and managing the worker threads.
## Usage
To integrate the thread pool into a project:
* `Include Files`: Ensure `thread_pool.h` is included in your project.
* `Initialization`: Create a thread pool using `thread_pool_init` specifying the number of worker threads.
* `Task Execution`: Create tasks using `thread_task_create` and add them to the pool with `thread_pool_add_task`.
* `Task Management`: Monitor task status and wait for completion using `thread_task_join`.
* `Shutdown`: Cleanup resources with `thread_pool_destroy` once all tasks are completed.
## Example
```c++
#include "thread_pool.h"
#include <stdio.h>

void task_function(void *arg) {
int *num = (int *)arg;
printf("Task executing with argument: %d\n", *num);
}

int main() {
// Initialize thread pool with 4 worker threads
thread_pool *pool = thread_pool_init(4);

    // Example tasks
    int arg1 = 10;
    int arg2 = 20;

    // Add tasks to the pool
    thread_task *task1 = thread_task_create(task_function, &arg1);
    thread_pool_add_task(pool, task1);

    thread_task *task2 = thread_task_create(task_function, &arg2);
    thread_pool_add_task(pool, task2);

    // Wait for tasks to complete
    thread_task_join(task1);
    thread_task_join(task2);

    // Cleanup
    thread_pool_destroy(pool);

    return 0;
}
```
## Conclusion
This thread pool implementation provides a lightweight yet efficient mechanism for managing concurrent tasks in C programs, minimizing overhead associated with thread creation and destruction. By utilizing this pool, developers can achieve better performance in applications requiring parallel task execution.

For further details on specific functions and their usage, refer to `thread_pool.h` and `thread_pool.c`.