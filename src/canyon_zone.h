// Canyon_zone.h
#pragma once
#include "maths/vector.h"

#define kZoneLength 2000.f
#define kNumZones	4
#define kZoneBlendDistance 400.f

// Texture
#define kZoneTextureWidth 128
#define kZoneTextureHeight 128
#define kZoneTextureStride 4	// 4 bytes, or 4 * 8 = 32 bit per pixel (RGBA8)

struct canyonZone_s {
	vector terrain_color;
	vector cliff_color;
	vector edge_color;
	vector sky_color;
	vector fog_color;
};

canyonZone zones[kNumZones];
extern texture* canyonZone_lookup_texture;
extern int canyon_zone_count;
extern vector zone_sample_point;

int canyon_zone( float v );
int canyon_zoneType( float v );

float canyonZone_progress( float v );
float canyonZone_totalBlend( float v );
float canyonZone_blend( float v );

// A float ratio for how strongly in the zone we are, use for zone colouring
float canyonZone_terrainBlend( float v );

// *** Colors
vector canyonZone_cliffColor( int zone );
vector canyonZone_terrainColor( int zone );
vector canyonZone_edgeColor( int zone );
vector canyonZone_terrainColorAtV( float v );
vector canyonZone_cliffColorAtV( float v );
vector canyonZone_edgeColorAtV( float v );

canyonZone* canyonZone_create();
void canyonZone_load( canyon* c, const char* filename );
void canyonZone_tick( canyon* c, float dt );
void canyonZone_skyFogBlend( canyonZone* a, canyonZone* b, float blend, vector* sky_color, vector* fog_color );
