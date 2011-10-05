// thread.c

#include "common.h"
#include "thread.h"
//-------------------------
#include <sched.h> // for sched_yield

vcondition	conditions[kMaxConditions];
vmutex		condition_mutices[kMaxConditions];
bool		condition_values[kMaxConditions];

// Static Initializer
void vthread_init() {
	// *** Initialize Conditions
	for ( int i = 0; i < kMaxConditions; i++ ) {
		pthread_cond_init( &conditions[i], NULL );
		pthread_mutex_init( &condition_mutices[i], NULL );
		condition_values[i] = false;
	}
}

// *** Threads

// Kick off a thread
vthread vthread_create( vthreadfunc func, void* args ) {
	vthread t;
	pthread_create( &t,  /*attributes*/ NULL, func, args );
	return t;
}

void vthread_yield() {
	sched_yield();
}

// *** Mutices

// Lock a Mutex, preventing other threads from accessing it
void vmutex_lock( vmutex* mutex ) {
	pthread_mutex_lock( mutex );
}

// Relinquish the lock on a Mutex, allowing other threads to access it
void vmutex_unlock( vmutex* mutex ) {
	pthread_mutex_unlock( mutex );
}

// *** Conditions

void vthread_signalCondition( int i ) {
//	printf( "THREAD: Setting condition %d.\n", i );
	vmutex*		condition_mutex	= &condition_mutices[i];
	vcondition*	condition		= &conditions[i];

	vmutex_lock( condition_mutex );
	condition_values[i] = true;
	pthread_cond_signal( condition );
	vmutex_unlock( condition_mutex );
}

void vthread_waitCondition( int i ) {
//	printf( "THREAD: Waiting for condition %d.\n", i );
	vmutex*		condition_mutex	= &condition_mutices[i];
	vcondition*	condition		= &conditions[i];

	vmutex_lock( condition_mutex );
	while ( condition_values[i] == false ) {
		pthread_cond_wait( condition, condition_mutex );
	}
//	printf( "THREAD: Received condition %d.\n", i );
	condition_values[i] = false;
	vmutex_unlock( condition_mutex );
}
