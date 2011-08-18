// file.h
#pragma once

// *** S-Expressions

enum tag_type {
	typeNull,
	typeList,
	typeAtom,
	typeFunc,
	typeModel,
	typeLight,
	typeTransform,
	typeFilename,
	typeVector
};

typedef struct sterm_s {	int		type;
							void*	head;
							struct sterm_s*	tail;	} sterm;

typedef void* (*sfunc)( sterm* s );

void sterm_free( sterm* s );



// *** Stream Reading

typedef struct inputStream_s {
	const char* source; // Never changes once initialised
	const char* stream; // Read head
	const char* end;	// One-past-the-end
} inputStream;


inputStream* inputStream_create( const char* source );
char* inputStream_nextToken( inputStream* stream );
void inputStream_skipToken( inputStream* stream );
bool inputStream_endOfFile( inputStream* in );
void inputStream_nextLine( inputStream* in );

// *** File Operations

FILE* vfile_open( const char* path, const char* mode );
void* vfile_contents(const char *path, int *length);
void vfile_writeContents( const char* path, void* buffer, int length );

// *** Parsing

sterm* parse( inputStream* stream );
sterm* parse_string( const char* string );
sterm* parse_file( const char* filename );

void* eval( sterm* data );



// *** Testing

void test_sfile( );
