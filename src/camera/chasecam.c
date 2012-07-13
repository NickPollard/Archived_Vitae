// chasecam.c

#include "common.h"
#include "chasecam.h"
//---------------------
#include "transform.h"
#include "maths/quaternion.h"
#include "maths/vector.h"

#include "render/render.h" // TODO remove

chasecam* chasecam_create() {
	chasecam* c = mem_alloc( sizeof( chasecam ));
	memset( c, 0, sizeof( chasecam ));
	c->position = Vector( 0.f, 0.f, 0.f, 1.f );
	c->rotation = Quaternion( 0.f, 0.f, 0.f, 1.f );
	camera_init( &c->cam );
	return c;
}

void chasecam_tick( void* data, float dt ) {
	chasecam* c = (chasecam*) data;

	// Calculate the position as the offset from the chase target
	vector offset = Vector( 0.f, 8.f, -15.f, 1.f );
	vector position = matrix_vecMul( c->target->world, &offset );

	const float lerp_speed = 4.f;
	float lerp = fclamp( lerp_speed * dt, 0.f, 1.f );
	c->position = vector_lerp( &c->position, &position, lerp );



	matrix world_space;
	quaternion target_rotation = transform_getWorldRotation( c->target );
	float slerp_factor = 0.f;
	c->rotation = quaternion_slerp( c->rotation, target_rotation, slerp_factor );

	matrix_fromRotationTranslation( world_space, c->rotation, c->position );
	matrix_print( world_space );
	render_validateMatrix( world_space );
	transform_setWorldSpace( c->cam.trans, world_space );	

	/*
	matrix world_space;
	// Rotation
	matrix_cpy( world_space, c->target->world );
	// Position
	matrix_setTranslation( world_space, &c->position );

	transform_setWorldSpace( c->cam.trans, world_space );	
	*/
}
