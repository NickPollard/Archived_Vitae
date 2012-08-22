// collision.c
#include "common.h"
#include "collision.h"
//---------------------
#include "engine.h"
#include "model.h"
#include "transform.h"
#include "render/debugdraw.h"

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
	//printf( "Collision! 0x" xPTRf " 0x" xPTRf "\n", (uintptr_t)a, (uintptr_t)b );
}

void collision_addBody( body* b ) {
	bodies[body_count++] = b;
}

#define kMaxDeadBodies 32
int dead_body_count = 0;
body* dead_bodies[kMaxDeadBodies];

void collision_removeBody( body* b ) {
	vAssert( dead_body_count < kMaxDeadBodies );
	dead_bodies[dead_body_count++] = b;
}

void collision_removeDeadBody( body*  b ) {
	int i = array_find( (void**)bodies, body_count, b );
	bodies[i] = bodies[--body_count];
}

void collision_removeDeadBodies( ) {
	for ( int i = 0; i < dead_body_count; ++i ) {
		collision_removeDeadBody( dead_bodies[i] );
	}
	dead_body_count = 0;
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

void collisionMesh_drawWireframe( collisionMesh* m, matrix trans, vector color ) {
	debugdraw_wireframeMesh( m->vert_count, m->verts, m->index_count, m->indices, trans, color );
}

void body_debugdraw( body* b ) {
	vector green = Vector( 0.f, 1.f, 0.f, 1.f );
	switch ( b->shape->type ) {
		case shapeInvalid:
			break;
		case shapeSphere:
			{
				const vector* position = transform_getWorldPosition( b->trans );
				debugdraw_sphere( *position, b->shape->radius, green );
			}
			break;
		case shapeMesh:
			collisionMesh_drawWireframe( b->shape->collision_mesh, b->trans->world, green );
			break;
	}
}

void collision_debugdraw() {
	for ( int i = 0; i < body_count; ++i ) {
		body_debugdraw( bodies[i] );
	}
}

// Check for any collisions this frame
void collision_tick( float dt ) {
	(void)dt;
	// clear collision events
	collision_clearEvents();

	collision_generateEvents();

	collision_runCallbacks();

	collision_removeDeadBodies();

	collision_debugdraw();
}

bool collisionFunc_SphereSphere( shape* a, shape* b, matrix matrix_a, matrix matrix_b ) {
	(void)matrix_a; (void)matrix_b;
	vector origin_a = matrix_vecMul( matrix_a, &a->origin );
	vector origin_b = matrix_vecMul( matrix_b, &b->origin );
	return vector_distance( &origin_a, &origin_b ) < ( a->radius + b->radius );
}

bool collisionFunc_MeshSphere( shape* mesh_, shape* sphere_, matrix matrix_mesh, matrix matrix_sphere ) {
	(void)mesh_;
	(void)sphere_;
	(void)matrix_mesh;
	(void)matrix_sphere;

	return false;
}

// Just swap the types around
bool collisionFunc_SphereMesh( shape* sphere_, shape* mesh_, matrix matrix_sphere, matrix matrix_mesh ) {
	return collisionFunc_MeshSphere( mesh_, sphere_, matrix_mesh, matrix_sphere );
}

bool triangle_intersectingMesh( vector a, vector b, vector c, collisionMesh* m ) {
	(void)a;
	(void)b;
	(void)c;
	(void)m;
	vAssert( 0 ); //NYI
	return false;
}

bool vertex_insideMesh( vector v, collisionMesh* m ) {
	(void)v;
	(void)m;
	vAssert( 0 ); //NYI
	return false;
}

bool mesh_insideMesh( collisionMesh* mesh_a, collisionMesh* mesh_b, matrix matrix_a, matrix matrix_b ) {
	// We build a combined matrix that translates verts from a-space into b-space
	// So we can then do the collision tests purely in b-space
	matrix a_space_to_b_space;
	matrix inv_b;
	matrix_inverse( inv_b, matrix_b );
	matrix_mul( a_space_to_b_space, inv_b, matrix_a );

	// Then test each vertex of MESH_A for whether it's wholly inside the other mesh
	bool inside = false;
	for ( int i = 0; !inside && ( i < mesh_a->vert_count ); ++i ) {
		vector vert = matrix_vecMul( a_space_to_b_space, &mesh_a->verts[i] );
		inside |= vertex_insideMesh( vert, mesh_b );
	}
	return inside;
}

bool mesh_intersectingMesh( collisionMesh* mesh_a, collisionMesh* mesh_b, matrix matrix_a, matrix matrix_b ) {
	// We build a combined matrix that translates verts from a-space into b-space
	// So we can then do the collision tests purely in b-space
	matrix a_space_to_b_space;
	matrix inv_b;
	matrix_inverse( inv_b, matrix_b );
	matrix_mul( a_space_to_b_space, inv_b, matrix_a );

	bool intersecting = false;
	for ( int i = 0; !intersecting && i < mesh_a->index_count; i = i+3 ) {
		vector a = matrix_vecMul( a_space_to_b_space, &mesh_a->verts[mesh_a->indices[i+0]]);
		vector b = matrix_vecMul( a_space_to_b_space, &mesh_a->verts[mesh_a->indices[i+1]]);
		vector c = matrix_vecMul( a_space_to_b_space, &mesh_a->verts[mesh_a->indices[i+2]]);
		intersecting |= triangle_intersectingMesh( a, b, c, mesh_b );
	}
	return intersecting;
}

bool collisionFunc_MeshMesh( shape* mesh_a, shape* mesh_b, matrix matrix_a, matrix matrix_b ) {
	// There are 3 possible situations here
	// 1. The meshes are not colliding at all
	// 2. At least one mesh has at least one vertex inside the other
	// 3. No vertices are inside the other mesh, but there are intersecting edges/tris
	
	bool colliding = false;
	// Test each vertex to see if it's wholly inside the other mesh
	// and vica versa
	colliding |= mesh_insideMesh( mesh_a->collision_mesh, mesh_b->collision_mesh, matrix_a, matrix_b );
	if ( !colliding ) {
		colliding |= mesh_insideMesh( mesh_b->collision_mesh, mesh_a->collision_mesh, matrix_b, matrix_a );
		if ( !colliding ) {
			// Test for intersections of triangles (without intersecting vertices)
			colliding |= mesh_intersectingMesh( mesh_b->collision_mesh, mesh_a->collision_mesh, matrix_b, matrix_a );
		}
	}
	
	return colliding;
}

bool collisionFunc_invalid( shape* a, shape* b, matrix matrix_a, matrix matrix_b ) {
	printf( "Error, attempting to use invalid shape collision func. Shape tpes %d and %d.\n", a->type, b->type );
	vAssert( false );
	(void)matrix_a;
	(void)matrix_b;
}

void collision_initCollisionFuncs() {
	for ( int i = 0; i < kMaxShapeTypes; ++i ) {
		for ( int j = 0; j < kMaxShapeTypes; ++j ) {
			collide_funcs[i][j] = collisionFunc_invalid;
		}
	}
	collide_funcs[shapeSphere][shapeSphere] = collisionFunc_SphereSphere;
	collide_funcs[shapeMesh][shapeSphere] = collisionFunc_MeshSphere;
	collide_funcs[shapeSphere][shapeMesh] = collisionFunc_SphereMesh;
	collide_funcs[shapeMesh][shapeMesh] = collisionFunc_MeshMesh;
}

collideFunc collision_func( enum shapeType type_a, enum shapeType type_b ) {
	return collide_funcs[type_a][type_b];
}

bool shape_colliding( shape* a, shape* b, matrix matrix_a, matrix matrix_b ) {
	collideFunc collide_func = collision_func( a->type, b->type );
	return collide_func( a, b, matrix_a, matrix_b );
}

bool body_colliding( body* a, body* b ) {
	vAssert( a->trans );
	vAssert( b->trans );
	bool test_collision =	( a->collide_with & b->layers ) |
	   						( a->layers & b->collide_with );
	return test_collision && shape_colliding( a->shape, b->shape, a->trans->world, b->trans->world );
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

/*
void vertexBuffer_asPositionsOnly( vector* verts, vertex* src_verts, int vert_count ) {
	for ( int i = 0; i < vert_count; ++i ) {
		verts[i] = src_verts[i].position;
	}
}
*/

collisionMesh* collisionMesh_fromRenderMesh( mesh* render_mesh ) {
	collisionMesh* m = mem_alloc( sizeof( collisionMesh ));
	m->vert_count = render_mesh->vert_count;
	m->index_count = render_mesh->index_count;

	// Allocate our buffers
	m->verts = mem_alloc( sizeof( vector ) * m->vert_count );
	m->indices = mem_alloc( sizeof( m->indices[0] ) * m->index_count );
	
	// Now fill them
	memcpy( m->indices, render_mesh->indices, sizeof( m->indices[0] ) * m->index_count );
	memcpy( m->verts, render_mesh->verts, sizeof( m->verts[0] ) * m->vert_count );
	return m;
}


shape* mesh_createFromRenderMesh( mesh* render_mesh ) {
	shape* s = mem_alloc( sizeof( shape ));
	s->type = shapeMesh;
	s->collision_mesh = collisionMesh_fromRenderMesh( render_mesh );
	return s;
}

body* body_create( shape* s, transform* t ) {
	body* b = mem_alloc( sizeof( body ));
	memset( b, 0, sizeof( body ));
	b->shape = s;
	b->trans = t;
	return b;
}

void collision_init() {
	body_count = 0;
	collision_clearEvents();
	collision_initCollisionFuncs();
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
	body_a->layers |= 0x1;
	body_b->layers |= 0x1;
	body_c->layers |= 0x1;
	body_a->collide_with |= 0x1;
	body_b->collide_with |= 0x1;
	body_c->collide_with |= 0x1;

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
