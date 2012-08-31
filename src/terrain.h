// terrain.h
#pragma once
#include "render/render.h"

typedef struct terrainBlock_s {
	float	u_min;
	float	u_max;
	float	v_min;
	float	v_max;
	int		u_samples;
	int		v_samples;

	// If true, we need to build buffers and collision etc. for this block
	bool	pending;

	// For Rendering
	int				index_count;
	vertex*			vertex_buffer;
	unsigned short*	element_buffer;
	GLuint*			vertex_VBO;
	GLuint*			element_VBO;

	// For collision
	body*			collision_body;
} terrainBlock;

typedef struct terrain_s {
	transform*	trans;
	float	u_radius;
	float	v_radius;
	int		u_samples;
	int		v_samples;
	vector	sample_point;
	// Block system
	int				total_block_count;
	int				u_block_count;
	int				v_block_count;
	terrainBlock**	blocks;		// An array of block pointers
	int				bounds[2][2];
} terrain;

// Create a procedural terrain
terrain* terrain_create();

// Set rendering parameters
void terrain_setSize( terrain* t, float u, float v );
void terrain_setResolution( terrain* t, int u, int v );

// The renderfunc
void terrain_render( void* data );

// The tickfunc
void terrain_tick( void* data, float dt, engine* eng );

float terrain_sample( float u, float v );

// *** Test
void test_terrain();
