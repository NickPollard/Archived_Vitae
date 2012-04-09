/*
 Vitae

 A Game Engine by Nick Pollard
 (c) 2010
 */

#ifndef ANDROID

#include "common.h"
#include "collision.h"
#include "engine.h"
#include "maths.h"
#include "particle.h"
#include "terrain.h"
#include "mem/allocator.h"
#include "render/modelinstance.h"
#include "system/file.h"
#include "system/hash.h"
#include "system/string.h"

#define TEST true

void test_lisp();

// ###################################

void test() {
	// Memory Tests
	test_allocator();

	test_hash();

	// System Tests
	test_sfile();
	
	test_lisp();

	test_matrix();

	test_property();

	test_string();

	test_terrain();

	test_aabb_calculate();

	test_collision();
}

// ###################################

int main(int argc, char** argv) {
	printf("Loading Vitae.\n");

	init(argc, argv);

	// *** Initialise Engine
	engine* e = engine_create();
	engine_init( e, argc, argv );

#if TEST
	test();
#endif

	engine_run( e );

	// Exit Gracefully
	return 0;
}
#endif // ANDROID
