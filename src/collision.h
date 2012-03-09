// collision.h

#define kMaxCollisionEvents 256
#define kMaxShapeTypes 2
#define kMaxCollidingBodies 256

#include "maths.h"

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

typedef struct body_s {
	transform* trans;
	shape* shape;
} body;

typedef struct collisionEvent_s {
	body* a;
	body* b;
} collisionEvent;

typedef bool (*collideFunc)( shape* a, shape* b, matrix matrix_a, matrix matrix_b );

// Initialize the collision system
void collision_init();

// Add a body to the collision system
void collision_addBody( body* b );

// Check for any collisions this frame
void collision_tick( float dt );

// Did the body hit anything this frame
bool body_collided( body* b );

// Did two bodies hit each other this frame
bool body_collidedBody( body* a, body* b );

// Unit tests
void test_collision();
