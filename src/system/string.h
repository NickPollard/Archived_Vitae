// string.h

#pragma once

const char* string_createCopy( const char* src );
bool string_equal( const char* a, const char* b );
const char* string_trim( const char* src );

bool charset_contains( const char* charset, char c );

void test_string( );
