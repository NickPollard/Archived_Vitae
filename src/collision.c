// collision.c

#include "src/common.h"
#include "src/collision.h"
//---------------------
#include "engine.h"
#include "transform.h"

collideFunc collide_funcs[kMaxShapeTypes][kMaxShapeTypes];

collisionEvent collision_events[kMaxCollisionEvents];
int event_count;

body* bodies[kMaxCollidingBodies];
int body_count;

// Forward Declarations
bool body_colliding( body* a, body* b );

void collision_clearEvents() {
	memset( collision_events, 0, sizeof( collisionEvent ) * kMaxCollisionEvents );
	event_count = 0;
}

void collision_event( body* a, body* b ) {
	collisionEvent* event = &collision_events[event_count++];
	event->a = a;
	event->b = b;
	printf( "Collision! 0x" xPTRf " 0x" xPTRf "\n", (uintptr_t)a, (uintptr_t)b );
}

void collision_init() {
	body_count = 0;
	collision_clearEvents();
}

void collision_addBody( body* b ) {
	bodies[body_count++] = b;
}

void collision_removeBody( body*  b ) {
	int i = array_find( (void**)bodies, body_count, b );
	bodies[i] = bodies[--body_count];
}

void collision_callback( body* a, body* b ) {
	if ( a->callback )
		a->callback( a, b, a->callback_data );
}

void collision_runCallbacks() {
	for ( int i = 0; i < event_count; ++i ) {
		collision_callback( collision_events[i].a, collision_events[i].b );
		collision_callback( collision_events[i].b, collision_events[i].a );
	}
}

void collision_generateEvents() {
	// for every body, check every other body
	for ( int i = 0; i < body_count; ++i )
		for ( int j = i + 1; j < body_count; j++ )
			if ( body_colliding( bodies[i], bodies[j] ))
				collision_event( bodies[i], bodies[j] );
}

// Check for any collisions this frame
void collision_tick( float dt ) {
	(void)dt;
	// clear collision events
	collision_clearEvents();

	collision_generateEvents();

	collision_runCallbacks();
}

bool collisionFunc_SphereSphere( shape* a, shape* b, matrix matrix_a, matrix matrix_b ) {
	(void)matrix_a; (void)matrix_b;
	vector origin_a = matrixVecMul( matrix_a, &a->origin );
	vector origin_b = matrixVecMul( matrix_b, &b->origin );
	return vector_distance( &origin_a, &origin_b ) < ( a->radius + b->radius );
}

collideFunc collision_func( enum shapeType type_a, enum shapeType type_b ) {
	(void)type_a; (void)type_b;
	return &collisionFunc_SphereSphere;
//	return collide_funcs[type_a][type_b];
}

bool shape_colliding( shape* a, shape* b, matrix matrix_a, matrix matrix_b ) {
	collideFunc collide_func = collision_func( a->type, b->type );
	return collide_func( a, b, matrix_a, matrix_b );
}

bool body_colliding( body* a, body* b ) {
	vAssert( a->trans );
	vAssert( b->trans );
	return shape_colliding( a->shape, b->shape, a->trans->world, b->trans->world );
}

bool body_collided( body* b ) {
	// Look for any event with this body
	for ( int i = 0; i < event_count; ++i )
		if ( collision_events[i].a == b || collision_events[i].b == b )
			return true;
	return false;
}

bool body_collidedBody( body* a, body* b ) {
	(void)a; (void)b;
	return false;
}

shape* sphere_create( float radius ) {
	shape* s = mem_alloc( sizeof( shape ));
	s->type = shapeSphere;
	s->radius = radius;
	s->origin = Vector( 0.f, 0.f, 0.f, 1.f );
	return s;
}

body* body_create( shape* s, transform* t ) {
	body* b = mem_alloc( sizeof( body ));
	memset( b, 0, sizeof( body ));
	b->shape = s;
	b->trans = t;
	return b;
}

void test_collision() {
	shape sphere_a;
   	sphere_a.type = shapeSphere;
	sphere_a.radius = 1.f;
	sphere_a.origin = Vector( 0.f, 0.f, 0.f, 0.f );
	shape sphere_b;
   	sphere_b.type = shapeSphere;
	sphere_b.radius = 1.f;
	sphere_b.origin = Vector( 0.f, 0.f, 0.f, 0.f );
	vAssert( shape_colliding( &sphere_a, &sphere_b, matrix_identity, matrix_identity ));

	shape sphere_c;
   	sphere_c.type = shapeSphere;
	sphere_c.radius = 1.f;
	sphere_c.origin = Vector( 0.f, 2.f, 0.f, 0.f );

	vAssert( !shape_colliding( &sphere_a, &sphere_c, matrix_identity, matrix_identity ));

	transform* t_a = transform_create();
	transform* t_b = transform_create();
	transform* t_c = transform_create();
	vector position = Vector( 0.f, 2.f, 0.f, 1.f );
	transform_setWorldSpacePosition( t_c, &position );

	body* body_a = body_create( sphere_create( 1.f ), t_a );
	body* body_b = body_create( sphere_create( 1.f ), t_b );
	body* body_c = body_create( sphere_create( 1.f ), t_c );

	collision_init();
	collision_addBody( body_a );
	collision_addBody( body_b );
	collision_addBody( body_c );
	collision_tick( 0.f );

	vAssert( body_collided( body_a ));
	vAssert( body_collided( body_b ));
	vAssert( !body_collided( body_c ));

	/*
	printf( "Bodies collided this frame: A: %d, B: %d, c:%d\n",
			body_collided( body_a ),
			body_collided( body_b ),
			body_collided( body_c ));
	vAssert( 0 );
	*/

	collision_removeBody( body_a );
	collision_removeBody( body_b );
	collision_removeBody( body_c );

	mem_free( body_a );
	mem_free( body_b );
	mem_free( body_c );
}
