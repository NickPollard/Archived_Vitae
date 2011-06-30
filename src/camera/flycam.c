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
	matrix_setIdentity( f->transform );
	f->translation = Vector( 0.f, 0.f, 10.f, 1.f );
	f->euler = Vector( 0.f, 0.f, 0.f, 0.f );
	return f;
}

void flycam_process( flycam* cam, flycamInput* in );

void flycam_input( flycam* cam, input* in  ) {
	flycamInput fly_in;
	fly_in.pan = Vector( 0.f, 0.f, 0.f, 0.f );
	fly_in.track = Vector( 0.f, 0.f, 0.f, 0.f );
	float delta = 0.01f;

	if ( input_keyHeld( in, KEY_SHIFT ) )
		delta = 0.1f;

	if ( input_keyHeld( in, KEY_W ))
		fly_in.track.coord.z = -delta;
	if ( input_keyHeld( in, KEY_S ))
		fly_in.track.coord.z = delta;
	if ( input_keyHeld( in, KEY_UP ) || input_keyHeld( in, KEY_Q ))
		fly_in.pan.coord.y = -0.01f;
	if ( input_keyHeld( in, KEY_DOWN ) || input_keyHeld( in, KEY_E ))
		fly_in.pan.coord.y = 0.01f;
	if ( input_keyHeld( in, KEY_LEFT ) || input_keyHeld( in, KEY_A ))
		fly_in.track.coord.x = -delta;
	if ( input_keyHeld( in, KEY_RIGHT ) || input_keyHeld( in, KEY_D ))
		fly_in.track.coord.x = delta;
/*	printf( "Flycam input: track: %.2f, %.2f, %.2f, %.2f\n", fly_in.track.coord.x, 
															fly_in.track.coord.y,
															fly_in.track.coord.z,
															fly_in.track.coord.w );*/
	flycam_process( cam, &fly_in );
}

// Read in an input structure
void flycam_process( flycam* cam, flycamInput* in ) {
	/*
	// *** Relative
	vector translation = matrixVecMul( cam->transform, &in->track );
	matrix cam_delta;
	matrix_fromEuler( cam_delta, &in->pan );
	
	// Combine both changes into one matrix
	matrix_setTranslation( cam_delta, &translation );
//	matrix cam_delta;
//	matrix_fromRotTrans( &cam_delta, &rotation, &translation );
	
	// Update the camera transform by the matrix
	matrix_mul( cam->transform, cam->transform, cam_delta );
*/
	// *** Absolute
	Add( &cam->euler, &cam->euler, &in->pan );
	matrix_fromEuler( cam->transform, &cam->euler );
	matrix inverse;
	matrix_inverse( inverse, cam->transform );
	vector translation_delta = matrixVecMul( inverse, &in->track );
	Add( &cam->translation, &cam->translation, &translation_delta );
	matrix_setTranslation( cam->transform, &cam->translation );
}

// Set the camera target to output frame data to
void flycam_setTarget( flycam* f, camera* c ) {
	f->camera_target = c;
}

// Update the flycam, setting the target data to latest
void flycam_tick( flycam* f, float dt ) {
	transform_setWorldSpace( f->camera_target->trans, f->transform );
}
