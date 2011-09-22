// parse.c

#include "common.h"
#include "parse.h"
//-----------------------
#include "light.h"
#include "maths.h"
#include "model.h"
#include "scene.h"
#include "mem/allocator.h"
#include "render/modelinstance.h"
#include "system/hash.h"
#include "system/string.h"

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
	inputStream* in = mem_alloc( sizeof( inputStream ));
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
	char* token = mem_alloc( sizeof( char ) * (length + 1) );
	strncpy( token, stream->stream, length );
	token[length] = '\0';
	stream->stream = ptr; // Advance past the end of the token
	return token;
}

// Returns the next token as a c string, advances the inputstream to the token's end
void inputStream_skipToken( inputStream* stream ) {
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
	stream->stream = ptr; // Advance past the end of the token
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
	slist* s = mem_alloc( sizeof( slist ));
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
	return isPropertyType( s, "diffuse" );
}

bool isVector( sterm* s ) {
	return ( s->type == typeVector );
}

sterm* sterm_create( int tag, void* ptr ) {
	sterm* term = mem_alloc( sizeof( sterm ) );
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
		mem_free( token ); // It's a bracket, discard it
		return NULL;
	}
	if ( isListStart( *token ) ) {
		mem_free( token ); // It's a bracket, discard it
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
	mem_free( stream );
	return s;
}

sterm* parse_file( const char* filename ) {
	int length = 0;
	printf( "FILE: Loading File \"%s\" for parsing.\n", filename );
	char* contents = vfile_contents( filename, &length );
	vAssert( (contents) );
	vAssert( (length != 0) );
	sterm* s = parse_string( contents );
	mem_free( contents );
	return s;
}

// *** Map functions

typedef void (*function)( void* );
typedef void (*function_v)( void*, void* );
typedef void (*function_vv)( void*, void*, void* );

void map_vv( sterm* list, function_vv f, void* data, void* data_ ) {
	f( list->head, data, data_ );
	if ( list->tail )
		map_vv( list->tail, f, data, data_ );
}
void map_v( sterm* list, function_v f, void* data ) {
	f( list->head, data );
	if ( list->tail )
		map_v( list->tail, f, data );
}
void map_( sterm* list, function f ) {
	f( list->head );
	if ( list->tail )
		map_( list->tail, f );
}

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

void scene_processObject( void* object_, void* scene_, void* transform_ );

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

sterm* cons( void* head, sterm* tail ) {
	sterm* term = mem_alloc( sizeof( sterm ));
	term->type = typeList;
	term->head = head;
	term->tail = tail;
	return term;
}

void* eval( sterm* data ) {
	if ( isAtom( data ) ) {
		// It's either a function, a string, or a number
		return lookup( data );
	}
	else if ( isList( data ) ) {
		// If evaluating a list, the head must eval to an atom
		// That atom must be a function?
		sterm* func = eval( data->head );
		assert( isFunction( func ) );
		return ((script_func)func->head)( data->tail );
	}
	else {
		printf( "Unrecognised Sterm type: %d.\n", data->type );
		assert( 0 );
	}
	return NULL;
}

sterm* eval_list( sterm* s ) {
	sterm* result = NULL;
	if ( s )
		result = cons( eval( s->head ), eval_list( s->tail ) );
	return result;
}

void sterm_free( sterm* s ) {
//	printf( "FILE: sterm_free.\n" );
	if ( !isList( s ) ) {
		mem_free( s->head );
		mem_free( s );
	}
	else {
		if ( s->tail )
			sterm_free( s->tail );
		if ( s->head )
			sterm_free( s->head );
		mem_free( s );
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
		char* tmp = mem_alloc( sizeof( char ) * ( size + extra + 1 ) );
		strncpy( tmp, string, size );
		strncpy( tmp + size, text, extra );
		if ( string )
			mem_free( string );
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

// Creates an sterm-form property
// This takes the form of a list where the first item is the property name
// and the second item is the property value
// ie.
//	p->head = property name
//	p->tail->head = property value
//	p->tail->tail = NULL
sterm* sterm_createProperty( const char* property_name, int property_type, void* property_data ) {
	return cons(	sterm_create( typeAtom, (void*)property_name ), 
					cons(	sterm_create( property_type, property_data ), 
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

void* s_readVector( const char* property_name, sterm* raw_elements ) {
	assert( raw_elements );
	sterm* elements = eval_list( raw_elements );
	// Should be a list of one single vector
	// So take the head and check that
	assert( isVector( (sterm*)elements->head ));
	// For now, copy the vector head from the vector sterm
	sterm* property = sterm_createProperty( property_name, typeVector, ((sterm*)elements->head)->head );
	return property;
}

void* s_diffuse( sterm* raw_elements ) {
	/*
	assert( raw_elements );
	sterm* elements = eval_list( raw_elements );
	// For now only allow vectors
	// Should be a list of one single vector
	// So take the head and check that
	assert( isVector( (sterm*)elements->head ));
	// For now, copy the vector head from the vector sterm
	sterm* s_diff = sterm_createProperty( "diffuse", typeVector, ((sterm*)elements->head)->head );
	return s_diff;
	*/
	return s_readVector( "diffuse", raw_elements );
}

void* s_vector( sterm* raw_elements ) {
	if ( raw_elements ) {
		sterm* elements = eval_list( raw_elements );
		vector* v = mem_alloc( sizeof( vector ));
		memset( v, 0, sizeof( vector ));
		sterm* element = elements;
		int i = 0;
		while ( element && i < 4 ) {
			// dereference head twice, as we have a list of atoms of floats
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

#define kMaxObjectTypes	16
#define kMaxProperties	16
map* object_offsets = NULL;

void register_propertyOffset( const char* type, const char* property, int offset ) {
	if ( !object_offsets ) {
		printf( "Creating Object_offsets hashmap.\n" );
		object_offsets = map_create( kMaxObjectTypes, sizeof( map* ));
	}

	int t = mhash( type );
	map* m = map_find( object_offsets, t );
	if ( !m ) {
		printf( "Creating offset hashmap for type \"%s\".\n", type );
		m = map_create( kMaxProperties, sizeof( int ));
		map_add( object_offsets, t, &m );
	}

	int p = mhash( property);
	printf( "Adding offset ( \"%s\", \"%s\", %d )\n", type, property, offset );
	map_add( m, p, &offset );
}

int propertyOffset( const char* type, const char* property ) {
	int t = mhash( type );
	int p = mhash( property );
	map* m = *(map**)map_find( object_offsets, t );
	vAssert( m );
//	printf( "Found object map: 0x%x.\n", (unsigned int)m );
//	printf( "looking for key: 0x%x.\n", p );
	int offset = *(int*)map_find( m, p );
	vAssert( offset >= 0 );
	printf( "Offset returned: ( \"%s\", \"%s\", %d )\n", type, property, offset );
	return offset;
}

void parse_init() {
	register_propertyOffset( "light", "diffuse", offsetof( light, diffuse_color ));
}

// Generic property process function
void processProperty( void* p, void* object ) {
	printf( "Process Property\n" );
	// We need the object type name and the property name
	sterm* property = p;
	const char* property_name = ((sterm*)property->head)->head;
	const char* type_name = "light";

	int property_type = ((sterm*)property->tail->head)->type;



	void* data = (uint8_t*)object + propertyOffset( type_name, property_name );
	// test
	/*
	light* l = object;
	printf( "l->diffuse_color: 0x%x\n", (unsigned int)&l->diffuse_color );
	printf( "data: 0x%x\n", (unsigned int)data );
	*/
	//
	if ( property_type == typeVector ) {
		vector_printf( "Found vector:", ((sterm*)property->tail->head)->head );
		*(vector*)data = *(vector*)((sterm*)property->tail->head)->head;
	}
}

// Process a list of properties
// Compare them to the property list of the type
// Set appropriately

void lightData_processProperty( void* object_, void* light_ ) {
	sterm* l = light_;
	sterm* property = object_;
	if ( isDiffuse( property )) {
		vector* diffuse_vector = (vector*)((sterm*)property->tail->head)->head;

		sterm* diffuse = l->tail->head;
		diffuse->head = mem_alloc( sizeof( vector ));
		*(vector*)diffuse->head = *diffuse_vector;
	}
}

void* s_light( sterm* raw_properties ) {
	printf( "slight.\n" );
	// Test our generic processProperty
	light* l = light_create();
	if ( raw_properties ) {
		sterm* properties = eval_list( raw_properties );
		map_v( properties, processProperty, l );
	}

	vector_printf( "Parsed light with diffuse:", &l->diffuse_color );

	// Build as a list
	vector* diffuse_vector = NULL; 
	sterm* data = sterm_create( typeAtom, "lightData" );
	sterm* diffuse = sterm_create( typeVector, diffuse_vector );
	sterm* lData = cons( data, cons( diffuse, NULL ));

	// Evaluate all sub-terms, then process them
	if ( raw_properties ) {
		sterm* properties = eval_list( raw_properties );
		map_v( properties, lightData_processProperty, lData );
	}

	return lData;
}

void scene_processTransform( scene* s, transform* parent, transformData* tData ) {
	transform* t = transform_create();
	matrix_setTranslation( t->local, &tData->translation );
	t->parent = parent;
	scene_addTransform( s, t );

	// If it has children, process those
	map_vv( tData->elements, scene_processObject, s, t );
}

void scene_processModel( scene* s, transform* parent, modelData* mData ) {
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

void scene_processObject( void* object_, void* scene_, void* transform_ ) {
	sterm* object = object_;
	scene* s = scene_;
	transform* parent = transform_;
	if ( isTransform( object ))
		scene_processTransform( s, parent, object->head );
	if ( isModel( object ))
		scene_processModel( s, parent, object->head );
	if ( isLight( object ))
		scene_processLight( s, parent, object );
}

/*
	call s_scene, with a list of sterms
	creates a scene, then evals the list and calls scene_processObjects with that list

	evalling the list causes s_transform to be called, with a list of sterms
	that creates the transformData
   */
void* s_scene( sterm* raw_scene_objects ) {
	scene* s = scene_create();
	sterm* scene_objects = eval_list( raw_scene_objects ); // TODO: Could eval_list be part of just eval?
	map_vv( scene_objects, scene_processObject, s, NULL );
	return s;
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

void test_sfile( ) {
//	sterm* p = parse_string( "(a (b c) (d))" );
//	debug_sterm_printList( p );

	printf( "FILE: Beginning test: test dat/test2.s\n" );
	sterm* s = parse_file( "dat/test2.s" );
	eval( s );
	sterm_free( s );
/*
	printf( "FILE: Beginning test: transform + translation.\n" );
	sterm* t = parse_string( "(transform (translation (vector 1.0 1.0 1.0 1.0)))" );
	sterm* e = eval( t );
	sterm_free( t );
	sterm_free( e );
*/
	printf( "FILE: Beginning test: test concat\n" );
	test_s_concat();
}
