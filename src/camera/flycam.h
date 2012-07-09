// flycam.h
#pragma once

#include "maths/maths.h"
#include "maths/matrix.h"
#include "maths/vector.h"
#include "camera.h"

//
// A debug FlyCam inplementation
//
// Use for free-form flying around the scene for debug purposes
//

typedef struct flycam_s {
	// Camera must come first
	camera	camera_target;
	matrix	transform;
	vector	euler;
	vector	translation;
	vector	pan_sensitivity;
	vector	track_sensitivity;
} flycam;

typedef struct flycam_input_s {
	vector	pan;		// Rotation
	vector	track;		// Movement
}flycamInput;

// Flycam constructor
flycam* flycam_create();

/*
// Set the camera target to output frame data to
void flycam_setTarget( flycam* f, camera* c );
*/

// Update the flycam, setting the target data to latest
void flycam_tick( void* data, float dt );

void flycam_input( void* data, input* in  );
