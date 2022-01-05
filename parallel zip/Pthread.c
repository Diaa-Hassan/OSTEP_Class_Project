
#include "thread_assert.h"
/////////////////////WRAPPER FUNCTIONS////////////////////////////
// wrapper function are a way of making sure that our function calls work fine while also maintaining clean code 
void Pthread_mutex_lock(pthread_mutex_t *mutex){
int rc = pthread_mutex_lock(mutex);
assert(rc ==0);
}

void Pthread_mutex_unlock(pthread_mutex_t *mutex){
int rc = pthread_mutex_unlock(mutex);
assert(rc ==0);
}

void Pthread_create(pthread_t *thread, const pthread_attr_t * attr, void * (*start_routine)(void*), void * arg){
int rc = pthread_create(thread,attr,start_routine,arg);
assert(rc ==0);
}

void Pthread_join(pthread_t thread, void **value_ptr){
int rc=pthread_join(thread,value_ptr);
assert(rc==0);
}

void Pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m){
int rc = pthread_cond_wait(c,m);
assert(rc ==0);
}
void Pthread_cond_signal(pthread_cond_t *c){
int rc = pthread_cond_signal(c);
assert(rc ==0);
}

void Pthread_cond_broadcast(pthread_cond_t *c){
int rc = pthread_cond_broadcast(c);
assert(rc ==0);
}

void Pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutexattr_t * attr){
int rc=pthread_mutex_init(mutex,attr);
	assert(rc==0);
	}

	
void Pthread_cond_init(pthread_cond_t *cond,pthread_condattr_t * attr){
int rc=pthread_cond_init(cond,attr);
	assert(rc==0);
	}
void Pthread_mutex_destroy(pthread_mutex_t *mutex){
int rc=pthread_mutex_destroy(mutex);
	assert(rc==0);
}
void Pthread_cond_destroy(pthread_cond_t *cond){
int rc=pthread_cond_destroy(cond);
	assert(rc==0);
}
