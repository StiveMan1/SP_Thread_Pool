#include "thread_pool.h"
#include <pthread.h>
#include <malloc.h>
#include <asm-generic/errno.h>
#include <bits/stdint-uintn.h>

struct thread_task {
    thread_task_f function;
    void *arg;
    void *result;

    bool pushed;
    bool finished;
    bool joined;
    bool delete;

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    struct thread_pool *pool;

    /* PUT HERE OTHER MEMBERS */
};

struct thread_pool {
    pthread_t *threads;

    int max_threads;
    int cur_threads;

    struct thread_task **tasks;
    int task_finished;
    int task_done;
    int task_counter;

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    bool shutdown;

    /* PUT HERE OTHER MEMBERS */
};


void *thread_pool_worker(void *arg) {
    struct thread_pool *pool = (struct thread_pool *) arg;
    struct thread_task *task = NULL;

    thread_task_f function = NULL;
    void *_arg = NULL;
    void *result = NULL;

    while (true) {
        pthread_mutex_lock(&pool->mutex);
        while (pool->task_done == pool->task_counter && !pool->shutdown) {
            pthread_cond_wait(&pool->cond, &pool->mutex);
        }
        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }
        task = pool->tasks[pool->task_done];
        pool->tasks[pool->task_done] = NULL;
        pool->task_done = (pool->task_done + 1) % (TPOOL_MAX_TASKS + 1);
        pthread_mutex_unlock(&pool->mutex);


        if(task == NULL) continue;

        pthread_mutex_lock(&task->mutex);
        function = task->function;
        _arg = task->arg;
        if (task->finished) {
            pthread_mutex_unlock(&task->mutex);
            continue;
        }
        pthread_mutex_unlock(&task->mutex);

        result = function(_arg);

        pthread_mutex_lock(&task->mutex);
        task->result = result;
        task->finished = true;
        if (task->delete) {
            pthread_mutex_unlock(&task->mutex);
            thread_task_delete(task);
            continue;
        }
        pthread_cond_signal(&task->cond);
        pthread_mutex_unlock(&task->mutex);
    }
    pthread_exit(NULL);
}

int thread_pool_new(int max_thread_count, struct thread_pool **pool) {
    if (max_thread_count < 1 || max_thread_count > TPOOL_MAX_THREADS) {
        return TPOOL_ERR_INVALID_ARGUMENT;
    }

    struct thread_pool *p = malloc(sizeof(struct thread_pool));

    p->threads = malloc(sizeof(pthread_t) * max_thread_count);
    p->tasks = malloc(sizeof(struct thread_task *) * (TPOOL_MAX_TASKS + 1));

    p->max_threads = max_thread_count;
    p->cur_threads = 0;

    p->task_finished = 0;
    p->task_done = 0;
    p->task_counter = 0;

    p->shutdown = 0;

    pthread_mutex_init(&p->mutex, NULL);
    pthread_cond_init(&p->cond, NULL);

    for (int i = 0; i < TPOOL_MAX_TASKS; i++) p->tasks[i] = NULL;
    *pool = p;
    return 0;
}

int thread_pool_thread_count(const struct thread_pool *pool) {
    return pool->cur_threads;
}

int thread_pool_delete(struct thread_pool *pool) {
    pthread_mutex_lock(&pool->mutex);

    if (pool->task_finished != pool->task_counter) {
        pthread_mutex_unlock(&pool->mutex);
        return TPOOL_ERR_HAS_TASKS;
    }

    pool->shutdown = 1;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);

    for (int i = 0; i < pool->cur_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->cond);

    free(pool->tasks);
    free(pool->threads);
    free(pool);
    return 0;
}

int thread_pool_push_task(struct thread_pool *pool, struct thread_task *task) {
    pthread_mutex_lock(&pool->mutex);
    if ((pool->task_counter + 1) % (TPOOL_MAX_TASKS + 1) == pool->task_finished) {
        pthread_mutex_unlock(&pool->mutex);
        return TPOOL_ERR_TOO_MANY_TASKS;
    }

    if(task->pool != NULL) pool->task_finished = (pool->task_finished + 1) % (TPOOL_MAX_TASKS + 1);
    if ((pool->task_counter + 1 + TPOOL_MAX_TASKS - pool->task_finished) % (TPOOL_MAX_TASKS)> pool->cur_threads
        && pool->cur_threads < pool->max_threads)
        pthread_create(&pool->threads[pool->cur_threads++], NULL, thread_pool_worker, pool);

    task->pushed = true;
    task->joined = false;
    task->delete = false;
    task->pool = pool;
    pool->tasks[pool->task_counter] = task;
    pool->task_counter = (pool->task_counter + 1) % (TPOOL_MAX_TASKS + 1);

    pthread_cond_signal(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);

    return 0;
}

int thread_task_new(struct thread_task **task, thread_task_f function, void *arg) {
    *task = malloc(sizeof(struct thread_task));
    (*task)->function = function;
    (*task)->arg = arg;

    (*task)->pushed = false;
    (*task)->finished = false;
    (*task)->joined = false;
    (*task)->delete = false;
    (*task)->pool = NULL;

    pthread_mutex_init(&(*task)->mutex, NULL);
    pthread_cond_init(&(*task)->cond, NULL);
    return 0;
}

bool thread_task_is_finished(const struct thread_task *task) {
    return task->finished;
}

bool thread_task_is_running(const struct thread_task *task) {
    return task->pushed;
}

int thread_task_join(struct thread_task *task, void **result) {
    if (!task->pushed) return TPOOL_ERR_TASK_NOT_PUSHED;

    pthread_mutex_lock(&task->mutex);
    while (!task->finished) {
        pthread_cond_wait(&task->cond, &task->mutex);
    }
    task->joined = true;
    pthread_mutex_unlock(&task->mutex);

    *result = task->result;
    return 0;
}

#ifdef NEED_TIMED_JOIN

int thread_task_timed_join(struct thread_task *task, double timeout, void **result) {
    if (!task->pushed) return TPOOL_ERR_TASK_NOT_PUSHED;
    if (timeout <= 0) return TPOOL_ERR_TIMEOUT;

    struct timespec abs_timeout;
    clock_gettime(CLOCK_REALTIME, &abs_timeout);

    abs_timeout.tv_sec += (long) (timeout);
    abs_timeout.tv_nsec += (long) (timeout * 1000000000) % 1000000000;


    pthread_mutex_lock(&task->mutex);
    while (!task->finished) {
        int err = pthread_cond_timedwait(&task->cond, &task->mutex, &abs_timeout);
        if (err == ETIMEDOUT) {
            pthread_mutex_unlock(&task->mutex);
            return TPOOL_ERR_TIMEOUT;
        } else if (err == 22) {
            clock_gettime(CLOCK_REALTIME, &abs_timeout);

            abs_timeout.tv_sec += (long) (timeout);
            abs_timeout.tv_nsec += (long) (timeout * 1000000000) % 1000000000;
        }
    }
    task->joined = true;
    pthread_mutex_unlock(&task->mutex);

    *result = task->result;
    return 0;
}

#endif

int thread_task_delete(struct thread_task *task) {
    if (task->pushed && !(task->joined || task->delete)) {
        return TPOOL_ERR_TASK_IN_POOL;
    }
    if(task->pool != NULL) {
        pthread_mutex_lock(&task->pool->mutex);
        task->pool->task_finished = (task->pool->task_finished + 1) % (TPOOL_MAX_TASKS + 1);
        pthread_mutex_unlock(&task->pool->mutex);
        task->pool = NULL;
    }

    pthread_mutex_destroy(&task->mutex);
    pthread_cond_destroy(&task->cond);
    free(task);
    return 0;
}

#ifdef NEED_DETACH

int thread_task_detach(struct thread_task *task) {
    if (!task->pushed) return TPOOL_ERR_TASK_NOT_PUSHED;

    pthread_mutex_lock(&task->mutex);
    task->delete = true;
    if (task->finished) {
        pthread_mutex_unlock(&task->mutex);
        thread_task_delete(task);
        return 0;
    }
    pthread_mutex_unlock(&task->mutex);
    return 0;
}

#endif
