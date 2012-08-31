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
	int vert_count;
	vertex* vertex_buffer;

	GLuint*			vertex_VBO;
	GLuint*			element_VBO;

	//temp
	vector* verts;
} canyonTerrainBlock;

typedef struct canyonTerrain_s {
	transform* trans;
	int block_count;
	canyonTerrainBlock** blocks;
} canyonTerrain;

// *** Functions 

canyonTerrain* canyonTerrain_create();
void canyonTerrain_init( canyonTerrain* t );
void canyonTerrain_render( canyonTerrain* t );
