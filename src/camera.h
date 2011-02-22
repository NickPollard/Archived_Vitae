// camera.h
#ifndef __CAMERA_H__
#define __CAMERA_H__

struct camera_s {
	transform* trans;
//	float focalLength;
//	float aperture;
};

camera* camera_create();

camera* camera_createWithTransform(scene* s);

vector* camera_getTranslation(camera* c);

void camera_setTranslation(camera* c, vector* v);

#endif // __CAMERA_H__