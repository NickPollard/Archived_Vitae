#ifndef __SPRITE_C__
#define __SPRITE_C__

#include "canvas.h"
#include "texture.h"

// GTK headers
#include <gtk/gtk.h>

typedef struct {
	int x, y;
	int width, height;
	int drawlayer;
	TextureHandle	image;
} sprite;

void sprite_render_to_canvas(sprite* s, canvas* c);

sprite* sprite_create_from_bitmap(TextureLibrary* lib, const char* bitmapName);

void sprite_set_x_y(sprite* s, int x, int y);

//
// SpriteMover
//

typedef struct spritemover_t {
	sprite* s;
	vec2	position;
	vec2	velocity;
} spritemover;

spritemover* spritemover_create(sprite* spr, float x, float y, float vx, float vy);

void spritemover_tick(void* mover, float dt);

#endif // __SPRITE_C__
