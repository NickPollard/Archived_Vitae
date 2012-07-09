// Maths.c
#include "common.h"
#include "maths.h"
//----------------------
#include <assert.h>

static const float epsilon = 0.0001f;

bool f_eq( float a, float b ) {
	return fabsf( a - b ) < epsilon;
}

float fclamp( float a, float bottom, float top ) {
	return fminf( fmaxf( a, bottom), top);
}

// return *value* rounded to the nearest *round*
float fround( float value, float round ) {
	return floorf( (value / round) + 0.5f ) * round;
}


int max( int a, int b ) {
	return ( a > b ) ? a : b;
}
int min( int a, int b ) {
	return ( a < b ) ? a : b;
}

int clamp( int a, int bottom, int top ) {
	return min( max( a, bottom), top);
}

bool contains( int a, int min, int max ) {
	return ( a >= min && a < max );
}

float lerp( float a, float b, float factor ) {
	return a * (1.f - factor) + b * factor;
}

float map_range( float point, float begin, float end ) {
	return ( point - begin ) / ( end - begin );
}

bool isPowerOf2( unsigned int n ) {
	/*
	printf( "n: %x.\n", n );
	printf( "n-1: %x.\n", n-1 );
	printf( "n & (n-1): %x.\n", n & ~(n-1) );
	*/
	return (n & (n - 1)) == 0;
}

quaternion quat_fromMatrix( matrix m ) {
	quaternion q;
	// Need to calculate axis and angle of rotation
	// Quaternion is:
	// v s where v is sin(2t) . axis
	// s is cos(2t)

	vector* z_old = NULL;
	const vector* z_new = matrix_getCol( m, 2 );
	// The axis of rotation must be perpendicular to both old and new angles
	// If both Z axis are identical, the axis of rotation must be the Z axis
	vector axis; 
	Cross( &axis, z_old, z_new );
	float cos_t = Dot( z_old, z_new );
	float t = acos( cos_t );
	(void)t;
	return q;
}


// Build a rotation quaternion from Euler Angle values
quaternion quaternion_fromEuler( vector* euler_angles ) {
	(void)euler_angles;
	// TODO: implement
	printf("Not Yet Implemented: quaternion_fromEuler.\n" );
	assert( 0 );
}

#ifdef UNIT_TEST
void test_maths() {
	test_matrix();
}
#endif // UNIT_TEST
