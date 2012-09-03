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
void canyonTerrainBlock_calculateBuffers( canyonTerrainBlock* b );
void canyonTerrainBlock_createBuffers( canyonTerrainBlock* b );
void canyonTerrainBlock_init( canyonTerrainBlock* b );

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

void canyonTerrain_render( void* data ) {
	canyonTerrain* t = data;
	render_resetModelView();
	matrix_mul( modelview, modelview, t->trans->world );

	for ( int i = 0; i < t->total_block_count; ++i ) {
		canyonTerrainBlock_render( t->blocks[i] );
	}

	//canyonTerrainBlock_render( t->blocks[1] );
}

canyonTerrainBlock* canyonTerrainBlock_create( canyonTerrain* t ) {
	canyonTerrainBlock* b = mem_alloc( sizeof( canyonTerrainBlock ));
	memset( b, 0, sizeof( canyonTerrainBlock ));
	b->vertex_VBO = NULL;
	b->element_VBO = NULL;
	b->u_samples = t->u_samples_per_block;
	b->v_samples = t->v_samples_per_block;
	return b;
}

int canyonTerrain_blockIndexFromUV( canyonTerrain* t, int u, int v ) {
	return u + v * t->u_block_count;
}

void canyonTerrainBlock_calculateExtents( canyonTerrainBlock* b, canyonTerrain* t, int coord[2] ) {
	float u_size = (2 * t->u_radius) / (float)t->u_block_count;
	float v_size = (2 * t->v_radius) / (float)t->v_block_count;
	b->u_min = ((float)coord[0] - 0.5f) * u_size;
	b->v_min = ((float)coord[1] - 0.5f) * v_size;
	b->u_max = b->u_min + u_size;
	b->v_max = b->v_min + v_size;
}

void canyonTerrain_blockContaining( int coord[2], canyonTerrain* t, vector* point ) {
	float u, v;
	terrain_canyonSpaceFromWorld( point->coord.x, point->coord.z, &u, &v );
	float block_width = (2 * t->u_radius) / (float)t->u_block_count;
	float block_height = (2 * t->v_radius) / (float)t->v_block_count;
	printf( "u: %.2f\n", u );
	coord[0] = fround( u / block_width, 1.f );
	coord[1] = fround( v / block_height, 1.f );
}

// Calculate the block bounds for the terrain, at a given sample point
void canyonTerrain_calculateBounds( int bounds[2][2], canyonTerrain* t, vector* sample_point ) {
	/*
	   Find the block we are in
	   Then extend by the correct radius

	   The center block [0] is from -half_block_width to half_block_width
	   */
	int block[2];
	canyonTerrain_blockContaining( block, t, sample_point );
	printf( "Sample point in block: %d, %d.\n", block[0], block[1] );
	int rx = ( t->u_block_count - 1 ) / 2;
	int ry = ( t->v_block_count - 1 ) / 2;
	bounds[0][0] = block[0] - rx;
	bounds[0][1] = block[1] - ry;
	bounds[1][0] = block[0] + rx;
	bounds[1][1] = block[1] + ry;
}

void canyonTerrain_createBlocks( canyonTerrain* t ) {
	vAssert( !t->blocks );

	t->total_block_count = t->u_block_count * t->v_block_count;
	vAssert( t->total_block_count > 0 );

	t->blocks = mem_alloc( sizeof( canyonTerrainBlock* ) * t->total_block_count );

	// Ensure the block bounds are initialised;
	canyonTerrain_calculateBounds( t->bounds, t, &t->sample_point );

	// Calculate block extents
	for ( int v = 0; v < t->v_block_count; v++ ) {
		for ( int u = 0; u < t->u_block_count; u++ ) {
			int coord[2];
			coord[0] = t->bounds[0][0] + u;
			coord[1] = t->bounds[0][1] + v;
			int i = canyonTerrain_blockIndexFromUV( t, u, v );

			t->blocks[i] = canyonTerrainBlock_create( t );
			//canyonTerrainBlock_init( t->blocks[i] );
			canyonTerrainBlock_createBuffers( t->blocks[i] );
			canyonTerrainBlock_calculateExtents( t->blocks[i], t, coord );
			canyonTerrainBlock_calculateBuffers( t->blocks[i] );
		}
	}
}

canyonTerrain* canyonTerrain_create( int u_blocks, int v_blocks ) {
	canyonTerrain* t = mem_alloc( sizeof( canyonTerrain ));
	memset( t, 0, sizeof( canyonTerrain ));
	t->u_block_count = u_blocks;
	t->v_block_count = v_blocks;
	t->u_samples_per_block = 20;
	t->v_samples_per_block = 20;
	t->u_radius = 320.f;
	t->v_radius = 640.f;

	canyonTerrain_createBlocks( t );

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
	b->u_min = -180.f;
	b->v_min = 0.f;
	b->u_max = 180.f;
	b->v_max = 640.f;
}

void canyonTerrainBlock_createBuffers( canyonTerrainBlock* b ) {
	b->element_count = canyonTerrainBlock_triangleCount( b ) * 3;
	vAssert( b->element_count > 0 );
	
	// Element Buffer
	b->element_buffer = mem_alloc( sizeof( unsigned short ) * b->element_count );
	initialiseDefaultElementBuffer( b->element_count, b->element_buffer );
	// Vertex Buffer
	b->vertex_buffer = mem_alloc( sizeof( vertex ) * b->element_count );

}

void canyonTerrainBlock_calculateBuffers( canyonTerrainBlock* b ) {
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

	// Unroll Verts
	canyonTerrainBlock_generateVertices( b, verts, normals );
	
	// Store verts
	b->verts = mem_alloc( sizeof( vector ) * vert_count );
	memcpy( b->verts, verts, sizeof( vector ) * vert_count );

	mem_free( verts );
	mem_free( normals );

	canyonTerrainBlock_initVBO( b );
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

void canyonTerrain_updateBlocks( canyonTerrain* t ) {
	/*
	   We have a set of current blocks, B
	   We have a set of projected blocks based on the new position, B'

	   Calculate the intersection I = B n B';
	   All blocks ( b | b is in I ) we keep, shifting their pointers to the correct position
	   All other blocks fill up the empty spaces, then are recalculated

	   We are not freeing or allocating any blocks here; only reusing existing ones
	   The block pointer array remains sorted
	   */

	int bounds[2][2];
	int intersection[2][2];
	canyonTerrain_calculateBounds( bounds, t, &t->sample_point );
	terrain_boundsIntersection( intersection, bounds, t->bounds );

	// Using alloca for dynamic stack allocation (just moves the stack pointer up)
	canyonTerrainBlock** newBlocks = alloca( sizeof( terrainBlock* ) * t->total_block_count );

	int empty_index = 0;
	// For Each old block
	for ( int i = 0; i < t->total_block_count; i++ ) {
		int coord[2];
		coord[0] = t->bounds[0][0] + ( i % t->u_block_count );
		coord[1] = t->bounds[0][1] + ( i / t->u_block_count );
		// if in new bounds
		if ( boundsContains( intersection, coord )) {
			// copy to new array;
			int new_u = coord[0] - bounds[0][0];
			int new_v = coord[1] - bounds[0][1];
			int new_index = new_u + ( new_v * t->u_block_count );
			vAssert( new_index >= 0 );
			vAssert( new_index < t->total_block_count );
			newBlocks[new_index] = t->blocks[i];
		}
		else {
			// Copy unused blocks
			// Find next empty index
			int new_coord[2];
			while ( true ) {
				new_coord[0] = bounds[0][0] + ( empty_index % t->u_block_count );
				new_coord[1] = bounds[0][1] + ( empty_index / t->u_block_count );
				if ( !boundsContains( intersection, new_coord ))
					break;
				empty_index++;
			}
			newBlocks[empty_index] = t->blocks[i];
			empty_index++;
		}
	}

	// For each new block
	for ( int i = 0; i < t->total_block_count; i++ ) {
		int coord[2];
		coord[0] = bounds[0][0] + ( i % t->u_block_count );
		coord[1] = bounds[0][1] + ( i / t->u_block_count );
		// if not in old bounds
		if ( !boundsContains( intersection, coord )) {
			canyonTerrainBlock_calculateExtents( newBlocks[i], t, coord );
			// mark it as new, buffers will be filled in later
			newBlocks[i]->pending = true;
		}
	}

	memcpy( t->bounds, bounds, sizeof( int ) * 2 * 2 );
	memcpy( t->blocks, newBlocks, sizeof( canyonTerrainBlock* ) * t->total_block_count );

	for ( int i = 0; i < t->total_block_count; ++i ) {
		canyonTerrainBlock* b = t->blocks[i];
		if ( b->pending ) {
			canyonTerrainBlock_calculateBuffers( b );
			//canyonTerrainBlock_calculateCollision( t, b );
			b->pending = false;
			break;
		}
	}
}

void canyonTerrain_tick( void* data, float dt, engine* eng ) {
	(void)dt;
	(void)eng;
	canyonTerrain* t = data;
	canyonTerrain_updateBlocks( t );
}


