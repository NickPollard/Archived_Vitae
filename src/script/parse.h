// parse.h
#pragma once
#include "system/file.h"

// *** S-Expressions

enum tag_type {
	typeNull,
	typeList,
	typeAtom,
	typeString,
	typeNumber,
	typeFunc,
	typeModel,
	typeLight,
	typeTransform,
	typeVector
};

typedef struct sterm_s sterm;

struct sterm_s {	
	int		type;
	void*	head;
	sterm*	tail;
};

// *** Script Function type
typedef void* (*script_func)( sterm* s );

// *** sterm

void sterm_free( sterm* s );


// *** Parsing

void	parse_init();
sterm*	parse( inputStream* stream );
sterm*	parse_string( const char* string );
sterm*	parse_file( const char* filename );

// *** Evaluation

void* eval( sterm* data );
