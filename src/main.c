/*
 Vitae

 A Game Engine by Nick Pollard
 (c) 2010
 */

#include "common.h"
#include "window.h"
#include "spritebuffer.h"
#include "texture.h"

// ###################################
// #
// # GTK Versions
// #
// ###################################

// init - initialises the engine
void init(int argc, char** argv) {
	gtk_init(&argc, &argv);
}

void test_load_and_add_sprite(TextureLibrary* textures, spritebuffer* b, const char* file, int x, int y) {
	sprite* s = sprite_create_from_bitmap(textures, file);
	spritebuffer_add_sprite(b, s);
	sprite_set_x_y(s, x, y);
}

// run - executes the main loop of the engine
void run(window* rootWindow) {
	TextureLibrary* textures = texture_library_create();
	// Sprite test
	sprite* s = sprite_create_from_bitmap(textures, "assets/img/test64.png");
	spritebuffer_add_sprite(rootWindow->buffer, s);
	sprite_set_x_y(s, 64, 64);

	test_load_and_add_sprite(textures, rootWindow->buffer, "assets/img/test64.png", 0, 128);
	test_load_and_add_sprite(textures, rootWindow->buffer, "assets/img/test64.png", 300, 300);

	gtk_main();
}

// ###################################

int main(int argc, char** argv) {
	printf("Loading Vitae.\n");

	init(argc, argv);

	window* rootWindow = window_create(640, 480);
	printf("Window created as (%dx%d).\n", rootWindow->width, rootWindow->height);
	window_show(rootWindow);

	run(rootWindow);

	// Exit Gracefully
	return 0;
}
