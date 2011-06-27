// flycam.c

#include "common.h"
#include "flycam.h"
//---------------------
#include "camera.h"
#include "transform.h"
#include "mem/allocator.h"

// Flycam constructor
flycam* flycam_create() {
	flycam* f = mem_alloc( sizeof( flycam ));
	f->pan_sensitivity = Vector(1.f, 1.f, 1.f, 0.f);
	f->track_sensitivity = Vector(1.f, 1.f, 1.f, 0.f);
	return f;
}


// Read in an input structure
void flycam_input( flycam* cam, flycamInput* in ) {
	vector translation = matrixVecMul( &cam->transform, &in->track );
//	quaternion rotation = quaternion_fromEuler( &in->pan );
	matrix cam_delta;
	matrix_fromEuler( &cam_delta, &in->pan );
	
	// Combine both changes into one matrix
	matrix_setTranslation( &cam_delta, &translation );
//	matrix cam_delta;
//	matrix_fromRotTrans( &cam_delta, &rotation, &translation );
	
	// Update the camera transform by the matrix
	matrix_mul( &cam->transform, &cam->transform, &cam_delta );
}

// Set the camera target to output frame data to
void flycam_setTarget( flycam* f, camera* c ) {
	f->camera_target = c;
}

// Update the flycam, setting the target data to latest
void flycam_tick( flycam* f, float dt ) {
	camera_setTranslation( f->camera_target, matrix_getTranslation( &f->transform ));
//	transform_setWorldSpace( f->camera_target->trans, &f->transform );
}
