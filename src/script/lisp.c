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

/*
   types of list term
   */
enum termType {
	typeList,
	typeAtom
	}

/*
	A lisp term. Has a type to define what variable is stored in [head];
	If it's a list, then [tail] stores the next item in the list
	If it's not a list, then [tail] is undefined (should not be used; likely NULL)
   */
struct term {
	termType type;
	void* head;
	term* tail;
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
	assert( !list->tail || isType( list, list->tail ));
	return list->tail;
	}

/*
   term accessors
   */
void* value( term* atom ) {
	assert( atom );
	assert( !isType( atom, typeList ));
	return atom->head;
	}

typedef map<ATOM, FUNC> context;

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
void* eval( term* expr ) {
	// Eval arguments, then pass arguments to the binding of the first element and run
	if ( isType( expr, typeAtom )) {
		return context[expr];
		}
	if ( isType( expr, typeValue )) {
		return expr;	// Do we return as a term of typeValue, or just return the value itself? Probably the first, for macros and hijinks
		}
	if ( isType( expr, typeList )) {
		e = map( eval, expr );
		head( e )( tail( e ));
		}
	}

/*
	Def - creates a context binding

	Used to assign variables, including functions
	(def my_func (a)
		(append a))
   */
void def( context* c, ATOM a, void* value ) {
	c[a] = value;
	}

void if () {

}

/*
   Lisp unit tests
   */
void test_lisp() {
	term* script = eval( parse( "(a b)" ));
	// Type tests
	assert( isType( script, typeList ));
	assert( isType( head( script ), typeAtom ));
	assert( isType( tail( script ), typeList ));
	assert( isType( head( tail( script )), typeAtom ));
	// Value tests
	assert( value( head( script )) == a );
	assert( value( head( tail( script )) == b );
	}
