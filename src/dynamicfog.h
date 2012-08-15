// dynamicfog.h
#pragma once
#include "src/maths/maths.h"

#define kNumFogs 4

typedef struct dynamicFog_s {
	scene* scene;
	float time;
	int fog_count;
	vector fog_colors[kNumFogs];
	vector sky_colors[kNumFogs];
} dynamicFog;

// Create a new default dynamicFog
dynamicFog* dynamicFog_create();

// the dynamicFog tick function implementation
void dynamicFog_tick( void* v, float dt, engine* eng );
