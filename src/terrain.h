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

void test_terrain();
