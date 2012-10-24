// physic.h
#pragma once
#include "maths/maths.h"

typedef struct physic_s {
	transform* trans;
	vector	velocity;
	float	mass;
	bool	to_delete;
} physic;

physic* physic_create();
void physic_delete( physic* p );

void physic_tick( void* data, float dt, engine* eng );
void physic_assertActive( physic* p );
