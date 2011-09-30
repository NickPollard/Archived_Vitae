// thread.c

#include "common.h"
#include "thread.h"
//-------------------------

// Kick off a thread
vthread vthread_create( vthreadfunc func, void* args ) {
	vthread t;
	pthread_create( &t,  /*attributes*/ NULL, func, args );
	return t;
}

// Lock a Mutex, preventing other threads from accessing it
void vmutex_lock( vmutex* mutex ) {
	pthread_mutex_lock( mutex );
}

// Relinquish the lock on a Mutex, allowing other threads to access it
void vmutex_unlock( vmutex* mutex ) {
	pthread_mutex_unlock( mutex );
}
