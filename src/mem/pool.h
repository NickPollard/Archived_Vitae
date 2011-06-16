// pool.h

// An object pool, for fixed size objects

#pragma once

typedef struct pool_modelInstance_s {
	int				size;
	bool*			free;
	modelInstance*	pool;
} pool_modelInstance;

pool_modelInstance* pool_modelInstance_create( int size );

// Allocate an object from the pool
// Currently linear time O(n), as it iterates through.
// Could improve this with one of the linked list solutions I have
// TODO: implement that
modelInstance* pool_modelInstance_allocate( pool_modelInstance* pool );

// Free an object
// Currently constant time O(1)
void pool_modelInstance_free( pool_modelInstance* pool, modelInstance* m );

// TEST
void test_pool();
