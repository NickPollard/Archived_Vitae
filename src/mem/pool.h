// pool.h

// An object pool, for fixed size objects

#pragma once

#include "transform.h"

/*

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

*/

#define DECLARE_POOL( type )				\
typedef struct pool_##type##_s {		\
	int				size;					\
	bool*			free;					\
	type*	pool;					\
} pool_##type;						\
											\
pool_##type* pool_##type##_create( int size );					\
type* pool_##type##_allocate( pool_##type* pool );		\
void pool_##type##_free( pool_##type* pool, type* m );

DECLARE_POOL( modelInstance )
DECLARE_POOL( transform )

// *** Test

// TEST
void test_pool();
