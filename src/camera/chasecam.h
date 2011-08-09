// chasecam.h
#pragma once

typedef struct chasecam_s {
	transform*	target;
	camera*		cam;
} chasecam;

#define DEFAULT_CREATE_HEAD(type) \
	type* type##_create();

#define DEFAULT_CREATE_SRC(type) \
	type* type##_create() { \
		type* t = mem_alloc( sizeof( type )); \
		memset( t, 0, sizeof( type ));\
		return t; \
	}

chasecam* chasecam_create();

void chasecam_tick( void* data, float dt );
