// chasecam.c

#include "common.h"
#include "chasecam.h"
//---------------------
#include "camera.h"
#include "transform.h"

DEFAULT_CREATE_SRC(chasecam)

void chasecam_tick( void* data, float dt ) {
	chasecam* c = (chasecam*) data;

	// Calculate the position as the offset from the chase target
	float offset_distance = -10.f;
	vector offset = Vector( 0.f, 0.f, offset_distance, 1.f );
	vector position = matrixVecMul( c->target->world, &offset );

	matrix world_space;
	// Rotation
	matrix_cpy( world_space, c->target->world );
	// Position
	matrix_setTranslation( world_space, &position );

	transform_setWorldSpace( c->cam->trans, world_space );	
}
