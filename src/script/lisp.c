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
#include "mem/allocator.h"
#include "system/hash.h"
#include "system/file.h"
#include "system/string.h"
#include <assert.h>

/*
   types of list term
   */
enum termType {
	typeList,
	typeAtom,
	typeValue
	};

/*
	A lisp term. Has a type to define what variable is stored in [head];
	If it's a list, then [tail] stores the next item in the list
	If it's not a list, then [tail] is undefined (should not be used; likely NULL)
   */
struct term_s;
typedef struct term_s term;
struct term_s {
	enum termType type;
	void* head;
	term* tail;
	};

term* term_create( enum termType type, void* value ) {
	term* t = mem_alloc( sizeof( term ));
	t->type = type;
	t->head = value;
	t->tail = NULL;
	return t;
	}

int _isListStart( char c ) {
	return c == '(';
}

int _isListEnd( char c ) {
	return c == ')';
}
// Read a token at a time from the inputstream, advancing the read head,
// and build it into an slist of atoms
term* lisp_parse( inputStream* stream ) {
	if ( inputStream_endOfFile( stream )) {
		printf( "ERROR: End of file reached during lisp_parse, with incorrect number of closing parentheses.\n" );
		vAssert( 0 );
		return NULL;
	}
	char* token = inputStream_nextToken( stream );
	if ( _isListEnd( *token ) ) {
		mem_free( token ); // It's a bracket, discard it
		return NULL;
	}
	if ( _isListStart( *token ) ) {
		mem_free( token ); // It's a bracket, discard it
		term* list = term_create( typeList, NULL );
		term* s = list;

		s->head = lisp_parse( stream );
		if ( !s->head ) { // The Empty list () 
			return list;
		}

		while ( true ) {
			term* sub_expr = lisp_parse( stream );				// parse a subexpr
			if ( sub_expr ) {								// If a valid return
				s->tail = term_create( typeList, NULL );	// Add it to the tail
				s = s->tail;
				s->head = sub_expr;
			} else {
				return list;
			}
		}
	}
	// When it's an atom, we keep the token, don't free it
	return term_create( typeAtom, (void*)token );
}

term* lisp_parse_string( const char* string ) {
	inputStream* stream = inputStream_create( string );
	term* s = lisp_parse( stream );
	mem_free( stream );
	return s;
}

term* lisp_parse_file( const char* filename ) {
	printf( "FILE: Loading File \"%s\" for parsing.\n", filename );
	size_t length = 0;
	char* contents = vfile_contents( filename, &length );
	vAssert( (contents) );
	vAssert( (length != 0) );
	term* s = lisp_parse_string( contents );
	mem_free( contents );
	return s;
}

bool isType( term* t, enum termType type ) {
	return t->type == type;
	}

/*  
   List accessors

   Only valid on lists
   list must not be NULL

   Head - returns the head pointer of a list, which must be another term
   Tail - returns the tail pointer of a list, which must be another term of typeList, or NULL;
   */

term* head( term* list ) {
	assert( list );
	assert( isType( list, typeList ));
	assert( list->head );
	return (term*)list->head;
	}

term* tail( term* list ) {
	assert( list );
	assert( isType( list, typeList ));
	// If there is a tail, it must be a list
	assert( !list->tail || isType( list->tail, typeList ));
	return list->tail;
	}

/*
   List Constructors
   */
term* _cons( void* head, term* tail ) {
	term* t = mem_alloc( sizeof( term ));
	t->type = typeList;
	t->head = head;
	t->tail = tail;
	return t;
	}

/*
   term accessors
   */
void* value( term* atom ) {
	assert( atom );
	assert( !isType( atom, typeList ));
	return atom->head;
	}

typedef map context;
typedef void* (*fmap_func)( term*, void* );
typedef void* (*lisp_func)( term* );

term* fmap_1( fmap_func f, void* arg, term* expr ) {
	if ( !expr )
		return NULL;
	return _cons( f( head( expr ), arg ), fmap_1( f, arg, tail( expr )));
	}

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

void* _eval( term* expr, void* _context ) {
	// Eval arguments, then pass arguments to the binding of the first element and run
	if ( isType( expr, typeAtom )) {
		printf( "Found atom: %s\n", (const char*)( expr->head ));
		return map_find(_context, mhash((const char*)( expr->head )));
		}
	if ( isType( expr, typeValue )) {
		return expr;	// Do we return as a term of typeValue, or just return the value itself? Probably the first, for macros and hijinks
		}
	if ( isType( expr, typeList )) {
		printf( "Found list.\n" );
		term* e = fmap_1( _eval, _context, expr );
		return ((lisp_func)head( e )) ( tail( e ));
		}
	return NULL;
	}

/*
	Def - creates a context binding

	Used to assign variables, including functions
	(def a 10)
	(def my_func (a)
		(append a))

		returns a context with a copy of the old context, plus the new binding
   */
context* def( context* c, const char* atom, void* value ) {
	context * new = mem_alloc( sizeof( context ));
	memcpy( new, c, sizeof( context ));
	map_add(new, mhash(atom), value);
	return new;
	}

/*
void if () {

}
*/

/*
   Lisp unit tests
   */
void test_lisp() {
	//context* c = NULL;
	term* script = lisp_parse_string( "(a b)" );
	// Type tests
	assert( isType( script, typeList ));
	assert( isType( head( script ), typeAtom ));
	assert( isType( tail( script ), typeList ));
	assert( isType( head( tail( script )), typeAtom ));
	// Value tests
	assert( string_equal( value( head( script )) , "a" ));
	assert( string_equal( value( head( tail( script ))) , "b" ));

	//term* test = _eval( lisp_parse_string( "(def a 10)" ), c);
	}
