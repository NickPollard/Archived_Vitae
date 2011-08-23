// particle.h
#pragma once
#include "maths.h"

#define kmax_particles 128
#define kmax_particle_verts (kmax_particles * 6)

#define kmax_property_values 16

enum particleEmitter_flags {
	kParticleWorldSpace = 0x1
};

typedef struct particle_s {
	float age;
	vector position;
} particle;

typedef struct property_s {
	int count;
	int stride;
	float* data;
} property;

typedef struct particleEmitter_s {
	transform*	trans;

	particle	particles[kmax_particles];
	int		start;
	int		count;

	// Spawn control
	float	lifetime;
	float	spawn_interval;
	float	next_spawn;
	vector	spawn_box;

	// Properties
	property* size;
	property* color;
	vector	velocity;

	// Flags
	uint8_t flags;
} particleEmitter;

particleEmitter* particleEmitter_create();
void particleEmitter_render( void* data );
void particleEmitter_tick( void* e, float dt );

void test_property();

property* property_create( int stride );
void property_addf( property* p, float time, float value );
void property_addv( property* p, float time, vector value );
