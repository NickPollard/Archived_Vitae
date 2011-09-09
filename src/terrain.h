// terrain.h
#pragma once
#include "src/model.h"

typedef struct terrain_s {
	transform*	trans;
	float	u_radius;
	float	v_radius;
	int		u_samples;
	int		v_samples;
	float	u_interval;
	float	v_interval;

	// For Rendering
	int index_count;
	vertex*		vertex_buffer;
	unsigned short*		element_buffer;
} terrain;

// Create a procedural terrain
terrain* terrain_create();

void terrain_setSize( terrain* t, float u, float v );
void terrain_setResolution( terrain* t, int u, int v );

// Calculate vertex and element buffers from procedural definition
void terrain_calculateBuffers( terrain* t );

// The renderfunc
void terrain_render( void* data );

// *** Test
void test_terrain();
