// file.h
#pragma once

FILE* vfile_open( const char* path, const char* mode );

typedef struct slist_s {
	struct slist_s* tail;
	void* head;
} slist;

typedef struct inputStream_s {
	const char* source; // Never changes
	const char* stream; // Read head
} inputStream;

void test_sfile( );
