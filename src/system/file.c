// file.c
#include "common.h"
#include "file.h"
// TEMP
#include "model.h"
#include "scene.h"
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
	// parse leading whitespace
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

bool isModel( sterm* s ) {
	return ( s->type == typeModel );
}

sterm* sterm_create( int tag, void* ptr ) {
	sterm* term = malloc( sizeof( term ) );
	term->type = tag;
	term->head = ptr;
	term->tail = NULL;
	return term;
}

// Read a token at a time from the inputstream, advancing the read head,
// and build it into an slist of atoms
sterm* parse( inputStream* stream ) {
	char* token = inputStream_nextToken( stream );
	if ( isListEnd( *token ) ) {
		free( token ); // It's a bracket, discard it
		return NULL;
	}
	if ( isListStart( *token ) ) {
		free( token ); // It's a bracket, discard it
		sterm* list = sterm_create( typeList, NULL );
		sterm* s = list;

		s->head = parse( stream );
		if ( !s->head ) { // The Empty list () 
			return list;
		}

		while ( true ) {
			sterm* sub_expr = parse( stream );					// parse a subexpr
			if ( sub_expr ) {									// If a valid return
				s->tail = sterm_create( typeList, NULL );	// Add it to the tail
				s = s->tail;
				s->head = sub_expr;
			} else {
				return list;
			}
		}
	}
	// When it's an atom, we keep the token, don't free it
	return sterm_create( typeAtom, (void*)token );
}

sterm* parse_string( const char* string ) {
	inputStream* stream = inputStream_create( string );
	sterm* s = parse( stream );
	free( stream );
	return s;
}

sterm* parse_file( const char* filename ) {
	int length = 0;
	char* contents = vfile_contents( filename, &length );
	assert( contents );
	assert( length != 0 );
	sterm* s = parse_string( contents );
	free( contents );
	return s;
}

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
void* s_model( sterm* s );
void* s_scene( sterm* s );

bool strEq( const char* a, const char* b ) {
	return ( strcmp( a, b ) == 0 );
}

#define S_FUNC( atom, func )	if ( strEq( data, atom ) ) { \
									sterm* s = sterm_create( typeFunc, func ); \
									return s; \
								}

// TODO PLACEHOLDER
void* lookup( const char* data ) {
	S_FUNC( "print", s_print )
	S_FUNC( "concat", s_concat )
	S_FUNC( "model", s_model )
	S_FUNC( "scene", s_scene )
	return (void*)data;
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

void sterm_free( sterm* s ) {
	if ( isAtom( s ) ) {
		free( s->head );
		free( s );
	}
	if ( isList( s ) ) {
		if ( s->tail )
			sterm_free( s->tail );
		if ( s->head )
			sterm_free( s->head );
		free( s );
	}
}

void* s_concat( sterm* s ) {
	int size = 0;
	char* string = NULL;
	while( s ) {
		const char* text = eval( s->head );
		//const char* text = (const char*)((sterm*)s->head)->head;
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

// S is a list of model attributes
void* s_model( sterm* s ) {
	printf( "Model\n" );
	while( s ) {
//		attribute a = eval( s->head );
		s = s->tail;
	}
	model* m = model_createTestCube();
	sterm* model_term = sterm_create( typeModel, m );
	return model_term;
}

void* s_scene( sterm* s ) {
	printf( "Scene\n" );
	scene* _scene = scene_create();
	while( s ) {
		sterm* e = eval( s->head );
		if ( isModel( e ) ) {
			model* _model = (model*)e->head;
			scene_addModel( _scene, _model );
		}
		s = s->tail;
	}
	return _scene;
}

// *** Testing

// Tests s_concat, eval, parse, inputStream
void test_s_concat() {
	sterm* s = parse_string( "(concat Hello World)" );
	char* result = eval( s );

	assert( strcmp( result, "HelloWorld" ) == 0 );
	assert( strcmp( result, "HellooWorld" ) != 0 );

	sterm_free( s );
	return;
}

// Tests s_scene, s_model, eval, parse, inputStream
void test_s_scene() {
	sterm* s = parse_string( "(scene (model))" );
	scene* _scene = eval( s );
	(void)_scene;

	sterm_free( s );
	return;
}

void test_sfile( ) {
	sterm* s = parse_file( "dat/test2.s" );
	eval( s );
	sterm_free( s );

	test_s_concat();
	test_s_scene();
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
