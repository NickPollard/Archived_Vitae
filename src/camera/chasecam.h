// chasecam.h
#pragma once

#include "maths/maths.h"
#include "camera.h"

#define kDefaultChasecamLerpSpeed 4.f
#define kDefaultChasecamSlerpSpeed 4.f

typedef struct chasecam_s {
	// Camera must come first
	camera		cam;
	transform*	target;

	quaternion	rotation;
	vector		position;
	float		lerp_speed;
	float		slerp_speed;
} chasecam;

#define DEFAULT_CREATE_HEAD(type) \
	type* type##_create();

#define DEFAULT_CREATE_SRC(type) \
	type* type##_create() { \
		type* t = mem_alloc( sizeof( type )); \
		memset( t, 0, sizeof( type ));\
		return t; \
	}

chasecam*	chasecam_create();
void		chasecam_tick( void* data, float dt, engine* eng );
void		chasecam_setTarget( chasecam* c, transform* t );
