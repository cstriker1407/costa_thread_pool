/*
一个线程池实现
1. written by cstriker1407@yeah.net   http://cstriker1407.info/blog/
2. Follow BSD
*/

#include "costa_thread_pool.h"

typedef struct
{
    unsigned int job_idx;
    unsigned int start_time;
    int thread_status;
} costa_thread_context;

typedef struct
{
    costa_thread_context s_thread_ctx[COSTA_TPOOL_MAX_THREADS];

    void (*costa_job_cb)(void *p_param);
    void *p_param;

} costa_tpool_context;

//函数指针
static unsigned int (*s_p_sys_get_time_ms)(void) = costa_null;
static void (*s_p_sys_lock)(void) = costa_null;
static void (*s_p_sys_unlock)(void) = costa_null;
static void (*s_p_sys_sleep)(unsigned int sleep_ms) = costa_null;
static void (*s_p_costa_job_event_cb)(void (*costa_job_cb)(void *p_param), COST_JOB_EVENT event) = costa_null;

static costa_tpool_context s_tpool_ctx = {0};

static unsigned int s_job_idx = 0;
static void __job_process(void *__p_thread_ctx)
{
    costa_thread_context *p_thread_ctx = (costa_thread_context *)__p_thread_ctx;

    p_thread_ctx->thread_status = 0; p_thread_ctx->start_time = s_p_sys_get_time_ms(); costa_debug("costa job process is inited. job-idx:%u start_time:%u thread_status:%d", p_thread_ctx->job_idx, p_thread_ctx->start_time, p_thread_ctx->thread_status);
    while(1)
    {
        s_p_sys_lock();
        if(costa_null == s_tpool_ctx.costa_job_cb)
        {
            costa_debug("no job todo. job-idx:%u stand by.", p_thread_ctx->job_idx);
            s_p_sys_unlock();

            s_p_sys_sleep(1000);
            continue;
        }

        void (*job_cb)(void *p_param) = s_tpool_ctx.costa_job_cb;
        void *p_param = s_tpool_ctx.p_param;
        s_tpool_ctx.costa_job_cb = costa_null;
        s_tpool_ctx.p_param = costa_null;

        costa_debug("got job. job-idx:%u. processing", p_thread_ctx->job_idx);
        s_p_sys_unlock();

        p_thread_ctx->thread_status = 1; p_thread_ctx->start_time = s_p_sys_get_time_ms(); costa_debug("costa job process is started. job-idx:%u start_time:%u thread_status:%d", p_thread_ctx->job_idx, p_thread_ctx->start_time, p_thread_ctx->thread_status);

        s_p_costa_job_event_cb(job_cb, COSTA_JOB_START);
        job_cb(p_param);
        s_p_costa_job_event_cb(job_cb, COSTA_JOB_FINISH);

        p_thread_ctx->thread_status = 2; p_thread_ctx->start_time = s_p_sys_get_time_ms(); costa_debug("costa job process is finished. job-idx:%u start_time:%u thread_status:%d", p_thread_ctx->job_idx, p_thread_ctx->start_time, p_thread_ctx->thread_status);
    }

    p_thread_ctx->start_time = s_p_sys_get_time_ms();
    p_thread_ctx->thread_status = 3;
    costa_debug("costa job process is exited. job-idx:%u start_time:%u thread_status:%d", p_thread_ctx->job_idx, p_thread_ctx->start_time, p_thread_ctx->thread_status);
}

int costa_init( unsigned int (*sys_get_time_ms)(void),
                void (*sys_new_thread_cb)( void (*job_process)(void *p_thread_param), void *p_thread_param),
                void (*sys_lock)(void),
                void (*sys_unlock)(void),
                void (*sys_sleep)(unsigned int sleep_ms),
                void (*costa_job_event_cb)(void (*costa_job_cb)(void *p_param), COST_JOB_EVENT event)
                )
{
    unsigned int tmp_idx = 0;
    if( (costa_null == sys_get_time_ms)   ||  (costa_null == sys_new_thread_cb) ||
        (costa_null == sys_lock)          ||  (costa_null == sys_unlock)        ||
        (costa_null == sys_sleep)         ||  (costa_null == costa_job_event_cb)
       )
    {
        costa_error("input func is null. costa init fail");
        return -1;
    }

    s_p_sys_get_time_ms = sys_get_time_ms;
    s_p_sys_lock = sys_lock;
    s_p_sys_unlock = sys_unlock;
    s_p_sys_sleep = sys_sleep;
    s_p_costa_job_event_cb = costa_job_event_cb;

    costa_memset(&s_tpool_ctx, 0, sizeof(costa_tpool_context));
    for(tmp_idx = 0; tmp_idx < COSTA_TPOOL_MAX_THREADS; tmp_idx++)
    {
        s_tpool_ctx.s_thread_ctx[tmp_idx].job_idx = tmp_idx;
        s_tpool_ctx.s_thread_ctx[tmp_idx].start_time = 0;
        s_tpool_ctx.s_thread_ctx[tmp_idx].thread_status = -1;

        costa_debug("start job thread, job-idx:%u", tmp_idx);
        sys_new_thread_cb( __job_process, s_tpool_ctx.s_thread_ctx + tmp_idx);

    }

    return 0;
}

int costa_add_job(void (*costa_job_cb)(void *p_param), void *p_param)
{
    s_p_sys_lock();

    if(s_tpool_ctx.costa_job_cb != costa_null)
    {
        s_p_sys_unlock();
        costa_error("no costa-job is available, add job fail");
        return -1;
    }
    s_tpool_ctx.costa_job_cb = costa_job_cb;
    s_tpool_ctx.p_param = p_param;
    s_p_sys_unlock();
    costa_debug("add job succes");
    return 0;
}

void costa_print_status(void)
{
    unsigned int tmp_idx = 0;
    costa_debug("--- costa job thread status ---");
    costa_debug("curr time: %u", s_p_sys_get_time_ms());

//    s_p_sys_lock();
    for(tmp_idx = 0; tmp_idx < COSTA_TPOOL_MAX_THREADS; tmp_idx++)
    {
        costa_debug("job-idx:%u start_time:%u thread_status:%d",
                    s_tpool_ctx.s_thread_ctx[tmp_idx].job_idx,
                    s_tpool_ctx.s_thread_ctx[tmp_idx].start_time,
                    s_tpool_ctx.s_thread_ctx[tmp_idx].thread_status
                    );
    }
//    s_p_sys_unlock();
}

































