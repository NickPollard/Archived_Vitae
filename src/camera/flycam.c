// flycam.c

#include "common.h"
#include "flycam.h"
//---------------------
#include "camera.h"
#include "input.h"
#include "transform.h"
#include "mem/allocator.h"

// Flycam constructor
flycam* flycam_create() {
	flycam* f = mem_alloc( sizeof( flycam ));
	f->pan_sensitivity = Vector(1.f, 1.f, 1.f, 0.f);
	f->track_sensitivity = Vector(1.f, 1.f, 1.f, 0.f);
	matrix_setIdentity( &f->transform );
	return f;
}

void flycam_process( flycam* cam, flycamInput* in );

void flycam_input( flycam* cam, input* in  ) {
	flycamInput fly_in;
	fly_in.pan = Vector( 0.f, 0.f, 0.f, 0.f );
	fly_in.track = Vector( 0.f, 0.f, 0.f, 0.f );
	if ( input_keyHeld( in, KEY_UP ))
		fly_in.track.coord.y = 0.01f;
	if ( input_keyHeld( in, KEY_DOWN ))
		fly_in.track.coord.y = -0.01f;
	if ( input_keyHeld( in, KEY_LEFT ))
		fly_in.track.coord.x = -0.01f;
	if ( input_keyHeld( in, KEY_RIGHT ))
		fly_in.track.coord.x = 0.01f;
/*	printf( "Flycam input: pan: %.2f, %.2f, %.2f, %.2f\n", fly_in.track.coord.x, 
															fly_in.track.coord.y,
															fly_in.track.coord.z,
															fly_in.track.coord.w );*/
	flycam_process( cam, &fly_in );
}

// Read in an input structure
void flycam_process( flycam* cam, flycamInput* in ) {
	matrix_print( &cam->transform );
	vector translation = matrixVecMul( &cam->transform, &in->track );
	printf( "cam translation: %.2f, %.2f\n", translation.coord.x, translation.coord.y );
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
