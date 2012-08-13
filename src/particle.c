// particle.c
#include "src/common.h"
#include "src/particle.h"
//---------------------
#include "model.h"
#include "maths/vector.h"
#include "mem/allocator.h"
#include "render/render.h"
#include "render/shader.h"
#include "render/texture.h"
#include "script/lisp.h"
#include "system/hash.h"

float property_samplef( property* p, float time );
vector property_samplev( property* p, float time );

particleEmitterDef* particleEmitterDef_create() {
	particleEmitterDef* def = mem_alloc( sizeof( particleEmitterDef ));
	memset( def, 0, sizeof( particleEmitterDef ));
	def->spawn_box = Vector( 0.f, 0.f, 0.f, 0.f );
	// TODO - shouldn't be doing this here, but need to do it only once per particle def
	texture_request( &def->texture_diffuse, "dat/img/cloud_rgba128.tga" );
	return def;
}

particleEmitter* particleEmitter_create() {
	particleEmitter* p = mem_alloc( sizeof( particleEmitter ));
	memset( p, 0, sizeof( particleEmitter ));
	p->definition = NULL;
	p->vertex_buffer = mem_alloc( sizeof( vertex ) * kMaxParticleVerts );
	p->element_buffer = mem_alloc( sizeof( vertex ) * kMaxParticleVerts );

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
	if ( !e->definition->flags & kParticleWorldSpace )
		p->position	= offset;
	else
		p->position = matrix_vecMul( e->trans->world, &offset );
	p->age = 0.f;
	if ( e->definition->flags & kParticleRandomRotation )
		p->rotation = frand( 0.f, 2*PI );
	else
		p->rotation = 0.f;
}

void particleEmitter_tick( void* data, float dt ) {
	particleEmitter* e = data;
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
	if ( e->definition->spawn_rate ) {
		e->next_spawn += dt;
		float spawn_rate = property_samplef( e->definition->spawn_rate, e->emitter_age );
		float spawn_interval = 1.f / spawn_rate;
		// We might spawn more than one particle per frame, if the frame is long or the spawn interval is short
		while ( e->next_spawn > spawn_interval ) {
			e->next_spawn = fmaxf( 0.f, e->next_spawn - spawn_interval);
			particleEmitter_spawnParticle( e );
		}
	}
	e->emitter_age += dt;
}

// Output the 4 verts of the quad to the target vertex array
// both position and normals
void particle_quad( particleEmitter* e, vertex* dst, vector* point, float rotation, float size, vector color ) {
	vector offset = Vector( size, size, 0.f, 0.f );

	vector p;
	if ( !e->definition->flags & kParticleWorldSpace )
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
	drawCall* draw = drawCall_create( &renderPass_alpha, resources.shader_particle, index_count, p->element_buffer, p->vertex_buffer, p->definition->texture_diffuse, modelview );
	draw->depth_mask = GL_FALSE;
}

property* property_create( int stride ) {
	property* p = mem_alloc( sizeof( property ));
	memset( p, 0, sizeof( property ));
	p->stride = stride;
	p->data = mem_alloc( sizeof( float ) * p->stride * kmax_property_values );
	return p;
}

property* property_copy( property* p ) {
	printf( "Copying property with stride %d\n", p->stride );
	property* p_copy = property_create( p->stride );
	p_copy->count = p->count;
	memcpy( p_copy->data, p->data, sizeof( float ) * p->stride * kmax_property_values );
	return p_copy;
}

// add [p->stride] number of float [values], at [time]
void property_addf( property* p, float time, float value ) {
	assert( p->count < kmax_property_values );
	assert( p->stride == 2 );
	int frame = p->count * p->stride;
	p->data[frame] = time;
	p->data[frame + 1] = value;
	++p->count;
}

// add [p->stride] number of float [values], at [time]
void property_addfv( property* p, float time, float* values ) {
	assert( p->count < kmax_property_values );
	int frame = p->count * p->stride;
	p->data[frame] = time;
	for ( int i = 0; i < p->stride - 1; ++i )
		p->data[frame + 1 + i] = values[i];
	++p->count;
}

void property_addv( property* p, float time, vector value ) {
	assert( p->count < kmax_property_values );
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
	// try to find it if it's already loaded
	int key = mhash( particle_file );
	void** result = map_find( particleEmitterAssets, key );
	if ( result ) {
		particleEmitterDef* def = *((particleEmitterDef**)result);
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
