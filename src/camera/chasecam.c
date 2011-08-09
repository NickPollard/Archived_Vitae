// chasecam.c

#include "common.h"
#include "chasecam.h"
//---------------------
#include "camera.h"
#include "transform.h"

chasecam* chasecam_create() {
	chasecam* c = mem_alloc( sizeof( chasecam ));
	memset( c, 0, sizeof( chasecam ));
	c->position = Vector( 0.f, 0.f, 0.f, 1.f );
	return c;
}

void chasecam_tick( void* data, float dt ) {
	chasecam* c = (chasecam*) data;

	// Calculate the position as the offset from the chase target
	vector offset = Vector( 0.f, 3.f, -15.f, 1.f );
	vector position = matrixVecMul( c->target->world, &offset );

	float lerp = fminf( 1.f, 4.f * dt );
	c->position = vector_lerp( &c->position, &position, lerp );


	matrix world_space;
	// Rotation
	matrix_cpy( world_space, c->target->world );
	// Position
	matrix_setTranslation( world_space, &c->position );

	transform_setWorldSpace( c->cam->trans, world_space );	
}
