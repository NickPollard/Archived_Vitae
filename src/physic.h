// physic.h
#pragma once
#include "maths/maths.h"

typedef struct physic_s {
	transform* trans;
	vector	velocity;
	float	mass;
} physic;

physic* physic_create();

void physic_tick( void* data, float dt, engine* eng );
