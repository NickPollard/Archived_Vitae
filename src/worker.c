// worker.c
/*
	Vitae Worker thread
   */
#include "common.h"
#include "worker.h"
//-----------------------
#include "system/thread.h"
#include <unistd.h>

#define kMaxWorkerTasks 64
int worker_task_count = 0;
vmutex worker_task_mutex = kMutexInitialiser;
worker_task worker_tasks[kMaxWorkerTasks];

// TODO worker task adding/removing should be lock free
void worker_addTask( worker_task t ) {
	vAssert( worker_task_count < kMaxWorkerTasks );
	vmutex_lock( &worker_task_mutex );
	{
		worker_tasks[worker_task_count++] = t;
	}
	vmutex_unlock( &worker_task_mutex );
}

worker_task worker_nextTask() {
	worker_task task;
	vmutex_lock( &worker_task_mutex );
	{
		task = worker_tasks[0];
		// Move worker tasks down
		for ( int i = 0; i < worker_task_count + 1; ++i ) {
			worker_tasks[i] = worker_tasks[i + 1];
		}
		--worker_task_count;
	}
	vmutex_unlock( &worker_task_mutex );
	
	return task;
}

void* worker_threadFunc( void* args ) {
	(void)args;
	while ( true ) {
		while ( worker_task_count > 0 ) {
			//printf( "Worker performing task.\n" );
			// Grab the first task
			worker_task task = worker_nextTask();
			task.func( task.args );
		}
		//usleep( 5 );
		vthread_yield();
	}
}
