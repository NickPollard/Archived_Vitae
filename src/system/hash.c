// hash.c
#include "common.h"
#include "hash.h"
//-----------------------
#include "mem/allocator.h"

unsigned int mhash( const char* src ) {
	unsigned int seed = 0x0;
	return MurmurHash2( src, strlen( src ), seed );
}
/*
void* hashtable_findOrAdd( hashtable, key ) {

}

void* hashtable_find( hashtable, key ) {
	// NYI
	assert( 0 );
	// find the key in the hashtable
	// If it's not there, return null
	return NULL;
}
*/

map* map_create( int max, int stride ) {
	printf( "map_create\n" );
	map* m = mem_alloc( sizeof( map ));
	m->max = max;
	m->count = 0;
	m->stride = stride;
	m->keys = mem_alloc( sizeof( int ) * max );
	m->values = mem_alloc( stride * max );
	memset( m->keys, 0, sizeof( int ) * max );
	memset( m->values, 0, stride * max );
	return m;
}

void* map_find( map* m, int key ) {
	printf( "map_find\n" );
	for ( int i = 0; i < m->count; i++ ) {
		if ( m->keys[i] == key )
			return m->values + (i * m->stride);
	}
	return NULL;
}

void map_add( map* m, int key, void* value ) {
	printf( "map_add\n" );
	assert( m->count < m->max );
	int i = m->count;
	m->keys[i] = key;
	memcpy( m->values + (i * m->stride), value, m->stride );
	m->count++;
}

// *** Test

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
