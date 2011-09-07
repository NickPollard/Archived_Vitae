/*
 Vitae

 A Game Engine by Nick Pollard
 (c) 2010
 */

#ifndef ANDROID

#include "common.h"
#include "engine.h"
#include "maths.h"
#include "particle.h"
#include "mem/allocator.h"
#include "system/file.h"
#include "system/hash.h"
#include "system/string.h"

#define TEST true


// ###################################

void test() {
	// Memory Tests
	test_allocator();

	test_hash();

	// System Tests
	test_sfile();

	test_matrix();

	test_property();

	test_string();
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

	engine_run( e );

	// Exit Gracefully
	return 0;
}
#endif // ANDROID
