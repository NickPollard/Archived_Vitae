// file.h
#pragma once

// *** S-Expressions

enum tag_type {
	typeNull,
	typeList,
	typeAtom,
	typeFunc,
	typeModel,
	typeTransform
};

typedef struct sterm_s {	int		type;
							void*	head;
							struct sterm_s*	tail;	} sterm;

typedef void* (*sfunc)( sterm* s );

void sterm_free( sterm* s );


/*
// *** Load data
typedef struct transform_data_s {
	transform	_transform;
	sterm*		children;	// All children as an s-list	
} transform_data;
*/


// *** Stream Reading

typedef struct inputStream_s {
	const char* source; // Never changes once initialised
	const char* stream; // Read head
} inputStream;



// *** File Operations

FILE* vfile_open( const char* path, const char* mode );

// *** Parsing

sterm* parse( inputStream* stream );
sterm* parse_string( const char* string );
sterm* parse_file( const char* filename );

void* eval( sterm* data );



// *** Testing

void test_sfile( );
