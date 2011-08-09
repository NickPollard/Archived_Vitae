// particle.c
#include "src/common.h"
#include "src/particle.h"
//---------------------
#include "mem/allocator.h"
#include "model.h"
#include "render/render.h"

particleEmitter* particleEmitter_create() {
	particleEmitter* p = mem_alloc( sizeof( particleEmitter ));
	memset( p, 0, sizeof( particleEmitter ));
	return p;
}

// Output the 4 verts of the quad to the target vertex array
// both position and normals
void particle_quad( vertex* dst, vector* point, float size ) {
	vector offset = Vector( size, size, 0.f, 0.f );
	Add( &dst[0].position, point, &offset );
	dst[0].normal = Vector( 0.f, 0.f, 1.f, 0.f );
	Sub( &dst[1].position, point, &offset );
	dst[1].normal = Vector( 0.f, 0.f, 1.f, 0.f );
	offset.coord.y = -size;
	Add( &dst[2].position, point, &offset );
	dst[2].normal = Vector( 0.f, 0.f, 1.f, 0.f );
	Sub( &dst[3].position, point, &offset );
	dst[3].normal = Vector( 0.f, 0.f, 1.f, 0.f );
}

void particleEmitter_addParticle( particleEmitter* e ) {
	particle* p = &e->particles[e->count++];
	p->position	= Vector( 0.f, 0.f, 0.f, 1.f );
}

void particleEmitter_tick( void* data, float dt ) {
	particleEmitter* e = data;
	// Update existing particles
	vector delta;
	vector_scale ( &delta, &e->velocity, dt );
	for ( int i = 0; i < e->count; i++ ) {
		Add( &e->particles[i].position, &e->particles[i].position, &delta );
	}

	// Spawn new particle
	e->next_spawn += dt;
	if ( e->next_spawn > e->spawn_interval ) {
		e->next_spawn = 0.f;
		particleEmitter_addParticle( e );
	}
}

// Render a particleEmitter system
void particleEmitter_render( void* data ) {
	printf( "Render!\n" );
	particleEmitter* p = data;

	vertex vertex_buffer[kmax_particle_verts];
	GLushort element_buffer[kmax_particle_verts];

	for ( int i = 0; i < p->count; i++ ) {
		particle_quad( &vertex_buffer[i*4], &p->particles[i].position, p->size );
		// TODO: Indices can be initialised once
		element_buffer[i*6+0] = i*4+0;
		element_buffer[i*6+1] = i*4+1;
		element_buffer[i*6+2] = i*4+2;
		element_buffer[i*6+3] = i*4+0;
		element_buffer[i*6+4] = i*4+1;
		element_buffer[i*6+5] = i*4+3;
	}

	int index_count = 6 * p->count;
	printf( "particleEmitter rendering %d indices.\n", index_count );
	/*
	for ( int i = 0; i < index_count; i++ ) {
		printf( "Index: %d. Position: ", element_buffer[i] );
		vector_print( &vertex_buffer[element_buffer[i]].position );
		printf( " Normal: " );
		vector_print( &vertex_buffer[element_buffer[i]].normal );
		printf( "\n" );
	}
	*/
//	assert( 0 );
	// Copy our data to the GPU
	// There are now <index_count> vertices, as we have unrolled them
	GLsizei vertex_buffer_size = index_count * sizeof( vertex );
	GLsizei element_buffer_size = index_count * sizeof( GLushort );
	// *** Vertex Buffer
	{
		// Activate our buffers
		glBindBuffer( GL_ARRAY_BUFFER, resources.vertex_buffer );
		glBufferData( GL_ARRAY_BUFFER, vertex_buffer_size, vertex_buffer, GL_STREAM_DRAW );
		// Set up position data
		glVertexAttribPointer( resources.attributes.position, /*vec4*/ 4, GL_FLOAT, /*Normalized?*/GL_FALSE, sizeof( vertex ), (void*)offsetof( vertex, position) );
		glEnableVertexAttribArray( resources.attributes.position );
		// Set up normal data
		glVertexAttribPointer( resources.attributes.normal, /*vec4*/ 4, GL_FLOAT, /*Normalized?*/GL_FALSE, sizeof( vertex ), (void*)offsetof( vertex, normal ) );
		glEnableVertexAttribArray( resources.attributes.normal );
	}
	// *** Element Buffer
	{
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, resources.element_buffer );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, element_buffer_size, element_buffer, GL_STREAM_DRAW );
	}

	// Draw!
	glDrawElements( GL_TRIANGLES, index_count, GL_UNSIGNED_SHORT, (void*)0 );

	// Cleanup
	glDisableVertexAttribArray( resources.attributes.position );
	glDisableVertexAttribArray( resources.attributes.normal );
}
