// physic.c
#include "src/common.h"
#include "src/physic.h"
//---------------------
#include "transform.h"

physic* physic_create()  {
	physic* p = mem_alloc( sizeof( physic ));
	memset( p, 0, sizeof( physic ));
	p->velocity = Vector( 0.f, 0.f, 0.f, 0.f );
	p->mass = 0.f;
	return p;
}

void physic_tick( void* data, float dt ) {
	physic* p = data;
	assert( p->trans );

	vector delta;
	vecScale( &delta, &p->velocity, dt );
	vector position;
	Add( &position, &delta, matrix_getTranslation( p->trans->world ));
	transform_setWorldSpacePosition( p->trans, &position );
}
