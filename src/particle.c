// particle.c
#include "src/common.h"
#include "src/particle.h"
//---------------------
#include "mem/allocator.h"
#include "model.h"
#include "render/render.h"
#include "render/shader.h"
#include "render/texture.h"
#include "system/hash.h"

float property_samplef( property* p, float time );
vector property_samplev( property* p, float time );

particleEmitterDef* particleEmitterDef_create() {
	particleEmitterDef* def = mem_alloc( sizeof( particleEmitterDef ));
	memset( def, 0, sizeof( particleEmitterDef ));
	return def;
}

particleEmitter* particleEmitter_create() {
	particleEmitter* p = mem_alloc( sizeof( particleEmitter ));
	memset( p, 0, sizeof( particleEmitter ));
	p->definition = particleEmitterDef_create();
	p->definition->spawn_box = Vector( 0.f, 0.f, 0.f, 0.f );
	p->definition->texture_diffuse = texture_loadTGA( "assets/img/cloud_rgba128.tga" );
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
		p->position = matrixVecMul( e->trans->world, &offset );
	p->age = 0.f;
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
	e->next_spawn += dt;
	if ( e->next_spawn > e->definition->spawn_interval ) {
		e->next_spawn = 0.f;
		particleEmitter_spawnParticle( e );
	}
}

// Output the 4 verts of the quad to the target vertex array
// both position and normals
void particle_quad( particleEmitter* e, vertex* dst, vector* point, float size, vector color ) {
	vector offset = Vector( size, size, 0.f, 0.f );

	vector p;
	if ( !e->definition->flags & kParticleWorldSpace )
		p = matrixVecMul( modelview, point );
	else
		p = matrixVecMul( camera_inverse, point );

	Add( &dst[0].position, &p, &offset );
	dst[0].normal = Vector( 0.f, 0.f, 1.f, 0.f );
	dst[0].uv = Vector( 1.f, 1.f, 0.f, 0.f );
	dst[0].color = color;

	Sub( &dst[1].position, &p, &offset );
	dst[1].normal = Vector( 0.f, 0.f, 1.f, 0.f );
	dst[1].uv = Vector( 0.f, 0.f, 0.f, 0.f );
	dst[1].color = color;

	offset.coord.y = -offset.coord.y;

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
	glDepthMask( GL_FALSE );
	// switch to particle shader
	shader_activate( resources.shader_particle );

	// Set up uniforms
	render_setUniform_matrix( *resources.uniforms.projection, perspective );
	render_setUniform_matrix( *resources.uniforms.worldspace, modelview );

	particleEmitter* p = data;
		
	// Textures
	GLint* tex = shader_findConstant( mhash( "tex" ));
	if ( tex )
		render_setUniform_texture( *tex, p->definition->texture_diffuse );

	// reset modelview matrix so we can billboard
	// particle_quad() will manually apply the modelview
	render_resetModelView();
	matrix_mul( modelview, modelview, p->trans->world );

	vertex vertex_buffer[kmax_particle_verts];
	GLushort element_buffer[kmax_particle_verts];

	for ( int i = 0; i < p->count; i++ ) {
		int particle_index = (p->start + i) % kMaxParticles;
		float size = property_samplef( p->definition->size, p->particles[particle_index].age );
		vector color = property_samplev( p->definition->color, p->particles[particle_index].age );
		particle_quad( p, &vertex_buffer[i*4], &p->particles[particle_index].position, size, color );
		assert( i*6 + 5 < kmax_particle_verts );
		// TODO: Indices can be initialised once
		element_buffer[i*6+0] = i*4+1;
		element_buffer[i*6+1] = i*4+0;
		element_buffer[i*6+2] = i*4+2;
		element_buffer[i*6+3] = i*4+0;
		element_buffer[i*6+4] = i*4+1;
		element_buffer[i*6+5] = i*4+3;
	}


	// For Billboard particles; cancel out the rotation of the matrix
	// The transformation has been applied already for particle positions
	matrix_setIdentity( modelview );
	render_setUniform_matrix( *resources.uniforms.modelview, modelview );

	int index_count = 6 * p->count;

	drawCall* particle_render = drawCall_create( index_count, element_buffer, vertex_buffer );
	render_drawCall( particle_render );
}

property* property_create( int stride ) {
	property* p = mem_alloc( sizeof( property ));
	memset( p, 0, sizeof( property ));
	p->stride = stride;
	p->data = mem_alloc( sizeof( float ) * p->stride * kmax_property_values );
	return p;
}

void property_addf( property* p, float time, float value ) {
	assert( p->count < kmax_property_values );
	p->data[p->count * p->stride] = time;
	p->data[p->count * p->stride + 1] = value;
	p->count++;
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
	property_addf( p, 0.f, 0.f );
	property_addf( p, 1.f, 3.f );
	property_addf( p, 2.f, 2.f );
	property_addf( p, 3.f, 3.f );
	property_samplef( p, 0.75f );
	property_samplef( p, 1.5f );
	property_samplef( p, 3.0f );
}
