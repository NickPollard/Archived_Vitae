// particle.h
#pragma once
#include "maths.h"

#define kmax_particle_verts 512
#define kmax_particles 128

typedef struct particle_s {
	vector position;
} particle;

typedef struct particleEmitter_s {
	particle	particles[kmax_particles];
	int		count;
	float	size;
	float	spawn_interval;
	float	next_spawn;
	vector	velocity;
} particleEmitter;

particleEmitter* particleEmitter_create();
void particleEmitter_addParticle( particleEmitter* p );
void particleEmitter_render( void* data );
void particleEmitter_tick( void* e, float dt );
