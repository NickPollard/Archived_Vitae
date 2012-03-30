// lisp.h
#pragma once

#define DEBUG_PARSE 0
#define DEBUG_CONTEXT 0

#if DEBUG_PARSE
#define PARSE_PRINT( ... ) printf( __VA_ARGS__ )
#else
#define PARSE_PRINT( ... ) 
#endif

#if DEBUG_CONTEXT
#define CONTEXT_PRINT( ... ) printf( __VA_ARGS__ )
#else
#define CONTEXT_PRINT( ... ) 
#endif



/*
   types of list term
   all terms aside from LIST and ATOM are considered to be value terms
   */
enum termType {
	_typeList,
	_typeAtom,
	_typeString,
	typeFloat,
	_typeVector,
	typeIntrinsic,
	_typeObject,
	typeFalse,		// Special FALSE datatype
	typeTrue		// Special TRUE datatype
	};

/*
	A lisp term. Has a type to define what variable is stored in HEAD;
	If it's a list, then TAIL stores the next item in the list
	If it's not a list, then TAIL is undefined (should not be used; likely NULL)
   */
struct term_s;
typedef struct term_s term;
struct term_s {
	enum termType type;
	union {
		term* head;
		void* data;
		char* string;
		float* number;
	};
	term* tail;
	int refcount;
	};

/*
   Execution context
   */
typedef struct context_s {
	struct context_s* parent;
	map* lookup;
	} context;

// Evaluate a lisp expression
term* _eval( term* expr, void* _context );

// Return a new lisp execution context
context* lisp_newContext();

// Parse the contents of [filename] as lisp code
term* lisp_parse_file( const char* filename );

// Init the lisp subsystem
void lisp_init();

// Take and hold a reference to term [t]
void term_takeRef( term* t );

// Release a held reference to term [t]
void term_deref( term* t );
