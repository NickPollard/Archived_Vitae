// Canyon_zone.c
#include "common.h"
#include "canyon_zone.h"
//-------------------------
#include "canyon.h"
#include "maths/maths.h"
#include "mem/allocator.h"
#include "render/texture.h"
#include "script/lisp.h"

bool canyon_zone_init = false;

texture* canyonZone_lookup_texture = NULL;
texture* canyonZone_lookup_pending = NULL;
int canyon_zone_count = 1;
int canyon_current_zone = 0;
vector zone_sample_point;

// What Zone are we in at a given distance V down the canyon? This forever increases
int canyon_zone( float v ) {
	vAssert( canyon_zone_init );
	return (int)floorf( v / kZoneLength );
}

// What ZoneType are we in at a given distance V down the canyon? This is canyon_zone modulo zone count
// (so repeating zones of the same type have the same index)
int canyon_zoneType( float v ) {
	return canyon_zone( v ) % canyon_zone_count;
}

// How far through the current zone are we?
float canyonZone_distance( float v ) {
	return v - floorf( v / kZoneLength ) * kZoneLength;
}

// A float ratio for how strongly in the zone we are, use for zone terrain colouring
// See-saws back and forth between 1.f and 0.f to deal with our texture building
float canyonZone_terrainBlend( float v ) {
	int zone = canyon_zone( v );
	float this = (float)(zone % 2);
	float next = 1.f - this;
	return lerp( this, next, canyonZone_blend( v ));
}

// Returns a blend value from x to x + 1.f, of how much we should blend to the next zone
// Where x is the current zone index
float canyonZone_totalBlend( float v ) {
	float base = floorf( v / kZoneLength );
	float distance_remaining = kZoneLength - canyonZone_distance( v );
	float blend = 1.f - fclamp( distance_remaining / kZoneBlendDistance, 0.f, 1.f );
	return base + blend;
}

// Returns a blend value from 0.f to 1.f, of how much we should blend to the next zone
float canyonZone_blend( float v ) {
	float distance_remaining = kZoneLength - canyonZone_distance( v );
	float blend = 1.f - fclamp( distance_remaining / kZoneBlendDistance, 0.f, 1.f );
	return blend;
}

float canyonZone_progress( float v ) {
	//int zone = canyon_zone( v );
	//float distance = canyonZone_distance( v );
	return v / kZoneLength;
}

// What color is the standard terrain for ZONE?
vector canyonZone_terrainColor( int zone ) {
	vAssert( canyon_zone_init );
	return zones[zone % canyon_zone_count].terrain_color;
}

vector canyonZone_cliffColor( int zone ) {
	vAssert( canyon_zone_init );
	return zones[zone % canyon_zone_count].cliff_color;
}

vector canyonZone_edgeColor( int zone ) {
	vAssert( canyon_zone_init );
	return zones[zone % canyon_zone_count].edge_color;
}

vector canyonZone_terrainColorAtV( float v ) {
	vAssert( canyon_zone_init );
	return canyonZone_terrainColor( canyon_zone( v ));
}

vector canyonZone_cliffColorAtV( float v ) {
	vAssert( canyon_zone_init );
	return canyonZone_cliffColor( canyon_zone( v ));
}

vector canyonZone_edgeColorAtV( float v ) {
	vAssert( canyon_zone_init );
	return canyonZone_edgeColor( canyon_zone( v ));
}

texture* canyonZone_buildTexture( canyonZone a, canyonZone b ) {
	uint8_t* bitmap = mem_alloc( sizeof( uint8_t ) * kZoneTextureWidth * kZoneTextureHeight * kZoneTextureStride );
	for ( int y = 0; y < kZoneTextureHeight; ++y ) {
		for ( int x = 0; x < kZoneTextureWidth; ++x ) {
			int index = ( x + y * kZoneTextureWidth ) * kZoneTextureStride;
			float zone = (float)x / (float)kZoneTextureWidth;
			float cliff = (float)y / (float)kZoneTextureHeight;
			vector terrain_color = vector_lerp( &a.terrain_color, &b.terrain_color, zone );
			vector cliff_color = vector_lerp( &a.cliff_color, &b.cliff_color, zone );
			//vector edge_color = vector_lerp( &a.edge_color, &b.edge_color, zone );
			vector color = vector_lerp( &terrain_color, &cliff_color, cliff );
			bitmap[index + 0] = (uint8_t)( color.coord.x * 255.f );
			bitmap[index + 1] = (uint8_t)( color.coord.y * 255.f );
			bitmap[index + 2] = (uint8_t)( color.coord.z * 255.f );
			bitmap[index + 3] = 1.f;
		}
	}
	texture* t = texture_loadFromMem( kZoneTextureWidth, kZoneTextureHeight, kZoneTextureStride, bitmap );
	return t;
}


void canyonZone_loadTextureForZone( int zone_index ) {
	int zone_type_index = zone_index % canyon_zone_count;
	int other_index = ( zone_index + 1 ) % canyon_zone_count;
	// We flip around alternate zones to deal with the blending
	if ( zone_index % 2 == 0 ) {
		canyonZone_lookup_pending = canyonZone_buildTexture( zones[zone_type_index], zones[other_index] );
	} else {
		canyonZone_lookup_pending = canyonZone_buildTexture( zones[other_index], zones[zone_type_index] );
	}
}

void canyonZone_tick( float dt ) {
	float u, v;
	terrain_canyonSpaceFromWorld( zone_sample_point.coord.x, zone_sample_point.coord.z, &u, &v );
	int zone = canyon_zone( v );
	if ( zone != canyon_current_zone ) {
		canyonZone_loadTextureForZone( zone );
		canyon_current_zone = zone;
	}
	(void)dt;
	if ( canyonZone_lookup_pending && 
			canyonZone_lookup_pending->gl_tex ) {
		texture_delete( canyonZone_lookup_texture );
		canyonZone_lookup_texture = canyonZone_lookup_pending;
		canyonZone_lookup_pending = NULL;	
	}
}

void canyonZone_staticInit() {
	/*
	// Ice
	zones[0].terrain_color = Vector( 0.8f, 0.9f, 1.0f, 1.f );
	zones[0].cliff_color = Vector( 0.2f, 0.3f, 0.6f, 1.f );
	zones[0].edge_color = Vector( 0.4f, 0.55f, 0.8f, 1.f );

	// Wasteland	
	zones[1].terrain_color = Vector( 0.7f, 0.3f, 0.2f, 1.f );
	zones[1].cliff_color = Vector( 0.3f, 0.2f, 0.2f, 1.f );
	zones[1].edge_color = Vector( 0.6f, 0.2f, 0.2f, 1.f );
	*/

	canyon_zone_init = true;
}


void canyonZone_load( const char* filename ) {
	context* c = lisp_newContext();
	term* t = lisp_eval_file( c, filename );
	// We should now have a list of zones
	term_takeRef( t );
	term* zone = t;
	int zone_count = list_length( t );
	vAssert( zone_count < kNumZones );
	int i = 0;
	while ( zone ) { 
		zones[i] = *(canyonZone*)( head( zone )->data );
		zone = zone->tail;
		++i;
	}
	// TODO - add this back and get it working
	//term_deref( t );
	canyon_zone_count = zone_count;
	
	vAssert( !canyonZone_lookup_texture );
	canyonZone_lookup_texture = canyonZone_buildTexture( zones[0], zones[1] );
}

canyonZone* canyonZone_create() {
	canyonZone* z = mem_alloc( sizeof( canyonZone ));
	memset( z, 0, sizeof( canyonZone ));
	return z;
}
