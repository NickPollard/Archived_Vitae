// file.h
#pragma once

FILE* vfile_open( const char* path, const char* mode );

enum tag_type {
	typeNull,
	typeList,
	typeAtom
};

typedef struct sterm_s {
	int		type_tag;
	void*	ptr;
} sterm;

typedef struct slist_s {
	struct slist_s* tail;
//	void* head;
	sterm head;
} slist;

typedef struct inputStream_s {
	const char* source; // Never changes
	const char* stream; // Read head
} inputStream;

void test_sfile( );
