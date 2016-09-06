#include <errno.h>
#include "../FLXCommon/flxthread.h"

#if defined (FLXLINUX) || defined (FLXUNIX)
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#endif

//************************************
// ��������:  	thread_attr_init
// ����:		��ʼ��pthread_attr_t�ṹ
// ����:		pthread_attr_t * attr
// ����ֵ:   	FLXInt32�� 0��ʾ�ɹ�
//************************************
FLXInt32 thread_attr_init(pthread_attr_t *attr)
{
#ifdef FLXWIN
  memset(attr, 0, sizeof(pthread_attr_t));
  return 0;
#elif defined(FLXLINUX) || defined(FLXUNIX)
  return pthread_attr_init(attr);
#endif
}

//************************************
// ��������:  	thread_create
// ����:		�����߳�
// ����:		FLXThread * tid���߳�ID
// ����:		const pthread_attr_t * attr��pthread_attr_t�ṹ
// ����:		* func���̺߳���ָ��
// ����:		void * arg�������̵߳Ĳ���
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32 thread_create(FLXThread *tid, const pthread_attr_t *attr,
                      void *(*func)(void*), void *arg)
{
#ifdef FLXWIN
  FLXThread handle;
  FLXInt32 ThreadId;
  FLXInt32 dwStackSize = 0;

  if(attr != NULL) 
  {
	  dwStackSize = attr->__stacksize;
  }
  handle = CreateThread(NULL,                    /* pointer to thread security attributes */
                         dwStackSize,             /* initial thread stack size, in bytes   */
 (LPTHREAD_START_ROUTINE)func,                    /* pointer to thread function            */
                         arg,                     /* argument for new thread               */
                         0,                       /* creation flags                        */
       (unsigned long *) &ThreadId                /* pointer to returned thread identifier */
                       );
  *tid = handle;  
   CloseHandle(handle);
  if(handle == NULL) 
  {
	  return -1;
  }
  else
  {
	  return 0;
  }
#elif defined(FLXLINUX) || defined(FLXUNIX)
  FLXInt32 ret = pthread_create(tid, attr, func, arg);
  pthread_detach(*tid);
  return ret;
#endif
}

//************************************
// ��������:  	thread_close_handle
// ����:		�ر��߳̾��
// ����:		FLXThread * tid���߳�ID
// ����ֵ:   	void
//************************************
void thread_close_handle(FLXThread *tid)
{
#ifdef FLXWIN
  CloseHandle(*tid);
#elif defined(FLXLINUX) || defined(FLXUNIX)
  if(tid == NULL) 
  {
	  return;
  }
#endif
}

//************************************
// ��������:  	thread_exit
// ����:		ǿ�ƽ����߳�
// ����:		void * status���̵߳ķ���ֵ(��thread_join)
// ����ֵ:   	void
//************************************
void thread_exit(void *status)
{
#ifdef FLXWIN
  DWORD *ptr;

  ptr = (DWORD *)status;
  if(status == NULL) 
  {
	  ExitThread((DWORD)0);
  }
  else 
  {
	  ExitThread(*ptr);
  }
#elif defined(FLXLINUX) || defined(FLXUNIX)
  pthread_exit(status);
#endif
}

//************************************
// ��������:  	thread_join
// ����:		����̵߳��˳���
// ����:		FLXThread tid���߳�ID
// ����:		void * * status���̵߳ķ���ֵ
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
#define USE_OLD_JOIN
FLXInt32 thread_join(FLXThread tid, void **status)
{
#ifdef FLXWIN
#ifdef USE_OLD_JOIN
  DWORD exitcode;

  while(1)
  {
    GetExitCodeThread(tid, &exitcode);
    if(exitcode != STILL_ACTIVE) 
	{
		return exitcode;
	}
    Sleep(10); /* sleep 10 msec */
  }
#else
  FLXInt32 result = 1;
  DWORD exitcode;
  DWORD dwWait = WaitForSingleObject(tid, INFINITE);
  if(dwWait == WAIT_OBJECT_0)
  {
    if(GetExitCodeThread(tid, &exitcode) == TRUE)
    {
      result = 0;
      *status = (FLXInt32) exitcode; // ???
    }
    else
    {
      result = GetLastError();
    }
  }
  else if(dwWait == WAIT_FAILED)
  {
    result = GetLastError();
  }
  return result;
#endif

#elif defined(FLXLINUX) || defined(FLXUNIX)
  return pthread_join(tid, status);
#endif
}

//************************************
// ��������:  	thread_mutex_init
// ����:		��ʼ��һ��mutex
// ����:		pthread_mutex_t * mptr��mutexָ��
// ����:		const pthread_mutexattr_t * attr��mutex����
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32 thread_mutex_init(pthread_mutex_t *mptr,
                          const pthread_mutexattr_t *attr)
{
#ifdef FLXWIN
  FLXHandle handle = CreateMutex(NULL, FALSE, NULL);
  if(handle)
  {
	  *mptr = handle;
  }
  //old InitializeCriticalSection(mptr);
  return 0;
#elif defined(FLXLINUX) || defined(FLXUNIX)
  return pthread_mutex_init(mptr, attr);
#endif
}

//************************************
// ��������:  	thread_mutex_destroy
// ����:		����һ��mutex
// ����:		pthread_mutex_t * mptr,mutexָ��
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32 thread_mutex_destroy(pthread_mutex_t *mptr)
{
#ifdef FLXWIN
  CloseHandle(*mptr);
  //old DeleteCriticalSection(mptr);
  return 0;
#elif defined(FLXLINUX) || defined(FLXUNIX)
  return pthread_mutex_destroy(mptr);
#endif
}

//************************************
// ��������:  	thread_mutex_lock
// ����:		��mutex
// ����:		pthread_mutex_t * mptr��mutexָ��
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32 thread_mutex_lock(pthread_mutex_t *mptr)
{
#ifdef FLXWIN
  if (WaitForSingleObject(*mptr, INFINITE) == WAIT_OBJECT_0) 
  {
	  return 0;
  }
  //old EnterCriticalSection(mptr); // pointer to critical section object
  return 0;
#elif defined(FLXLINUX) || defined(FLXUNIX)
  return pthread_mutex_lock(mptr);
#endif
}

#ifdef FLXWIN
WINBASEAPI BOOL WINAPI TryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
#endif

//************************************
// ��������:  	thread_mutex_trylock
// ����:		����ȥ��mutex����ֹ����
// ����:		pthread_mutex_t * mptr��mutexָ��
// ����ֵ:   	FLXInt32��0��ʧ�ܣ�1���ɹ�
//************************************
FLXInt32 thread_mutex_trylock(pthread_mutex_t *mptr)
{
#ifdef FLXWIN
  DWORD ret;

  ret = WaitForSingleObject(*mptr, 0);
  if(ret == WAIT_OBJECT_0)
  {
	  return 1;
  }
  else
  {
	  return 0;
  }
  //old ret = TryEnterCriticalSection(mptr); // pointer to critical section object
  return ret;
#elif defined(FLXLINUX) || defined(FLXUNIX)
  FLXInt32 ret;

  ret = pthread_mutex_trylock(mptr);
  if(ret == EBUSY) 
  {
	  return 0;
  }
  return 1;
#endif
}

//************************************
// ��������:  	thread_mutex_unlock
// ����:		����mutex
// ����:		pthread_mutex_t * mptr��mutexָ��
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32 thread_mutex_unlock(pthread_mutex_t *mptr)
{
#ifdef FLXWIN
  ReleaseMutex(*mptr);
  //old LeaveCriticalSection(mptr);
  return 0;
#elif defined(FLXLINUX) || defined(FLXUNIX)
  return pthread_mutex_unlock(mptr);
#endif
}

//************************************
// ��������:  	thread_cancel
// ����:		�˳��߳�
// ����:		FLXThread tid���߳�ID
// ����ֵ:   	FLXInt32��0��ʾ�ɹ������򷵻ش�����
//************************************
FLXInt32 thread_cancel(FLXThread tid)
{
#ifdef FLXWIN
  return (FLXInt32)CloseHandle(tid);
#elif defined(FLXLINUX) || defined(FLXUNIX)
  return pthread_cancel(tid);
#endif
}

//************************************
// ��������:  	rapinit_semaphore
// ����:		��ʼ���ź���
// ����:		WSEMAPHORE * s��s��ָ��WSEMAPHORE�ṹ��ָ��
// ����:		FLXInt32 cmax���ź�����������
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32 rapinit_semaphore(WSEMAPHORE *s, FLXInt32 cmax)
{
/* Create a semaphore with initial count=0 max. counts of cmax. */
#ifdef FLXWIN
  s->cmax = cmax;
  s->hSemaphore = CreateSemaphore(
    NULL,   /* no security attributes */
    0,      /* initial count */
    cmax,   /* maximum count */
    NULL);  /* unnamed semaphore */

  if(s->hSemaphore == NULL) 
  {
	  return -1; /* Check for error. */
  }
  return 0;

#elif defined(FLXLINUX) || defined(FLXUNIX)
  s->cmax   = cmax;
  s->nready = 0;
  thread_mutex_init(&s->mutex, NULL);
  pthread_cond_init(&s->cond, NULL);
  return 0;
#endif
}

//************************************
// ��������:  	rapdestroy_semaphore
// ����:		�����ź���
// ����:		WSEMAPHORE * s��s��ָ��WSEMAPHORE�ṹ��ָ��
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32 rapdestroy_semaphore(WSEMAPHORE *s)
{
#ifdef FLXWIN
  CloseHandle(s->hSemaphore);
#elif defined(FLXLINUX) || defined(FLXUNIX)
  thread_mutex_destroy(&s->mutex);
#endif
  return 0;
}

//************************************
// ��������:  	rapincrement_semaphore
// ����:		����һ���ź���
// ����:		WSEMAPHORE * s��s��ָ��WSEMAPHORE�ṹ��ָ��
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32 rapincrement_semaphore(WSEMAPHORE *s)
{
/* Increment the count of the semaphore. */
#ifdef FLXWIN
  if(!ReleaseSemaphore(
        s->hSemaphore,  /* handle of semaphore */
        1,              /* increase count by one */
        NULL) )         /* not interested in previous count */
  {
    return -1; /* Deal with the error. */
  }
  return 0;

#elif defined(FLXLINUX) || defined(FLXUNIX)
  pthread_mutex_lock(&s->mutex);
  if(s->nready == 0) 
  {
	  pthread_cond_signal(&s->cond);
  }
  s->nready ++;
  pthread_mutex_unlock(&s->mutex);
  return 0;
#endif
}

//************************************
// ��������:  	rapwait_semaphore
// ����:		�ȴ��ź���
// ����:		WSEMAPHORE * s��ָ��WSEMAPHORE�ṹ��ָ��
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32 rapwait_semaphore(WSEMAPHORE *s)
{
#ifdef FLXWIN
  FLXInt32 ret;
  ret = WaitForSingleObject(
        s->hSemaphore,   /* handle of semaphore */
        INFINITE);       /* infinite time-out interval */
  if(ret) 
  {
	  return 0;
  }
  return 0;

#elif defined(FLXLINUX) || defined(FLXUNIX)
  pthread_mutex_lock(&s->mutex);
  while(s->nready == 0)
  {
    pthread_cond_wait(&s->cond, &s->mutex);
  }  
  s->nready --;
  pthread_mutex_unlock(&s->mutex);
  return 0;
#endif
}

//************************************
// ��������:  	thread_sleep
// ����:		sleep
// ����:		long msec����ʱ����
// ����ֵ:   	FLXInt32��0��ʾ�ɹ�
//************************************
FLXInt32 thread_sleep(long msec)
{
#ifdef FLXWIN
  Sleep(msec);
  return 0;

#elif defined(FLXLINUX) || defined(FLXUNIX)
  fd_set wset, rset, eset;
  struct timeval timeout;

  FD_ZERO(&rset);
  FD_ZERO(&wset);
  FD_ZERO(&eset);
  timeout.tv_sec  = msec / 1000;
  timeout.tv_usec = (msec % 1000) * 1000;
  select(1, &rset, &wset, &eset, &timeout);
  return 0;

#elif defined __VMS
  struct timespec interval;

  interval.tv_sec  =  msec / 1000;
  interval.tv_nsec = (msec % 1000) * 1000 * 1000; /* wait msec msec */
  pthread_delay_np(&interval);
  return 0;
#endif
}

//************************************
// ��������:  	rlsleep
// ����:		sleep
// ����:		long msec����ʱ����
// ����ֵ:   	void
//************************************
void rlsleep(long msec)
{
  thread_sleep(msec);
}
