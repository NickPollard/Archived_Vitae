#ifndef __SPRITE_C__
#define __SPRITE_C__

#include "canvas.h"

// GTK headers
#include <gtk/gtk.h>

typedef struct {
	int x, y;
	int width, height;
	int drawlayer;
	GdkPixbuf*	image;
} sprite;

void sprite_render_to_canvas(sprite* s, const canvas* c);

sprite* sprite_create_from_bitmap(const char* bitmapName);

void sprite_set_x_y(sprite* s, int x, int y);
#endif // __SPRITE_C__
