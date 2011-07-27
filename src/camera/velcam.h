// velcam.h
// A velocity-based flycam
#pragma once
#include "physic.h"
#include "engine.h"

typedef struct velcam_s {
	transform* trans;
	physic* phys;
//	matrix	transform;
	vector	euler;
	vector	translation;
	camera*	camera_target;
	vector	pan_sensitivity;
	vector	track_sensitivity;
} velcam;

typedef struct velcam_input_s {
	vector	pan;		// Rotation
	vector	track;		// Movement
}velcamInput;

// velcam constructor
velcam* velcam_create( engine* e );

// Set the camera target to output frame data to
void velcam_setTarget( velcam* f, camera* c );

// Update the velcam, setting the target data to latest
void velcam_tick( velcam* f, float dt );

void velcam_input( velcam* cam, input* in  );
