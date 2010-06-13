#include "common.h"
#include "spritebuffer.h"

#include "canvas.h"

// spritebuffer_render_to_canvas - renders all sprites in the buffer to the canvas
// TAKES spritebuffer to use, canvas to render to
void spritebuffer_render_to_canvas(spritebuffer* s, canvas* c) {
	printf("Rendering %d sprites.\n", s->count);
	for (int i = 0; i < s->count; i++) {
		if (s->sprites[i])
			sprite_render_to_canvas(s->sprites[i], c);
	}
}

// spritebuffer_add_sprite - adds a sprite to the spritebuffer
// TAKES pointer to spritebuffer, pointer to sprite
// YIELDS id of sprite added
int spritebuffer_add_sprite(spritebuffer* buffer, sprite* s) {
	int id = -1;
	for (int i = 0; i < buffer->max; i++) {
		if (!buffer->sprites[i]) {
			id = i;
			break;
		}
	}
	if (id != -1) {
		buffer->sprites[id] = s;
		buffer->count++;
	}
	else {
		fprintf(stderr, "Tried to add sprite to spritebuffer but buffer is full.\n");
	}
	return id;
}

// spritebuffer_create - creates a new spritebuffer, allocates memory for it
// TAKES requested max size for the sprite array
spritebuffer* spritebuffer_create(int request_size) {
	// Allocate enough size for the spritebuffer struct plus the sprite array
	void* p = malloc(sizeof(spritebuffer) + sizeof(sprite*) * request_size);
	spritebuffer* s = (spritebuffer*)p;
	s->count = 0;
	s->max = request_size;
	// Struct and array are contiguous in memory;
	s->sprites = (sprite**)&s[1];
	// Zero out the pointers array
	memset(s->sprites, 0, sizeof(sprite*) * request_size);
	return s;
}
