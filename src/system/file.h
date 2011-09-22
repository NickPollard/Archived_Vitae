// file.h
#pragma once


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

// *** File Operations

FILE* vfile_open( const char* path, const char* mode );
void* vfile_contents(const char *path, int *length);
void vfile_writeContents( const char* path, void* buffer, int length );




// *** Testing

void test_sfile( );
