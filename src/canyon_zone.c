// Canyon_zone.c
#include "common.h"
#include "canyon_zone.h"
//-------------------------
#include "maths/maths.h"
#include "mem/allocator.h"
#include "render/texture.h"

bool canyon_zone_init = false;

texture* canyonZone_lookup_texture = NULL;

// What Zone are we in at a given distance V down the canyon?
int canyon_zone( float v ) {
	vAssert( canyon_zone_init );
	return (int)floorf( v / kZoneLength );
}

int canyon_zoneType( float v ) {
	return canyon_zone( v ) % kNumZones;
}


// What color is the standard terrain for ZONE?
vector canyonZone_terrainColor( int zone ) {
	vAssert( canyon_zone_init );
	return zones[zone % kNumZones].terrain_color;
}

vector canyonZone_cliffColor( int zone ) {
	vAssert( canyon_zone_init );
	return zones[zone % kNumZones].cliff_color;
}

vector canyonZone_edgeColor( int zone ) {
	vAssert( canyon_zone_init );
	return zones[zone % kNumZones].edge_color;
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

void canyonZone_staticInit() {
	// Ice
	zones[0].terrain_color = Vector( 0.8f, 0.9f, 1.0f, 1.f );
	zones[0].cliff_color = Vector( 0.2f, 0.3f, 0.6f, 1.f );
	zones[0].edge_color = Vector( 0.4f, 0.55f, 0.8f, 1.f );

	// Wasteland	
	zones[1].terrain_color = Vector( 0.7f, 0.3f, 0.2f, 1.f );
	zones[1].cliff_color = Vector( 0.3f, 0.1f, 0.1f, 1.f );
	zones[1].edge_color = Vector( 0.6f, 0.2f, 0.2f, 1.f );

	canyonZone_lookup_texture = canyonZone_buildTexture( zones[0], zones[1] );

	canyon_zone_init = true;
}
