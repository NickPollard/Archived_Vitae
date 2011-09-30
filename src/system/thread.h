// thread.h
#pragma once
#include <pthread.h>

// Use pthread by default for internal threading
typedef pthread_t vthread;
typedef pthread_mutex_t vmutex;
#define kMutexInitialiser PTHREAD_MUTEX_INITIALIZER;

// Thread functions take a void* (Args) and return a void*
typedef void* (*vthreadfunc)( void* );

// Kick off a thread
vthread vthread_create( vthreadfunc func, void* args );

//
// *** Mutex
//

// Lock a Mutex, preventing other threads from accessing it
void vmutex_lock( vmutex* mutex );
// Relinquish the lock on a Mutex, allowing other threads to access it
void vmutex_unlock( vmutex* mutex );
