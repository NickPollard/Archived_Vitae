// camera.c

#include "common.h"
#include "camera.h"
//---------------------
#include "maths.h"
#include "transform.h"
#include "mem/allocator.h"

static const float default_z_near = 1.f;
static const float default_z_far = 500.f;
static const float default_fov = 0.8f; // In Radians

camera* camera_create() {
	camera* c = mem_alloc( sizeof( camera ));
	c->trans = NULL;
	camera_init( c );
	return c;
}

void camera_init( camera* c ) {
	c->z_near = default_z_near;
	c->z_far = default_z_far;
	c->fov = default_fov;
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

	/*
	vector_printf( "cam pos: ", &p );
	vector_printf( "cam dir: ", &view );
*/

	vector front = view;
	front.coord.w = Dot( &p, &view ) + c->z_near;
	vector back = view;
	back.coord.w = Dot( &p, &view ) + c->z_far;

//	vector_printf( "Near plane: ", &front );
//	vector_printf( "Far plane: ", &back );

	frustum[0] = front;
	frustum[1] = back;
}
