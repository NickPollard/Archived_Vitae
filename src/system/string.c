// string.c

#include "common.h"
#include "string.h"
//-----------------------
#include "mem/allocator.h"

// Allocates and copies a standard null-terminated c string, then returns the new copy
const char* string_createCopy( const char* src ) {
	int length = strlen( src );
	char* dst = mem_alloc( sizeof( char ) * (length + 1));
	memcpy( dst, src, length+1 );
	return dst;
}

bool string_equal( const char* a, const char* b ) {
	return ( strcmp( a, b ) == 0 );
}
