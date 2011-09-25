// terrain.c
#include "common.h"
#include "terrain.h"
//-----------------------
#include "mem/allocator.h"
#include "render/render.h"
#include "render/shader.h"
#include "render/texture.h"
#include "system/hash.h"

GLint terrain_texture = -1;

// Create a procedural terrain
terrain* terrain_create() {
	terrain* t = mem_alloc( sizeof( terrain ));
	memset( t, 0, sizeof( terrain ));
	t->u_samples = 1;
	t->v_samples = 1;
	t->sample_point = Vector( 0.f, 0.f, 0.f, 1.f );

	if ( terrain_texture == -1 )
		terrain_texture = texture_loadTGA( "assets/3rdparty/img/rock02_tile_small.tga" );

	return t;
}

// The procedural function
float terrain_sample( float u, float v ) {
#if 0
//	return 0.f;
	return sin( u * 0.25f ) * sin( v * 0.25f ) * 5.f;
#else
	float detail =
	(
			0.5 * sin( u ) * sin( v ) +
			sin( u / 3.f ) * sin( v / 3.f ) +
			5 * sin( u / 10.f ) * sin( v / 10.f ) * sin( u / 10.f ) * sin( v / 10.f )
	);

	float scale = 5.f;
	u /= scale;
	u += sin( v / (scale * 5.f) );
	float height = 5.f;

	float mask = cos( fclamp( u / 4.f, -PI/2.f, PI/2.f ));
	float canyon = fclamp( powf( u, 4.f ), 0.f, 1.f );

	return (
			mask * canyon
		   ) * height
		+ detail;
#endif
}

void terrain_updateIntervals( terrain* t ) {
	t->u_interval = t->u_radius / ((float)(t->u_samples - 1) * 0.5);
	t->v_interval = t->v_radius / ((float)(t->v_samples - 1) * 0.5);
}

void terrain_setSize( terrain* t, float u, float v ) {
	t->u_radius = u;
	t->v_radius = v;
	terrain_updateIntervals( t );
}

void terrain_setResolution( terrain* t, int u, int v ) {
	t->u_samples = u;
	t->v_samples = v;
	terrain_updateIntervals( t );

	// Setup buffers
	int triangle_count = ( t->u_samples - 1 ) * ( t->v_samples - 1 ) * 2;
	t->index_count = triangle_count * 3;
	if ( !t->vertex_buffer ) {
		t->vertex_buffer = mem_alloc( t->index_count * sizeof( vertex ));
	}
	if ( !t->element_buffer ) {
		t->element_buffer = mem_alloc( t->index_count * sizeof( GLushort ));
	}
}

// Calculate vertex and element buffers from procedural definition
void terrain_calculateBuffers( terrain* t ) {

/*
	We have a grid of x * y points
	So (x-1) * (y-1) quads
	So (x-1) * (y-1) * 2 tris
	So (x-1) * (y-1) * 6 indices
	*/

	int triangle_count = ( t->u_samples - 1 ) * ( t->v_samples - 1 ) * 2;
	int vert_count = t->u_samples * t->v_samples;

	vector* verts = mem_alloc( vert_count * sizeof( vector ));
	vector* normals = mem_alloc( vert_count * sizeof( vector ));

/*
	We want to draw the mesh from ( -u_radius, -v_radius ) to ( u_radius, v_radius )
	We sample at an interval of ( u_interval, v_interval )
   */

	int vert_index = 0;
	float u_min = fround( t->sample_point.coord.x - t->u_radius, t->u_interval );
	float v_min = fround( t->sample_point.coord.z - t->v_radius, t->v_interval );
	const float u_max = u_min + ( 2 * t->u_radius ) + ( 0.5 * t->u_interval );
	const float v_max = v_min + ( 2 * t->v_radius ) + ( 0.5 * t->v_interval );
	for ( float u = u_min; u < u_max; u+= t->u_interval ) {
		for ( float v = v_min; v < v_max; v+= t->v_interval ) {
//			float u_round = fround( u, t->u_interval );
//			float v_round = fround( v, t->v_interval );
			float h = terrain_sample( u, v );
			verts[vert_index++] = Vector( u, h, v, 1.f );
//			vAssert( u != 0.f || v != 0.f || h != 0.f );
		}
	}
	assert( vert_index == vert_count );

	for ( int i = 0; i < vert_count; i++ ) {
			normals[i] = Vector( 0.f, 1.f, 0.f, 0.f );
#if 0
		if ( /*i-1 < 0 || 
			i + 1 >= vert_count ||*/
			i - t->u_samples < 0 ||							// Top Edge
			i + t->u_samples >= vert_count ||				// Bottom Edge
		  	i % t->u_samples == 0 ||						// Left edge
		  	i % t->u_samples == ( t->u_samples - 1 ) ) {	// Right Edge
			normals[i] = Vector( 0.f, 1.f, 0.f, 0.f );
			continue;
		}
		vector center = verts[i];
		vector left = verts[i - t->u_samples];
		vector right = verts[i + t->u_samples];
		vector top = verts[i-1];
		vector bottom = verts[i+1];

		vector a, b, c, d, e, f, x, y;
		x = Vector( -1.f, 0.f, 0.f, 0.f ); // Negative to ensure cross products come out correctly
		y = Vector( 0.f, 0.f, 1.f, 0.f );

		// Calculate two vertical vectors
		Sub( &a, &center, &top );
		Sub( &b, &bottom, &center );
		// Take cross product to calculate normals
		Cross( &c, &x, &a );
		Cross( &d, &x, &b );

		// Calculate two horizontal vectors
		Sub( &a, &center, &left );
		Sub( &b, &right, &center );
		// Take cross product to calculate normals
		Cross( &e, &y, &a );
		Cross( &f, &y, &b );

		// Average normals
		vector total = Vector( 0.f, 0.f, 0.f, 0.f );
		Add( &total, &total, &c );
		Add( &total, &total, &d );
		Add( &total, &total, &e );
		Add( &total, &total, &f );
		total.coord.w = 0.f;
		Normalize( &total, &total );
		normals[i] = total;
#endif
	}

	float texture_scale = 0.125f;

	int element_count = 0;
	// Calculate elements
	int tw = ( t->u_samples - 1 ) * 2; // Triangles per row
	for ( int i = 0; i < triangle_count; i+=2 ) {
		GLushort tl = ( i / 2 ) + i / tw;
		GLushort tr = tl + 1;
		GLushort bl = tl + t->u_samples;
		GLushort br = bl + 1;
/*
		vAssert( tl >= 0 );
		vAssert( tr >= 0 );
		vAssert( bl >= 0 );
		vAssert( br >= 0 );

		vAssert( tl < vert_count );
		vAssert( tr < vert_count );
		vAssert( bl < vert_count );
		vAssert( br < vert_count );
		*/
		// Unroll this, do two triangles at a time
		// bottom right triangle - br, bl, tr
		t->element_buffer[element_count+0] = br;
		t->element_buffer[element_count+1] = bl;
		t->element_buffer[element_count+2] = tr;
		// top left triangle - tl, tr, bl
		t->element_buffer[element_count+3] = tl;
		t->element_buffer[element_count+4] = tr;
		t->element_buffer[element_count+5] = bl;
		element_count += 6;
	}

	// TODO: Should be able to skip this by generating them unrolled? Or not unrolling them at all?

	// For each element index
	// Unroll the vertex/index bindings
	for ( int i = 0; i < t->index_count; i++ ) {
		// Copy the required vertex position, normal, and uv
		t->vertex_buffer[i].position = verts[t->element_buffer[i]];
		t->vertex_buffer[i].normal = normals[t->element_buffer[i]];
		t->vertex_buffer[i].uv = Vector( verts[t->element_buffer[i]].coord.x * texture_scale, verts[t->element_buffer[i]].coord.z * texture_scale, 0.f, 0.f );
		t->element_buffer[i] = i;
	}
	/*	
	// Test print
	for ( int i = 0; i < t->index_count; i++ ) {
	printf( "Index %d: Vert:", i );
	vector_print( &t->vertex_buffer[i].position );
	printf( "\n" );
	}
	*/
	mem_free( verts );
	mem_free( normals );
}

// Send the buffers to the GPU
void terrain_render( void* data ) {
	terrain* t = data;

	// Switch to terrain shader
	shader_activate( resources.shader_terrain );

	render_resetModelView();
	matrix_mul( modelview, modelview, t->trans->world );
	// Set up uniforms
	render_setUniform_matrix( *resources.uniforms.projection, perspective );
	render_setUniform_matrix( *resources.uniforms.modelview, modelview );
	render_setUniform_matrix( *resources.uniforms.worldspace, modelview );

	glDepthMask( GL_TRUE );

	terrain_calculateBuffers( t );

	// Copy our data to the GPU
	// There are now <index_count> vertices, as we have unrolled them
	GLsizei vertex_buffer_size = t->index_count * sizeof( vertex );
	GLsizei element_buffer_size = t->index_count * sizeof( GLushort );

	// Textures
	GLint* tex = shader_findConstant( mhash( "tex" ));
	if ( tex )
		render_setUniform_texture( *tex, terrain_texture );


	VERTEX_ATTRIBS( VERTEX_ATTRIB_LOOKUP );
	// *** Vertex Buffer
	glBindBuffer( GL_ARRAY_BUFFER, resources.vertex_buffer );
	glBufferData( GL_ARRAY_BUFFER, vertex_buffer_size, t->vertex_buffer, GL_DYNAMIC_DRAW );// OpenGL ES only supports DYNAMIC_DRAW or STATIC_DRAW
	VERTEX_ATTRIBS( VERTEX_ATTRIB_POINTER );
	// *** Element Buffer
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, resources.element_buffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, element_buffer_size, t->element_buffer, GL_DYNAMIC_DRAW ); // OpenGL ES only supports DYNAMIC_DRAW or STATIC_DRAW

	// Draw!
	glDrawElements( GL_TRIANGLES, t->index_count, GL_UNSIGNED_SHORT, (void*)0 );

	// Cleanup
	VERTEX_ATTRIBS( VERTEX_ATTRIB_DISABLE_ARRAY )
}

void terrain_delete( terrain* t ) {
	if ( t->vertex_buffer )
		mem_free( t->vertex_buffer );
	if ( t->element_buffer )
		mem_free( t->element_buffer );
	mem_free( t );
}

void test_terrain() {
	terrain* t = terrain_create();
	terrain_setSize( t, 5.f, 5.f );
	terrain_setResolution( t, 8, 8 );
	terrain_calculateBuffers( t );
	terrain_delete( t );
//	vAssert( 0 );
}
