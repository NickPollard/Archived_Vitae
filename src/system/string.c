// string.c

#include "common.h"
#include "string.h"
//-----------------------
#include "maths.h"
#include "mem/allocator.h"
#include <stdarg.h>
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

#define kMaxStreamBufferChars 256

streamWriter* streamWriter_create( char* buffer, char* end ) {
	streamWriter* s = mem_alloc( sizeof( streamWriter ));
	s->write_head =	s->string = buffer;
	s->end = end;
	return s;
}

void stream_printf( streamWriter* stream, const char* fmt, ... ) {
	va_list args;
	va_start( args, fmt );
	char buffer[kMaxStreamBufferChars];
	vsnprintf( buffer, kMaxStreamBufferChars, fmt, args );
	buffer[kMaxStreamBufferChars-1] = '\0'; // Ensure Null-terminated
	// snprintf caps at n characters, and will make the last of those a \0
	// So we want to include the \0 from buffer in what we write, so that
	// we don't overwrite any characters we want to keep
	size_t length = strlen( buffer ) + 1;	
	vAssert(( stream->write_head + length ) < stream->end );
	length = min( length, stream->end - stream->write_head );
	snprintf( stream->write_head, length, "%s", buffer );
	//printf( "Stream writer: %d chars, \"%s\"\n", length, buffer );
	//printf( "Stream: \"%s\"\n", stream->string );
	// We only move the write_head along to the \0 we wrote, so that we will
	// overwrite it if we append more to the stream
	stream->write_head += length - 1;
	va_end( args );
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
