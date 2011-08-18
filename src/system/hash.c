// hash.c
#include "common.h"
#include "hash.h"
//-----------------------

unsigned int mhash( const char* src ) {
	unsigned int seed = 0x0;
	return MurmurHash2( src, strlen( src ), seed );
}

void test_murmurHash( const char* source ) {
	printf( "Hash of: \"%s\" is 0x%x.\n", source, mhash( source ));
}

void test_hash() {
	test_murmurHash( "test" );
	test_murmurHash( "blarg" );
	test_murmurHash( "scrund" );
	test_murmurHash( "people" );
	test_murmurHash( "simples" );
}
