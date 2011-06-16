/*
 Vitae

 A Game Engine by Nick Pollard
 (c) 2010
 */

#include "common.h"
#include "engine.h"
#include "mem/allocator.h"
#include "mem/pool.h"
#include "system/file.h"

// ###################################

int main(int argc, char** argv) {
	printf("Loading Vitae.\n");

	init(argc, argv);

	// Memory Tests
	test_allocator();
	test_pool();

	// System Tests
	test_sfile();

	run();

	// Exit Gracefully
	return 0;
}
