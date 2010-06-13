#ifndef __SPRITEBUFFER_C__
#define __SPRITEBUFFER_C__

// GTK headers
#include <gtk/gtk.h>

#include "sprite.h"

typedef struct {
	int count;
	int max;
	sprite** sprites;
} spritebuffer;

// spritebuffer_render_to_canvas - renders all sprites in the buffer to the canvas
// TAKES spritebuffer to use, canvas to render to
void spritebuffer_render_to_canvas(spritebuffer* s, canvas* c);

// spritebuffer_add_sprite - adds a sprite to the spritebuffer
// TAKES pointer to spritebuffer, pointer to sprite
// YIELDS id of sprite added
int spritebuffer_add_sprite(spritebuffer* buffer, sprite* s);

// spritebuffer_create - creates a new spritebuffer, allocates memory for it
// TAKES requested max size for the sprite array
spritebuffer* spritebuffer_create(int request_size);

#endif // __SPRITEBUFFER_C__
