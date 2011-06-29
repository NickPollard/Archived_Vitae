// camera.c

#include "common.h"
#include "camera.h"
//---------------------
#include "maths.h"
#include "transform.h"
#include "mem/allocator.h"


camera* camera_create() {
	camera* c = mem_alloc( sizeof( camera ));
	c->trans = NULL;
	return c;
}

camera* camera_createWithTransform( scene* s ) {
	camera* c = camera_create();
	c->trans = transform_createAndAdd( s );
	return c;
}

const vector* camera_getTranslation(camera* c) {
	return matrix_getTranslation( c->trans->local );
}

void camera_setTranslation(camera* c, const vector* v) {
	matrix_setTranslation( c->trans->local, v );
}
