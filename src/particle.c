// particle.c
#include "src/common.h"
#include "src/particle.h"
//---------------------
#include "model.h"
#include "maths/vector.h"
#include "mem/allocator.h"
#include "render/debugdraw.h"
#include "render/render.h"
#include "render/shader.h"
#include "render/texture.h"
#include "script/lisp.h"
#include "system/file.h"
#include "system/hash.h"

float property_samplef( property* p, float time );
vector property_samplev( property* p, float time );
float property_valuef( property* p, int key );
property* property_range( property* p, float from, float to, property* buffer );

// Debug
particleEmitter* active_particles[1024];
int active_particle_count = 0;

particleEmitterDef* particleEmitterDef_create() {
	particleEmitterDef* def = mem_alloc( sizeof( particleEmitterDef ));
	memset( def, 0, sizeof( particleEmitterDef ));
	def->spawn_box = Vector( 0.f, 0.f, 0.f, 0.f );
	def->velocity = Vector( 0.f, 0.f, 0.f, 0.f );
	return def;
}

// De-init the particleEmitterDef, freeing all its members
void particleEmitterDef_deInit( particleEmitterDef* def ) {
	if ( def->size ) {
		mem_free( def->size );
		def->size = NULL;
	}
	if ( def->color ) {
		mem_free( def->color );
		def->color = NULL;
	}
	if ( def->spawn_rate ) {
		mem_free( def->spawn_rate );
		def->spawn_rate = NULL;
	}
}

particleEmitter* particleEmitter_create() {
	particleEmitter* p = mem_alloc( sizeof( particleEmitter ));
	memset( p, 0, sizeof( particleEmitter ));
	p->definition = NULL;
	p->vertex_buffer = mem_alloc( sizeof( vertex ) * kMaxParticleVerts );
	p->element_buffer = mem_alloc( sizeof( GLushort ) * kMaxParticleVerts );
	p->destroyed = false;
	
	//printf( "Adding particle 0x" xPTRf ".\n", (uintptr_t)p );
	array_add( (void**)active_particles, &active_particle_count, (void*)p );

	return p;
}

void particleEmitter_spawnParticle( particleEmitter* e ) {
	int index = (e->start + e->count) % kMaxParticles;
	particle* p = &e->particles[index];
	e->count++;
	if ( e->count > kMaxParticles ) {
		e->count--;
		e->start++;
	}

	// Generate spawn position
	vector r;
	r.coord.x = frand( -1.f, 1.f );
	r.coord.y = frand( -1.f, 1.f );
	r.coord.z = frand( -1.f, 1.f );
	r.coord.w = 1.f;
	vector offset = vector_mul( &r, &e->definition->spawn_box );
	offset.coord.w = 1.f;

	// If worldspace, spawn at the particle emitter position
	if ( !(e->definition->flags & kParticleWorldSpace ))
		p->position	= offset;
	else
		p->position = matrix_vecMul( e->trans->world, &offset );
	p->age = 0.f;
	if ( e->definition->flags & kParticleRandomRotation )
		p->rotation = frand( 0.f, 2*PI );
	else
		p->rotation = 0.f;
}

void particleEmitter_tick( void* data, float dt, engine* eng ) {
	particleEmitter* e = data;

	if ( e->destroyed && e->count <= 0 ) {
		engine_removeRender( eng, e, particleEmitter_render );
		stopTick( eng, e, particleEmitter_tick );
		particleEmitter_delete( e );
		return;
	}

#ifdef DEBUG_PARTICLE_LIVENESS_TEST
	particleEmitter_assertActive( e );
#endif // DEBUG_PARTICLE_LIVENESS_TEST

	// Update existing particles
	vector delta;
	vector_scale ( &delta, &e->definition->velocity, dt );
	int count = e->count;
	int start = e->start;
	for ( int i = 0; i < count; i++ ) {
		int index = (start + i) % kMaxParticles;
		// Update age
		e->particles[index].age += dt;
		if ( e->particles[index].age > e->definition->lifetime ) {
			e->count--;
			e->start++;
		}
		// Apply Velocity
		Add( &e->particles[index].position, &e->particles[index].position, &delta );
	}

	// Spawn new particle
	vAssert( e->definition->spawn_rate );
	// Only spawn if we haven't been requested to destroy
	if ( !e->destroyed ) {
		// Burst mode means we batch-spawn particles on keys, otherwise spawn nothing
		if ( e->definition->flags & kParticleBurst ) {
			// Use some stack space to save on a mem_alloc
			// We just create a buffer and pass that to property_range()
			size_t alloc_size = sizeof( property ) + sizeof( float ) * e->definition->spawn_rate->stride * kMaxPropertyValues;
			property* buffer = alloca( alloc_size );

			property* keys = property_range( e->definition->spawn_rate, e->emitter_age, e->emitter_age + dt, buffer );
			for ( int key = 0; key < keys->count; ++key ) {
				int count = (int)property_valuef( e->definition->spawn_rate, key );
				for ( int i = 0; i < count; ++i ) {
					particleEmitter_spawnParticle( e );
				}
			}
		}
		// Default is normal interpolated spawning
		else {
			e->next_spawn += dt;
			float spawn_rate = property_samplef( e->definition->spawn_rate, e->emitter_age );
			float spawn_interval = 1.f / spawn_rate;
			// We might spawn more than one particle per frame, if the frame is long or the spawn interval is short
			while ( e->next_spawn > spawn_interval ) {
				e->next_spawn = fmaxf( 0.f, e->next_spawn - spawn_interval);
				particleEmitter_spawnParticle( e );
			}
		}
	}
	e->emitter_age += dt;

	// TEST
	/*
	if ( e->emitter_age > 5.f ) {
		engine_removeRender( eng, e, particleEmitter_render );
		stopTick( eng, e, particleEmitter_tick );
	}
	*/
}

// Output the 4 verts of the quad to the target vertex array
// both position and normals
void particle_quad( particleEmitter* e, vertex* dst, vector* point, float rotation, float size, vector color ) {
	vector offset = Vector( size, size, 0.f, 0.f );

	vector p;
	if ( !(e->definition->flags & kParticleWorldSpace ))
		p = matrix_vecMul( modelview, point );
	else
		p = matrix_vecMul( camera_inverse, point );

	// Particle Rotation
	matrix m;
	matrix_rotZ( m, rotation );
	offset = matrix_vecMul( m, &offset );

	Add( &dst[0].position, &p, &offset );
	dst[0].normal = Vector( 0.f, 0.f, 1.f, 0.f );
	dst[0].uv = Vector( 1.f, 1.f, 0.f, 0.f );
	dst[0].color = color;

	Sub( &dst[1].position, &p, &offset );
	dst[1].normal = Vector( 0.f, 0.f, 1.f, 0.f );
	dst[1].uv = Vector( 0.f, 0.f, 0.f, 0.f );
	dst[1].color = color;

	offset = Vector( size, -size, 0.f, 0.f );
	offset = matrix_vecMul( m, &offset );

	Add( &dst[2].position, &p, &offset );
	dst[2].normal = Vector( 0.f, 0.f, 1.f, 0.f );
	dst[2].uv = Vector( 1.f, 0.f, 0.f, 0.f );
	dst[2].color = color;

	Sub( &dst[3].position, &p, &offset );
	dst[3].normal = Vector( 0.f, 0.f, 1.f, 0.f );
	dst[3].uv = Vector( 0.f, 1.f, 0.f, 0.f );
	dst[3].color = color;
}

// Render a particleEmitter system
void particleEmitter_render( void* data ) {
	particleEmitter* p = data;

	if ( p->destroyed )
		return;

#ifdef DEBUG_PARTICLE_LIVENESS_TEST
	particleEmitter_assertActive( p );
#endif // DEBUG_PARTICLE_LIVENESS_TEST
		
	// reset modelview matrix so we can billboard
	// particle_quad() will manually apply the modelview
	render_resetModelView();
	matrix_mul( modelview, modelview, p->trans->world );

	for ( int i = 0; i < p->count; i++ ) {
		int index = (p->start + i) % kMaxParticles;

		// Sample properties
		float	size	= property_samplef( p->definition->size, p->particles[index].age );
		vector	color	= property_samplev( p->definition->color, p->particles[index].age );

		particle_quad( p, &p->vertex_buffer[i*4], &p->particles[index].position, p->particles[index].rotation, size, color );

		vAssert( ( i*6 + 5 ) < kMaxParticleVerts );

		// TODO: Indices can be initialised once
		p->element_buffer[i*6+0] = i*4+1;
		p->element_buffer[i*6+1] = i*4+0;
		p->element_buffer[i*6+2] = i*4+2;
		p->element_buffer[i*6+3] = i*4+0;
		p->element_buffer[i*6+4] = i*4+1;
		p->element_buffer[i*6+5] = i*4+3;
	}

	// For Billboard particles; cancel out the rotation of the matrix
	// The transformation has been applied already for particle positions
	matrix_setIdentity( modelview );
	int index_count = 6 * p->count;
	// We only need to send this to the GPU if we actually have something to draw (i.e. particles have been emitted)
	if ( index_count > 0 ) {
		drawCall* draw = drawCall_create( &renderPass_alpha, resources.shader_particle, index_count, p->element_buffer, p->vertex_buffer, 
											p->definition->texture_diffuse->gl_tex, modelview );
		draw->depth_mask = GL_FALSE;
	}
}

property* property_init( property* p, int stride ) {
	size_t alloc_size = sizeof( property ) + sizeof( float ) * stride * kMaxPropertyValues;
	memset( p, 0, alloc_size );
	p->stride = stride;
	p->data = (float*)((uint8_t*)p + sizeof( property ));
	return p;
}

property* property_create( int stride ) {
	size_t alloc_size = sizeof( property ) + sizeof( float ) * stride * kMaxPropertyValues;
	property* p = mem_alloc( alloc_size );
	property_init( p, stride );
	return p;
}

property* property_copy( property* p ) {
	//printf( "Copying property with stride %d\n", p->stride );
	property* p_copy = property_create( p->stride );
	p_copy->count = p->count;
	memcpy( p_copy->data, p->data, sizeof( float ) * p->stride * kMaxPropertyValues );
	return p_copy;
}

// add [p->stride] number of float [values], at [time]
void property_addf( property* p, float time, float value ) {
	assert( p->count < kMaxPropertyValues );
	assert( p->stride == 2 );
	int frame = p->count * p->stride;
	p->data[frame] = time;
	p->data[frame + 1] = value;
	++p->count;
}

// add [p->stride] number of float [values], at [time]
void property_addfv( property* p, float time, float* values ) {
	assert( p->count < kMaxPropertyValues );
	int frame = p->count * p->stride;
	p->data[frame] = time;
	for ( int i = 0; i < p->stride - 1; ++i )
		p->data[frame + 1 + i] = values[i];
	++p->count;
}

void property_addv( property* p, float time, vector value ) {
	assert( p->count < kMaxPropertyValues );
	p->data[p->count * p->stride] = time;
	p->data[p->count * p->stride + 1] = value.coord.x;
	p->data[p->count * p->stride + 2] = value.coord.y;
	p->data[p->count * p->stride + 3] = value.coord.z;
	p->data[p->count * p->stride + 4] = value.coord.w;
	p->count++;
}

void property_sample( property* p, float time, int* before, int* after, float* factor ) {
	float t_after = 0.f, t_before = 0.f;
	*after = -1;
	while ( t_after < time && *after < p->count ) {
		(*after)++;
		t_before = t_after;
		t_after = p->data[*after*p->stride];
	}
	*before = clamp( *after - 1, 0, p->count-1 );
	*after = clamp( *after, 0, p->count-1 );
	*factor = map_range( time, t_before, t_after );
	if ( *after == *before )
		*factor = 0.f;
	assert( *factor <= 1.f && *factor >= 0.f );
}

float property_samplef( property* p, float time ) {
	float factor;
	int before, after;
	property_sample( p, time, &before, &after, &factor );
	float value = lerp( p->data[before*p->stride+1], p->data[after*p->stride+1], factor );
	return value;
}

vector property_samplev( property* p, float time ) {
	float factor;
	int before, after;
	property_sample( p, time, &before, &after, &factor );
	vector value = vector_lerp( (vector*)&p->data[before*p->stride+1], (vector*)&p->data[after*p->stride+1], factor );
	return value;
}

float property_valuef( property* p, int key ) {
	float f = p->data[key * p->stride + 1]; // we add 1 to skip the domain
	return f;
}

float property_keyDomain( float* key ) {
	return *key;
}

// Return a new property that contains only the keys in the given domain range
// BUFFER is a pointer to space big enough to use for the range
// this is passed in so we can use stack allocation ot save a mem_alloc traversal
property* property_range( property* p, float from, float to, property* buffer ) {
	property* range = property_init( buffer, p->stride );
	//property* range = property_create( p->stride );
	float* key = p->data;
	float* max = key + p->stride * p->count;
	while ( key < max && property_keyDomain( key ) < from ) {
		key += p->stride;
	}
	while ( key < max && property_keyDomain( key ) < to ) {
		if ( p->stride == 2 ) {
			property_addf( range, key[0], key[1] );
		} else if ( p->stride == 5 ) {
			vector v = Vector( key[1], key[2], key[3], key[4] );
			property_addv( range, key[0], v );
		}
		key += p->stride;
	}
	return range;
}

void test_property() {
	property* p = property_create( 2 );
	float f[] = { 0.f, 3.f, 2.f, 3.f };
	property_addfv( p, 0.f, &f[0] );
	property_addfv( p, 1.f, &f[1] );
	property_addfv( p, 2.f, &f[2] );
	property_addfv( p, 3.f, &f[3] );
	property_samplef( p, 0.75f );
	property_samplef( p, 1.5f );
	property_samplef( p, 3.0f );
}

map* particleEmitterAssets = NULL;
#define kMaxParticleAssets 256

void particle_init() {
	particleEmitterAssets = map_create( kMaxParticleAssets, sizeof( particleEmitterDef* ));
}

particleEmitterDef* particle_loadAsset( const char* particle_file ) {
	int key = mhash( particle_file );
	// try to find it if it's already loaded
	void** result = map_find( particleEmitterAssets, key );
	if ( result ) {
		particleEmitterDef* def = *((particleEmitterDef**)result);
		// If the file has changed, we want to update this (live-reloading)
		if ( vfile_modifiedSinceLast( particle_file )) {
			// Load the new file
			term* particle_term = lisp_eval_file( lisp_global_context, particle_file );
			particleEmitterDef* new = particle_term->data;
			// Save over the old
			particleEmitterDef_deInit( def );
			*def = *new;
		}
		return def;
	}
	
	// otherwise load it and add it
	term* particle_term = lisp_eval_file( lisp_global_context, particle_file );
	particleEmitterDef* def = particle_term->data;
	map_add( particleEmitterAssets, key, &def );
	return def;
}

particleEmitter* particle_newEmitter( particleEmitterDef* definition ) {
	particleEmitter* p = particleEmitter_create();
	p->definition = definition;
	return p;
}

void particleEmitter_destroy( particleEmitter* e ) {
	e->destroyed = true;
}

void particleEmitter_delete( particleEmitter* e ) {
	//printf( "Removing particle 0x" xPTRf ".\n", (uintptr_t)e );
	array_remove( (void**)active_particles, &active_particle_count, (void*)e );

	// Does not explicitly remove the definition
	vAssert( e );
	vAssert( e->vertex_buffer );
	vAssert( e->element_buffer );
	mem_free( e->vertex_buffer );
	mem_free( e->element_buffer );
	mem_free( e );
}

#ifdef DEBUG_PARTICLE_LIVENESS_TEST
void particleEmitter_assertActive( particleEmitter* e ) {
	int found = array_find( (void**)active_particles, active_particle_count, (void*)e );
	vAssert( found != -1 )
}
#endif // DEBUG_PARTICLE_LIVENESS_TEST
