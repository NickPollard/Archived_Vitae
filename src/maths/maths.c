// Maths.c
#include "common.h"
#include "maths.h"
//----------------------
#include "maths/matrix.h"
#include "maths/quaternion.h"
#include "maths/vector.h"

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

float fsign( float f ) {
	return ( f >= 0.f ) ? 1.f : -1.f;
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

#ifdef UNIT_TEST
void test_maths() {
	printf( "--- Beginning Unit Test: Maths ---\n" );

	test_quaternion();

	test_matrix();
}
#endif // UNIT_TEST
