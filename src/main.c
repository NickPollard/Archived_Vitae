/*
 Vitae

 A Game Engine by Nick Pollard
 (c) 2010
 */

#include "common.h"
#include "engine.h"
#include "maths.h"
#include "mem/allocator.h"
#include "system/file.h"

#define TEST true

// ###################################

void test() {
	// Memory Tests
	test_allocator();

	// System Tests
	test_sfile();

	test_matrix();
}

// ###################################

int main(int argc, char** argv) {
	printf("Loading Vitae.\n");

	init(argc, argv);

	// *** Initialise Engine
	engine* e = engine_create();
	engine_init( e, argc, argv );
	static_engine_hack = e;

#if TEST
	test();
#endif

//	run();
	engine_run( e );

	// Exit Gracefully
	return 0;
}
