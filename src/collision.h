// collision.h

#define kMaxCollisionEvents 256
#define kMaxShapeTypes 2
#define kMaxCollidingBodies 256

#include "maths/maths.h"
#include "maths/matrix.h"
#include "maths/vector.h"

enum shapeType {
	shapeInvalid,
	shapeSphere
};

typedef struct shape_s {
	enum shapeType type;
	union {
		float radius;
	};
	vector origin;
} shape;

typedef struct body_s body;

typedef void (*collisionCallback)( body* this, body* collided_width, void* data );
typedef uint8_t collision_layers_t;

struct body_s {
	transform* trans;
	shape* shape;
	union {
		void* data;	// Pointer for storing arbitrary data - e.g. the owner 
		int intdata;
	};
	collision_layers_t layers;
	collision_layers_t collide_with;
	collisionCallback callback;
	void* callback_data;
};

typedef struct collisionEvent_s {
	body* a;
	body* b;
} collisionEvent;

typedef bool (*collideFunc)( shape* a, shape* b, matrix matrix_a, matrix matrix_b );

// Initialize the collision system
void collision_init();

// Add a body to the collision system
void collision_addBody( body* b );
void collision_removeBody( body* b );

// Check for any collisions this frame
void collision_tick( float dt );

// Did the body hit anything this frame
bool body_collided( body* b );

// Did two bodies hit each other this frame
bool body_collidedBody( body* a, body* b );

body* body_create( shape* s, transform* t );
shape* sphere_create( float radius );

// Unit tests
void test_collision();
