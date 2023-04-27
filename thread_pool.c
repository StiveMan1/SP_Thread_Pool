#include "thread_pool.h"
#include <pthread.h>
<<<<<<< HEAD
#include <malloc.h>
#include <asm-generic/errno.h>
#include <bits/stdint-uintn.h>
=======
#include <stdlib.h>
>>>>>>> 22ef47c09950bc2339d16ccdc1d03f30d08c5332

struct thread_task {
    thread_task_f function;
    void *arg;
    void *result;
<<<<<<< HEAD

    bool finished;
    bool running;
    bool joined;

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    struct thread_pool *pool;

    /* PUT HERE OTHER MEMBERS */
=======
    int state;
    pthread_mutex_t task_mutex;
    pthread_cond_t task_cond;

    int *task_fin;
>>>>>>> 22ef47c09950bc2339d16ccdc1d03f30d08c5332
};

struct thread_pool {
    pthread_t *threads;
<<<<<<< HEAD

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
=======
    int max_thread_count;

    struct thread_task **tasks;
    int task_count;
    int task_done;
    int task_fin;

    pthread_mutex_t task_mutex;
    pthread_cond_t task_cond;
    int shutdown;
>>>>>>> 22ef47c09950bc2339d16ccdc1d03f30d08c5332
};


<<<<<<< HEAD
void *thread_pool_worker(void *arg) {
=======
void *
thread_pool_worker(void *arg) {
>>>>>>> 22ef47c09950bc2339d16ccdc1d03f30d08c5332
    struct thread_pool *pool = (struct thread_pool *) arg;
    struct thread_task *task = NULL;
    bool first_time = true;

    while (true) {
<<<<<<< HEAD
        pthread_mutex_lock(&pool->mutex);
        while (pool->task_done == pool->task_counter && !pool->shutdown) {
            pthread_cond_wait(&pool->cond, &pool->mutex);
=======
        pthread_mutex_lock(&pool->task_mutex);

        while (pool->task_count == pool->task_done && !pool->shutdown) {
            // Wait until there's a task in the queue or the thread pool is shut down
            pthread_cond_wait(&pool->task_cond, &pool->task_mutex);
>>>>>>> 22ef47c09950bc2339d16ccdc1d03f30d08c5332
        }
        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }
        task = pool->tasks[pool->task_done];
        pool->tasks[pool->task_done] = NULL;
        pool->task_done = (pool->task_done + 1) % (TPOOL_MAX_TASKS + 1);
        if (first_time) {
            first_time = false;
            pool->cur_threads++;
        }
        pthread_mutex_unlock(&pool->mutex);

<<<<<<< HEAD
=======
        // Get the first task from the queue
        int task_id = pool->task_fin;
        struct thread_task *task = pool->tasks[task_id];
        pool->tasks[task_id] = NULL;
        pool->task_done = (pool->task_done + 1) % TPOOL_MAX_TASKS;
>>>>>>> 22ef47c09950bc2339d16ccdc1d03f30d08c5332

        if(task == NULL) continue;
        bool delete = false;

        pthread_mutex_lock(&task->mutex);
        while(!task->joined) {
            pthread_cond_wait(&task->cond, &task->mutex);
        }
        task->result = task->function(task->arg);
<<<<<<< HEAD
        task->finished = true;
        pthread_cond_signal(&task->cond);
        pthread_mutex_unlock(&task->mutex);
=======

        task->state = TPOOL_HAVE_RESULT;

        // Signal the main thread that a task has finished
        pthread_cond_signal(&task->task_cond);
        pthread_mutex_unlock(&task->task_mutex);
>>>>>>> 22ef47c09950bc2339d16ccdc1d03f30d08c5332
    }
    pthread_exit(NULL);
}

int thread_pool_new(int max_thread_count, struct thread_pool **pool) {
    if (max_thread_count < 1 || max_thread_count > TPOOL_MAX_THREADS) {
        return TPOOL_ERR_INVALID_ARGUMENT;
    }

    struct thread_pool *p = malloc(sizeof(struct thread_pool));

    p->threads = malloc(sizeof(pthread_t) * max_thread_count);
    p->tasks = malloc(sizeof(struct thread_task *) * TPOOL_MAX_TASKS);
<<<<<<< HEAD

    p->max_threads = max_thread_count;
    p->cur_threads = 0;

    p->task_finished = 0;
    p->task_done = 0;
    p->task_counter = 0;

=======
    for (int i = 0; i < TPOOL_MAX_TASKS; i++) p->tasks[i] = NULL;

    p->max_thread_count = max_thread_count;
    p->task_count = p->task_done = p->task_fin = 0;
>>>>>>> 22ef47c09950bc2339d16ccdc1d03f30d08c5332
    p->shutdown = 0;

    pthread_mutex_init(&p->mutex, NULL);
    pthread_cond_init(&p->cond, NULL);

    for (int i = 0; i < TPOOL_MAX_TASKS; i++) p->tasks[i] = NULL;
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

<<<<<<< HEAD
int thread_pool_thread_count(const struct thread_pool *pool) {
    return pool->cur_threads;
=======
int
thread_pool_thread_count(const struct thread_pool *pool) {
    if (!pool) return TPOOL_ERR_INVALID_ARGUMENT;
    return (pool->task_count + TPOOL_MAX_TASKS - pool->task_fin) % TPOOL_MAX_TASKS;
>>>>>>> 22ef47c09950bc2339d16ccdc1d03f30d08c5332
}

int thread_pool_delete(struct thread_pool *pool) {
    pthread_mutex_lock(&pool->mutex);

<<<<<<< HEAD
    if (pool->task_finished != pool->task_counter) {
        pthread_mutex_unlock(&pool->mutex);
=======
    if (pool->task_count != pool->task_fin) {
        pthread_mutex_unlock(&pool->task_mutex);
>>>>>>> 22ef47c09950bc2339d16ccdc1d03f30d08c5332
        return TPOOL_ERR_HAS_TASKS;
    }

<<<<<<< HEAD
    pool->shutdown = 1;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->mutex);

    for (int i = 0; i < pool->max_threads; i++) {
=======
    for (int i = 0; i < pool->max_thread_count; i++) {
>>>>>>> 22ef47c09950bc2339d16ccdc1d03f30d08c5332
        pthread_join(pool->threads[i], NULL);
    }

    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->cond);

    for (int i = 0; i < TPOOL_MAX_TASKS; i++) {
        if(pool->tasks[i] != NULL) free(pool->tasks[i]);
    }
    free(pool->tasks);
    free(pool->threads);
    free(pool);
    return 0;
}

<<<<<<< HEAD
int thread_pool_push_task(struct thread_pool *pool, struct thread_task *task) {
    pthread_mutex_lock(&pool->mutex);
    if ((pool->task_counter + 1) % (TPOOL_MAX_TASKS + 1) == pool->task_finished) {
        pthread_mutex_unlock(&pool->mutex);
        return TPOOL_ERR_TOO_MANY_TASKS;
    }

    if(task->pool != NULL) task->pool->task_finished = (task->pool->task_finished + 1) % (TPOOL_MAX_TASKS + 1);
=======
int
thread_pool_push_task(struct thread_pool *pool, struct thread_task *task) {
    pthread_mutex_lock(&pool->task_mutex);

    if(task->task_fin != NULL) *task->task_fin = (*task->task_fin + 1) % TPOOL_MAX_TASKS;
    if ((pool->task_count + 1) % TPOOL_MAX_TASKS == pool->task_fin) {
        pthread_mutex_unlock(&pool->task_mutex);
        return TPOOL_ERR_TOO_MANY_TASKS;
    }

    pool->tasks[pool->task_count] = task;
    task->state = TPOOL_IN_POOL;
    task->task_fin = &pool->task_fin;
    pool->task_count = (pool->task_count + 1) % TPOOL_MAX_TASKS;
>>>>>>> 22ef47c09950bc2339d16ccdc1d03f30d08c5332

    task->running = true;
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

<<<<<<< HEAD
    (*task)->running = false;
    (*task)->finished = false;
    (*task)->joined = false;
    (*task)->pool = NULL;
=======
    new_task->function = function;
    new_task->arg = arg;
    new_task->task_fin = NULL;
    new_task->state = TPOOL_INITIALIZED;
    pthread_cond_init(&new_task->task_cond, NULL);
    pthread_mutex_init(&new_task->task_mutex, NULL);
    pthread_cond_init(&new_task->task_cond, NULL);

    *task = new_task;
>>>>>>> 22ef47c09950bc2339d16ccdc1d03f30d08c5332

    pthread_mutex_init(&(*task)->mutex, NULL);
    pthread_cond_init(&(*task)->cond, NULL);
    return 0;
}

<<<<<<< HEAD
bool thread_task_is_finished(const struct thread_task *task) {
    return task->finished;
}

bool thread_task_is_running(const struct thread_task *task) {
    return task->running;
=======
bool
thread_task_is_finished(const struct thread_task *task) {
    return (task->state == TPOOL_HAVE_RESULT);
}

bool
thread_task_is_running(const struct thread_task *task) {
    return (task->state == TPOOL_IN_POOL);
>>>>>>> 22ef47c09950bc2339d16ccdc1d03f30d08c5332
}

int thread_task_join(struct thread_task *task, void **result) {
    if (!task->running) return TPOOL_ERR_TASK_NOT_PUSHED;

    pthread_mutex_lock(&task->mutex);
    if(!task->joined) {
        task->joined = true;
        pthread_cond_signal(&task->cond);
    }
    while (!task->finished) {
        pthread_cond_wait(&task->cond, &task->mutex);
    }
    pthread_mutex_unlock(&task->mutex);

    *result = task->result;
    return 0;
}

<<<<<<< HEAD
#ifdef NEED_TIMED_JOIN

int thread_task_timed_join(struct thread_task *task, double timeout, void **result) {
    if (!task->running) return TPOOL_ERR_TASK_NOT_PUSHED;
    if (timeout <= 0) return TPOOL_ERR_TIMEOUT;

    struct timespec abs_timeout;
    clock_gettime(CLOCK_MONOTONIC, &abs_timeout);
    abs_timeout.tv_sec += (long)(timeout);
    abs_timeout.tv_nsec += (long)(timeout * 1000000000) % 1000000000;
    if (abs_timeout.tv_nsec > 10000000000) {
        abs_timeout.tv_sec += 1;
        abs_timeout.tv_nsec -= 10000000000;
    }


    int err = pthread_mutex_timedlock(&task->mutex, &abs_timeout);
    if (err != 0) return TPOOL_ERR_TIMEOUT;
    if(!task->joined) {
        task->joined = true;
        pthread_cond_signal(&task->cond);
    }
    while (!task->finished) {
        err = pthread_cond_timedwait(&task->cond, &task->mutex, &abs_timeout);
        if (err != 0) {
            pthread_mutex_unlock(&task->mutex);
            return TPOOL_ERR_TIMEOUT;
        }
    }
    pthread_mutex_unlock(&task->mutex);

    *result = task->result;
    return 0;
}

#endif

int thread_task_delete(struct thread_task *task) {
    if (task->running && !task->finished) {
        return TPOOL_ERR_TASK_IN_POOL;
    }
    if(task->pool != NULL) {
        pthread_mutex_lock(&task->pool->mutex);
        task->pool->task_finished = (task->pool->task_finished + 1) % (TPOOL_MAX_TASKS + 1);
        pthread_mutex_unlock(&task->pool->mutex);
    }

    pthread_mutex_destroy(&task->mutex);
    pthread_cond_destroy(&task->cond);
=======
int
thread_task_delete(struct thread_task *task) {
    if (thread_task_is_running(task)) return TPOOL_ERR_TASK_IN_POOL;
    if(task->task_fin != NULL) *task->task_fin = (*task->task_fin + 1) % TPOOL_MAX_TASKS;
    pthread_mutex_destroy(&task->task_mutex);
    pthread_cond_destroy(&task->task_cond);
>>>>>>> 22ef47c09950bc2339d16ccdc1d03f30d08c5332
    free(task);
    return 0;
}

#ifdef NEED_DETACH

int thread_task_detach(struct thread_task *task) {
    if (!task->running) return TPOOL_ERR_TASK_NOT_PUSHED;

    pthread_mutex_lock(&task->mutex);
    if(!task->joined) {
        task->joined = true;
        pthread_cond_signal(&task->cond);
    }
    while (!task->finished) {
        pthread_cond_wait(&task->cond, &task->mutex);
    }
    pthread_mutex_unlock(&task->mutex);
    thread_task_delete(task);
    return 0;
}

#endif
