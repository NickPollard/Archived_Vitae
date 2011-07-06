// file.c
#include "common.h"
#include "file.h"
//-----------------------
#include "system/string.h"
#include "render/modelinstance.h"
#include <assert.h>
// TEMP
#include "light.h"
#include "maths.h"
#include "model.h"
#include "render/modelinstance.h"
#include "scene.h"

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
void* vfile_contents( const char* path, int* length ) {
    FILE *f = fopen( path, "r" );
    void *buffer;

    if (!f) {
        fprintf(stderr, "vfile: Unable to open %s for reading\n", path);
		assert( 0 );
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    *length = ftell(f);
    fseek(f, 0, SEEK_SET);

    buffer = mem_alloc(*length+1);
    *length = fread(buffer, 1, *length, f);
    fclose(f);
    ((char*)buffer)[*length] = '\0'; // TODO should I be doing this? Distinction between binary and ASCII?

    return buffer;
}

void vfile_writeContents( const char* path, void* buffer, int length ) {
	FILE *f = fopen( path, "w+" );

    if ( !f ) {
        fprintf(stderr, "vfile: Unable to open %s for writing\n", path);
		assert( 0 );
        return;
    }

	fwrite( buffer, 1, length, f );
	fclose( f );
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
	in->end = in->source + strlen( in->source );
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

// Check whether we have reached our end pointer
bool inputStream_endOfFile( inputStream* in ) {
	return in->stream >= in->end;
}


void inputStream_advancePast( inputStream* in, char c ) {
	while ( *in->stream != c && !inputStream_endOfFile( in ))
		in->stream++;
}
// Advance to just past the next end-of-line
void inputStream_nextLine( inputStream* in ) {
	inputStream_advancePast( in, '\n' );
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

bool isPropertyType( sterm* s, const char* propertyName ) {
	return ( s->head &&
		   ((sterm*)s->head)->type == typeAtom &&
		    string_equal( ((sterm*)s->head)->head, propertyName ));
}

bool isLight( sterm* s ) {
	return isPropertyType( s, "lightData" );
}

bool isTransform( sterm* s ) {
	return ( s->type == typeTransform );
}


bool isTranslation( sterm* s ) {
	return isPropertyType( s, "translation" );
}

bool isDiffuse( sterm* s ) {
	return ( s->head &&
		   ((sterm*)s->head)->type == typeAtom &&
		    string_equal( ((sterm*)s->head)->head, "diffuse" ));
}

bool isVector( sterm* s ) {
	return ( s->type == typeVector );
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
	mem_free( contents );
	return s;
}

#if 1

// Load a scene from a .sc file (s-expression based)

void* s_print( sterm* s );
void* s_concat( sterm* s );
void* s_model( sterm* s );
void* s_light( sterm* s );
void* s_transform( sterm* s );
void* s_scene( sterm* s );
void* s_translation( sterm* s );
void* s_diffuse( sterm* s );
void* s_filename( sterm* s );
void* s_vector( sterm* s );

#define S_FUNC( atom, func )	if ( string_equal( (const char*)data->head, atom ) ) { \
									sterm* s = sterm_create( typeFunc, func ); \
									return s; \
								}

// TODO PLACEHOLDER
// ( should be a hash lookup or similar )
void* lookup( sterm* data ) {
	S_FUNC( "print", s_print )
	S_FUNC( "concat", s_concat )
	S_FUNC( "model", s_model )
	S_FUNC( "light", s_light )
	S_FUNC( "transform", s_transform )
	S_FUNC( "scene", s_scene )
	S_FUNC( "translation", s_translation )
	S_FUNC( "diffuse", s_diffuse )
	S_FUNC( "vector", s_vector )
	S_FUNC( "filename", s_filename )

	return data;
}

bool isFunction( sterm* s ) {
	return s->type == typeFunc;
}

const char* getAtom( sterm* term ) {
	assert( term->type = typeAtom );
	return (const char*)term->head;
}

void debug_sterm_print( sterm* term ) {
	if ( isList( term ) )
	{
		if ( isList( term->head )) {
			printf( "(" );
		}
		debug_sterm_print( term->head );
		if ( isList( term->head )) {
			printf( ")" );
		}
	}
	if ( isAtom( term ) )
		printf( "%s ", (const char*)term->head );

	if ( isModel( term ))
		printf( "typeModel " );

	if ( isTransform( term ))
		printf( "typeTransform " );

	if ( term->tail )
		debug_sterm_print( term->tail );
}

void debug_sterm_printList( sterm* term ) {
	printf( "List: " );
	if ( isList( term ))
		printf( "(" );
	debug_sterm_print( term );
	if ( isList( term ))
		printf( ")" );
	printf( "\n" );
}

void* eval( sterm* data ) {
	if ( isAtom( data ) ) {
//		printf( " Evalling: %s\n", (char*)data->head );
		// It's either a function, a string, or a number
		return lookup( data );
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
		// TODO - fix this, need separate string type
		// For now just assume atom
		sterm* stext = eval( s->head );
		const char* text = stext->head;
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
	printf( "Beginning test: test_s_scene()\n" );
	sterm* s = parse_string( "(scene (model))" );
	scene* _scene = eval( s );
	(void)_scene;

	sterm_free( s );
	return;
}

void test_sfile( ) {
	sterm* p = parse_string( "(a (b c) (d))" );
	debug_sterm_printList( p );

	printf( "Beginning test: test dat/test2.s\n" );
	sterm* s = parse_file( "dat/test2.s" );
	eval( s );
	sterm_free( s );

	sterm* t = parse_string( "(transform (translation (vector 1.0 1.0 1.0 1.0)))" );
	sterm* e = eval( t );
	sterm_free( t );
	sterm_free( e );

	test_s_concat();
//	test_s_scene();
}

/*
// called once for each argument to scene
void scene_process( void* context, slist* data ) {
	
//( scene ( object ( model "cube.obj" )
//				 ( position ( vector 0.0 0.0 0.0 ))))

//		create a scene, scene->parse( subtree)
//		create an object, object->parase(subtree);
}
*/


sterm* cons( void* head, sterm* tail ) {
	sterm* term = mem_alloc( sizeof( sterm ));
	term->type = typeList;
	term->head = head;
	term->tail = tail;
	return term;
}

sterm* eval_list( sterm* s ) {
	sterm* result = NULL;
	if ( s )
		result = cons( eval( s->head ), eval_list( s->tail ) );
	return result;
}

// TODO - could this be removed, just an slist but with the term above having the typeTransform?
typedef struct transformData_s {
	sterm* elements;
	vector	translation;
} transformData;

transformData* transformData_create() {
	transformData* t =  (transformData*)mem_alloc( sizeof( transformData ));
	t->elements = NULL;
	t->translation = Vector( 0.f, 0.f, 0.f, 1.f );
	return t;
}

void transformData_processElement( transformData* t, sterm* element ) {
	if ( isModel( element ) || isTransform( element ) || isLight( element)) {
//		printf( "Adding child to Transform.\n" );
		t->elements = cons( element, t->elements );
	}
	// If it's a translation, copy the vector to the transformData
	if ( isTranslation( element )) {
		vector* translation = (vector*)((sterm*)element->tail->head)->head;
		t->translation = *translation;
//		t->translation = *(vector*)element->head;
//		printf( "Transform loaded translation: %.2f, %.2f, %.2f, %.2f\n", t->translation.val[0], t->translation.val[1], t->translation.val[2], t->translation.val[3] );
	}
}

// Receives a heterogenous list of elements, some might be transform properties
// some might be sub-elements
void transformData_processElements( transformData* t, sterm* elements ) {
	transformData_processElement( t, elements->head );
	if ( elements->tail )
		transformData_processElements( t, elements->tail );
}

// Creates a transformdata
// with a list of all modelInstancs passed into it
// and all other sub transforms
// So returns the whole sub tree from this point
void* s_transform( sterm* raw_elements ) {
//	printf( "s_transform\n" );
	transformData* tData = transformData_create();
//	debug_sterm_printList( raw_elements );
	if ( raw_elements ) {
		sterm* elements = eval_list( raw_elements );
//		debug_sterm_printList( elements );
		transformData_processElements( tData, elements );
	}
	sterm* t = sterm_create( typeTransform, tData );
	return t;
}

sterm* sterm_createProperty( const char* property_name, int property_type, void* property_data ) {
	return cons( sterm_create( typeAtom, (void*)property_name ), 
				cons( sterm_create( property_type, property_data ), 
					NULL ));
}

// Creates a translation
void* s_translation( sterm* raw_elements ) {
	assert( raw_elements );
	sterm* elements = eval_list( raw_elements );
	sterm* element = elements;
	// For now only allow vectors
	// Should be a list of one single vector
	// So take the head and check that
	assert( isVector( (sterm*)element->head ));
	// For now, copy the vector head from the vector sterm
	sterm* s_trans = sterm_createProperty( "translation", typeVector, ((sterm*)element->head)->head );
//	sterm_free( element );
	return s_trans;
}

void* s_diffuse( sterm* raw_elements ) {
	printf( "s_diffuse\n" );
	assert( raw_elements );
	sterm* elements = eval_list( raw_elements );
	sterm* element = elements;
	// For now only allow vectors
	// Should be a list of one single vector
	// So take the head and check that
	assert( isVector( (sterm*)element->head ));
	// For now, copy the vector head from the vector sterm
//	sterm* s_diff = cons( sterm_create( typeAtom, "diffuse" ), 
//						cons( sterm_create( typeVector, ((sterm*)element->head)->head ), 
//							NULL ));
	sterm* s_diff = sterm_createProperty( "diffuse", typeVector, ((sterm*)element->head)->head );
//	sterm_free( element );
	return s_diff;
}

void* s_vector( sterm* raw_elements ) {
	if ( raw_elements ) {
//		debug_sterm_printList( raw_elements );
		sterm* elements = eval_list( raw_elements );
//		debug_sterm_printList( elements );
//		vector* v = vector_processElements( elements );
		vector* v = mem_alloc( sizeof( vector ));
		memset( v, 0, sizeof( vector ));
		sterm* element = elements;
		int i = 0;
		while ( element && i < 4 ) {
			// dereference head twice, as we have a list of atoms of floats
//			printf( "s_vector found value: %s\n", (const char*)((sterm*)element->head)->head );
			v->val[i] = strtof( (const char*)((sterm*)element->head)->head, NULL );
			i++;
			element = element->tail;
		}
//		printf( "Loaded vector: %.2f, %.2f, %.2f, %.2f.\n", v->val[0], v->val[1], v->val[2], v->val[3] );
		sterm* sv = sterm_create( typeVector, v );
		return sv;
	}
	assert( 0 );
	return NULL;
}

typedef struct modelData_s {
	const char* filename;
} modelData;

modelData* modelData_create() {
	modelData* m = (modelData*)mem_alloc( sizeof( modelData ));
	m->filename = "dat/model/sphere.obj";
	return m;
}

// Creates a translation
void* s_filename( sterm* raw_elements ) {
	assert( raw_elements );
	sterm* elements = eval_list( raw_elements );
	sterm* element = elements;
	// Should be a single string
	// So take the head and check that
	assert( isAtom( (sterm*)element->head ));
	// For now, reference the string from the atom
	// TODO - need to clear up string/atom memory ownership
	sterm* sf = sterm_create( typeFilename, ((sterm*)element->head)->head );
//	sterm_free( element );
	return sf;
}
// Applies the properties to the modeldata
void modelData_processProperty( modelData* m, sterm* property ) {
	// TODO: implement
	if ( property->type == typeFilename ) {
		m->filename = property->head;
	}
}
void modelData_processProperties( modelData* m, sterm* properties ) {
	modelData_processProperty( m, properties->head );
	if ( properties->tail )
		modelData_processProperty( m, properties->tail );
}

// Creates a model instance
// Returns that model instance
void* s_model( sterm* raw_properties ) {
//	printf( "s_model\n" );
	modelData* mData = modelData_create();
	if ( raw_properties ) {
		sterm* properties = eval_list( raw_properties );
		modelData_processProperties( mData, properties );
	}
	sterm* m = sterm_create( typeModel, mData );
	return m;
}

typedef struct lightData_s {
	vector diffuse;
} lightData;

lightData* lightData_create( ) {
	lightData* lData = malloc( sizeof( lightData ));
	memset( lData, 0, sizeof( lightData ));
	return lData;
}
void lightData_processProperty( sterm* l, sterm* property ) {
	if ( isDiffuse( property )) {
		vector* diffuse = (vector*)((sterm*)property->tail->head)->head;

		((sterm*)l->tail->head)->head = malloc( sizeof( vector ));
		*(vector*)(((sterm*)l->tail->head)->head) = *diffuse;
		printf( "Setting light diffuse: " );
		vector_print( ((sterm*)l->tail->head)->head );
		printf( "\n" );
	}
}
void lightData_processProperties( sterm* l, sterm* properties ) {
	lightData_processProperty( l, properties->head );
	if ( properties->tail )
		lightData_processProperties( l, properties->tail );
}

void map( sterm* list, function f ) {
	f( list->head );
	if ( list->tail )
		map( list->tail, f );
}

void* s_light( sterm* raw_properties ) {
	// Build as a list
	vector* diffuse_vector = NULL; 
	sterm* data = sterm_create( typeAtom, "lightData" );
	sterm* diffuse = sterm_create( typeVector, diffuse_vector );
	sterm* lData = cons( data, cons( diffuse, NULL ));
	//sterm* lData = cons( data, cons( diffuse, cons( specular, NULL )));
	if ( raw_properties ) {
		sterm* properties = eval_list( raw_properties );
		lightData_processProperties( lData, properties );
	}

	return lData;
}

void scene_processObject( scene* s, transform* parent, sterm* object );
void scene_processObjects( scene* s, transform* parent, sterm* objects );
void scene_processTransform( scene* s, transform* parent, transformData* tData );

void scene_processTransform( scene* s, transform* parent, transformData* tData ) {
//	printf( "Creating Transform! Translation: %.2f, %.2f, %.2f\n", tData->translation.val[0], tData->translation.val[1], tData->translation.val[2] );
	transform* t = transform_create();
	matrix_setTranslation( t->local, &tData->translation );
	t->parent = parent;
	scene_addTransform( s, t );

	// If it has children, process those
	scene_processObjects( s, t, tData->elements );
	
}

void scene_processModel( scene* s, transform* parent, modelData* mData ) {
//	printf( "Model!\n" );
	modelHandle handle = model_getHandleFromFilename( mData->filename );
	modelInstance* m = modelInstance_create( handle );
	m->trans = parent;
	scene_addModel( s, m );
}

void scene_processLight( scene* s, transform* parent, sterm* lData ) {
	light* l = light_create();
	vector* v = ((sterm*)lData->tail->head)->head;
	light_setDiffuse( l, v->val[0], v->val[1], v->val[2], v->val[3] );
	l->trans = parent;
	scene_addLight( s, l );
}

void scene_processObject( scene* s, transform* parent, sterm* object ) {
	// TODO: implement	
	if ( isTransform( object ))
		scene_processTransform( s, parent, object->head );
	if ( isModel( object ))
		scene_processModel( s, parent, object->head );
	if ( isLight( object ))
		scene_processLight( s, parent, object );
}

void scene_processObjects( scene* s, transform* parent, sterm* objects ) {
	scene_processObject( s, parent, objects->head );
	if ( objects->tail )
		scene_processObjects( s, parent, objects->tail );
}

/*
	call s_scene, with a list of sterms
	creates a scene, then evals the list and calls s_process with that list

	evalling the list causes s_transform to be called, with a list of sterms
	that creates the transformData

   */
void* s_scene( sterm* raw_scene_objects ) {
//	printf( "s_scene\n" );
	scene* s = scene_create();
	sterm* scene_objects = eval_list( raw_scene_objects ); // TODO: Could eval_list be part of just eval?
//	debug_sterm_printList( scene_objects );
	scene_processObjects( s, NULL /*root has no parent*/, scene_objects );
	return s;
}

#endif
