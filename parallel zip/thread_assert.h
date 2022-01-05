#ifndef _thread_assert_h
#define _thread_assert_h
#include <assert.h>
#include <pthread.h>
void Pthread_mutex_lock(pthread_mutex_t *mutex);
void Pthread_mutex_unlock(pthread_mutex_t *mutex);
void Pthread_create(pthread_t *thread, const pthread_attr_t * attr, void * (*start_routine)(void*), void * arg);
void Pthread_join(pthread_t thread, void **value_ptr);
void Pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m);
void Pthread_cond_signal(pthread_cond_t *c);
void Pthread_cond_broadcast(pthread_cond_t *c);
void Pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t * attr);
void Pthread_cond_init(pthread_cond_t *cond,pthread_condattr_t * attr);
void Pthread_mutex_destroy(pthread_mutex_t *mutex);
void Pthread_cond_destroy(pthread_cond_t *cond);

#endif
