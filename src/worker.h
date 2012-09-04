// worker.h
#pragma once

typedef void* (*taskFunc)( void* );

typedef struct worker_task_s {
	taskFunc func;
	void* args;
} worker_task;

void* worker_threadFunc( void* args );

void worker_addTask( worker_task t );
worker_task worker_nextTask();
