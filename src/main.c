/*
 Vitae

 A Game Engine by Nick Pollard
 (c) 2010
 */

#include "common.h"
#include "window.h"
#include "spritebuffer.h"

// ###################################
// #
// # GTK Versions
// #
// ###################################

// init - initialises the engine
void init(int argc, char** argv) {
	gtk_init(&argc, &argv);
}

// run - executes the main loop of the engine
void run(window* rootWindow) {
	// Sprite test
	sprite* s = sprite_create_from_bitmap("assets/img/test64.png");
	spritebuffer_add_sprite(rootWindow->buffer, s);
	sprite_set_x_y(s, 64, 64);

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
