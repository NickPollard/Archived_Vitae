// light.h
#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "common.fwd.h"
#include "maths.h"

// *** Light ***
struct light_s {
	transform*	trans;
	color		diffuseCol;
	float		attenuationConstant;
	float		attenuationLinear;
	float		attenuationQuadratic;
};

light* light_create();

void light_setAttenuation(light* l, float constant, float linear, float quadratic);

void light_setDiffuse(light* l, float r, float g, float b, float a);

void light_render(GLenum index, light* l);

void light_setPosition(light* l, vector* pos);

#endif // __LIGHT_H__
