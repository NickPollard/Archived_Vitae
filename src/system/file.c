// file.c
#include "common.h"
#include "file.h"
//-----------------------
#include <assert.h>

FILE* vfile_open( const char* path, const char* mode ) {
   // *** load the ttf file
   FILE* file = fopen( path, mode );
   if ( !file ) {
	   printf( "Error loading file: \"%s\"\n", path );
	   assert( file );
   }
   return file;
}


void* vfile_contents(const char *path, int *length)
{
    FILE *f = fopen(path, "r");
    void *buffer;

    if (!f) {
        fprintf(stderr, "Unable to open %s for reading\n", path);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);

    buffer = malloc(*length+1);
    *length = fread(buffer, 1, *length, f);
    fclose(f);
    ((char*)buffer)[*length] = '\0';

    return buffer;
}

// Is the character a whitespace character?
// eg. space, tab
int isWhiteSpace( char c ) {
	return ( c == ' ' );
}

// Is the character a terminator character
// Eg. NULL-terminator or end of line
int isTerminator( char c ) {
	return ( c == '\0' || c == '\n' ); // Or EoF
}

int isListStart( char c ) {
	return c == '(';
}

int isListEnd( char c ) {
	return c == ')';
}


// Returns the next token as a c string, advances the inputstream to the token's end
const char* stream_nextToken( inputStream* stream ) {
	// Consume leading whitespace
	while ( isWhiteSpace( *stream->stream ) )
			stream->stream++;
	// measure the length of the token
	const char* ptr = stream->stream;
	while ( !isWhiteSpace( *ptr ) && !isTerminator( *ptr ))
		ptr++;
	// make a copy of it
	int length = ptr - stream->stream;
	char* token = malloc( sizeof( char ) * (length + 1) );
	strncpy( token, stream->stream, length );
	token[length] = '\0';
	stream->stream++; // Advance past the end of the token
	return token;
}

slist* slist_create() {
	slist* s = malloc( sizeof( slist ));
	memset( s, 0, sizeof( slist ));
	return s;
}

// Read a token at a time from the inputstream, advancing the read head,
// and build it into an slist of atoms
void sexpr_consume( slist* s, inputStream* in ) {
	while ( true ) {
		const char* token = stream_nextToken( in );
		// Step down a level
		if ( isListStart( *token ) ) {
			printf( "Found (\n" );
			s->head = slist_create();
			sexpr_consume( (slist*)s->head, in );
			free( (void*)token ); // discard non-atom tokens
			continue;
		}
		// Step up a level
		if ( isListEnd( *token ) ) {
			printf( "Found )\n" );
			free( (void*)token ); // discard non-atom tokens
			return;
		}
		if ( isTerminator( *token )) {
			return; // End of stream
		}
		// Add an atom
		if ( 1 /*isAtom( *token )*/ ) {
			printf( "Found atom: %s\n", token );
			if ( s->head ) {
				s->tail = slist_create();
				s = s->tail;	
			}
			s->head = (void*)token;
		}
	}
}

slist* vfile_sread( inputStream* in ) {
	slist* s = slist_create();
	s->tail = NULL; // This is the parent list of the file, so has no tail
	sexpr_consume( s, in );
	return s;
}

void inputStream_reset( inputStream* in ) {
	in->stream = in->source;
}

inputStream* inputStream_create( const char* source ) {
	inputStream* in = malloc( sizeof( inputStream ));
	memset( in, 0, sizeof( inputStream ));
	in->source = source;
	in->stream = in->source;
	return in;
}

void test_sfile( ) {
	int length = 0;
	const char* contents = vfile_contents( "dat/test.s", &length );

	// TODO can this be stack allocated? Look at more stack allocation rather than heap
	inputStream* in = inputStream_create( contents );
	slist* s = vfile_sread( in );
	(void)s;
}

/*
sexpr_read( sexpr s, inputStream* in) {
	if ( is_atom( s ))
		return atom_read( s );
	for each sexpr su: sexpr_read (su)
}
*/

