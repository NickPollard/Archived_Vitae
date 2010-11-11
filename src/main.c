/*
 Vitae

 A Game Engine by Nick Pollard
 (c) 2010
 */

#include "common.h"
#include "engine.h"
#include "mem/allocator.h"

// ###################################

int main(int argc, char** argv) {
	printf("Loading Vitae.\n");

	init(argc, argv);

	test_allocator();

	run();

	// Exit Gracefully
	return 0;
}
