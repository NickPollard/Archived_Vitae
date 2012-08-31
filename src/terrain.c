// terrain.c
#include "common.h"
#include "terrain.h"
//-----------------------
#include "canyon.h"
#include "collision.h"
#include "maths/geometry.h"
#include "maths/vector.h"
#include "mem/allocator.h"
#include "render/debugdraw.h"
#include "render/render.h"
#include "render/shader.h"
#include "render/texture.h"
#include "system/hash.h"

#define TERRAIN_USE_VBO 1

// Forward declarations
void terrain_calculateBounds( int bounds[2][2], terrain* t, vector* sample_point );
void terrainBlock_calculateBuffers( terrain* t, terrainBlock* b );
void terrainBlock_calculateCollision( terrain* t, terrainBlock* b );
void terrainBlock_fillTrianglesForVertex( terrainBlock* b, vector* positions, vertex* vertices, int lod_interval, int u_index, int v_index, vertex* vert );
void terrainBlock_cliffSmooth( terrainBlock* b, int vert_count, vector* verts );

GLuint terrain_texture = 0;
GLuint terrain_texture_cliff = 0;

terrainBlock* terrainBlock_create( ) {
	terrainBlock* b = mem_alloc( sizeof( terrainBlock ));
	memset( b, 0, sizeof( terrainBlock ));
	b->collision_body = NULL;
	return b;
}

void terrainBlock_createBuffers( terrain* t, terrainBlock* b ) {
	b->u_samples = t->u_samples;
	b->v_samples = t->v_samples;
	// Setup buffers
	int triangle_count = ( b->u_samples - 1 ) * ( b->v_samples - 1 ) * 2;
	b->index_count = triangle_count * 3;

	vAssert( !b->vertex_buffer );
	vAssert( !b->element_buffer );
	b->vertex_buffer = mem_alloc( b->index_count * sizeof( vertex ));
	b->element_buffer = mem_alloc( b->index_count * sizeof( GLushort ));
}

// Create a procedural terrain
terrain* terrain_create() {
	terrain* t = mem_alloc( sizeof( terrain ));
	memset( t, 0, sizeof( terrain ));
	t->u_samples = 1;
	t->v_samples = 1;
	t->sample_point = Vector( 0.f, 0.f, 0.f, 1.f );

	// Temp for now
	t->u_block_count = 5;
	t->v_block_count = 5;

	if ( terrain_texture == 0 ) {
		texture_request( &terrain_texture, "dat/img/terrain/snow_2_rgba.tga" );
	}
	if ( terrain_texture_cliff == 0 ) {
		texture_request( &terrain_texture_cliff, "dat/img/terrain/cliffs_2_rgba.tga" );
	}

	return t;
}

void terrainBlock_calculateExtents( terrainBlock* b, terrain* t, int coord[2] ) {
	float u_size = (2 * t->u_radius) / (float)t->u_block_count;
	float v_size = (2 * t->v_radius) / (float)t->v_block_count;
	b->u_min = ((float)coord[0] - 0.5f) * u_size;
	b->v_min = ((float)coord[1] - 0.5f) * v_size;
	b->u_max = b->u_min + u_size;
	b->v_max = b->v_min + v_size;
}

void terrain_createBlocks( terrain* t ) {
	vAssert( !t->blocks );

	t->total_block_count = t->u_block_count * t->v_block_count;
	vAssert( t->total_block_count > 0 );

	t->blocks = mem_alloc( sizeof( terrainBlock* ) * t->total_block_count );

	// Ensure the block bounds are initialised;
	terrain_calculateBounds( t->bounds, t, &t->sample_point );

	// Calculate block extents
	int i = 0;
	for ( int v = 0; v < t->v_block_count; v++ ) {
		for ( int u = 0; u < t->u_block_count; u++ ) {
			int coord[2];
			coord[0] = t->bounds[0][0] + u;
			coord[1] = t->bounds[0][1] + v;

			t->blocks[i] = terrainBlock_create( );
			terrainBlock_createBuffers( t, t->blocks[i] );
			terrainBlock_calculateExtents( t->blocks[i], t, coord );
			terrainBlock_calculateBuffers( t, t->blocks[i] );
			terrainBlock_calculateCollision( t, t->blocks[i] );
			i++;
		}
	}
}

// Returns a value between 0.f and height_m
float terrain_mountainHeight( float u, float v ) {
	float scale_m_u = 40.f;
	float scale_m_v = 40.f;
	float height_m = 40.f;
	return (1.0f + sinf( u / scale_m_u ) * sinf( v / scale_m_v )) * 0.5f * height_m;
}

float terrain_detailHeight( float u, float v ) {
	return	0.5 * sinf( u ) * sinf( v ) +
			sinf( u / 3.f ) * sinf( v / 3.f ) +
			5 * sinf( u / 10.f ) * sinf( v / 10.f ) * sinf( u / 10.f ) * sinf( v / 10.f );
}

// The procedural function
float terrain_sample( float u, float v ) {
	float mountains = terrain_mountainHeight( u, v );
	float detail = 0.f; //terrain_detailHeight( u, v );
	float canyon = terrain_newCanyonHeight( u, v );
	return mountains + detail - canyon;
}

// Could be called during runtime, in which case reinit render variables
void terrain_setSize( terrain* t, float u, float v ) {
	t->u_radius = u;
	t->v_radius = v;

	// TODO: Could be called during runtime, in which case reinit render variables
	// For now just disallow that
	vAssert( !t->blocks );
}

// Set the resolution PER BLOCK
// Could be called during runtime, in which case reinit render variables
void terrain_setResolution( terrain* t, int u, int v ) {
	t->u_samples = u;
	t->v_samples = v;
	terrain_createBlocks( t );
}

// Create GPU vertex buffer objects to hold our data and save transferring to the GPU each frame
void terrainBlock_initVBO( terrainBlock* b ) {
	if ( !b->vertex_VBO ) {
		b->vertex_VBO = render_requestBuffer( GL_ARRAY_BUFFER, b->vertex_buffer, sizeof( vertex ) * b->index_count );
	} else {
		// If we've already allocated a buffer at some point, just re-use it
		render_bufferCopy( GL_ARRAY_BUFFER, *b->vertex_VBO, b->vertex_buffer, sizeof( vertex ) * b->index_count );
	}
	if ( !b->element_VBO ) {
		b->element_VBO = render_requestBuffer( GL_ELEMENT_ARRAY_BUFFER, b->element_buffer, sizeof( GLushort ) * b->index_count );
	} else {
		// If we've already allocated a buffer at some point, just re-use it
		render_bufferCopy( GL_ELEMENT_ARRAY_BUFFER, *b->element_VBO, b->element_buffer, sizeof( GLushort ) * b->index_count );
	}
}

// *** Utility Functions

int terrainBlock_triangleCount( terrainBlock* b, int lod_interval ) {
	return ( b->u_samples - 1 ) * ( b->v_samples - 1 ) * 2 / ( lod_interval * lod_interval );
}

int terrainBlock_vertCount( terrainBlock* b ) {
	return b->u_samples * b->v_samples;
}

float terrainBlock_uInterval( terrainBlock* b ) {
	return ( b->u_max - b->u_min ) / ( b->u_samples - 1);
}

float terrainBlock_vInterval( terrainBlock* b ) {
	return ( b->v_max - b->v_min ) / ( b->v_samples - 1);
}

vector terrainBlock_center ( terrainBlock* b ) {
	vector center = Vector(( b->u_min + b->u_max ) * 0.5f, 0.f, ( b->v_min + b->v_max ) * 0.5f, 1.f );
	return center;
}

int terrainBlock_indexFromUV( terrainBlock* b, int u, int v  ) {
	return v + ( u * b->v_samples );
}

// ***

void terrainBlock_calculateHeights( terrainBlock* b, int vert_count, vector* verts ) {
	// Calculate bounds and intervals
	vAssert( b->u_max > b->u_min );
	vAssert( b->v_max > b->v_min );
	float u_interval = terrainBlock_uInterval( b );
	float v_interval = terrainBlock_vInterval( b );
	// Loosen max edges to ensure final verts aren't dropped
	float u_max = b->u_max + (u_interval * 0.5f);
	float v_max = b->v_max + (v_interval * 0.5f);
	int vert_index = 0;
	for ( float u = b->u_min; u < u_max; u+= u_interval ) {
		for ( float v = b->v_min; v < v_max; v+= v_interval ) {
			float h = terrain_sample( u, v );
			verts[vert_index++] = Vector( u, h, v, 1.f );
		}
	}
	vAssert( vert_index == vert_count );
}

void terrainBlock_initialiseElementBuffer( int count, unsigned short* buffer ) {
	for ( int i = 0; i < count; i++ )
		buffer[i] = i;
}

void terrainBlock_calculateNormals( terrainBlock* b, int vert_count, vector* verts, vector* normals ) {
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
		x = Vector( -1.f, 0.f, 0.f, 0.f ); // Negative to ensure cross products come out correctly
		y = Vector( 0.f, 0.f, 1.f, 0.f );

		// Calculate vertical vector
		// Take cross product to calculate normals
		Sub( &a, &bottom, &top );
		Cross( &b, &x, &a );

		// Calculate horizontal vector
		// Take cross product to calculate normals
		Sub( &a, &right, &left );
		Cross( &c, &y, &a );

		// Average normals
		vector total = Vector( 0.f, 0.f, 0.f, 0.f );
		Add( &total, &total, &b );
		Add( &total, &total, &c );
		total.coord.w = 0.f;
		Normalize( &total, &total );
		normals[i] = total;
	}
}

void terrainBlock_generateVertices( terrainBlock* b, int lod_interval, vector* verts, vector* normals ) {
	const float texture_scale = 0.0325f;
	for ( int u = 0; u < b->u_samples; u += lod_interval ) {
		for ( int v = 0; v < b->v_samples; v += lod_interval ) {
			int i = terrainBlock_indexFromUV( b, u, v );
			vertex vert;
			vert.position = verts[i];
			vert.normal = normals[i];
			vert.uv = Vector( vert.position.coord.x * texture_scale, vert.position.coord.z * texture_scale, 0.f, 0.f );
			terrainBlock_fillTrianglesForVertex( b, verts, b->vertex_buffer, lod_interval, u, v, &vert );
		}
	}
}

/*
   Calculate vertex and element buffers for a given block from a given
   terrain
   */
void terrainBlock_calculateBuffers( terrain* t, terrainBlock* b ) {
	(void)t;
	int vert_count = terrainBlock_vertCount( b );
	vector* verts	= mem_alloc( vert_count * sizeof( vector ));
	vector* normals	= mem_alloc( vert_count * sizeof( vector ));

	terrainBlock_calculateHeights( b, vert_count, verts );

	terrainBlock_cliffSmooth( b, vert_count, verts );

	terrainBlock_calculateNormals( b, vert_count, verts, normals );

	int lod_interval = 1;
	terrainBlock_generateVertices( b, lod_interval, verts, normals );

	b->index_count = terrainBlock_triangleCount( b, lod_interval ) * 3;
	terrainBlock_initialiseElementBuffer( b->index_count, b->element_buffer );

	mem_free( verts );
	mem_free( normals );

#if TERRAIN_USE_VBO
	terrainBlock_initVBO( b );
#endif
}

// Do a smoothing operation by averaging positions against their neighbours, weighted by how close in Y they are
void terrainBlock_cliffSmooth( terrainBlock* b, int vert_count, vector* verts ) {
	vector* new_verts = mem_alloc( sizeof( vector ) * vert_count );
	int vert_index = 0;
	for ( int u = 0; u < b->u_samples; ++u ) {
		for ( int v = 0; v < b->v_samples; ++v ) {
			if (( u == 0 || u == b->u_samples - 1 ) ||
				( v == 0 || v == b->v_samples - 1 )) {
				new_verts[vert_index] = verts[vert_index];
				++vert_index;
			}
			else {
				vector v = verts[vert_index];
				vector left		= verts[vert_index - b->u_samples];
				vector right	= verts[vert_index + b->u_samples];
				vector top		= verts[vert_index - 1];
				vector bottom	= verts[vert_index + 1];
				float right_k = 1.f / fabsf( right.coord.y - v.coord.y );
				float left_k = 1.f / fabsf( left.coord.y - v.coord.y );
				float top_k = 1.f / fabsf( top.coord.y - v.coord.y );
				float bottom_k = 1.f / fabsf( bottom.coord.y - v.coord.y );
				float k = right_k + left_k + top_k + bottom_k;
				right_k = right_k / k;
				left_k = left_k / k;
				top_k = top_k / k;
				bottom_k = bottom_k / k;
				// Clamping
				const float min = 0.05f;
				const float max = 0.4f;
				right_k = fclamp( right_k, min, max );
				left_k = fclamp( left_k, min, max );
				top_k = fclamp( top_k, min, max );
				bottom_k = fclamp( bottom_k, min, max );
				k = right_k + left_k + top_k + bottom_k;
				right_k = right_k / k;
				left_k = left_k / k;
				top_k = top_k / k;
				bottom_k = bottom_k / k;

				v.coord.x = right.coord.x * right_k + 
					left.coord.x * left_k + 
					top.coord.x * top_k + 
					bottom.coord.x * bottom_k;
				v.coord.z = right.coord.z * right_k + 
					left.coord.z * left_k + 
					top.coord.z * top_k + 
					bottom.coord.z * bottom_k;
				new_verts[vert_index] = v;
				++vert_index;
			}
		}
	}
	memcpy( verts, new_verts, sizeof( vector ) * vert_count );
}

bool terrainBlock_triangleInvalid( terrainBlock* b, int lod_interval, int u_index, int v_index, int u_offset, int v_offset ) {
	u_offset = u_offset / 2 + u_offset % 2;
	u_offset = min( u_offset, 0 );
	u_offset *= lod_interval;
	v_offset *= lod_interval;
	return ( u_index + u_offset >= b->u_samples - 1 ) ||
		( u_index + u_offset < 0 ) ||
		( v_index + v_offset >= b->v_samples - 1 ) ||
		( v_index + v_offset < 0 );
}

// Given a vertex that has been generated, fill in the correct triangles for it after it has been unrolled
void terrainBlock_fillTrianglesForVertex( terrainBlock* b, vector* positions, vertex* vertices, int lod_interval, int u_index, int v_index, vertex* vert ) {
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
	const int triangle_vert_indices[6] = { 0, 1, 2, 2, 1, 0 };
	const int v_offset[6] = { -1, -1, -1, 0, 0, 0 };
	const int u_offset[6] = { -1, 0, 1, -2, -1, 0 };
	
	int triangles_per_row = (( b->u_samples - 1 ) / lod_interval ) * 2;
	
	for ( int i = 0; i < 6; ++i ) {
		// Calculate triangle index
		int row = v_index + ( v_offset[i] * lod_interval );
		int column = ( 2 * u_index ) + ( u_offset[i] * lod_interval );
		int triangle_index = triangles_per_row * row + column; 

		if ( terrainBlock_triangleInvalid( b, lod_interval, u_index, v_index, u_offset[i], v_offset[i]) )
			continue;

		// if it's a valid triangle (not out-of-bounds)
		int vert_index = triangle_index * 3 + triangle_vert_indices[i];
		vertices[vert_index] = *vert;

		// Cliff coloring
		const int triangle_u_offset[6][2] = { { 0, -1 }, { 1, 0 }, { 1, 1 }, { -1, -1 }, { -1, 0 }, { 0, 1 } };
		const int triangle_v_offset[6][2] = { { -1, 0 }, { -1, -1 }, { 0, -1 }, { 0, 1 }, { 1, 1 }, { 1, 0 } };
		
		// Calculate indices of other 2 triangle verts
		int index_b = terrainBlock_indexFromUV( b, u_index + triangle_u_offset[i][0] * lod_interval, v_index + triangle_v_offset[i][0] * lod_interval );
		int index_c = terrainBlock_indexFromUV( b, u_index + triangle_u_offset[i][1] * lod_interval, v_index + triangle_v_offset[i][1] * lod_interval );

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


// Calculate the intersection of the two block bounds specified
void terrain_boundsIntersection( int intersection[2][2], int a[2][2], int b[2][2] ) {
	intersection[0][0] = max( a[0][0], b[0][0] );
	intersection[0][1] = max( a[0][1], b[0][1] );
	intersection[1][0] = min( a[1][0], b[1][0] );
	intersection[1][1] = min( a[1][1], b[1][1] );
}

void terrain_blockContaining( int coord[2], terrain* t, vector* point ) {
	float block_width = (2 * t->u_radius) / (float)t->u_block_count;
	float block_height = (2 * t->v_radius) / (float)t->v_block_count;
	coord[0] = fround( point->coord.x / block_width, 1.f );
	coord[1] = fround( point->coord.z / block_height, 1.f );
}

// Calculate the block bounds for the terrain, at a given sample point
void terrain_calculateBounds( int bounds[2][2], terrain* t, vector* sample_point ) {
	/*
	   Find the block we are in
	   Then extend by the correct radius

	   The center block [0] is from -half_block_width to half_block_width
	   */
	int block[2];
	terrain_blockContaining( block, t, sample_point );
	int rx = ( t->u_block_count - 1 ) / 2;
	int ry = ( t->v_block_count - 1 ) / 2;
	bounds[0][0] = block[0] - rx;
	bounds[0][1] = block[1] - ry;
	bounds[1][0] = block[0] + rx;
	bounds[1][1] = block[1] + ry;
}

bool boundsContains( int bounds[2][2], int coord[2] ) {
	return ( coord[0] >= bounds[0][0] &&
			coord[1] >= bounds[0][1] &&
			coord[0] <= bounds[1][0] &&
			coord[1] <= bounds[1][1] );
}

void terrain_updateBlocks( terrain* t ) {
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
	terrain_calculateBounds( bounds, t, &t->sample_point );
	terrain_boundsIntersection( intersection, bounds, t->bounds );

	// Using alloca for dynamic stack allocation (just moves the stack pointer up)
	terrainBlock** newBlocks = alloca( sizeof( terrainBlock* ) * t->total_block_count );

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
			terrainBlock_calculateExtents( newBlocks[i], t, coord );
			// mark it as new, buffers will be filled in later
			newBlocks[i]->pending = true;
		}
	}

	memcpy( t->bounds, bounds, sizeof( int ) * 2 * 2 );
	memcpy( t->blocks, newBlocks, sizeof( terrainBlock* ) * t->total_block_count );

	for ( int i = 0; i < t->total_block_count; ++i ) {
		terrainBlock* b = t->blocks[i];
		if ( b->pending ) {
			terrainBlock_calculateBuffers( t, b );
			terrainBlock_calculateCollision( t, b );
			b->pending = false;
			break;
		}
	}
}

void terrainBlock_render( terrainBlock* b ) {
	drawCall* draw = drawCall_create( &renderPass_main, resources.shader_terrain, b->index_count, b->element_buffer, b->vertex_buffer, terrain_texture, modelview );
	draw->texture_b = terrain_texture_cliff;
	(void)draw;
#if TERRAIN_USE_VBO
	if ( *b->vertex_VBO != 0 ) {
		draw->vertex_VBO = *b->vertex_VBO;
		draw->element_VBO = *b->element_VBO;
	}
#endif
}

void terrainBlock_debugDraw( terrainBlock* b ) {

	vector green = Vector( 0.f, 1.f, 0.f, 1.f );
	int vert_count = b->u_samples * b->v_samples;
	vAssert( vert_count < 512 );
	vector verts[512];
	for ( int i = 0; i < vert_count; ++i ) {
		verts[i] = b->vertex_buffer[i].position;
	}

	debugdraw_wireframeMesh( vert_count, verts, b->index_count, b->element_buffer, matrix_identity, green );
}

void terrain_tick( void* data, float dt, engine* eng ) {
	(void)dt;
	(void)eng;
	terrain* t = data;
	terrain_updateBlocks( t );
}

// Send the buffers to the GPU
void terrain_render( void* data ) {
	terrain* t = data;

	render_resetModelView();
	matrix_mul( modelview, modelview, t->trans->world );

	// *** Render the blocks
	for ( int i = 0; i < t->total_block_count; i++ ) {
		terrainBlock_render( t->blocks[i] );
	}
}

void terrain_delete( terrain* t ) {
	mem_free( t );
}

heightField* terrainBlock_createHeightField( terrain* t, terrainBlock* b ) {
	(void)t;
	float width = b->u_max - b->u_min;
	float length = b->v_max - b->v_min;
	heightField* h = heightField_create( width, length, b->u_samples, b->v_samples );

	float u_interval = ( b->u_max - b->u_min ) / ( b->u_samples - 1);
	float v_interval = ( b->v_max - b->v_min ) / ( b->v_samples - 1);
	// Loosen max edges to ensure final verts aren't dropped
	float u_max = b->u_max + (u_interval * 0.5f);
	float v_max = b->v_max + (v_interval * 0.5f);
	int vert_index = 0;
	for ( float u = b->u_min; u < u_max; u+= u_interval ) {
		for ( float v = b->v_min; v < v_max; v+= v_interval ) {
			h->verts[vert_index++] = terrain_sample( u, v );
		}
	}
	
	return h;
}

// Calculate the collision for a given block
void terrainBlock_calculateCollision( terrain* t, terrainBlock* b ) {
	if ( !b->collision_body ) {
		b->collision_body = body_create( NULL, NULL );
		b->collision_body->trans = transform_create();
		b->collision_body->layers |= 0x2; // Enemy
		b->collision_body->collide_with |= 0x1; // Player
		collision_addBody( b->collision_body );
		printf( "Adding terrain collision_Body\n" );
	}
	if ( b->collision_body->shape ) {
		shape_delete( b->collision_body->shape );
	}

	heightField* h = terrainBlock_createHeightField( t, b );
	b->collision_body->shape = shape_heightField_create( h );
	// Set transform for centre of heightfield
	vector position = terrainBlock_center( b );
	transform_setWorldSpacePosition( b->collision_body->trans, &position );
}

void test_terrain() {
	// Bounds test
	int i[2][2];
	int a[2][2] = {{1, 1}, {3, 3}};
	int b[2][2] = {{2, 2}, {4, 4}};
	terrain_boundsIntersection( i, a, b );
	vAssert( i[0][0] == 2 );
	vAssert( i[0][1] == 2 );
	vAssert( i[1][0] == 3 );
	vAssert( i[1][1] == 3 );

	a[0][0] = -1;
	b[0][0] = -2;
	a[0][1] = -3;
	b[0][1] = -4;
	terrain_boundsIntersection( i, a, b );
	vAssert( i[0][0] == -1 );
	vAssert( i[0][1] == -3 );
	vAssert( i[1][0] == 3 );
	vAssert( i[1][1] == 3 );

	terrain* t = terrain_create();
	terrain_setSize( t, 5.f, 5.f );
	terrain_setResolution( t, 8, 8 );

	// Should be -2, -2, 2, 2, if the block radius is 5	
	vector v = Vector( 0.f, 0.f, 0.f, 1.f );
	int bounds[2][2];
	terrain_calculateBounds( bounds, t, &v );
	vAssert( bounds[0][0] == -2 );
	vAssert( bounds[0][1] == -2 );
	vAssert( bounds[1][0] == 2 );
	vAssert( bounds[1][1] == 2 );

	int coord[2] = { 0, 2 };
	vAssert( boundsContains( bounds, coord ));
	coord[0] = 3;
	vAssert( !boundsContains( bounds, coord ));
	coord[0] = 2;
	vAssert( boundsContains( bounds, coord ));

	terrain_delete( t );
}


