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
