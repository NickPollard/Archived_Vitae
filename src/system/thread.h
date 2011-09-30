// thread.h
#pragma once
#include <pthread.h>

typedef pthread_t vthread;

// Thread functions take a void* (Args) and return a void*
typedef void* (*vthreadfunc)( void* );

// Kick off a thread
vthread vthread_create( vthreadfunc func, void* args );
