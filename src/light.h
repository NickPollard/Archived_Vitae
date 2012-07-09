// light.h
#pragma once

#include "common.fwd.h"
#include "maths/maths.h"

#define DEBUG_RENDER_LIGHTS 0

#define MAX_RENDER_LIGHTS 2


// *** Light ***
struct light_s {
	transform*	trans;
	color		diffuse_color;
	color		specular_color;
	float		attenuationConstant;
	float		attenuationLinear;
	float		attenuationQuadratic;
};

light* light_create();

light* light_createWithTransform(scene* s);

void light_setAttenuation(light* l, float constant, float linear, float quadratic);

// Set the diffuse component for the light
void light_setDiffuse(light* l, float r, float g, float b, float a);

// Set the specular component for the light
void light_setSpecular( light* l, float r, float g, float b, float a );

void light_renderLights( int count, light** lights );

void light_setPosition(light* l, vector* pos);
