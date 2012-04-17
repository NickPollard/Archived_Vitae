// dynamicfog.h
#pragma once
#include "src/maths.h"

#define kNumFogs 4

typedef struct dynamicFog_s {
	scene* scene;
	float time;
	vector fog_colors[kNumFogs];
} dynamicFog;

dynamicFog* dynamicFog_create();
void dynamicFog_tick( void* v, float dt );
