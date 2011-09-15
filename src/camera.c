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

const vector* camera_getTranslation( camera* c ) {
	return matrix_getTranslation( c->trans->local );
}

void camera_setTranslation( camera* c, const vector* v ) {
	matrix_setTranslation( c->trans->local, v );
}

// Calculate the planes of the view frustum defined by the camera *c*
void camera_calculateFrustum( camera* c, vector* frustum ) {
	vAssert( frustum );
	// 6 planes: Front, Back, Top, Bottom, Left, Right
	// We can define a plane in a standard 4-element vector:
	// First 3 elements are the normal, last is the D value

	vector v = Vector( 0.0, 0.0, 1.0, 0.0 ); // 0.0 w coordinate since vector not point
	vector view = matrixVecMul( c->trans->world, &v );
	vector p = *matrix_getTranslation( c->trans->world );

	vector front = view;
	front.coord.w = Dot( &p, &view ) + c->near;
	vector back = view;
	back.coord.w = Dot( &p, &view ) + c->far;
/*
	printf( "Near plane: " );
	vector_print( &front );
	printf( ".\nFar Plane: " );
	vector_print( &back );
	printf( "\n" );
*/	
	frustum[0] = front;
	frustum[1] = back;
}
