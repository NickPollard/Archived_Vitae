// file.c
#include "common.h"
#include "file.h"
//-----------------------
#include <assert.h>

//
// *** File
//

// file open wrapper that asserts on failure
FILE* vfile_open( const char* path, const char* mode ) {
   // *** load the ttf file
   FILE* file = fopen( path, mode );
   if ( !file ) {
	   printf( "Error loading file: \"%s\"\n", path );
	   assert( file );
   }
   return file;
}

// Load the entire contents of a file into a heap-allocated buffer of the same length
// returns a pointer to that buffer
// It its the caller's responsibility to free the buffer
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

//
// *** Parsing
//

// Is the character a whitespace character?
// eg. space, tab
int isWhiteSpace( char c ) {
	return ( c == ' ' || c == '\n' || c == '\t' );
}

// Is the character a terminator character
// Eg. NULL-terminator or end of line
int isTerminator( char c ) {
	return ( c == '\0' /*|| c == '\n'*/ ); // Or EoF
}

int isListStart( char c ) {
	return c == '(';
}

int isListEnd( char c ) {
	return c == ')';
}

//
// *** Stream Reading
//

void inputStream_reset( inputStream* in ) {
	in->stream = in->source;
}

inputStream* inputStream_create( const char* source ) {
	inputStream* in = malloc( sizeof( inputStream ));
	assert( in );
	in->source = source;
	in->stream = in->source;
	return in;
}

// Returns the next token as a c string, advances the inputstream to the token's end
char* inputStream_nextToken( inputStream* stream ) {
	// Consume leading whitespace
	while ( isWhiteSpace( *stream->stream ) )
			stream->stream++;
	// measure the length of the token - ptr should become one-past-the-end
	const char* ptr = stream->stream;
	while ( !isWhiteSpace( *ptr ) && !isTerminator( *ptr )) {
		if ( isListStart( *ptr ) || isListEnd( *ptr ) ) {
			if ( ( ptr - stream->stream ) == 0 ) // if parenthesis is first char, push pas it
				ptr++;	
			break;
		}
		ptr++;
	}
	// make a copy of it
	int length = ptr - stream->stream;
	char* token = malloc( sizeof( char ) * (length + 1) );
	strncpy( token, stream->stream, length );
	token[length] = '\0';
	stream->stream = ptr; // Advance past the end of the token
	return token;
}

//
// *** S-Expressions
//

/*
slist* slist_create() {
	slist* s = malloc( sizeof( slist ));
	memset( s, 0, sizeof( slist ));
	return s;
}
*/

bool isAtom( sterm* s ) {
	return ( s->type == typeAtom );
}

bool isList( sterm* s ) {
	return ( s->type == typeList );
}

sterm* sterm_create( int tag, void* ptr ) {
	sterm* term = malloc( sizeof( term ) );
	term->type = tag;
	term->head = ptr;
	term->tail = NULL;
	return term;
}

/*
// Read a token at a time from the inputstream, advancing the read head,
// and build it into an slist of atoms
void sexpr_consume( sterm* s, inputStream* stream ) {
	while ( true ) {
		const char* token = inputStream_nextToken( stream );
		// Step down a level
		if ( isListStart( *token ) ) {
//			printf( "Found (\n" );
			s->type = typeList;
			s->head = sterm_create( typeNull, NULL );
			sexpr_consume( s->head, stream );
			free( (void*)token ); // discard non-atom tokens
			continue;
		}
		// Step up a level
		if ( isListEnd( *token ) ) {
//			printf( "Found )\n" );
			free( (void*)token ); // discard non-atom tokens
			return;
		}
		if ( isTerminator( *token )) {
			return; // End of stream
		}
		// Add an atom
		if ( 1  ) {
//			printf( "Found atom: %s\n", token );
			if ( s->head ) {
				s->tail = sterm_create( typeList, NULL );
				s = s->tail;	
			}
			s->head = sterm_create( typeAtom, (void*)token );
			return;
		}
	}
}
*/

// Read a token at a time from the inputstream, advancing the read head,
// and build it into an slist of atoms
sterm* consume( inputStream* stream ) {
	char* token = inputStream_nextToken( stream );
	if ( isListEnd( *token ) ) {
		free( token ); // It's a bracket, discard it
		return NULL;
	}
	if ( isListStart( *token ) ) {
		free( token ); // It's a bracket, discard it
		printf( "List: (\n" );
		sterm* list = sterm_create( typeList, NULL );
		sterm* s = list;

		s->head = consume( stream );
		if ( !s->head ) { // The Empty list () 
			printf( "List: )\n" );
			return list;
		}

		while ( true ) {
			sterm* sub_expr = consume( stream );					// Consume a subexpr
			if ( sub_expr ) {									// If a valid return
				s->tail = sterm_create( typeList, NULL );	// Add it to the tail
				s = s->tail;
				s->head = sub_expr;
			} else {
				printf( "List: )\n" );
				return list;
			}
		}
	}
	printf( "Atom: %s\n", token );
	// When it's an atom, we keep the token, don't free it
	return sterm_create( typeAtom, (void*)token );
}

/*
sexpr_read( sexpr s, inputStream* in) {
	if ( is_atom( s ))
		return atom_read( s );
	for each sexpr su: sexpr_read (su)
}
*/
#if 1

// Load a scene from a .sc file (s-expression based)
/*
scene* scene_load( slist* data ) {
	scene* s = scene_create();
	process( data );
}
*/

void* s_print( sterm* s );
void* s_concat( sterm* s );

bool strEq( const char* a, const char* b ) {
	return ( strcmp( a, b ) == 0 );
}

// TODO PLACEHOLDER
void* lookup( const char* data ) {
	if ( strEq( data, "print" ) ) {
		sterm* print = sterm_create( typeFunc, s_print );
		return print;
	}
	if ( strEq( data, "concat" ) ) {
		sterm* concat = sterm_create( typeFunc, s_concat );
		return concat;
	}
	return (void*)data;
//	printf( "Lookup invalid.\n" );
//	assert( 0 );
//	return NULL;
}

bool isFunction( sterm* s ) {
	return s->type == typeFunc;
}

const char* getAtom( sterm* term ) {
	assert( term->type = typeAtom );
	return (const char*)term->head;
}

void* eval( sterm* data ) {
	if ( isAtom( data ) ) {
		// It's either a function, a string, or a number
		return lookup( (void*)getAtom( data ) );
	}
	else if ( isList( data ) ) {
		// If evaluating a list, the head must eval to an atom
		// That atom must be a function?
		sterm* func = eval( data->head );
		assert( isFunction( func ) );
		return ((sfunc)func->head)( data->tail );
	}
	else {
		printf( "Unrecognised Sterm type: %d.\n", data->type );
		assert( 0 );
	}
	return NULL;
}

void* s_concat( sterm* s ) {
	int size = 0;
	char* string = NULL;
	while( s ) {
		const char* text = (const char*)((sterm*)s->head)->head;
		int extra = strlen( text );
		char* tmp = malloc( sizeof( char ) * ( size + extra + 1 ) );
		strncpy( tmp, string, size );
		strncpy( tmp + size, text, extra );
		free( string );
		string = tmp;
		size += extra;
		string[size] = '\0';
		s = s->tail;
	}
	return string;
}

void* s_print( sterm* s ) {
	printf( "> s_print(): " );
	while( s ) {
		const char* string = eval( s->head );
		printf( "%s", string );
		s = s->tail;
	}
	printf( "\n" );
	return NULL;
}

void test_sfile( ) {
	int length = 0;
	char* contents = vfile_contents( "dat/test2.s", &length );

	// TODO can this be stack allocated? Look at more stack allocation rather than heap
	inputStream* in = inputStream_create( contents );
	sterm* s = consume( in );
	(void)s;

	eval( s );

	free( contents );
}
/*
// Process a vector
// Expects a list of floats
void* vector_process( sterm* s ) {
	vector v = vector_create();
	v[3] = 0.f;
	int i = 0;
	while( s && i < 4 ) {
		sterm result = eval( s->head );
		assert( asFloat( result ) );
		v[i] = asFloat( result );
		i++;
		s = s->tail;
	}
	assert( i > 2 );
	return v;
}
*/

/*
// Evaluate a list of sterms
void* eval( slist* data ) {
	if ( isAtom( data->head ) ) {
//		return lookup( data->head.ptr );
	}
	else if ( isList( data->head )) {
		return eval( data->head->head )( data->head->tail );
	}
}
// Create the objects indicated by a data file
// The file should have only one root
void load( slist* data ) {
	// find the function defined by the head
	// Call it with the data defined by the tail
	eval( data );
}

void file_load( const char* filename ) {
	int length = 0;
	const char* contents = vfile_contents( filename, &length );
	inputStream* in = inputStream_create( contents );
	slist* s = sexpr_readFile( in );
	load( s );
}


void scene_parse( scene* s, slist* data ) {
	


	if slist->head isToken
	{
		token = slist->head
		if ( token == 'object' ) {
			object obj = object_create();
			object_parse( obj, slist->tail );
		}
	}	
}

// called once for each argument to scene
void scene_process( void* context, slist* data ) {
	
//( scene ( object ( model "cube.obj" )
//				 ( position ( vector 0.0 0.0 0.0 ))))

//		create a scene, scene->parse( subtree)
//		create an object, object->parase(subtree);
}
*/
#endif
