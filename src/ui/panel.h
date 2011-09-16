// panel.h
#pragma once

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
};
