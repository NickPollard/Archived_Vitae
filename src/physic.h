// physic.h
#pragma once
#include "maths/maths.h"

//#define DEBUG_PHYSIC_LIVENESS_TEST

typedef struct physic_s {
	transform* trans;
	vector	velocity;
	float	mass;
	bool	to_delete;
} physic;

physic* physic_create();
void physic_delete( physic* p );

void physic_tick( void* data, float dt, engine* eng );

#ifdef DEBUG_PHYSIC_LIVENESS_TEST
void physic_assertActive( physic* p );
#endif // DEBUG_PHYSIC_LIVENESS_TEST
