// dynamicfog.c
#include "common.h"
#include "dynamicfog.h"
//-----------------------
#include "scene.h"
#include "maths/vector.h"
#include "mem/allocator.h"

dynamicFog* dynamicFog_create() {
	dynamicFog* f = mem_alloc( sizeof( dynamicFog ));
	memset( f, 0, sizeof( dynamicFog ));
	f->scene = NULL;
	f->time = 0.f;
	f->fog_count = 3;
	f->fog_colors[0] = Vector( 1.f, 0.4f, 0.f, 1.f );
	f->fog_colors[1] = Vector( 0.8f, 0.2f, 0.3f, 1.f );
	f->fog_colors[2] = Vector( 0.8f, 0.8f, 1.f, 1.f );
	f->fog_colors[3] = Vector( 1.f, 1.f, 1.f, 1.f );

	f->sky_colors[0] = Vector( 0.f, 0.6f, 1.f, 1.f );
	f->sky_colors[1] = Vector( 0.05f, 0.f, 0.3f, 1.f );
	f->sky_colors[2] = Vector( 0.7f, 0.6f, 0.5f, 1.f );
	return f;
}

// the dynamicFog tick function implementation
void dynamicFog_tick( void* v, float dt, engine* eng ) {
	(void)eng;
	(void)dt;
	(void)v;
	/*
	dynamicFog* fog = v;
	fog->time += dt;
	const float timeCycle = 120.f;
	while ( fog->time > timeCycle )
		fog->time -= timeCycle;

	// Get a value between 0 and 1;
	float f = fog->time / timeCycle;
	f = f * (float)fog->fog_count; // mul up per number of fogs;
	int old = (int)floorf( f ) % fog->fog_count;
	int new = (old + 1) % fog->fog_count;

	float blend = f - floorf( f );
	dynamicFog_blend( fog, old, new, blend );
	*/
}

// blend the sky and fog to the correct interpolated value
void dynamicFog_blend( dynamicFog* fog, int previous, int next, float blend ) {
	printf( "Fog blend: %.2f\n", blend );
	vector fog_color = vector_lerp( &fog->fog_colors[previous], &fog->fog_colors[next], blend );
	vector sky_color = vector_lerp( &fog->sky_colors[previous], &fog->sky_colors[next], blend );
	scene_setFogColor( fog->scene, &fog_color );
	scene_setSkyColor( fog->scene, &sky_color );
}
