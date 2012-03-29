// file.h
#pragma once

extern heapAllocator* global_string_heap;

// *** Stream Reading

typedef struct inputStream_s {
	const char* source; // Never changes once initialised
	const char* stream; // Read head
	const char* end;	// One-past-the-end
} inputStream;


inputStream*	inputStream_create( const char* source );
char*	inputStream_nextToken( inputStream* stream );
void	inputStream_skipToken( inputStream* stream );
bool	inputStream_endOfFile( inputStream* in );
void	inputStream_nextLine( inputStream* in );
void	inputStream_skipPast( inputStream* stream, const char* string );

bool token_isFloat( const char* token );
bool token_isString( const char* token );
const char* sstring_create( const char* token );

int isNewLine( char c );

// *** File Operations

FILE* vfile_open( const char* path, const char* mode );
void* vfile_contents(const char *path, size_t *length);
void vfile_writeContents( const char* path, void* buffer, int length );




// *** Testing

void test_sfile( );
