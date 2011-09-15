// Maths.h
#ifndef __MATHS_H__
#define __MATHS_H__

#include <GL/glfw.h>

#include <math.h>

#define PI 3.1415926535897932

union vector_u {
   	struct {
		float x;
		float y;
		float z;
		float w;
	} coord;
	float	val[4];
};

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
/*
typedef union matrix_u {
	vector cols[4];
	float val[4][4];
} matrix;
*/

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

typedef float matrix[4][4];

// *** Vectors

vector Vector(float x, float y, float z, float w);

// Vector Addition
void Add(vector* dst, const vector* srcA, const vector* srcB);

// Vector subtraction
void Sub(vector *dst, const vector* srcA, const vector* srcB);

// Vector dot product
float Dot( const vector* A, const vector* B );

// Vector cross product
void Cross(vector* dst, const vector* srcA, const vector* srcB);

// Normalise a vector
// No use of restrict; dst *can* alias src
void Normalize( vector* dst, const vector* src );
bool isNormalized( const vector* v );

void vector_scale( vector* dst, vector* src, float scale );
vector vector_lerp( vector* from, vector* to, float amount );

vector vector_mul( vector* a, vector* b );
vector vector_max( vector* a, vector* b );
vector vector_min( vector* a, vector* b );

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

// *** Test
void test_matrix();

void matrix_print( matrix src );

void vector_print( const vector* v );

bool vector_equal( const vector* a, const vector* b );

#endif // __MATHS_H__
