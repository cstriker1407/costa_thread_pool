/*
一个线程池实现
1. written by cstriker1407@yeah.net   http://cstriker1407.info/blog/
2. Follow BSD
*/

#ifndef COSTA_THREAD_POOL_H
#define COSTA_THREAD_POOL_H

#include <stdio.h>
#include <string.h>
#define costa_memset memset

#define costa_null 0
#define costa_debug(fmt, ...) printf("[DBG %4d]"fmt"\r\n", __LINE__, ##__VA_ARGS__)
#define costa_error(fmt, ...) printf("[ERR %4d]"fmt"\r\n", __LINE__, ##__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

//线程池最大线程数目
#define COSTA_TPOOL_MAX_THREADS 5

typedef enum
{
    COSTA_JOB_START,
    COSTA_JOB_FINISH
}COST_JOB_EVENT;

/* 初始化一个线程池 */
int costa_init( unsigned int (*sys_get_time_ms)(void),           //获取当前时间毫秒数
                void (*sys_new_thread_cb)( void (*job_process)(void *p_thread_param), void *p_thread_param), //系统开启线程回调
                void (*sys_lock)(void),                          //加锁
                void (*sys_unlock)(void),                        //解锁
                void (*sys_sleep)(unsigned int sleep_ms),        //系统休眠回调
                void (*costa_job_event_cb)(void (*costa_job_cb)(void *p_param), COST_JOB_EVENT event) //内存池状态回调, nolock!!!
                );

//线程池执行任务定义
typedef void (*costa_job_cb)(void *p_param);
/* 添加一个任务 */
int costa_add_job(void (*costa_job_cb)(void *p_param), void *p_param);

/* 打印线程池状态 */
void costa_print_status(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif // COSTA_THREAD_POOL_H
