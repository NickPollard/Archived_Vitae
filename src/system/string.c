// string.c

#include "common.h"
#include "string.h"
//-----------------------
#include "mem/allocator.h"
#include <strings.h>

// Allocates and copies a standard null-terminated c string, then returns the new copy
const char* string_createCopy( const char* src ) {
	int length = strlen( src );
	char* dst = mem_alloc( sizeof( char ) * (length + 1));
	memcpy( dst, src, length );
	dst[length] = '\0';
	return dst;
}

bool string_equal( const char* a, const char* b ) {
	return ( strcmp( a, b ) == 0 );
}

bool charset_contains( const char* charset, char c ) {
	return ( index( charset, c ) != NULL );
}

// Trim cruft from a string to return just the main alpha-numeric name
// Eg. remove leading/trailing whitespace, punctuation
const char* string_trim( const char* src ) {
	const char* begin = src;
	const char* end = src + strlen( src ) - 1;

	while ( charset_contains( ";", *begin ) && begin <= end ) {
		begin++;
	}

	while ( charset_contains ( ";", *end ) && begin <= end) {
		end--;
	}
	int length = end + 1 - begin;
	assert( length >= 0 );
	if ( length == 0 )
		return NULL;
	char* dst = mem_alloc( sizeof( char ) * (length + 1));
	memcpy( dst, begin, sizeof( char ) * (length + 1));
	dst[length] = '\0';
	return dst;
}

void test_string_trim( ) {
	const char* src = ";test;";
	const char* dst = string_trim( src );
	assert( string_equal( dst, "test" ));
	assert( string_trim( ";;;" ) == NULL );
	assert( string_equal( string_trim( ";;t;;;" ), "t" ));
}

void test_string() {
	assert( charset_contains( "test", 't' ));
	assert( charset_contains( "test", 'e' ));
	assert( charset_contains( "test", 's' ));
	assert( !charset_contains( "test", 'p' ));

	test_string_trim();
}
