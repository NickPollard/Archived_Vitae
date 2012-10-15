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
	int element_count_render; // The one currently used to render with; for smooth LoD switching
	unsigned short* element_buffer;
	vertex* vertex_buffer;

	// We double-buffer the terrain blocks for LOD purposes, so we can switch instantaneously
	GLuint*			vertex_VBO;
	GLuint*			element_VBO;
	GLuint*			vertex_VBO_alt;
	GLuint*			element_VBO_alt;

	//temp
//	vector* verts;

	bool pending;	// Whether we need to recalculate the block
	int	lod_level;	// Current lod-level

	canyon* canyon;
	int	coord[2];
	canyonTerrain* terrain;

} canyonTerrainBlock;

struct canyonTerrain_s {
	transform* trans;

	float	u_radius;
	float	v_radius;
	int u_block_count;
	int v_block_count;
	int total_block_count;
	canyonTerrainBlock** blocks;
	
	int u_samples_per_block;
	int v_samples_per_block;

	int lod_interval_u;
	int lod_interval_v;
	
	int				bounds[2][2];
	vector			sample_point;

	canyon*			canyon;
};

extern texture* terrain_texture;
extern texture* terrain_texture_cliff;

// *** Functions 

canyonTerrain* canyonTerrain_create( canyon* c, int u_blocks, int v_blocks, int u_samples, int v_samples, float u_radius, float v_radius );
void canyonTerrain_setLodIntervals( canyonTerrain* t, int u, int v );
void canyonTerrain_render( void* data );
void canyonTerrain_tick( void* data, float dt, engine* eng );

float canyonTerrain_sample( float u, float v );
