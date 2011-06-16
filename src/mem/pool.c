// pool.c

// An object pool, for fixed size objects

#include "src/common.h"
#include "pool.h"
//---------------------
#include "allocator.h"
#include "render/modelinstance.h"

IMPLEMENT_POOL( modelInstance )

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
}
