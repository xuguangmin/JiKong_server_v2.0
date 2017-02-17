//*************************************************************************
//
// Copyright (C) 2010-2100, PHILISENSE TECHNOLOGY CO
//
//
//*************************************************************************
////////////////////////////////////////////////////////////////////////////////////
//�ļ����ƣ�flxthread.h
//
//�ļ�˵����
//
//�汾��	v 1.0
//
//����ʱ��: 2010/11/22 14:51
//
//�����ˣ�	chen zhi tao
//
//�޸��ˣ�	XX
//
//�޸�ʱ�䣺XXXX��x��X��
//
//�޸����ݣ�
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
// ��������:  	thread_attr_init
// ����:		��ʼ��pthread_attr_t�ṹ
// ����:		pthread_attr_t * attr
// ����ֵ:   	FLXInt32�� 0��ʾ�ɹ�
//************************************
FLXInt32  thread_attr_init(pthread_attr_t *attr);

//************************************
// ��������:  	thread_create
// ����:		�����߳�
// ����:		FLXThread * tid���߳�ID
// ����:		const pthread_attr_t * attr��pthread_attr_t�ṹ
// ����:		* func���̺߳���ָ��
// ����:		void * arg�������̵߳Ĳ���
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32  thread_create(FLXThread *tid, const pthread_attr_t *attr, void *(*func)(void*), void *arg);

//************************************
// ��������:  	thread_close_handle
// ����:		�ر��߳̾��
// ����:		FLXThread * tid���߳�ID
// ����ֵ:   	void
//************************************
void thread_close_handle(FLXThread *tid);

//************************************
// ��������:  	thread_exit
// ����:		ǿ�ƽ����߳�
// ����:		void * status���̵߳ķ���ֵ(��thread_join)
// ����ֵ:   	void
//************************************
void thread_exit(void *status);

//************************************
// ��������:  	thread_join
// ����:		����̵߳��˳���
// ����:		FLXThread tid���߳�ID
// ����:		void * * status���̵߳ķ���ֵ
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32  thread_join(FLXThread tid, void **status);

//************************************
// ��������:  	thread_mutex_init
// ����:		��ʼ��һ��mutex
// ����:		pthread_mutex_t * mptr��mutexָ��
// ����:		const pthread_mutexattr_t * attr��mutex����
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32  thread_mutex_init(pthread_mutex_t *mptr, const pthread_mutexattr_t *attr);

//************************************
// ��������:  	thread_mutex_destroy
// ����:		����һ��mutex
// ����:		pthread_mutex_t * mptr,mutexָ��
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32  thread_mutex_destroy(pthread_mutex_t *mptr);

//************************************
// ��������:  	thread_mutex_lock
// ����:		��mutex
// ����:		pthread_mutex_t * mptr��mutexָ��
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32  thread_mutex_lock(pthread_mutex_t *mptr);

//************************************
// ��������:  	thread_mutex_trylock
// ����:		����ȥ��mutex����ֹ����
// ����:		pthread_mutex_t * mptr��mutexָ��
// ����ֵ:   	FLXInt32��0��ʧ�ܣ�1���ɹ�
//************************************
FLXInt32  thread_mutex_trylock(pthread_mutex_t *mptr);

//************************************
// ��������:  	thread_mutex_unlock
// ����:		����mutex
// ����:		pthread_mutex_t * mptr��mutexָ��
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32  thread_mutex_unlock(pthread_mutex_t *mptr);

//************************************
// ��������:  	thread_cancel
// ����:		�˳��߳�
// ����:		FLXThread tid���߳�ID
// ����ֵ:   	FLXInt32��0��ʾ�ɹ������򷵻ش�����
//************************************
FLXInt32  thread_cancel(FLXThread tid);

//************************************
// ��������:  	rapinit_semaphore
// ����:		��ʼ���ź���
// ����:		WSEMAPHORE * s��s��ָ��WSEMAPHORE�ṹ��ָ��
// ����:		FLXInt32 cmax���ź�����������
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32  rapinit_semaphore(WSEMAPHORE *s, FLXInt32 cmax);

//************************************
// ��������:  	rapdestroy_semaphore
// ����:		�����ź���
// ����:		WSEMAPHORE * s��s��ָ��WSEMAPHORE�ṹ��ָ��
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32  rapdestroy_semaphore(WSEMAPHORE *s);

//************************************
// ��������:  	rapincrement_semaphore
// ����:		����һ���ź���
// ����:		WSEMAPHORE * s��s��ָ��WSEMAPHORE�ṹ��ָ��
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32  rapincrement_semaphore(WSEMAPHORE *s);

//************************************
// ��������:  	rapwait_semaphore
// ����:		�ȴ��ź���
// ����:		WSEMAPHORE * s��ָ��WSEMAPHORE�ṹ��ָ��
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32  rapwait_semaphore(WSEMAPHORE *s);

//************************************
// ��������:  	thread_sleep
// ����:		sleep
// ����:		long msec����ʱ����
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32  thread_sleep(long msec);

//************************************
// ��������:  	rlsleep
// ����:		sleep
// ����:		long msec����ʱ����
// ����ֵ:   	void
//************************************
void rlsleep(long msec);

#endif // FLX_THREAD_HEADER_FILE






