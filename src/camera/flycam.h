// flycam.h
#pragma once

#include "maths.h"

//
// A debug FlyCam inplementation
//
// Use for free-form flying around the scene for debug purposes
//

typedef struct flycam_s {
	matrix	transform;
	camera*	camera_target;
	vector	pan_sensitivity;
	vector	track_sensitivity;
} flycam;

typedef struct flycam_input_s {
	vector	pan;		// Rotation
	vector	track;		// Movement
}flycamInput;

// Flycam constructor
flycam* flycam_create();

// Set the camera target to output frame data to
void flycam_setTarget( flycam* f, camera* c );

// Update the flycam, setting the target data to latest
void flycam_tick( flycam* f, float dt );

void flycam_input( flycam* cam, input* in  );
