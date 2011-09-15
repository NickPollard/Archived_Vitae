// camera.h
#ifndef __CAMERA_H__
#define __CAMERA_H__

struct camera_s {
	transform* trans;
	// z-clipping
	float near;
	float far;
//	float focalLength;
//	float aperture;
};

camera* camera_create();

camera* camera_createWithTransform(scene* s);

const vector* camera_getTranslation(camera* c);

void camera_setTranslation(camera* c, const vector* v);

// Calculate the planes of the view frustum defined by the camera *c*
void camera_calculateFrustum( camera* c, vector* frustum );

#endif // __CAMERA_H__
