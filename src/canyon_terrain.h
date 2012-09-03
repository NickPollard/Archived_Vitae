// canyon_terrain.h
#pragma once
#include "render/render.h"

typedef struct canyonTerrainBlock_s {
	int u_samples;
	int v_samples;

	float u_min;
	float u_max;
	float v_min;
	float v_max;

	int element_count;
	unsigned short* element_buffer;
	vertex* vertex_buffer;

	GLuint*			vertex_VBO;
	GLuint*			element_VBO;

	//temp
	vector* verts;

	bool pending;	// Whether we need to recalculate the block

} canyonTerrainBlock;

typedef struct canyonTerrain_s {
	transform* trans;

	float	u_radius;
	float	v_radius;
	int u_block_count;
	int v_block_count;
	int total_block_count;
	canyonTerrainBlock** blocks;
	
	int u_samples_per_block;
	int v_samples_per_block;
	
	int				bounds[2][2];
	vector			sample_point;
} canyonTerrain;

// *** Functions 

canyonTerrain* canyonTerrain_create();
void canyonTerrain_render( void* data );
void canyonTerrain_tick( void* data, float dt, engine* eng );
