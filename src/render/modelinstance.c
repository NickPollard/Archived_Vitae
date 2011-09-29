// modelinstance.c

#include "common.h"
#include "modelinstance.h"
//-----------------------
#include "camera.h"
#include "render/render.h"
#include "particle.h"
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

void modelInstance_createSubTransforms( modelInstance* instance ) {
	model* m = model_fromInstance( instance );
	for ( int i = 0; i < m->transform_count; i++ ) {
		instance->transforms[i] = transform_create();
		matrix_cpy( instance->transforms[i]->local, m->transforms[i]->local );
	}
	instance->transform_count = m->transform_count;
}

void modelInstance_createSubEmitters( modelInstance* instance ) {
	model* m = model_fromInstance( instance );
	for ( int i = 0; i < m->emitter_count; i++ ) {
		instance->emitters[i] = particleEmitter_create();

		// TEST setup particle stuff
		// TODO cleanup particle/particleDef split (inc. creation)
		vAssert( m->emitters[i]->definition );
		mem_free( instance->emitters[i]->definition );

		instance->emitters[i]->definition = m->emitters[i]->definition;

		// Take parent transform if in model
		// This is stored as an index rather than a pointer
		int transform_index = (int)m->emitters[i]->trans;
#define NULL_INDEX -1
		if ( transform_index == NULL_INDEX )
			instance->emitters[i]->trans = NULL;
		else
			instance->emitters[i]->trans = instance->transforms[transform_index];
	}
	instance->emitter_count = m->emitter_count;
}

modelInstance* modelInstance_create( modelHandle m ) {
	modelInstance* instance = modelInstance_createEmpty();
	instance->model = m;

	modelInstance_createSubTransforms( instance );
	modelInstance_createSubEmitters( instance );
	
	vAssert( !instance->trans );

	return instance;
}

aabb aabb_calculate( int vert_count, vector* verts, matrix m ) {
	vAssert( vert_count > 1 );
	aabb bb;
	vector vert = verts[0];
	if ( m )
		vert = matrixVecMul( m, &verts[0] );
	bb.min = vert;
	bb.max = vert;
	for ( int i = 1; i < vert_count; i++ ) {
		vert = verts[i];
		if ( m )
			vert = matrixVecMul( m, &verts[i] );

		bb.min = vector_min( &bb.min, &vert );
		bb.max = vector_max( &bb.max, &vert );
	}
	return bb;
}

void test_aabb_calculate( ) {
	vector verts[] = {
		{{ 1.f, 0.f, -1.f, 1.f }},
		{{ -1.f, 1.f, 0.f, 1.f }},
		{{ 0.f, -1.f, 1.f, 1.f }}
	};
	aabb bb = aabb_calculate( 3, verts, NULL );
#if 0
	printf( "AABB: ( " );
	vector_print( &bb.min );
	printf( " ) to ( " );
	vector_print( &bb.max );
	printf( ")\n" );
#endif
	vector v_min = Vector( -1.f, -1.f, -1.f, 1.f );
	vector v_max = Vector( 1.f, 1.f, 1.f, 1.f );
	vAssert( vector_equal( &bb.min, &v_min ));
	vAssert( vector_equal( &bb.max, &v_max ));
}

void modelInstance_calculateBoundingBox( modelInstance* instance ) {
	instance->bb.min = Vector( 0.0, 0.0, 0.0, 1.0 );
	instance->bb.max = Vector( 0.0, 0.0, 0.0, 1.0 );
	mesh* m = model_fromInstance( instance )->meshes[0];
	instance->bb = aabb_calculate( m->vert_count, m->verts, instance->trans->world );
#if 0
	printf( "AABB: ( " );
	vector_print( &bb.min );
	printf( " ) to ( " );
	vector_print( &bb.max );
	printf( ")\n" );
#endif
}

void aabb_expand( aabb* bb, vector* points ) {
	points[0] = bb->min;
	points[1] = bb->max;
	points[2] = Vector( bb->max.coord.x, bb->min.coord.y, bb->min.coord.z, 1.f );
	points[3] = Vector( bb->min.coord.x, bb->max.coord.y, bb->min.coord.z, 1.f );
	points[4] = Vector( bb->min.coord.x, bb->min.coord.y, bb->max.coord.z, 1.f );
	points[5] = Vector( bb->max.coord.x, bb->max.coord.y, bb->min.coord.z, 1.f );
	points[6] = Vector( bb->min.coord.x, bb->max.coord.y, bb->max.coord.z, 1.f );
	points[7] = Vector( bb->max.coord.x, bb->min.coord.y, bb->max.coord.z, 1.f );
}

bool plane_cull( aabb* bb, vector* plane ) {
	vector points[8];
	aabb_expand( bb, points );
	for ( int i = 0; i < 6; i++ ) {
		if ( Dot( &points[i], plane ) > plane->coord.w )
			return false;
	}
	return true;
}

// Cull the AABB *bb* against the frustum defined as 6 planes in *frustum*
bool frustum_cull( aabb* bb, vector* frustum ) {
	if ( plane_cull( bb, &frustum[0] )) {
		return true;
	}
	/*
	if ( plane_cull( bb, &frustum[1] ))
		return true;
		*/
	return false;
}

void modelInstance_draw( modelInstance* instance, camera* cam ) {
	// Bounding box cull
	modelInstance_calculateBoundingBox( instance );

	vector frustum[6];
	camera_calculateFrustum( cam, frustum );
	if ( frustum_cull( &instance->bb, frustum ) )
		return;

	render_resetModelView();
	matrix_mul( modelview, modelview, instance->trans->world );
	render_setUniform_matrix( *resources.uniforms.modelview, modelview );

	model_draw( model_fromInstance( instance ) );
}
