#include <stdio.h>

#include "costa_thread_pool.h"

#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

unsigned int sys_get_time_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

typedef struct
{
    pthread_t thread;
    void (*job_process)(void *p_thread_param);
    void *p_thread_param;
}linux_thread_ctx;

void *linux_start_routine(void *__p_ctx)
{
    linux_thread_ctx *p_ctx = (linux_thread_ctx *)__p_ctx;
    p_ctx->job_process(p_ctx->p_thread_param);
    return 0;
}

void sys_new_thread_cb( void (*job_process)(void *p_thread_param), void *p_thread_param)
{
    linux_thread_ctx *p_leak_ctx = malloc(sizeof(linux_thread_ctx));
    memset(p_leak_ctx, 0, sizeof(linux_thread_ctx));
    p_leak_ctx->job_process = job_process;
    p_leak_ctx->p_thread_param = p_thread_param;
    pthread_create(&(p_leak_ctx->thread), 0, linux_start_routine, p_leak_ctx);
    pthread_detach(p_leak_ctx->thread);
}

static pthread_mutex_t s_mutex = {0};

void sys_lock(void)
{
    pthread_mutex_lock(&s_mutex);
}

void sys_unlock(void)
{
    pthread_mutex_unlock(&s_mutex);
}

void sys_sleep(unsigned int sleep_ms)
{
    sleep(sleep_ms/1000);
    usleep(sleep_ms % 1000 * 1000);
}


void test_job(void *p_param)
{
    int sleep_s = (int)p_param;

    printf("new job start. sleep %d \r\n", sleep_s);
    sleep(sleep_s);
    printf("new job finish. sleep %d \r\n", sleep_s);
}

void costa_job_event_cb(void (*costa_job_cb)(void *p_param), COST_JOB_EVENT event)
{
    printf("event:%d \r\n", event);
    static int sleep_s = 2;
    costa_add_job(test_job, (void *)sleep_s);
    sleep_s = 1 + (sleep_s+2) % 4;
}

int main(void)
{
    printf("Hello World!\n");
    pthread_mutex_init(&s_mutex, 0);

    costa_init( sys_get_time_ms,
                sys_new_thread_cb,
                sys_lock,
                sys_unlock,
                sys_sleep,
                costa_job_event_cb
                );

    sleep(2);

    while(1)
    {
        costa_add_job(test_job, (void *)2);
        costa_print_status();
        sleep(2);
    }

    return 0;
}
