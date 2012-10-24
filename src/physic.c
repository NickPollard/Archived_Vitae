// physic.c
#include "src/common.h"
#include "src/physic.h"
//---------------------
#include "engine.h"
#include "transform.h"
#include "maths/vector.h"

// Debug
physic* active_physics[1024];
int active_physic_count;

physic* physic_create()  {
	physic* p = mem_alloc( sizeof( physic ));
	memset( p, 0, sizeof( physic ));
	p->velocity = Vector( 0.f, 0.f, 0.f, 0.f );
	p->mass = 0.f;
	p->to_delete = false;

	array_add( (void**)active_physics, &active_physic_count, (void*)p );

	return p;
}

void physic_delete( physic* p ) {
	p->to_delete = true;
}

void physic_tick( void* data, float dt, engine* eng ) {
	physic* p = data;
	
	physic_assertActive( p );
	vAssert( p->trans );

	// If requested to delete
	if ( p->to_delete ) {
		mem_free( p );
		stopTick( eng, p, physic_tick );
		array_remove( (void**)active_physics, &active_physic_count, (void*)p );
		return;
	}

	vector delta = vector_scaled( p->velocity, dt );
	vector position = vector_add( delta, *matrix_getTranslation( p->trans->world ));
	transform_setWorldSpacePosition( p->trans, &position );
}

void physic_assertActive( physic* p ) {
	int found = array_find( (void**)active_physics, active_physic_count, (void*)p );
	vAssert( found != -1 );
}
