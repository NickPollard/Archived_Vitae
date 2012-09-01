// canyon_terrain.c
#include "common.h"
#include "canyon_terrain.h"
//-----------------------
#include "canyon.h"
#include "terrain.h"
#include "maths/geometry.h"
#include "maths/vector.h"
#include "mem/allocator.h"
#include "render/debugdraw.h"
#include "render/texture.h"

// *** Forward declarations
void canyonTerrainBlock_initVBO( canyonTerrainBlock* b );

// *** Utility functions

int canyonTerrainBlock_indexFromUV( canyonTerrainBlock* b, int u, int v ) {
	return u + v * b->u_samples ;
}

int canyonTerrainBlock_vertCount( canyonTerrainBlock* b ) {
	return b->u_samples * b->v_samples;
}

void canyonTerrainBlock_positionsFromUV( canyonTerrainBlock* b, int u_index, int v_index, float* u, float* v ) {
	float u_interval = ( b->u_max - b->u_min ) / (float)( b->u_samples - 1 );
	float v_interval = ( b->v_max - b->v_min ) / (float)( b->v_samples - 1 );
	*u = (float)u_index * u_interval + b->u_min;
	*v = (float)v_index * v_interval + b->v_min;
}

int canyonTerrainBlock_triangleCount( canyonTerrainBlock* b ) {
	return ( b->u_samples - 1 ) * ( b->v_samples - 1 ) * 2;
}

// ***


void canyonTerrainBlock_render( canyonTerrainBlock* b ) {
	drawCall* draw = drawCall_create( &renderPass_main, resources.shader_terrain, b->element_count, b->element_buffer, b->vertex_buffer, terrain_texture, modelview );
	draw->texture_b = terrain_texture_cliff;
	(void)draw;
	if ( *b->vertex_VBO != 0 ) {
		draw->vertex_VBO = *b->vertex_VBO;
		draw->element_VBO = *b->element_VBO;
	}

	/* 
	int vert_count = canyonTerrainBlock_vertCount( b );
	const float radius = 2.f;
	for ( int i = 0; i < vert_count; ++i ) {
		debugdraw_sphere( b->verts[i], radius, color_green );
	}
	*/
}

void canyonTerrain_render( canyonTerrain* t ) {
	render_resetModelView();
	matrix_mul( modelview, modelview, t->trans->world );

	canyonTerrainBlock_render( t->blocks[0] );
}


canyonTerrainBlock* canyonTerrainBlock_create() {
	canyonTerrainBlock* b = mem_alloc( sizeof( canyonTerrainBlock ));
	memset( b, 0, sizeof( canyonTerrainBlock ));
	return b;
}

canyonTerrain* canyonTerrain_create() {
	canyonTerrain* t = mem_alloc( sizeof( canyonTerrain ));
	memset( t, 0, sizeof( canyonTerrain ));
	int block_count = 1;
	t->blocks = mem_alloc( sizeof( canyonTerrainBlock* ) * block_count );
	t->blocks[0] = canyonTerrainBlock_create();
	t->trans = transform_create();

	if ( terrain_texture == 0 ) {
		texture_request( &terrain_texture, "dat/img/terrain/snow_2_rgba.tga" );
	}
	if ( terrain_texture_cliff == 0 ) {
		texture_request( &terrain_texture_cliff, "dat/img/terrain/cliffs_2_rgba.tga" );
	}

	return t;
}

bool canyonTerrainBlock_triangleInvalid( canyonTerrainBlock* b, int u_index, int v_index, int u_offset, int v_offset ) {
	u_offset = u_offset / 2 + u_offset % 2;
	u_offset = min( u_offset, 0 );
	return ( u_index + u_offset >= b->u_samples - 1 ) ||
		( u_index + u_offset < 0 ) ||
		( v_index + v_offset >= b->v_samples - 1 ) ||
		( v_index + v_offset < 0 );
}

// Given a vertex that has been generated, fill in the correct triangles for it after it has been unrolled
void canyonTerrainBlock_fillTrianglesForVertex( canyonTerrainBlock* b, vector* positions, vertex* vertices, int u_index, int v_index, vertex* vert ) {
	// Each vertex is in a maximum of 6 triangles
	// The triangle indices can be computed as: (where row == ( u_samples - 1 ) * 2)
	//  first row:
	//    row * (v_index - 1) + 2 * u_index - 1
	//    row * (v_index - 1) + 2 * u_index
	//    row * (v_index - 1) + 2 * u_index + 1
	//	second row:
	//    row * v_index + 2 * u_index - 2
	//    row * v_index + 2 * u_index - 1
	//    row * v_index + 2 * u_index
	// (discarding any that fall outside the range 0 -> tri_count)
	// Triangle vert index:
	// top: 0, 2, 1
	// bottom: 1, 2, 0
	// finished index = triangle_index * 3 + triangle_vert_index
	const int triangle_vert_indices[6] = { 0, 2, 1, 1, 2, 0 };
	const int v_offset[6] = { -1, -1, -1, 0, 0, 0 };
	const int u_offset[6] = { -1, 0, 1, -2, -1, 0 };
	
	int triangles_per_row = ( b->u_samples - 1 ) * 2;
	
	for ( int i = 0; i < 6; ++i ) {
		// Calculate triangle index
		int row = v_index + ( v_offset[i] );
		int column = ( 2 * u_index ) + ( u_offset[i] );
		int triangle_index = triangles_per_row * row + column; 

		if ( canyonTerrainBlock_triangleInvalid( b, u_index, v_index, u_offset[i], v_offset[i]) )
			continue;

		// if it's a valid triangle (not out-of-bounds)
		int vert_index = triangle_index * 3 + triangle_vert_indices[i];
		vertices[vert_index] = *vert;

		// Cliff coloring
		const int triangle_u_offset[6][2] = { { 0, -1 }, { 1, 0 }, { 1, 1 }, { -1, -1 }, { -1, 0 }, { 0, 1 } };
		const int triangle_v_offset[6][2] = { { -1, 0 }, { -1, -1 }, { 0, -1 }, { 0, 1 }, { 1, 1 }, { 1, 0 } };
		
		// Calculate indices of other 2 triangle verts
		int index_b = canyonTerrainBlock_indexFromUV( b, u_index + triangle_u_offset[i][0] , v_index + triangle_v_offset[i][0] );
		int index_c = canyonTerrainBlock_indexFromUV( b, u_index + triangle_u_offset[i][1] , v_index + triangle_v_offset[i][1] );

		vector v_b = positions[index_b];
		vector v_c = positions[index_c];

		vector normal;
		float d;
		plane( vertices[vert_index].position, v_b, v_c, &normal, &d );
		float angle = acosf( Dot( &normal, &y_axis ));
		const float cliff_angle = PI / 4.f;
		bool cliff = angle > cliff_angle;
		if ( cliff ) {
			vertices[vert_index].color = Vector( 0.2f, 0.3f, 0.5f, 1.f );
		} else {
			vertices[vert_index].color = Vector( 0.8f, 0.9f, 1.0f, 0.f );
		}
	}
}

void canyonTerrainBlock_generateVertices( canyonTerrainBlock* b, vector* verts, vector* normals ) {
	const float texture_scale = 0.0325f;
	for ( int v = 0; v < b->v_samples; v ++ ) {
		for ( int u = 0; u < b->u_samples; u ++  ) {
			int i = canyonTerrainBlock_indexFromUV( b, u, v );
			vertex vert;
			vert.position = verts[i];
			vert.normal = normals[i];
			vert.uv = Vector( vert.position.coord.x * texture_scale, vert.position.coord.z * texture_scale, 0.f, 0.f );
			canyonTerrainBlock_fillTrianglesForVertex( b, verts, b->vertex_buffer, u, v, &vert );
		}
	}
}

void canyonTerrainBlock_calculateNormals( canyonTerrainBlock* b, int vert_count, vector* verts, vector* normals ) {
	// *** Generate Normals
	// Do top and bottom edges first
	for ( int i = 0; i < b->u_samples; i++ )
		normals[i] = y_axis;
	for ( int i = vert_count - b->u_samples; i < vert_count; i++ )
		normals[i] = y_axis;

	//Now the rest
	for ( int i = b->u_samples; i < ( vert_count - b->u_samples ); i++ ) {
		// Ignoring Left and Right Edges
		if ( i % b->u_samples == 0 || i % b->u_samples == ( b->u_samples - 1 ) ) {    
			normals[i] = y_axis;
			continue;
		}
		vector left		= verts[i - b->u_samples];
		vector right	= verts[i + b->u_samples];
		vector top		= verts[i - 1];
		vector bottom	= verts[i + 1];

		vector a, b, c, x, y;
		//x = Vector( -1.f, 0.f, 0.f, 0.f ); // Negative to ensure cross products come out correctly
		//y = Vector( 0.f, 0.f, 1.f, 0.f );

		// Calculate vertical vector
		// Take cross product to calculate normals
		Sub( &a, &bottom, &top );
		Cross( &x, &a, &y_axis );
		Cross( &b, &x, &a );

		// Calculate horizontal vector
		// Take cross product to calculate normals
		Sub( &a, &right, &left );
		Cross( &y, &a, &y_axis );
		Cross( &c, &y, &a );

		Normalize( &b, &b );
		Normalize( &c, &c );

		// Average normals
		vector total = Vector( 0.f, 0.f, 0.f, 0.f );
		Add( &total, &total, &b );
		Add( &total, &total, &c );
		total.coord.w = 0.f;
		Normalize( &total, &total );
		normals[i] = total;
	}
}

void initialiseDefaultElementBuffer( int count, unsigned short* buffer ) {
	for ( int i = 0; i < count; i++ )
		buffer[i] = i;
}

void canyonTerrainBlock_init( canyonTerrainBlock* b ) {
	b->vertex_VBO = NULL;
	b->element_VBO = NULL;

	b->u_samples = 80;
	b->v_samples = 60;

	b->u_min = -180.f;
	b->v_min = 0.f;
	b->u_max = 180.f;
	b->v_max = 640.f;

	int vert_count = canyonTerrainBlock_vertCount( b );
	
	vector* verts = mem_alloc( sizeof( vector ) * vert_count );
	vector* normals = mem_alloc( sizeof( vector ) * vert_count );

	for ( int v_index = 0; v_index < b->v_samples; ++v_index ) {
		for ( int u_index = 0; u_index < b->u_samples; ++u_index ) {
			// Generate a vertex
			int i = canyonTerrainBlock_indexFromUV( b, u_index, v_index );
			float u, v;
			canyonTerrainBlock_positionsFromUV( b, u_index, v_index, &u, &v );
			float vert_x, vert_z;
			terrain_worldSpaceFromCanyon( u, v, &vert_x, &vert_z );
			float vert_y = terrain_sample( vert_x, vert_z  );

			verts[i] = Vector( vert_x, vert_y, vert_z, 1.f );
			normals[i] = y_axis;
		}
	}

	canyonTerrainBlock_calculateNormals( b, vert_count, verts, normals );

	// Element List
	b->element_count = canyonTerrainBlock_triangleCount( b ) * 3;
	b->element_buffer = mem_alloc( sizeof( unsigned short ) * b->element_count );
	initialiseDefaultElementBuffer( b->element_count, b->element_buffer );

	// Unroll Verts
	b->vertex_buffer = mem_alloc( sizeof( vertex ) * b->element_count );
	canyonTerrainBlock_generateVertices( b, verts, normals );
	
	// Store verts
	b->verts = mem_alloc( sizeof( vector ) * vert_count );
	memcpy( b->verts, verts, sizeof( vector ) * vert_count );

	mem_free( verts );
	mem_free( normals );

	canyonTerrainBlock_initVBO( b );
}

void canyonTerrain_init( canyonTerrain* t ) {
	canyonTerrainBlock_init( t->blocks[0] );
}

// Create GPU vertex buffer objects to hold our data and save transferring to the GPU each frame
// If we've already allocated a buffer at some point, just re-use it
void canyonTerrainBlock_initVBO( canyonTerrainBlock* b ) {
	if ( !b->vertex_VBO )
		b->vertex_VBO = render_requestBuffer( GL_ARRAY_BUFFER, b->vertex_buffer, sizeof( vertex ) * b->element_count );
	else
		render_bufferCopy( GL_ARRAY_BUFFER, *b->vertex_VBO, b->vertex_buffer, sizeof( vertex ) * b->element_count );
	if ( !b->element_VBO )
		b->element_VBO = render_requestBuffer( GL_ELEMENT_ARRAY_BUFFER, b->element_buffer, sizeof( GLushort ) * b->element_count );
	else
		render_bufferCopy( GL_ELEMENT_ARRAY_BUFFER, *b->element_VBO, b->element_buffer, sizeof( GLushort ) * b->element_count );
}
