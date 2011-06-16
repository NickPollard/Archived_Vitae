// pool.c

// An object pool, for fixed size objects

#include "src/common.h"
#include "pool.h"
//---------------------
#include "allocator.h"
#include "render/modelinstance.h"

/*
pool_modelInstance* pool_modelInstance_create( int size ) {
	void* data = mem_alloc( sizeof( pool_modelInstance ) + 
							sizeof( bool ) * size +
							sizeof( modelInstance ) * size );
	pool_modelInstance* p = data;
	p->size = size;
	p->free = data + sizeof( pool_modelInstance );
	p->pool = data + sizeof( pool_modelInstance ) + sizeof( bool ) * size;
	// init free table
	memset( p->free, 1, sizeof( bool ) * size );
	return p;
}

// Allocate an object from the pool
// Currently linear time O(n), as it iterates through.
// Could improve this with one of the linked list solutions I have
// TODO: implement that
modelInstance* pool_modelInstance_allocate( pool_modelInstance* pool ) {
	for ( int i = 0; i < pool->size; i++) {
		if ( pool->free[i] ) {
			pool->free[i] = false;
			return &pool->pool[i];
		}
	}
	printf( "Pool is full; cannot allocate new object.\n" );
	assert( 0 );
	return NULL;
}

// Free an object
// Currently constant time O(1)
void pool_modelInstance_free( pool_modelInstance* pool, modelInstance* m ) {
	int index = m - pool->pool;
	assert( index >= 0 );
	assert( index < pool->size );
	pool->free[index] = true;	
}
*/

#define IMPLEMENT_POOL( type )											\
																		\
pool_##type* pool_##type##_create( int size ) {							\
	void* data = mem_alloc( sizeof( pool_##type ) + 					\
							sizeof( bool ) * size +						\
							sizeof( type ) * size );					\
	pool_##type* p = data;												\
	p->size = size;														\
	p->free = data + sizeof( pool_##type );								\
	p->pool = data + sizeof( pool_##type ) + sizeof( bool ) * size;		\
	memset( p->free, 1, sizeof( bool ) * size );						\
	return p;															\
}																		\
type* pool_##type##_allocate( pool_##type* pool ) {						\
	for ( int i = 0; i < pool->size; i++) {								\
		if ( pool->free[i] ) {											\
			pool->free[i] = false;										\
			return &pool->pool[i];										\
		}																\
	}																	\
	printf( "Pool is full; cannot allocate new object.\n" );			\
	assert( 0 );														\
	return NULL;														\
}																		\
void pool_##type##_free( pool_##type* pool, type* m ) {					\
	int index = m - pool->pool;											\
	assert( index >= 0 );												\
	assert( index < pool->size );										\
	pool->free[index] = true;											\
}

IMPLEMENT_POOL( modelInstance )
IMPLEMENT_POOL( transform )

// *** Test

void test_pool() {
	pool_modelInstance* p = pool_modelInstance_create( 4 );
	modelInstance* m[8];
	int count = 4;
	for ( int i = 0; i < count; i++ )   
		m[i] = pool_modelInstance_allocate( p );
	for ( int i = 0; i < count; i++ )   
		pool_modelInstance_free( p, m[i] );

	count = 8;
	for ( int i = 0; i < count; i++ )   {
		m[i] = pool_modelInstance_allocate( p );
		pool_modelInstance_free( p, m[i] );
	}

	pool_transform* pt = pool_transform_create( 4 );
	(void)pt;
}
