//*************************************************************************
//
// Copyright (C) 2010-2100, PHILISENSE TECHNOLOGY CO
//
//
//*************************************************************************
////////////////////////////////////////////////////////////////////////////////////
//文件名称：flxthread.h
//
//文件说明：
//
//版本：	v 1.0
//
//创建时间: 2010/11/22 14:51
//
//创建人：	chen zhi tao
//
//修改人：	XX
//
//修改时间：XXXX年x月X日
//
//修改内容：
//
///////////////////////////////////////////////////////////////////////////////////

#ifndef FLX_THREAD_HEADER_FILE
#define FLX_THREAD_HEADER_FILE

#include "flxtypes.h"

#ifdef  FLXWIN
#include <windows.h>
#include <winbase.h>
#include <stddef.h>
#include <string.h>

typedef FLXHandle FLXThread;
/* Attributes for threads */
typedef struct __sched_param
{
	FLXInt32 sched_priority;
}SCHED_PARAM;

typedef struct
{
	FLXInt32     __detachstate;
	FLXInt32     __schedpolicy;
	struct  __sched_param __schedparam;
	FLXInt32     __inheritsched;
	FLXInt32     __scope;
	size_t  __guardsize;
	FLXInt32     __stackaddr_set;
	void   *__stackaddr;
	size_t  __stacksize;
}pthread_attr_t;

typedef FLXHandle pthread_mutex_t;
//old typedef CRITICAL_SECTION pthread_mutex_t;
typedef long             pthread_mutexattr_t;

#elif defined(FLXLINUX) || defined(FLXUNIX)
#include <pthread.h>

typedef pthread_t FLXThread;
#endif

#ifndef _WRAPTHREAD_
#ifndef _WTHREAD_H_
typedef struct
{
#ifdef FLXWIN
	FLXInt32    cmax;
	FLXHandle hSemaphore;
#elif defined(FLXLINUX) || defined(FLXUNIX)
	FLXInt32              cmax;
	FLXInt32              nready;
	pthread_mutex_t  mutex;
	pthread_cond_t   cond;
#endif
}WSEMAPHORE;
#endif
#endif

//************************************
// 函数名称:  	thread_attr_init
// 功能:		初始化pthread_attr_t结构
// 参数:		pthread_attr_t * attr
// 返回值:   	FLXInt32， 0表示成功
//************************************
FLXInt32  thread_attr_init(pthread_attr_t *attr);

//************************************
// 函数名称:  	thread_create
// 功能:		创建线程
// 参数:		FLXThread * tid，线程ID
// 参数:		const pthread_attr_t * attr，pthread_attr_t结构
// 参数:		* func，线程函数指针
// 参数:		void * arg，传入线程的参数
// 返回值:   	FLXInt32，0表示成功
//************************************
FLXInt32  thread_create(FLXThread *tid, const pthread_attr_t *attr, void *(*func)(void*), void *arg);

//************************************
// 函数名称:  	thread_close_handle
// 功能:		关闭线程句柄
// 参数:		FLXThread * tid，线程ID
// 返回值:   	void
//************************************
void thread_close_handle(FLXThread *tid);

//************************************
// 函数名称:  	thread_exit
// 功能:		强制结束线程
// 参数:		void * status，线程的返回值(见thread_join)
// 返回值:   	void
//************************************
void thread_exit(void *status);

//************************************
// 函数名称:  	thread_join
// 功能:		获得线程的退出码
// 参数:		FLXThread tid，线程ID
// 参数:		void * * status，线程的返回值
// 返回值:   	FLXInt32，0表示成功
//************************************
FLXInt32  thread_join(FLXThread tid, void **status);

//************************************
// 函数名称:  	thread_mutex_init
// 功能:		初始化一个mutex
// 参数:		pthread_mutex_t * mptr，mutex指针
// 参数:		const pthread_mutexattr_t * attr，mutex属性
// 返回值:   	FLXInt32，0表示成功
//************************************
FLXInt32  thread_mutex_init(pthread_mutex_t *mptr, const pthread_mutexattr_t *attr);

//************************************
// 函数名称:  	thread_mutex_destroy
// 功能:		销毁一个mutex
// 参数:		pthread_mutex_t * mptr,mutex指针
// 返回值:   	FLXInt32，0表示成功
//************************************
FLXInt32  thread_mutex_destroy(pthread_mutex_t *mptr);

//************************************
// 函数名称:  	thread_mutex_lock
// 功能:		锁mutex
// 参数:		pthread_mutex_t * mptr，mutex指针
// 返回值:   	FLXInt32，0表示成功
//************************************
FLXInt32  thread_mutex_lock(pthread_mutex_t *mptr);

//************************************
// 函数名称:  	thread_mutex_trylock
// 功能:		尝试去锁mutex，防止阻塞
// 参数:		pthread_mutex_t * mptr，mutex指针
// 返回值:   	FLXInt32，0锁失败，1锁成功
//************************************
FLXInt32  thread_mutex_trylock(pthread_mutex_t *mptr);

//************************************
// 函数名称:  	thread_mutex_unlock
// 功能:		解锁mutex
// 参数:		pthread_mutex_t * mptr，mutex指针
// 返回值:   	FLXInt32，0表示成功
//************************************
FLXInt32  thread_mutex_unlock(pthread_mutex_t *mptr);

//************************************
// 函数名称:  	thread_cancel
// 功能:		退出线程
// 参数:		FLXThread tid，线程ID
// 返回值:   	FLXInt32，0表示成功，否则返回错误编号
//************************************
FLXInt32  thread_cancel(FLXThread tid);

//************************************
// 函数名称:  	rapinit_semaphore
// 功能:		初始化信号量
// 参数:		WSEMAPHORE * s，s是指向WSEMAPHORE结构的指针
// 参数:		FLXInt32 cmax，信号量的最大个数
// 返回值:   	FLXInt32，0表示成功
//************************************
FLXInt32  rapinit_semaphore(WSEMAPHORE *s, FLXInt32 cmax);

//************************************
// 函数名称:  	rapdestroy_semaphore
// 功能:		销毁信号量
// 参数:		WSEMAPHORE * s，s是指向WSEMAPHORE结构的指针
// 返回值:   	FLXInt32，0表示成功
//************************************
FLXInt32  rapdestroy_semaphore(WSEMAPHORE *s);

//************************************
// 函数名称:  	rapincrement_semaphore
// 功能:		增加一个信号量
// 参数:		WSEMAPHORE * s，s是指向WSEMAPHORE结构的指针
// 返回值:   	FLXInt32，0表示成功
//************************************
FLXInt32  rapincrement_semaphore(WSEMAPHORE *s);

//************************************
// 函数名称:  	rapwait_semaphore
// 功能:		等待信号量
// 参数:		WSEMAPHORE * s，指向WSEMAPHORE结构的指针
// 返回值:   	FLXInt32，0表示成功
//************************************
FLXInt32  rapwait_semaphore(WSEMAPHORE *s);

//************************************
// 函数名称:  	thread_sleep
// 功能:		sleep
// 参数:		long msec，延时毫秒
// 返回值:   	FLXInt32，0表示成功
//************************************
FLXInt32  thread_sleep(long msec);

//************************************
// 函数名称:  	rlsleep
// 功能:		sleep
// 参数:		long msec，延时毫秒
// 返回值:   	void
//************************************
void rlsleep(long msec);

#endif // FLX_THREAD_HEADER_FILE






