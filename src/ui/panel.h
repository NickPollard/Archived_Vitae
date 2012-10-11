// panel.h
#pragma once
#include "render/render.h"

enum position_mode {
	kFlow,
	kAbsolute,
	kRelative
};

enum anchor {
	kTopLeft = 0x0,
	kTopRight = 0x1,
	kBottomLeft = 0x2,
	kBottomRight = 0x3
};

typedef struct panel_s panel;

struct panel_s {
	panel*	parent;
	unsigned int position_mode; // How the panel should be drawn
	float x;
	float y;
	float width;
	float height;
	unsigned int local_anchor;
	unsigned int remote_anchor;
	vector color;
	bool visible;

	// Temp
	GLuint	texture;
	vertex vertex_buffer[4];
};

// Create a Panel
panel* panel_create();

// Show/Hide the panel
void panel_hide( engine* e, panel* p );
void panel_show( engine* e, panel* p );

// The draw function gets passed the current 'cursor' x and y
void panel_draw( panel* p, float x, float y );
void panel_render( void* panel_ );
