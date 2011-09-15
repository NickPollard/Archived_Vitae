// modelinstance.c

#include "common.h"
#include "modelinstance.h"
//-----------------------
#include "model.h"
#include "render/render.h"
#include "transform.h"

IMPLEMENT_POOL( modelInstance )

pool_modelInstance* static_modelInstance_pool = NULL;

void modelInstance_initPool() {
	static_modelInstance_pool = pool_modelInstance_create( 256 );
}

modelInstance* modelInstance_createEmpty( ) {
	modelInstance* i = pool_modelInstance_allocate( static_modelInstance_pool );
	memset( i, 0, sizeof( modelInstance ));
	return i;
}

modelInstance* modelInstance_create( modelHandle m ) {
	modelInstance* i = modelInstance_createEmpty();
	i->model = m;
	return i;
}

typedef struct aabb_s {
	vector min;
	vector max;
} aabb;

aabb aabb_calculate( int vert_count, vector* verts ) {
	vAssert( vert_count > 1 );
	aabb bb;
	bb.min = verts[0];
	bb.max = verts[0];
	for ( int i = 1; i < vert_count; i++ ) {
		bb.min = vector_min( &bb.min, &verts[i] );
		bb.max = vector_max( &bb.max, &verts[i] );
	}
	return bb;
}

void test_aabb_calculate( ) {
	vector verts[] = {
		{{ 1.f, 0.f, -1.f, 1.f }},
		{{ -1.f, 1.f, 0.f, 1.f }},
		{{ 0.f, -1.f, 1.f, 1.f }}
	};
	aabb bb = aabb_calculate( 3, verts );
	printf( "AABB: ( " );
	vector_print( &bb.min );
	printf( " ) to ( " );
	vector_print( &bb.max );
	printf( ")\n" );
	vector v_min = Vector( -1.f, -1.f, -1.f, 1.f );
	vector v_max = Vector( 1.f, 1.f, 1.f, 1.f );
	vAssert( vector_equal( &bb.min, &v_min ));
	vAssert( vector_equal( &bb.max, &v_max ));
}

void modelInstance_calculateBoundingBox( modelInstance* instance ) {
	aabb bb;
	bb.min = Vector( 0.0, 0.0, 0.0, 1.0 );
	bb.max = Vector( 0.0, 0.0, 0.0, 1.0 );
	mesh* m = model_fromInstance( instance )->meshes[0];
	bb = aabb_calculate( m->vert_count, m->verts );
	printf( "AABB: ( " );
	vector_print( &bb.min );
	printf( " ) to ( " );
	vector_print( &bb.max );
	printf( ")\n" );
}


void modelInstance_draw( modelInstance* instance ) {
	// Bounding box cull
//	if ( frustum_cull( bounding_box, frustum ) )
//		return;

	modelInstance_calculateBoundingBox( instance );

	render_resetModelView();
	matrix_mul( modelview, modelview, instance->trans->world );
	render_setUniform_matrix( *resources.uniforms.modelview, modelview );

	model_draw( model_fromInstance( instance ) );
}
