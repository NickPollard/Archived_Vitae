// string.h

#pragma once

const char* string_createCopy( const char* src );
bool string_equal( const char* a, const char* b );
const char* string_trim( const char* src );

bool charset_contains( const char* charset, char c );

typedef struct streamWriter_s {
	char* string;
	char* write_head;
	char* end;
} streamWriter;

streamWriter* streamWriter_create( char* buffer, char* end );
void stream_printf( streamWriter* stream, const char* fmt, ... );

void test_string( );
