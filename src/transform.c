#include "common.h"
#include "transform.h"
//-------------------------
#include "scene.h"
#include "mem/allocator.h"
#include "debug/debugtext.h"

IMPLEMENT_POOL( transform )

pool_transform* static_transform_pool = NULL;

void transform_initPool() {
	static_transform_pool = pool_transform_create( 256 );
}

void transform_setWorldSpace( transform* t, matrix world ) {
//	matrix_cpy( t->world, world );
	if ( t->parent ) {
		matrix inv_parent;
	  	matrix_inverse( inv_parent, t->parent->world );
		matrix_mul( t->local, world, inv_parent );
	}
	else
		matrix_cpy( t->local, world );

	transform_markDirty( t );
	transform_concatenate( t );
//	t->local = t->world * inverse( t->parent->world );
}

void transform_setWorldSpacePosition( transform* t, vector* position ) {
	// Concat in case world space rotation is not up to date
	// Could we just set localspace position actually?
	/*
	transform_concatenate( t );
	matrix m;
	matrix_cpy( m, t->world );
	matrix_setTranslation( m, position );
	transform_setWorldSpace( t, m );
	*/
	if ( !t->parent ) {
		matrix_setTranslation( t->local, position );
	} else {
		vector position_local;
		Sub( &position_local, position, matrix_getTranslation( t->parent->world ));
		matrix_setTranslation( t->local, &position_local );
	}
	transform_concatenate( t );
}

void transform_setLocalSpace();

// Create a new default transform
transform* transform_create() {
	transform* t = pool_transform_allocate( static_transform_pool );
	matrix_setIdentity( t->local );
	matrix_setIdentity( t->world );
	t->parent = NULL;
	t->isDirty = 1; // All transforms are initially dirty, to force initial update
#if DEBUG_STRINGS
	t->debug_name = debug_string( "Transform" );
#endif
	return t;
}

transform* transform_createAndAdd( scene* s ) {
	assert( s->transform_count < MAX_TRANSFORMS );
	transform* t = transform_create();
	scene_addTransform( s, t );
	return t;
}

// Create a new default transform
transform* transform_createEmpty() {
	transform* t = mem_alloc( sizeof( transform ));
	matrix_setIdentity(t->local);
	matrix_setIdentity(t->world);
	t->parent = NULL;
	t->isDirty = 0;
	return t;
}


// Create a new default transform with the given parent
transform* transform_create_Parent(scene* s, transform* parent) {
	transform* t = transform_create(s);
	t->parent = parent;
	return t;
}

/*
// Concatenate the parent world space transforms to produce this world space transform from local
void transform_concatenate(transform* t) {
	if (transform_isDirty(t)) {
		transform_markClean(t);
		if (t->parent) {
			transform_concatenate(t->parent);
			t->world = matrix_mul(t->local, t->parent->world);
		}
		else
			t->world = t->local;
	}
}
*/

// Concatenate the parent world space transforms to produce this world space transform from local
int transform_concatenate(transform* t) {
//	printf( "concatenate transform. This = %x, Parent = %x\n", (unsigned int)t, (unsigned int)t->parent );
	if (t->parent)	{
		if (transform_concatenate(t->parent) || transform_isDirty(t)) {
			transform_markDirty(t);
			matrix_mul(t->world, t->local, t->parent->world);
			return true;
		}
	}
	else if (transform_isDirty(t)) {
		matrix_cpy( t->world, t->local );
		return true;
	}
	return false;
}


// Mark the transform as dirty (needs concatenation)
void transform_markDirty(transform* t) {
	t->isDirty = 1;
//	t->local[3][3] = 2.f;
}

// Mark the transform as clean (doesn't need concatenation)
void transform_markClean(transform* t) {
	t->isDirty = 0;
//	t->local[3][3] = 1.f;
}

// Is the transform dirty? (does it need concatenating?)
int transform_isDirty(transform* t) {
	return t->isDirty;
//	return (t->local[3][3] > 1.f);
}

// Set the translation of the localspace transformation matrix
void transform_setLocalTranslation(transform* t, vector* v) {
	matrix_setTranslation(t->local, v);
	transform_markDirty(t);
}

void transform_printDebug( transform* t, debugtextframe* f ) {
	char string[128];
#if DEBUG_STRINGS
	sprintf( string, "Transform: Name: %s, Translation %.2f, %.2f, %.2f", 
			t->debug_name,
			t->world[3][0], 
			t->world[3][1], 
			t->world[3][2] );
#else
	sprintf( string, "Transform: Translation %.2f, %.2f, %.2f", 
			t->world[3][0], 
			t->world[3][1], 
			t->world[3][2] );
#endif
	PrintDebugText( f, string );
}

void transform_yaw( transform* t, float yaw ) {
	matrix m;
	vector angles = Vector( 0.f, yaw, 0.f, 0.f );
	matrix_fromEuler( m, &angles );
	matrix_mul( t->local, t->local, m );
}
