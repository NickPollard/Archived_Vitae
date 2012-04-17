// dynamicfog.c
#include "common.h"
#include "dynamicfog.h"
//-----------------------
#include "src/mem/allocator.h"
#include "src/scene.h"

dynamicFog* dynamicFog_create() {
	dynamicFog* f = mem_alloc( sizeof( dynamicFog ));
	memset( f, 0, sizeof( dynamicFog ));
	f->scene = NULL;
	f->time = 0.f;
	f->fog_colors[0] = Vector( 1.f, 0.f, 0.f, 1.f );
	f->fog_colors[1] = Vector( 0.f, 1.f, 0.f, 1.f );
	f->fog_colors[2] = Vector( 0.f, 0.f, 1.f, 1.f );
	f->fog_colors[3] = Vector( 1.f, 1.f, 1.f, 1.f );
	return f;
}

void dynamicFog_tick( void* v, float dt ) {
	dynamicFog* fog = v;
	fog->time += dt;
	const float timeCycle = 60.f;
	while ( fog->time > timeCycle )
		fog->time -= timeCycle;

	// Get a value between 0 and 1;
	float f = fog->time / timeCycle;
	f = f * (float)kNumFogs; // mul up per number of fogs;
	int old = (int)floorf( f ) % kNumFogs;
	int new = (old + 1) % kNumFogs;

	float blend = f - floorf( f );
	vector fog_color = vector_lerp( &fog->fog_colors[old], &fog->fog_colors[new], blend );
	scene_setFogColor( fog->scene, &fog_color );
}
