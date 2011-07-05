// light.c

#include "common.h"
#include "light.h"
//---------------------
#include "transform.h"
#include "mem/allocator.h"
#include "render/debugdraw.h"
#include <assert.h>

light* light_create() {
	light* l = mem_alloc(sizeof(light));
	light_setDiffuse(l, 1.f, 1.f, 1.f, 1.f);
	light_setAttenuation(l, 1.f, 1.f, 0.f);
	return l;
}

light* light_createWithTransform(scene* s) {
	light* l = light_create();
	transform* t = transform_createAndAdd( s );
	l->trans = t;
	return l;
}

void light_setAttenuation(light* l, float constant, float linear, float quadratic) {
	l->attenuationConstant = constant;
	l->attenuationLinear = linear;
	l->attenuationQuadratic = quadratic;
}

void light_setDiffuse(light* l, float r, float g, float b, float a) {
	l->diffuseCol.val[0] = r;
	l->diffuseCol.val[1] = g;
	l->diffuseCol.val[2] = b;
	l->diffuseCol.val[3] = a;
}

void light_setPosition(light* l, vector* pos) {
	assert(l->trans != NULL);
	matrix_setTranslation(l->trans->local, pos);
}

void light_render(GLenum index, light* l) {
	glEnable( index );

	glLightfv(index, GL_DIFFUSE, (GLfloat*)(&l->diffuseCol));
	glLightfv(index, GL_POSITION, (GLfloat*)matrix_getTranslation(l->trans->world));

	// Attenuation
	glLightf(index, GL_CONSTANT_ATTENUATION, (GLfloat)l->attenuationConstant);
	glLightf(index, GL_LINEAR_ATTENUATION, (GLfloat)l->attenuationLinear);
	glLightf(index, GL_QUADRATIC_ATTENUATION, (GLfloat)l->attenuationQuadratic);
	
	debugdraw_cross( matrix_getTranslation( l->trans->world ), 1.f);
}
