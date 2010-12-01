// flycam.h
#ifndef __FLYCAM_H__
#define __FLYCAM_H__

//
// A debug FlyCam inplementation
//
// Use for free-form flying around the scene for debug purposes
//

typedef struct flycam_s {
	matrix	transform;
	camera	camera_target;
	vector	pan_sensitivity;
	vector	track_sensitivity;
} flycam;

typedef struct flycam_input_s {
	vector	pan;		// Rotation
	vector	track;		// Movement
} flycam_input;

#endif // __FLYCAM_H__
