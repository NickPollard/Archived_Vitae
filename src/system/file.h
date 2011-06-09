// file.h
#pragma once

// *** S-Expressions

enum tag_type {
	typeNull,
	typeList,
	typeAtom,
	typeFunc,
	typeModel
};

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
