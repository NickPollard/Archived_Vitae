// ribbon.h
#pragma once
#include "maths/vector.h"

#define kMaxRibbonPairs 32

typedef struct ribbonEmitter_s { 
	transform*	trans;
	vector	begin;
	vector	end;
	vector	vertex_array[kMaxRibbonPairs][2];
	int		vertex_count;
	int		vertex_first;
} ribbonEmitter;

ribbonEmitter* ribbonEmitter_create();
void ribbonEmitter_tick( void* emitter_void, float dt, engine* eng );
void ribbonEmitter_render( void* emitter_void );
