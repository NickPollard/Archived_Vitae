// physic.c
#include "src/common.h"
#include "src/physic.h"
//---------------------
#include "engine.h"
#include "transform.h"
#include "maths/vector.h"

physic* physic_create()  {
	physic* p = mem_alloc( sizeof( physic ));
	memset( p, 0, sizeof( physic ));
	p->velocity = Vector( 0.f, 0.f, 0.f, 0.f );
	p->mass = 0.f;
	p->to_delete = false;
	return p;
}

void physic_delete( physic* p ) {
	p->to_delete = true;
}

void physic_tick( void* data, float dt, engine* eng ) {
	(void)eng;
	physic* p = data;
	assert( p->trans );

	vector delta;
	vector_scale( &delta, &p->velocity, dt );
	vector position;
	Add( &position, &delta, matrix_getTranslation( p->trans->world ));
	transform_setWorldSpacePosition( p->trans, &position );

	// If requested to delete
	if ( p->to_delete ) {
		mem_free( p );
		stopTick( eng, p, physic_tick );
	}
}
