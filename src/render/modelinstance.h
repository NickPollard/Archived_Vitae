// modelinstance.h

#pragma once
#include "mem/pool.h"
#include "maths/maths.h"

#include "model.h"

/*
	ModelInstance

	A ModelInstance represents a unique instance of a model to be rendered; multiple
	modelInstances can have the same model (and therefore appearance), but can be
	in different positions/situations
   */

typedef struct aabb_s {
	vector min;
	vector max;
} aabb;

struct modelInstance_s {
	modelHandle	model;
	transform* trans;
	// Sub Elements
	int		transform_count;
	int		emitter_count;
	transform*			transforms[kMaxSubTransforms];
	particleEmitter*	emitters[kMaxSubEmitters];
	aabb	bb;
};

DECLARE_POOL( modelInstance )

void modelInstance_initPool();

modelInstance* modelInstance_createEmpty( );
modelInstance* modelInstance_create( modelHandle m );
void modelInstance_delete( modelInstance* t );

void modelInstance_draw( modelInstance* instance, camera* cam );

void test_aabb_calculate();
