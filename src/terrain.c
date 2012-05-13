// terrain.c
#include "common.h"
#include "terrain.h"
//-----------------------
#include "mem/allocator.h"
#include "render/render.h"
#include "render/shader.h"
#include "render/texture.h"
#include "system/hash.h"

// Forward declarations
void terrain_calculateBounds( int bounds[2][2], terrain* t, vector* sample_point );
void terrainBlock_calculateBuffers( terrain* t, terrainBlock* b );

GLuint terrain_texture = 0;

terrainBlock* terrainBlock_create( ) {
	terrainBlock* b = mem_alloc( sizeof( terrainBlock ));
	memset( b, 0, sizeof( terrainBlock ));
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
		texture_request( &terrain_texture, "dat/3rdparty/img/rock02_tile_small.tga" );
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
			i++;
		}
	}
}

// The procedural function
float terrain_sample( float u, float v ) {
	float scale_m_u = 40.0;
	float scale_m_v = 40.0;
	float height_m = 40.0;
	float mountains = (1.0f + sin( u / scale_m_u ) * sin( v / scale_m_v )) * 0.5f * height_m;

	float detail =
	(
			0.5 * sin( u ) * sin( v ) +
			sin( u / 3.f ) * sin( v / 3.f ) +
			5 * sin( u / 10.f ) * sin( v / 10.f ) * sin( u / 10.f ) * sin( v / 10.f )
	);

	float canyon_length_scale = 250.f;
	float scale = 5.f;
	u /= scale;
	float canyon_width_scale = 100.f;
	u += sin( v / (canyon_length_scale) ) * canyon_width_scale;
	float height = 40.f;
	float width = 4.f;
	float base_radius = 4.f;
	float new_u = u;
	if ( u < 0.f )
	{
		new_u = fminf( u + base_radius, 0.f );
	}
	else
	{
		new_u = fmaxf( u - base_radius, 0.f );
	}

	float mask = cos( fclamp( new_u / 4.f, -PI/2.f, PI/2.f ));
	float canyon = 1.f - fclamp( powf( new_u / width, 4.f ), 0.f, 1.f );

	return mountains + detail - ( mask * canyon ) * height;
}

// Could be called during runtime, in which case reinit render variables
void terrain_setSize( terrain* t, float u, float v ) {
	t->u_radius = u;
	t->v_radius = v;

	// TODO:
	// Could be called during runtime, in which case reinit render variables
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

/*
   Calculate vertex and element buffers for a given block from a given
   terrain
   */
void terrainBlock_calculateBuffers( terrain* t, terrainBlock* b ) {
/*	We have a grid of x * y points
	So (x-1) * (y-1) quads
	So (x-1) * (y-1) * 2 tris
	So (x-1) * (y-1) * 6 indices */

	int triangle_count = ( b->u_samples - 1 ) * ( b->v_samples - 1 ) * 2;
	int vert_count = b->u_samples * b->v_samples;

	// Calculate bounds and intervals
	vAssert( b->u_max > b->u_min );
	vAssert( b->v_max > b->v_min );
	float u_interval = ( b->u_max - b->u_min ) / ( b->u_samples - 1);
	float v_interval = ( b->v_max - b->v_min ) / ( b->v_samples - 1);
	// Loosen max edges to ensure final verts aren't dropped
	float u_max = b->u_max + (u_interval * 0.5f);
	float v_max = b->v_max + (v_interval * 0.5f);

	vector* verts	= mem_alloc( vert_count * sizeof( vector ));
	vector* normals	= mem_alloc( vert_count * sizeof( vector ));

	int vert_index = 0;
	for ( float u = b->u_min; u < u_max; u+= u_interval ) {
		for ( float v = b->v_min; v < v_max; v+= v_interval ) {
			float h = terrain_sample( u, v );
			verts[vert_index++] = Vector( u, h, v, 1.f );
		}
	}
	assert( vert_index == vert_count );

	// Do top and bottom edges first
	for ( int i = 0; i < t->u_samples; i++ )
		normals[i] = Vector( 0.f, 1.f, 0.f, 0.f );
	for ( int i = vert_count - t->u_samples; i < vert_count; i++ )
		normals[i] = Vector( 0.f, 1.f, 0.f, 0.f );

	//Now the rest
	for ( int i = t->u_samples; i < ( vert_count - t->u_samples ); i++ ) {
		// Ignoring Left and Right Edges
		if ( i % t->u_samples == 0 || i % t->u_samples == ( t->u_samples - 1 ) ) {    
			normals[i] = Vector( 0.f, 1.f, 0.f, 0.f );
			continue;
		}
		vector left		= verts[i - t->u_samples];
		vector right	= verts[i + t->u_samples];
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

	int element_count = 0;
	// Calculate elements
	int tw = ( t->u_samples - 1 ) * 2; // Triangles per row
	for ( int i = 0; i < triangle_count; i+=2 ) {
		GLushort tl = ( i / 2 ) + i / tw;
		GLushort tr = tl + 1;
		GLushort bl = tl + t->u_samples;
		GLushort br = bl + 1;

		// Unroll this, do two triangles at a time
		// bottom right triangle - br, bl, tr
		b->element_buffer[element_count+0] = br;
		b->element_buffer[element_count+1] = bl;
		b->element_buffer[element_count+2] = tr;
		// top left triangle - tl, tr, bl
		b->element_buffer[element_count+3] = tl;
		b->element_buffer[element_count+4] = tr;
		b->element_buffer[element_count+5] = bl;
		element_count += 6;
	}

	float texture_scale = 0.125f;

	// For each element index
	// Unroll the vertex/index bindings
	for ( int i = 0; i < b->index_count; i++ ) {
		// Copy the required vertex position, normal, and uv
		b->vertex_buffer[i].position = verts[b->element_buffer[i]];
		b->vertex_buffer[i].normal = normals[b->element_buffer[i]];
		b->vertex_buffer[i].uv = Vector( verts[b->element_buffer[i]].coord.x * texture_scale, verts[b->element_buffer[i]].coord.z * texture_scale, 0.f, 0.f );
		b->element_buffer[i] = i;
	}
	mem_free( verts );
	mem_free( normals );

	// Create GPU 
	b->vertex_VBO = render_requestBuffer( GL_ARRAY_BUFFER, b->vertex_buffer, sizeof( vertex ) * b->index_count );
	b->element_VBO = render_requestBuffer( GL_ELEMENT_ARRAY_BUFFER, b->element_buffer, sizeof( GLushort ) * b->index_count );
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

	// Could double buffer this and avoid a mem_alloc/free?
	terrainBlock**	newBlocks = mem_alloc( sizeof( terrainBlock* ) * t->total_block_count );

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
			terrainBlock_calculateBuffers( t, newBlocks[i] );
		}
	}

	memcpy( t->bounds, bounds, sizeof( int ) * 2 * 2 );
	memcpy( t->blocks, newBlocks, sizeof( terrainBlock* ) * t->total_block_count );

	mem_free( newBlocks );
}

void terrainBlock_render( terrainBlock* b ) {
	drawCall* draw = drawCall_create( &renderPass_main, resources.shader_terrain, b->index_count, b->element_buffer, b->vertex_buffer, terrain_texture, modelview );
//	vAssert( *b->vertex_VBO != 0 );
//	vAssert( *b->element_VBO != 0 );
	if ( *b->vertex_VBO != 0 ) {
		draw->vertex_VBO = *b->vertex_VBO;
		draw->element_VBO = *b->element_VBO;
	}
}

void terrain_tick( void* data, float dt ) {
	(void)dt;
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
