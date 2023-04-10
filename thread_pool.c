#include "thread_pool.h"
#include <pthread.h>
#include <malloc.h>

struct thread_task {
    thread_task_f function;
    void *arg;
    void *result;
    int state;
    pthread_mutex_t task_mutex;
    pthread_cond_t task_cond;
};

struct thread_pool {
    pthread_t *threads;
    int thread_count, max_thread_count;
    struct thread_task **tasks;
    int task_count;
    int task_fin;
    pthread_mutex_t task_mutex;
    pthread_cond_t task_cond;
    int shutdown;
};

enum thread_task_status {
    TPOOL_INITIALIZED = 0,
    TPOOL_IN_POOL = 1,
    TPOOL_HAVE_RESULT = 2,
};

void *
thread_pool_worker(void *arg)
{
    struct thread_pool *pool = (struct thread_pool *) arg;

    while (true) {
        pthread_mutex_lock(&pool->task_mutex);

        while (pool->task_count == pool->task_fin && !pool->shutdown) {
            // Wait until there's a task in the queue or the thread pool is shut down
            pthread_cond_wait(&pool->task_cond, &pool->task_mutex);
        }

        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->task_mutex);
            break;
        }

        // Get the first task from the queue
        int task_id = pool->task_fin;
        struct thread_task *task = pool->tasks[task_id];
        pool->tasks[task_id] = NULL;
        pool->task_fin = (pool->task_fin + 1) % TPOOL_MAX_TASKS;

        pthread_mutex_unlock(&pool->task_mutex);

        pthread_mutex_lock(&task->task_mutex);

        // Execute the task
        task->result = task->function(task->arg);

        task->state |= TPOOL_HAVE_RESULT;

        // Signal the main thread that a task has finished
        pthread_cond_signal(&task->task_cond);
        pthread_mutex_unlock(&task->task_mutex);
    }

    pthread_exit(NULL);
}

int
thread_pool_new(int max_thread_count, struct thread_pool **pool) {
    if (max_thread_count < 1 || max_thread_count > TPOOL_MAX_THREADS || !pool) {
        return TPOOL_ERR_INVALID_ARGUMENT;
    }

    struct thread_pool *p = malloc(sizeof(struct thread_pool));
    if (!p) {
        return TPOOL_ERR_INVALID_ARGUMENT;
    }

    p->threads = malloc(sizeof(pthread_t) * max_thread_count);
    p->tasks = malloc(sizeof(struct thread_task *) * TPOOL_MAX_TASKS);
    for(int i=0;i<TPOOL_MAX_TASKS;i++) p->tasks[i] = NULL;


    p->thread_count = 0;
    p->max_thread_count = max_thread_count;
    p->task_count = p->task_fin = 0;
    p->shutdown = 0;
    pthread_mutex_init(&p->task_mutex, NULL);
    pthread_cond_init(&p->task_cond, NULL);

    for (int i = 0; i < max_thread_count; i++) {
        if (pthread_create(&p->threads[i], NULL, thread_pool_worker, p) != 0) {
            for (int j = 0; j < i; j++) {
                pthread_cancel(p->threads[j]);
            }
            free(p->tasks);
            free(p->threads);
            free(p);
            return TPOOL_ERR_INVALID_ARGUMENT;
        }
    }

    *pool = p;
    return 0;
}

int
thread_pool_thread_count(const struct thread_pool *pool) {
    if (!pool) return TPOOL_ERR_INVALID_ARGUMENT;
    return pool->thread_count;
}

int
thread_pool_delete(struct thread_pool *pool) {
    pthread_mutex_lock(&pool->task_mutex);

    if (pool->task_count >= 0) {
        pthread_mutex_unlock(&pool->task_mutex);
        return TPOOL_ERR_HAS_TASKS;
    }
    pool->shutdown = 1;
    pthread_cond_broadcast(&pool->task_cond);
    pthread_mutex_unlock(&pool->task_mutex);

    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    for (int i = 0; i < TPOOL_MAX_TASKS; i++) {
        free(pool->tasks[i]);
    }

    pthread_mutex_destroy(&pool->task_mutex);
    pthread_cond_destroy(&pool->task_cond);
    free(pool->tasks);
    free(pool->threads);
    free(pool);
    return 0;
}

int
thread_pool_push_task(struct thread_pool *pool, struct thread_task *task) {
    pthread_mutex_lock(&pool->task_mutex);

    if ((pool->task_count + 1) % TPOOL_MAX_TASKS == pool->task_fin) {
        pthread_mutex_unlock(&pool->task_mutex);
        return TPOOL_ERR_TOO_MANY_TASKS;
    }

    pool->tasks[pool->task_count] = task;
    task->state |= TPOOL_IN_POOL;
    pool->task_count = (pool->task_count + 1) % TPOOL_MAX_TASKS;

    pthread_cond_signal(&pool->task_cond);
    pthread_mutex_unlock(&pool->task_mutex);

    return 0;
}

int
thread_task_new(struct thread_task **task, thread_task_f function, void *arg) {
    struct thread_task *new_task = malloc(sizeof(struct thread_task));

    new_task->function = function;
    new_task->arg = arg;
    new_task->state = TPOOL_INITIALIZED;
    pthread_cond_init(&new_task->task_cond, NULL);

    *task = new_task;

    return 0;
}

bool
thread_task_is_finished(const struct thread_task *task) {
    return (task->state & TPOOL_HAVE_RESULT);
}

bool
thread_task_is_running(const struct thread_task *task) {
    return (task->state & TPOOL_IN_POOL);
}

int
thread_task_join(struct thread_task *task, void **result) {
    if (!thread_task_is_running(task)) return TPOOL_ERR_TASK_NOT_PUSHED;
    pthread_cond_wait(&task->task_cond, &task->task_mutex);

    *result = task->result;
    return 0;
}

int
thread_task_delete(struct thread_task *task) {
    if (thread_task_is_running(task)) return TPOOL_ERR_TASK_IN_POOL;
    free(task);
    return 0;
}

#ifdef NEED_DETACH

int
thread_task_detach(struct thread_task *task)
{
    /* IMPLEMENT THIS FUNCTION */
    return TPOOL_ERR_NOT_IMPLEMENTED;
}

#endif