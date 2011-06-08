// file.h
#pragma once

// *** S-Expressions

enum tag_type {
	typeNull,
	typeList,
	typeAtom,
	typeFunc
};
/*
typedef struct sterm_s {
	int		type;
	void*	ptr;
} sterm;

typedef struct slist_s {
	struct slist_s* tail;
	sterm*	head;
} slist;
*/

typedef struct sterm_s {	int		type;
							void*	head;
							struct sterm_s*	tail;	} sterm;

typedef void* (*sfunc)( sterm* s );



// *** Stream Reading

typedef struct inputStream_s {
	const char* source; // Never changes once initialised
	const char* stream; // Read head
} inputStream;



// *** File Operations

FILE* vfile_open( const char* path, const char* mode );



// *** Testing

void test_sfile( );
