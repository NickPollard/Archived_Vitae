// TODO

// lisp_assertType (with stringified type output)
// lisp argument validation (arg count, arg types)

/*
	What we want to do:

	(object (property-name (property-type (property-value))))

	eg.

	(particle (diffuse (color (1.0 1.0 1.0 1.0))))

	This says we want to create a [particle], with a property called [diffuse], set to type [color] with value [1.0 1.0 1.0 1.0]

	The idea of using type functions means we can easily construct different type values. Eg. vector, matrix, colour. We can also construct them
	differently, ie.
	(color (1.0, 1.0, 1.0, 1.0))
	(color "red")
	(color (256 256 256))
	Would all result in the same final type (color)

	Evaluation steps:
	(color (1.0 1.0 1.0 1.0)) -> Calls color() function with a list of values. This creates a node of type color, with the value 1.0 1.0 1.0 1.0
	This is a simple, standard function call. We don't want to evaluate the list, so we backquote it
	(color `(1.0 1.0 1.0 1.0))

	The result is a typed value, NOT a list
	(diffuse [color]) -> creates a property pair with the atom tag 'diffuse' and the value [color] (Which is a typed object). 
	This is just passing a list, so backquote it.
	`(diffuse (color `(1.0 1.0 1.0 1.0)))

	(particle (property)) -> calls particle() with a list of properties. Each property has a name atom-tag and a typed-value - enough to identify
	the variable.
	The same as doing:
	color diffuse = color( 1.0, 1.0, 1.0, 1.0 );

	struct typeValue {
		enum type,
		void* value
		}

	struct list {
		void* head;
		list* tail;
		}

	struct value {
		enum type;
		void* value;
		value* tail;
		}

	list( a, b ) {
			value( type, a );
			value( type, b );
			value( list, a );
			value( list, b );
			list_a->tail = list_b;
		}

	l = (a, b)

	head( l ) == a
	tail( l ) == ( b )
	head(tail( l )) == b

   */

#include "common.h"
#include "lisp.h"
//--------------------------------------------------------
#include "maths.h"
#include "mem/allocator.h"
#include "model.h"
#include "model_loader.h"
#include "particle.h"
#include "system/hash.h"
#include "system/file.h"
#include "system/string.h"
#include <assert.h>

static term lisp_false = { 
	typeFalse, 
	{ NULL },		// Head 
	NULL,			// Tail
	0x0fffffff		// Refcount
	};
static term lisp_true = { 
	typeTrue, 
	{ NULL },		// Head 
	NULL,			// Tail
	0x0fffffff		// Refcount
	};

const term* lisp_true_ptr = &lisp_true;
const term* lisp_false_ptr = &lisp_false;

static const size_t kLispHeapSize = 1 << 20;
static heapAllocator* lisp_heap = NULL;
static passthroughAllocator* context_heap = NULL;

bool isType( term* t, enum termType type ) {
	return t->type == type;
	}

bool isValue( term* t ) {
	return !isType( t, _typeList ) && !isType( t, _typeAtom );
	}

void term_delete( term* t );

void term_takeRef( term* t ) {
	++t->refcount;
	}

void term_deref( term* t ) {
	--t->refcount;
	vAssert( t->refcount >= 0 );
	if ( t->refcount == 0 )
		term_delete( t );
	}

#ifdef MEM_STACK_TRACE
static const char* kLispTermAllocString = "lisp.c:term_create()";
static const char* kLispValueAllocString = "lisp.c:value_create()";
#endif // MEM_STACK_TRACE

#define kDebugLispStackDepth 128
const char* debug_lisp_stack[kDebugLispStackDepth];
int debug_lisp_stack_ptr = 0;

void debug_lisp_stack_dump() {
	printf( "######################################################\n" );
	for ( int i = 0; i < debug_lisp_stack_ptr; i++ ) {
		printf( "%d ## %s\n", i, debug_lisp_stack[i] );
	}
	printf( "######################################################\n" );
}

void debug_lisp_stack_push( const char* str ) {
	vAssert( debug_lisp_stack_ptr < kDebugLispStackDepth );
	debug_lisp_stack[debug_lisp_stack_ptr] = string_createCopy( str );
	++debug_lisp_stack_ptr;
}

void debug_lisp_stack_pop( ) {
	vAssert( debug_lisp_stack_ptr > 0 );
	--debug_lisp_stack_ptr;
	// Cast away constness
	mem_free( (char*)debug_lisp_stack[debug_lisp_stack_ptr] );
}

void lisp_assert( bool b ) {
	if ( !b ) {
		debug_lisp_stack_dump();
	}
	vAssert( b );
}

term* term_create( enum termType type, void* value ) {
	mem_pushStack( kLispTermAllocString );
	term* t = heap_allocate( lisp_heap, sizeof( term ));
	mem_popStack();
	t->type = type;
	t->head = value;
	if ( type == _typeList && value ) {
		term_takeRef( (term*)value );
		}
	t->tail = NULL;
	t->refcount = 0;
	return t;
	}

term* term_copy( term* t ) {
	return term_create( t->type, t->head );
}

void term_delete( term* t ) {
	if ( isType( t, _typeList )) {
		// If, not assert, as it could be the empty list
		if( t->head )
			term_deref( t->head );
		if ( t->tail )
			term_deref( t->tail );
		}
	heap_deallocate( lisp_heap, t );
	}

int _isListStart( char c ) {
	return c == '(';
}

int _isListEnd( char c ) {
	return c == ')';
}

int _isLineComment( const char* token ) {
	return string_equal( "#", token );	
}

void* value_create( size_t size ) {
	mem_pushStack( kLispValueAllocString );
	void* mem = heap_allocate( lisp_heap, size );
	mem_popStack();
	return mem;
	}

/*  
   List accessors

   Only valid on lists
   list must not be NULL

   Head - returns the head pointer of a list, which must be another term
   Tail - returns the tail pointer of a list, which must be another term of _typeList, or NULL;
   */

term* head( term* list ) {
	assert( list );
	assert( isType( list, _typeList ));
	assert( list->head );
	return (term*)list->head;
	}

term* tail( term* list ) {
	assert( list );
	assert( isType( list, _typeList ));
	// If there is a tail, it must be a list
	assert( !list->tail || isType( list->tail, _typeList ));
	return list->tail;
	}

/*
   List Constructors
   */
term* _cons( void* head, term* tail ) {
	term* t = term_create( _typeList, head );
	t->tail = tail;
	if ( tail )
		term_takeRef( tail );
	return t;
	}

term* _append( term* list, term* appendee ) {
	if ( tail( list ))
		return _cons( head( list ), _append( tail( list ), appendee ));
	else
		return _cons( head( list ), _cons( appendee, NULL ));	
}

// Get the next interesting lisp token
// skips past comments and newlines
char* lisp_nextToken( inputStream* stream ) {
	char* token = inputStream_nextToken( stream );
	while ( _isLineComment( token ) || isNewLine( *token )) {
		// skip the line
		if ( _isLineComment( token )) {
			//printf( " # Comment found; skipping line.\n" );
			inputStream_skipPast( stream, "\n" );
		}
		else
			//printf( " newline\n" );
		mem_free( token );
		token = inputStream_nextToken( stream );
		}
	return token;
}

// Read a token at a time from the inputstream, advancing the read head,
// and build it into an slist of atoms
term* lisp_parse( inputStream* stream ) {
	if ( inputStream_endOfFile( stream )) {
		printf( "ERROR: End of file reached during lisp_parse, with incorrect number of closing parentheses.\n" );
		vAssert( 0 );
		return NULL;
		}

	char* token = lisp_nextToken( stream );
	//printf( "lisp token \"%s\"\n", token );

	if ( _isListEnd( *token ) ) {
		mem_free( token ); // It's a bracket, discard it
		return NULL;
		}
	if ( _isListStart( *token ) ) {
		mem_free( token ); // It's a bracket, discard it
		term* list = term_create( _typeList, lisp_parse( stream ));
		term* s = list;

		if ( !s->head ) { // The Empty list () 
			return list;
			}

		while ( true ) {
			term* sub_expr = lisp_parse( stream );				// parse a subexpr
			if ( sub_expr ) {								// If a valid return
				s->tail = term_create( _typeList, sub_expr );	// Add it to the tail
				term_takeRef( s->tail );						// Take a ref (from the head to the tail)
				s = s->tail;
			} else {
				return list;
				}
			}
		}
	if ( token_isString( token )) {
		const char* string = sstring_create( token );
		return term_create( _typeString, (char*)string );
		}
	if ( token_isFloat( token )) {
		float* f = value_create( sizeof( float ));
		*f = strtof( token, NULL );
		return term_create( typeFloat, f );
		}
	// When it's an atom, we keep the token, don't free it
	return term_create( _typeAtom, (void*)token );
}

term* lisp_parse_string( const char* string ) {
	inputStream* stream = inputStream_create( string );
	term* s = lisp_parse( stream );
	mem_free( stream );
	return s;
}

term* lisp_parse_exprList( inputStream* stream ) {
	PARSE_PRINT( "lisp_parse_exprList: \"%s\"\n", stream->stream );
	inputStream* peeker = inputStream_create( stream->stream );
	lisp_nextToken( peeker );
	//printf( "Next lisp token: \"%s\"\n", token );
	bool eof = inputStream_endOfFile( peeker );
	mem_free( peeker );
	if ( eof )
		return NULL;

	term* t = lisp_parse( stream );
	term* list = _cons( t, lisp_parse_exprList( stream ) );
	return list;
}

term* lisp_parse_file( const char* filename ) {
	printf( "FILE: Loading File \"%s\" for parsing.\n", filename );
	size_t length = 0;
	char* contents = vfile_contents( filename, &length );
	vAssert( (contents) );
	vAssert( (length != 0) );
	
	inputStream* stream = inputStream_create( contents );
	term* t = lisp_parse_exprList( stream );

	mem_free( contents );
	return t;
}

term* _eval_list( term* list, context* c ) {
	lisp_assert( isType( list, _typeList ));
	term* s = _eval( head( list ), c );
	if ( tail( list ))
		return _eval_list( tail( list ), c );
	return s;
}

term* lisp_eval_file( context* c, const char* filename ) {
	term* t = lisp_parse_file( filename );
	return _eval_list( t, c );
}

void list_delete( term* t ) {
	assert( isType( t, _typeList ));
	if ( t->tail ) {
		list_delete( t->tail );
		}
	term_delete( t );
	}

void list_deref( term* t ) {
	assert( isType( t, _typeList ));
	if ( t->tail ) {
		list_deref( t->tail );
		}
	term_deref( t );
	}

int list_length( term* t ) {
	if ( t ) {
		assert( isType( t, _typeList ));
		return 1 + list_length( t->tail );
	}
	else {
		return 0;
		}
	}
/*
   term accessors
   */
void* value( term* atom ) {
	assert( atom );
	assert( !isType( atom, _typeList ));
	return atom->head;
	}

void* context_lookup( context* c, int atom ) {
	assert( c );
	assert( c->lookup );
	term** v = (term**)map_find( c->lookup, atom );
	if ( v ) {
		return *v;
		}
	if ( c->parent ) {
		return context_lookup( c->parent, atom );
		}
	return NULL;
	}

void context_add( context* c, const char* name, term* t ) {
	map_add( c->lookup, mhash( name ), &t );
	term_takeRef( t );
	}

void term_debugPrint( term* t ) {
	if ( !t )
		return;

	if ( isType( t, _typeList )) {
		printf( "(" );
		term* tmp = t;
		while ( tmp ) {
			assert( isType( tmp, _typeList ));
			if ( tmp->head )
				term_debugPrint( head( tmp ));
			tmp = tmp->tail;
		}
		printf( ")" );
		}

	if ( isType( t, _typeAtom ) )
		printf( "%s ", (const char*)t->head );
	if ( isType( t, _typeString ) )
		printf( "\"%s\" ", (const char*)t->head );
	if ( isType( t, typeFloat ))
		printf( "%.2f ", *(float*)t->head );
	if ( isType( t, _typeObject ))
		printf( "[Object] " );
}
void term_debugStreamPrint( streamWriter* s, term* t ) {
	if ( !t )
		return;

	if ( isType( t, _typeList )) {
		stream_printf( s, "(" );
		term* tmp = t;
		while ( tmp ) {
			assert( isType( tmp, _typeList ));
			if ( tmp->head )
				term_debugStreamPrint( s, head( tmp ));
			tmp = tmp->tail;
		}
		stream_printf( s, ")" );
		}

	if ( isType( t, _typeAtom ) )
		stream_printf( s, "%s ", (const char*)t->head );
	if ( isType( t, _typeString ) )
		stream_printf( s, "\"%s\" ", (const char*)t->head );
	if ( isType( t, typeFloat ))
		stream_printf( s, "%.2f ", *(float*)t->head );
	if ( isType( t, _typeObject ))
		stream_printf( s, "[Object] " );
	if ( isType( t, typeIntrinsic ))
		stream_printf( s, "[Intrinsic] " );
}

typedef term* (*fmap_func)( term*, void* );
typedef term* (*lisp_func)( context*, term* );

term* fmap_1( fmap_func f, void* arg, term* expr ) {
	if ( !expr )
		return NULL;
	vAssert( isType( expr, _typeList ));
	return _cons( f( head( expr ), arg ),
		   			fmap_1( f, arg, tail( expr )));
	}

void* exec( context* c, term* func, term* args);

/*
   Eval

   Where the magic happens

   An ATOM evaluates to its binding in the Context - a function or variable
   A VALUE evaluates to itself
   A LIST evaluates to the value of it's first element executed with the remaining elements as arguments
   All list elements are EVALuated before the list is.

   A -> Context[A];
   "A" -> "A"
   ( A ) -> Context[A]()
   ( A "B" ) -> Context[A]( "B" )
   ( A ( B ( C ))) -> Context[A]( Context[B]( Context[C]()))
   */

term* _eval( term* expr, void* _context ) {
#ifdef DEBUG
	const int kDebugStackLength = 1024;
	char debug_expr[kDebugStackLength];
	streamWriter stackStream;
	stackStream.write_head = stackStream.string = debug_expr;
	stackStream.end = stackStream.string + kDebugStackLength;
	sprintf( debug_expr, "Test" );
	term_debugStreamPrint( &stackStream, expr );
	debug_lisp_stack_push( debug_expr );
#endif

	term_takeRef( expr );
	term* result = NULL;
	// Eval arguments, then pass arguments to the binding of the first element and run
	if ( isType( expr, _typeAtom )) {
		term* value = context_lookup(_context, mhash(expr->string));
		if ( !value )
		{
			printf( "## ERROR ## LISP: Cannot find binding for atom \"%s\"\n", expr->string );
		}
		lisp_assert( value );
		result = value;
		}
	if ( isValue( expr )) {
		// Do we return as a term of typeValue, or just return the value itself? Probably the first, for macros and hijinks
		result = term_copy( expr );
		}
	if ( isType( expr, _typeList )) {
		// TODO - need to check for intrinsic here
		// as if it's a special form, we might not want to eval all (eg. if)
		term* h = _eval( head( expr ), _context );
		if ( !isType( h, typeIntrinsic )) {
			//debug_lisp_stack_push( "function" );
			term* e = fmap_1( _eval, _context, tail( expr ));
			if ( e )
				term_takeRef( e );
			result = exec( _context, h, e );
			if ( e )
				term_deref( e );
			}
		else {
			//debug_lisp_stack_push( "intrinsic" );
			result = exec( _context, h, tail( expr ));
			}
		//debug_lisp_stack_pop();
		}
	term_deref( expr );
	debug_lisp_stack_pop();
	return result;
	}

context* context_create( context* parent ) {
	context* c = passthrough_allocate( context_heap, sizeof( context ));
	int max = 128, stride = sizeof( term* );
	c->lookup = map_create( max, stride );
	c->parent = parent;
	return c;
	}

void context_delete( context* c ) {
	// Deref all the contents
	CONTEXT_PRINT( "Deleting context with %d keys.\n", c->lookup->count );
	for ( int i = 0; i < c->lookup->count; ++i ) {
		term* t = *(term**)(c->lookup->values + (i * c->lookup->stride));
		if ( t )
			term_deref( t );
		}

	passthrough_deallocate( context_heap, c );
	}

/*
	Def - creates a context binding

	Used to assign variables, including functions
	(def a 10)
	(def my_func (a)
		(append a))

		returns a context with a copy of the old context, plus the new binding
   */
/*
context* def( context* c, const char* atom, void* value ) {
	context * new = context_create( c );
	context_add( new, atom, value);
	return new;
	}
*/

/*
   Execute a lisp function
   )
   The lisp function is defined as it's argument bindings followed by it's expression
   eg.
   (def my_func (a b)
   		(+ a b))

		FUNC is a list where the first item (head) is a list of argument names
		and the second item (tail->head) is the function expression
		ARGS is a list of argument values
   */
typedef void (*zip1_func)( term*, term*, void* );

void zip1( term* a_list, term* b_list, zip1_func func, void* arg ) {
	assert( isType( a_list, _typeList ));
	assert( isType( b_list, _typeList ));
	func( head( a_list ), head( b_list ), arg );
	if ( tail( a_list )) {
		assert( tail( b_list ));
		zip1( tail( a_list ), tail( b_list ), func, arg );
		}
	}

void define_arg( term* symbol, term* value, void* _context ) {
	context* c = _context;
	/*
	printf( "Mapping argument %s to value: ", (const char*)symbol->head );
	term_debugPrint( value );
	printf( "\n" );
	*/
	context_add( c, symbol->string, value );
	}

void* exec( context* c, term* func, term* args ) {
	lisp_assert( func );
	lisp_assert( isType( func, _typeList ) || isType( func, typeIntrinsic ));
	lisp_assert( !args || isType( args, _typeList ));

	if ( isType( func, _typeList )) {
		lisp_assert( head( func ));			// The argument binding
		term* arg_list = head( func );
		term* expr = head( tail( func ));

		context* local = context_create( c );

		// Map all the arguments to the arglist
		if ( args ) {
			zip1( arg_list, args, define_arg, local );  
			}

		// Evaluate the function in the local context including the argument bindings
		term* ret = _eval( expr, local );
		context_delete( local );
		return ret;
		}

	if ( isType( func, typeIntrinsic )) {
		lisp_func f = func->data;
		term* ret = f( c, args );
		return ret;
		}

	assert( 0 );
	return NULL;
	}

void define_function( context* c, const char* name, const char* value ) {
	term* func = lisp_parse_string( value );
	context_add( c, name, func );
	}

void define_lispfunction( context* c, const char* name, term* func ) {
	context_add( c, name, func );
}

void define_cfunction( context* c, const char* name, lisp_func implementation ) {
	term* func = term_create( typeIntrinsic, implementation );
	context_add( c, name, func );
	}

// Intrinsic Maths functions
term* lisp_func_add( context* c, term* raw_args ) {
	assert( isType( raw_args, _typeList ));
	assert( raw_args->tail );	
	assert( isType( raw_args->tail, _typeList ));	
	term* args = fmap_1( _eval, c, raw_args );
	term_takeRef( args );
	assert( isType( args, _typeList ));
	assert( isType( head( args ), typeFloat ));
	assert( isType( head( tail( args )), typeFloat ));

	float a = *(float*)head( args )->head;
	float b = *(float*)head( tail( args ))->head;
	mem_pushStack( kLispValueAllocString );
	float* result = heap_allocate( lisp_heap, sizeof( float )  );
	mem_popStack( );
	*result = a + b;
	term* ret = term_create( typeFloat, result );
	term_deref( args );
	return ret;
	}

term* lisp_func_sub( context* c, term* raw_args ) {
	term* args = fmap_1( _eval, c, raw_args );
	term_takeRef( args );
	assert( isType( args, _typeList ));
	assert( isType( head( args ), typeFloat ));
	assert( isType( head( tail( args )), typeFloat ));

	float a = *(float*)head( args )->head;
	float b = *(float*)head( tail( args ))->head;
	mem_pushStack( kLispValueAllocString );
	float* result = heap_allocate( lisp_heap, sizeof( float )  );
	mem_popStack( );
	*result = a - b;
	term* ret = term_create( typeFloat, result );
	term_deref( args );
	return ret;
	}

term* lisp_func_mul( context* c, term* raw_args ) {
	term* args = fmap_1( _eval, c, raw_args );
	term_takeRef( args );
	assert( isType( args, _typeList ));
	assert( isType( head( args ), typeFloat ));
	assert( isType( head( tail( args )), typeFloat ));

	float a = *(float*)head( args )->head;
	float b = *(float*)head( tail( args ))->head;
	mem_pushStack( kLispValueAllocString );
	float* result = heap_allocate( lisp_heap, sizeof( float )  );
	mem_popStack( );
	*result = a * b;
	term* ret = term_create( typeFloat, result );
	term_deref( args );
	return ret;
	}

term* lisp_func_div( context* c, term* raw_args ) {
	term* args = fmap_1( _eval, c, raw_args );
	term_takeRef( args );
	assert( isType( args, _typeList ));
	assert( isType( head( args ), typeFloat ));
	assert( isType( head( tail( args )), typeFloat ));

	float a = *(float*)head( args )->head;
	float b = *(float*)head( tail( args ))->head;
	mem_pushStack( kLispValueAllocString );
	float* result = heap_allocate( lisp_heap, sizeof( float )  );
	mem_popStack( );
	*result = a / b;
	term* ret = term_create( typeFloat, result );
	term_deref( args );
	return ret;
	}

term* lisp_func_greaterthan( context* c, term* raw_args ) {
	term* args = fmap_1( _eval, c, raw_args );
	term_takeRef( args );
	assert( isType( args, _typeList ));
	assert( isType( head( args ), typeFloat ));
	assert( isType( head( tail( args )), typeFloat ));
	float a = *(float*)head( args )->head;
	float b = *(float*)head( tail( args ))->head;
	term* ret = NULL;
	if ( a > b )
		ret = term_create( typeTrue, NULL );
	else
		ret = term_create( typeFalse, NULL );
	term_deref( args );
	return ret;
	}

term* lisp_func_length( context* c, term* raw_args ) {
	term* args = fmap_1( _eval, c, raw_args );
	term_takeRef( args );

	int len = list_length( head( args ));

	mem_pushStack( kLispValueAllocString );
	float* result = heap_allocate( lisp_heap, sizeof( float )  );
	mem_popStack( );
	*result = (float)len;

	term* tf = term_create( typeFloat, result );
	term_deref( args );
	return tf;
	}

term* lisp_func_vector( context* c, term* raw_args ) {
	term* args = fmap_1( _eval, c, raw_args );
	term_takeRef( args );
	
	vector* v = value_create( sizeof( vector ));
	*v = Vector( 0.0f, 0.0f, 0.0f, 0.0f );
	float* floats = (float*)&(*v);
	int i = 0;
	term* t = args;
	while ( i < 4 && t ) {
		assert( isType( head( t ), typeFloat ));
		floats[i] = *(float*)head( t )->head;
		t = t->tail;
	}
	
	term* vec = term_create( _typeVector, v );;
	term_deref( args );
	return vec;
	}

term* lisp_func_color( context* c, term* raw_args ) {
	term* args = fmap_1( _eval, c, raw_args );
	term_takeRef( args );
	// This should be called with a valid argument, which is one of the following:
	//	> A Vector ( the rgba values from 0->1, eg. [1.0 0.0 0.0 1.0] )
	//	> A list of floats ( the rgba values from 0->1, eg. (1.0 0.0 0.0 1.0) )
	//	> A string ( name of the color, eg. "red" )
	assert( isType( args, _typeList ) && list_length( args ) == 1 );
	assert( isType( head( args ), _typeVector ) || isType( head( args ), _typeList ) || isType( head( args ), _typeString ) );
	term* color = term_create( _typeVector, NULL );
	color->head = value_create( sizeof( vector ));
	vector v;
	*(vector*)color->head = v;

	term_deref( args );
	return color;
	}

// Need to work out what returns true
// Any non-empty list should probably return true?
// Any non-false term should return true?
bool isTrue( term* t ) {
	(void)t;
	return ( !isType( t, typeFalse ));
	}

term* eval_string( const char* str, context* c ) {
	return _eval( lisp_parse_string( str ), c );
	}

// lisp 'if' statement
// of the form:
//    (if (cond) a b)
// evaluates cond
// if cond is true, then returns _eval( a )
// else returns _eval( b )
term* lisp_func_if( context* c, term* args ) {
		term* cond = head( args );
		term* result_then = head( tail( args ));
		term* result_else = head( tail( tail( args )));

		term* first = _eval( cond, c );
		term_takeRef( first );
		term* ret;
		if ( isTrue( first )) {
			ret = _eval( result_then, c );
			}
		else {
			ret = _eval( result_else, c );
			}
		term_deref( first );
		return ret;
	}

typedef struct test_struct_s {
	vector v;
	float a;
	float b;
} test_struct;

#define LISP_OBJECT_FUNC( CLASS, ATTR ) \
term* lisp_func_##CLASS##_##ATTR( context* c, term* raw_args ) { \
	term* args = fmap_1( _eval, c, raw_args ); \
	term_takeRef( args ); \
	lisp_assert( list_length( args ) == 2 ); \
	term* value = head( args ); \
	term* object = head( tail( args )); \
	lisp_assert( isType( value, typeFloat )); \
	CLASS* ob = object->data; \
	ob->ATTR = *value->number; \
	term_deref( args ); \
	return object; \
}

LISP_OBJECT_FUNC( test_struct, a );
LISP_OBJECT_FUNC( test_struct, b );

void* object_createType( const char* string ) {
	void* data = NULL;
	if ( string_equal( string, "test_struct" ))
	{
		data = mem_alloc( sizeof( test_struct ));
		memset( data, 0, sizeof( test_struct ));
	}
	else
		data = NULL;
	return data;
	}

term* lisp_func_new( context* c, term* raw_args ) {
	term* args = fmap_1( _eval, c, raw_args );
	term_takeRef( args );
	term* type = head( args );
	vAssert( type && isType( type, _typeAtom ));
	void* object = object_createType( type->string );
	term* ret = term_create( _typeObject, object );
	term_deref( args );
	return ret;
	}

// (object_process object function)
term* lisp_func_object_process( context *c, term* raw_args ) {
	term* args = fmap_1( _eval, c, raw_args );
	term_takeRef( args );
	vAssert( list_length( args ) == 2 );
	term* ret = head( args );
	term* func = head( tail( args ));
	term* list = _append( func, ret );
	term_debugPrint( list );
	printf( "\n" );	
	_eval( list, c);
	term_deref( args );
	return ret;
}

term* lisp_func_quote( context* c, term* raw_args ) {
	(void)c;
	return head( raw_args );
	}

term* list_copy( term* list ) {
	if ( tail( list ))
		return _cons( list->head, list_copy( list->tail ));
	else
		return _cons( list->head, NULL );
}

term* lisp_func_tail( context* c, term* raw_args ) {
	term* args = fmap_1( _eval, c, raw_args );
	term_takeRef( args );
	term* list = head( args );
	vAssert( isType( list, _typeList ));
	term* t = tail( list );
	if ( !t )
		t = &lisp_false;
	else {
		t = list_copy( t );
	}
	term_deref( args );
	return t;
}

term* lisp_func_head( context* c, term* raw_args ) {
	term* args = fmap_1( _eval, c, raw_args );
	term_takeRef( args );
	term* list = head( args );
	term* h = head( list );
	term* ret = NULL;
	if ( isType( h, _typeList )) {
		ret = list_copy( h );
	}
	else {
		ret = term_create( h->type, h->data );
	}
	term_deref( args );
	return ret;
}

void lisp_debug_stack_init() {
	memset( debug_lisp_stack, 0, sizeof( const char*) * kDebugLispStackDepth );
}

void lisp_init() {
	lisp_heap = heap_create( kLispHeapSize );
	assert( lisp_heap->total_allocated == 0 );

	context_heap = passthrough_create( lisp_heap );
	lisp_debug_stack_init();
}

#define NO_PARENT NULL

/*
	( model ( mesh ( filename ... )))
	// Creates a string called filename
	// loads a mesh from that filename
	// creates a model with that mesh
   */

term* lisp_func_model( context* c, term* raw_args ) {
	(void)c;
	assert( isType( raw_args, _typeList ));
	term* args = fmap_1( _eval, c, raw_args );
	term_takeRef( args );
	assert( isType( args, _typeList ));
	assert( isType( head( args ), _typeObject )); // Looking for a mesh
	model* m = model_createModel( 1 ); // default to one mesh
	m->meshes[0] = head( args )->data;
	term* ret = term_create( _typeObject, m );
	//term_deref( args );	
	return ret;
}
/*
	(mesh (filename f))

	What we want to do:

	> Evaluate (filename f) to a 'filename' ( it might be quoted )
	> Create a Mesh
	> fold the property args over the mesh
		> i.e. run the filename property func with the Mesh as an arg
	> a possible post-lisp init of the mesh
	> return the mesh

   */
term* lisp_func_mesh( context* c, term* raw_args ) {
	assert( isType( raw_args, _typeList ));
	term* args = fmap_1( _eval, c, raw_args );
	term_takeRef( args );
	assert( isType( args, _typeList ));
	assert( isType( head( args ), _typeObject )); // Looking for a mesh

	mesh* msh = mesh_loadObj( "dat/model/sphere.obj" );
	term* ret = term_create( _typeObject, msh );
	term_deref( args );	
	return ret;
}

// Create an empty mesh
term* lisp_func_mesh_create( context* c, term* raw_args ) {
	assert( isType( raw_args, _typeList ));
	term* args = fmap_1( _eval, c, raw_args );
	term_takeRef( args );
	assert( isType( args, _typeList ));
	assert( isType( head( args ), _typeObject )); // Looking for a mesh

	mesh* msh = NULL;
	term* ret = term_create( _typeObject, msh );
	term_deref( args );	
	return ret;
}

term* lisp_func_filename( context* c, term* raw_args ) {
	lisp_assert( isType( raw_args, _typeList ));
	term* args = fmap_1( _eval, c, raw_args );
	term_takeRef( args );
	lisp_assert( isType( args, _typeList ));
	lisp_assert( isType( head( args ), _typeString ));

	mesh* msh = mesh_loadObj( "dat/model/sphere.obj" );
	term* ret = term_create( _typeObject, msh );
	term_deref( args );	
	return ret;
}

// PLACEHOLDER
term* lisp_func_transform( context* c, term* raw_args ) {
	(void)c;
	assert( isType( raw_args, _typeList ));
	/*
	term* args = fmap_1( _eval, c, raw_args );
	term_takeRef( args );
	assert( isType( args, _typeList ));
	assert( isType( head( args ), _typeObject )); // Looking for a mesh
	*/
	//term_deref( args );	
	return &lisp_false;
}

void lisp_assertArgs_1( term* t, enum termType type ) {
	lisp_assert( isType( t, _typeList ));
	lisp_assert( list_length( t ) == 1 );
	lisp_assert( isType( head( t ), type ));
}

// (property_create stride)
term* lisp_func_property_create( context* c, term* raw_args ) {
	(void)c;
	(void)raw_args;
	
	lisp_assert( isType( raw_args, _typeList ));
	term* args = fmap_1( _eval, c, raw_args );
	term_takeRef( args );
	
	lisp_assertArgs_1( args, typeFloat );

	// get the stride
	term* s = head( args );
	int stride = (int)(*s->number);
	property* p = property_create( stride );
	term* tp = term_create( _typeObject, p );

	term_deref( args );	
	return tp;
}

term* lisp_func_property_addkey( context* c, term* raw_args ) {
	(void)c;
	lisp_assert( isType( raw_args, _typeList ));
	term* args = fmap_1( _eval, c, raw_args );
	term_takeRef( args );
	lisp_assert( isType( args, _typeList ));

	term* tp = head( args );
	property* p = tp->data;
	// get first key for now
	term* key = head( tail( args ));
	lisp_assert( isType( key, _typeList ));
	// get the two floats out;
	float k = *head( key )->number;
	float v = *head( tail( key ))->number;

	printf( "Adding key: %.2f %.2f\n", k, v );
	property_addf( p, k, v );

	term_deref( args );	
	return tp;
}


// Define a new lisp function and bind it to the context; in lisp
/*
   (defun myFunc (some args) 
   			(some code (using args)))
   */
term* lisp_func_defun( context* c, term* raw_args ) {
	lisp_assert( isType( head( raw_args ), _typeAtom ));
	const char* name = head( raw_args )->string;
	term* value = tail( raw_args );
	printf( "LISP: DEFUN \"%s\"\n", name );
	define_lispfunction( c, name, value );
	return &lisp_true;
}

void lisp_initContext( context* c ) {
	// Every lisp context needs this in order to pull in other functions, including the libraries
	define_cfunction( c, "defun", lisp_func_defun );

	// General lisp funcs
	define_function( c, "map",		"(( func list ) (cons (func (head list)) (if (tail list) (map func (tail list)) null)))" );
	define_function( c, "filter",	"(( func list ) (if (func (head list)) (cons (head list) (filter func (tail list))) (filter func (tail list))))" );
	define_function( c, "foldl",	"(( func item list ) (if (tail list) (foldl func (func item (head list)) (tail list)) (func item (head list))))" );
	
	define_cfunction( c, "vector", lisp_func_vector );
	define_cfunction( c, "color", lisp_func_color );
	define_cfunction( c, "quote", lisp_func_quote );

	define_cfunction( c, "length", lisp_func_length );

	define_cfunction( c, "+", lisp_func_add );
	define_cfunction( c, "-", lisp_func_sub );
	define_cfunction( c, "*", lisp_func_mul );
	define_cfunction( c, "/", lisp_func_div );

	// Model loading functions
	define_cfunction( c, "model", lisp_func_model );
	//define_cfunction( c, "mesh", lisp_func_mesh );
	define_cfunction( c, "transform", lisp_func_transform );
	define_cfunction( c, "mesh_create", lisp_func_mesh_create );
	define_cfunction( c, "filename", lisp_func_filename );
	define_cfunction( c, "property_create", lisp_func_property_create );
	define_cfunction( c, "property_addKey", lisp_func_property_addkey );

	define_function( c, "mesh", "( args ) (foldl object_process (mesh_create) args)" );
	//define_function( c, "filename", "(() b )" );

	// load the default vlisp library
	lisp_eval_file( c, "src/script/lisp/vliblisp.s" );
}

context* lisp_newContext() {
	context* c = context_create( NO_PARENT );
	lisp_initContext( c );
	return c;
}

void test_lisp() {
	term* script = lisp_parse_string( "(a b)" );
	term_takeRef( script );
	// Type tests
	assert( isType( script, _typeList ));
	assert( isType( head( script ), _typeAtom ));
	assert( isType( tail( script ), _typeList ));
	assert( isType( head( tail( script )), _typeAtom ));
	// Value tests
	assert( string_equal( value( head( script )) , "a" ));
	assert( string_equal( value( head( tail( script ))) , "b" ));

	term_deref( script );
	assert( lisp_heap->allocations == 0 );
	
	context* c = lisp_newContext();
	term* hello = term_create( _typeString, "Hello World" );
	term* goodbye = term_create( _typeString, "Goodbye World" );
	context_add( c, "b", hello );
	context_add( c, "goodbye", goodbye );

	term* search = context_lookup( c, mhash("b"));
	assert( search->head );

	// Test #1 - Variable binding
	term* result = _eval( lisp_parse_string( "b" ), c );
	term_takeRef( result );
	assert( isType( result, _typeString ) && string_equal( result->string, "Hello World" ));
	term_deref( result );

	//assert( lisp_heap->allocations == 3 );

	define_function( c, "value_of_b", "(() b )" );

	// Test #2 - Function calling
	result = _eval( lisp_parse_string( "( value_of_b )" ), c );
	assert( string_equal( result->string, "Hello World" ));


	// Test #3 - Single function argument
	define_function( c, "value_of_arg", "(( arg ) arg )" );
	result = _eval( lisp_parse_string( "( value_of_arg b )" ), c );
	term_takeRef( result );
	assert( string_equal( result->string, "Hello World" ));
	term_deref( result );
	
	//assert( lisp_heap->allocations == 12 );

	// Test #4 - Multiple function argument mapping
	define_function( c, "value_of_first_arg", "(( arg1 arg2 ) arg1 )" );
	define_function( c, "value_of_second_arg", "(( arg1 arg2 ) arg2 )" );
	
	result = _eval( lisp_parse_string( "( value_of_first_arg b goodbye )" ), c );
	assert( string_equal( result->string, "Hello World" ));
	result = _eval( lisp_parse_string( "( value_of_second_arg goodbye b )" ), c );
	assert( string_equal( result->string, "Hello World" ));
	result = _eval( lisp_parse_string( "( value_of_first_arg goodbye b )" ), c );
	assert( !string_equal( result->string, "Hello World" ));
	result = _eval( lisp_parse_string( "( value_of_second_arg b goodbye )" ), c );
	assert( !string_equal( result->string, "Hello World" ));

	// Test #5 - Numeric types and intrinsic functions
	float num = 5.f;
	term* five = term_create( typeFloat, &num );
	context_add( c,  "five", five );

	float num_2 = 2.f;
	result = term_create( typeFloat, &num_2 );
	context_add( c,  "two", result );

	result = _eval( lisp_parse_string( "five" ), c );

	assert( isType( result, typeFloat ));
	assert( *result->number == 5.f );

	//assert( lisp_heap->allocations == 28 );

	result = _eval( lisp_parse_string( "(+ five five)" ), c );
	term_takeRef( result );
	assert( *result->number == 10.f );
	term_deref( result );

	// Test #6 - function definitions using intrinsics
	define_function( c, "double", "(( a ) (+ a a))" );
	result = _eval( lisp_parse_string( "(double five)" ), c );
	assert( *result->number == 10.f );
	
	result = _eval( lisp_parse_string( "(double five)" ), c );
	assert( *result->number == 10.f );
	
	result = _eval( lisp_parse_string( "(double (double five))" ), c );
	assert( *result->number == 20.f );
	
	result = _eval( lisp_parse_string( "(* five five))" ), c );
	assert( *result->number == 25.f );

	// If (intrinsic)
	define_cfunction( c, "if", lisp_func_if );
	result = eval_string( "(if b b a)", c );
	assert( result == eval_string( "b", c ) );

	// False
	// TRUE and FALSE are static lisp terms, we don't use context_add as we can't (and don't) add a ref
	map_add( c->lookup, mhash("false"), (term**)&lisp_false_ptr ); // Casting away const (cleaner way of doing this?)
	map_add( c->lookup, mhash("true"), (term**)&lisp_true_ptr ); // Casting away const (cleaner way of doing this?)
	result = eval_string( "(if false a (if false a b))", c );
	assert( result == eval_string( "b", c ) );
	result = eval_string( "(if false a (if false b goodbye))", c );
	assert( result != eval_string( "b", c ) );

	// Greater-than
	define_cfunction( c, ">", lisp_func_greaterthan );
	result = eval_string( "(> five two )", c );
	assert( result->type == typeTrue );
	result = eval_string( "(> two five )", c );
	assert( result->type == typeFalse );

	define_function( c, "<=", "(( a b ) (if (> b a) true false))" );
	result = eval_string( "(<= two five)", c );
	assert( result->type == typeTrue );

	// Numeric types
	assert( eval_string( "( > 5.0 3.0 )", c )->type == typeTrue );
	assert( eval_string( "( > 2.0 4.0 )", c )->type == typeFalse );

	// And / Or
	define_function( c, "and", "(( a b ) (if a (if b true false) false))");
	define_function( c, "or", "(( a b ) (if a true (if b true false)))");
	result = eval_string( "(or true false)", c );
	term_takeRef( result );
	assert( result->type == typeTrue );
	term_deref( result );
	result = eval_string( "(or false false)", c );
	term_takeRef( result );
	assert( result->type == typeFalse );
	term_deref( result );
	result = eval_string( "(and true false)", c );
	term_takeRef( result );
	assert( result->type == typeFalse );
	term_deref( result );
	result = eval_string( "(and true true)", c );
	term_takeRef( result );
	assert( result->type == typeTrue );
	term_deref( result );

	term* vec = _eval( lisp_parse_string( "(vector 0.0 0.0 1.0)" ), c );
	term_takeRef( vec );
	assert( isType( vec, _typeVector ));
	term_deref( vec );

	term* property_color = _eval( lisp_parse_string( "(color (vector 0.0 0.0 1.0))" ), c );
	(void)property_color;

	define_cfunction( c, "new", lisp_func_new );
	define_cfunction( c, "object_process", lisp_func_object_process );
	define_cfunction( c, "test_a", lisp_func_test_struct_a );
	define_cfunction( c, "test_b", lisp_func_test_struct_b );
	define_cfunction( c, "tail", lisp_func_tail );
	define_cfunction( c, "head", lisp_func_head );

	// Test Head and Tail
	term* h = _eval( lisp_parse_string( "(head (quote (1.0 2.0)))" ), c );
	term_takeRef( h );
	vAssert( *h->number == 1.f );
	term_deref( h );

	h = _eval( lisp_parse_string( "(head (tail (quote (1.0 2.0))))" ), c );
	term_takeRef( h );
	vAssert( *h->number == 2.f );
	term_deref( h );

	term* test = _eval( lisp_parse_string( "(new (quote test_struct))" ), c );
	vAssert( isType( test, _typeObject ));
	test_struct* object = test->data;
	//printf( "object: v ( %.2f %.2f %.2f ), a %.2f, b %.2f\n", object->v.coord.x, object->v.coord.y, object->v.coord.z, object->a, object->b );

	term* test_b = _eval( lisp_parse_string( "(object_process (new (quote test_struct)) (quote (test_a 1.0)))" ), c );
	object = test_b->data;
	//printf( "object: v ( %.2f %.2f %.2f ), a %.2f, b %.2f\n", object->v.coord.x, object->v.coord.y, object->v.coord.z, object->a, object->b );

	term* test_c = _eval( lisp_parse_string( "(foldl object_process (new (quote test_struct)) (quote ((test_a 2.0) (test_b 3.0))))" ), c );
	object = test_c->data;
	//printf( "object: v ( %.2f %.2f %.2f ), a %.2f, b %.2f\n", object->v.coord.x, object->v.coord.y, object->v.coord.z, object->a, object->b );
	(void)object;

	// Model
	// (model (mesh (filename "dat/model/cityscape.obj" ))	)
	//
	// Create a mesh with that filename
	// ( args ) (if (list-contains (filename)) (mesh_loadObj (find filename args)) (false))
	// create a model with that mesh
	// ( meshes ) (foldl add_mesh (model_create (count meshes )) meshes )

	// Scene?


	//term* test = _eval( lisp_parse_string( "(test_struct (a 1.0) (b -2.0) (v (vector 1.0 2.0 3.0)))" ), c );
	//(void)test;

	// Need to translate this into:
	/* 
	   (foldl object_process (new test_struct)
	   		((a 1.0) (b -2.0) (v (vector 1.0 2.0 3.0))))
	*/

	// The final test!
	/*
	test_struct* object = _eval( lisp_parse_string( "(test_struct ((color (vector 0.0 0.0 1.0)) (size 1.0) (lifetime 2.0)))" ));
	assert( object->size == 1.f );
	assert( object->lifetime == 2.f );
	assert( vector_equal( object->color, vector( 0.f, 0.f, 1.f, 1.f )));
	*/
	
	term* t = lisp_parse_file( "src/script/lisp/particle.s" );
	// just load the definitions in the file
	_eval_list( t, c );

	// now use them
	term* tp = _eval( lisp_parse_string( "(property (quote ((0.1 1.1 1.0 1.0) ( 0.2 2.0 1.0 1.0) (3.0 5.0 1.0 1.0)) ))" ), c );

	property* p = tp->data;
	(void)p;
	vAssert( p->stride == 4 );

	vAssert( 0 );

	context_delete( c );

	heap_dumpUsedBlocks( lisp_heap );

	printf( "Lisp heap storing %d bytes in %d allocations.\n", lisp_heap->total_allocated, lisp_heap->allocations );
	printf( "Context heap storing %d bytes in %d allocations.\n", context_heap->total_allocated, context_heap->allocations );
	//assert( 0 );
	}

/*
	Garbage Collecting:

	Using a refcount. Refcount (RC) is incremented when something acquires a reference to it,
	then decremented when that reference is lost.

	When refcount is 0, the term is deleted.

	A reference can be held by:
	a list - holds references to all the sub-items it holds.
		rc is incremented on creation of the list, decremented on destruction
   */
