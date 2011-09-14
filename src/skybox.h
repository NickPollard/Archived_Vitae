// skybox.h
#pragma once
#include "maths.h"

#define SKYBOX_VERTEX_ATTRIBS( f ) \
	f( position ) \
	f( uv )

typedef struct skybox_vertex_s {
	vector	position;
	vector	uv;
} skybox_vertex;

// Initialise static data for the skybox system
void skybox_init( );

// Render the skybox
void skybox_render( void* data );
