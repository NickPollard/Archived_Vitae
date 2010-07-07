/*
 Vitae

 A Game Engine by Nick Pollard
 (c) 2010
 */

#include "common.h"
#include "engine.h"
#include "window.h"
#include "spritebuffer.h"
#include "texture.h"

// ###################################
// #
// # GTK Versions
// #
// ###################################


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
