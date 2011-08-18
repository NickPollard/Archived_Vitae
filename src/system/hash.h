// hash.h
#pragma once

unsigned int mhash( const char* src );
unsigned int MurmurHash2 ( const void * key, int len, unsigned int seed );

void test_murmurHash( const char* source );
void test_hash();
