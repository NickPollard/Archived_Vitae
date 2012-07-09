// Maths.h
#pragma once

#include "maths/vector.h"
#include "render/vgl.h"
#include <math.h>

#define PI 3.1415926535897932

typedef float matrix[4][4];

extern matrix matrix_identity;

typedef struct quat_s {
	float x;
	float y;
	float z;
	float s;
} quaternion;

// Matrix is COLUMN Major
// columns are laid out contiguously in memory
// ie val[0][x] = a point in the first column
// Translation values are in the 12th, 13th, 14th, 15th addresses of the buffer

bool f_eq( float a, float b );

float fclamp( float a, float bottom, float top );
float fround( float value, float round );

int max( int a, int b );
int min( int a, int b );

int clamp( int a, int bottom, int top );
bool contains( int a, int min, int max );

float lerp( float a, float b, float factor );
// map a point to the range from begin-end, returning a value from 0.f to 1.f
float map_range( float point, float begin, float end );

bool isPowerOf2( unsigned int n );

// Matrix Vector multiply
vector matrixVecMul(matrix m, const vector* v);

// Matrix multiply
//matrix matrixMul( const matrix a, const matrix b );

// Matrix inverse
void matrix_inverse( matrix dst, matrix src );

void Set(vector* v, float x, float y, float z, float w);

// Set a column in a matrix to a given vector
void matrix_setColumn(matrix m, int col, const vector* v);

// Set a row in a matrix to a given vector
void matrix_setRow(matrix m, int row, const vector* v);

void matrix_setRotation( matrix dst, matrix src );

const vector* matrix_getCol( matrix m, int i );

// Set the translation component of a 4x4 matrix
void matrix_setTranslation(matrix m, const vector* v);
void matrix_clearTranslation( matrix m );

// Get the translation component of a 4x4 matrix
const vector* matrix_getTranslation(matrix m);

// Initialise a matrix to the identity
void matrix_setIdentity(matrix m);

// Convert a V matrix to an OGL matrix
const GLfloat* matrix_getGlMatrix(matrix m);

// Multiply two matrices together
void matrix_mul(matrix dst, matrix m1, matrix m2);

// Build a matrix from a rotation and translation
void matrix_fromRotTrans( matrix dst, quaternion* rotation, vector* translation );

// Copy one matrix to another
void matrix_cpy(matrix dst, matrix src);

// Build a rotation matrix from given Euler Angles
void matrix_fromEuler( matrix dst, vector* euler_angles );

void matrix_normalize( matrix m );

// Build a rotation quaternion from Euler Angle values
quaternion quaternion_fromEuler( vector* euler_angles );

void matrix_rotX( matrix dst, float angle );
void matrix_rotY( matrix dst, float angle );
void matrix_rotZ( matrix dst, float angle );

// *** Test
#ifdef UNIT_TEST
void test_maths();
#endif // UNIT_TEST

void matrix_print( matrix src );
