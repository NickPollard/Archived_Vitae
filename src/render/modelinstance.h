// modelinstance.h

#pragma once
#include "mem/pool.h"

/*
	ModelInstance

	A ModelInstance represents a unique instance of a model to be rendered; multiple
	modelInstances can have the same model (and therefore appearance), but can be
	in different positions/situations
   */

struct modelInstance_s {
	modelHandle	model;
	transform* trans;
};

DECLARE_POOL( modelInstance )

void modelInstance_initPool();

modelInstance* modelInstance_create( modelHandle m );

void modelInstance_draw( modelInstance* instance );
