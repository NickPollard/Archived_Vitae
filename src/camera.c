// camera.c

#include "common.h"
#include "camera.h"
//---------------------
#include "maths.h"
#include "transform.h"


camera* camera_create() {
	camera* c = malloc(sizeof(camera));
	c->trans = NULL;
	return c;
}

camera* camera_createWithTransform( scene* s ) {
	camera* c = camera_create();
	c->trans = transform_create( s );
	return c;
}

vector* camera_getTranslation(camera* c) {
	return matrix_getTranslation(&c->trans->local);
}

void camera_setTranslation(camera* c, vector* v) {
	matrix_setTranslation(&c->trans->local, v);
}
