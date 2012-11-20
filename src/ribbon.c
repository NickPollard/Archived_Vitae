// ribbon.c
#include "src/common.h"
#include "src/ribbon.h"
//---------------------
#include "transform.h"
#include "maths/matrix.h"
#include "mem/allocator.h"

ribbonEmitter* ribbonEmitter_create() {
	ribbonEmitter* r = mem_alloc( sizeof( ribbonEmitter ));
	memset( r, 0, sizeof( ribbonEmitter ));
	return r;
}

void ribbonEmitter_tick( void* emitter_void, float dt, engine* eng ) {
	(void)dt; (void)eng;
	ribbonEmitter* r = emitter_void;
	// Emit a new ribbon vertex pair
	int vertex_last = r->vertex_first + r->vertex_count % kMaxRibbonPairs;

	r->vertex_array[vertex_last][0] = matrix_vecMul( r->trans->world, &r->begin );
	r->vertex_array[vertex_last][1]	= matrix_vecMul( r->trans->world, &r->end );

	if ( r->vertex_count < kMaxRibbonPairs ) {
		++r->vertex_count;
	}
	else {
		++r->vertex_first;
	}

}

void ribbonEmitter_render( void* emitter_void ) {
	ribbonEmitter* r = emitter_void;
	(void)r;
}
