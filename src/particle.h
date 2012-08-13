// particle.h
#pragma once
#include "maths/maths.h"
#include "render/vgl.h"

#define kMaxParticles 128
#define kMaxParticleVerts (kMaxParticles * 6)

#define kmax_property_values 16

enum particleEmitter_flags {
	kParticleWorldSpace = 0x1,
	kParticleRandomRotation = 0x2,
	kParticleBurst = 0x3
};

typedef struct particle_s {
	float age;
	vector position;
	float rotation;
} particle;

typedef struct property_s {
	int count;
	int stride;
	float* data;
} property;

typedef struct particleEmitterDef_s {
	float	lifetime;
	float	spawn_interval;
	vector	spawn_box;
	// Properties
	property* size;
	property* color;
	property* spawn_rate;
	vector	velocity;
	GLuint	texture_diffuse;
	uint8_t	flags;
} particleEmitterDef;

struct particleEmitter_s {
	transform*	trans;
	particle	particles[kMaxParticles];
	int		start;
	int		count;
	float	next_spawn;
	float	emitter_age;
	particleEmitterDef*	definition;

	vertex*		vertex_buffer;
	GLushort*	element_buffer;
};

// *** System static init
void particle_init();

// *** Emitter functions
particleEmitterDef* particleEmitterDef_create();
particleEmitter* particleEmitter_create();
particleEmitter* particle_newEmitter( particleEmitterDef* definition );
void particleEmitter_render( void* data );
void particleEmitter_tick( void* e, float dt );

// *** Asset loading
particleEmitterDef* particle_loadAsset( const char* particle_file );

property* property_create( int stride );
property* property_copy( property* p );
void property_addf( property* p, float time, float value );
void property_addfv( property* p, float time, float* values );
void property_addv( property* p, float time, vector value );

// *** Test

void test_property();
