// Maths.c

#include "common.h"
#include "maths.h"
//----------------------
#include <assert.h>

vector Vector(float x, float y, float z, float w) {
	vector v;
	v.coord.x = x;
	v.coord.y = y;
	v.coord.z = z;
	v.coord.w = w;
	return v;
}

// Vector Addition
void Add(vector* dst, vector* srcA, vector* srcB) {
	for (int i = 0; i < 4; i++)
		dst->val[i] = srcA->val[i] + srcB->val[i];
}

// Vector subtraction
void Sub(vector* dst, vector* srcA, vector* srcB) {
	for (int i = 0; i < 4; i++)
		dst->val[i] = srcA->val[i] - srcB->val[i];
}

// Vector dot product
float Dot(vector* A, vector* B) {
	return (A->coord.x * B->coord.x + A->coord.y + B->coord.y + A->coord.z + B->coord.z);
}

// Vector cross product
void Cross(vector* dst, vector* srcA, vector* srcB) {
	dst->coord.x = (srcA->coord.y * srcB->coord.z) - (srcA->coord.z * srcB->coord.y);
	dst->coord.y = (srcA->coord.z * srcB->coord.x) - (srcA->coord.x * srcB->coord.z);
	dst->coord.z = (srcA->coord.x * srcB->coord.y) - (srcA->coord.y * srcB->coord.x);
	dst->coord.w = 1.f;
}

// Matrix Vector multiply
vector matrixVecMul( const matrix* m, const vector* v) {
	vector out;
	out.coord.x = m->val[0][0] * v->coord.x + 
			m->val[0][1] * v->coord.y + 
			m->val[0][2] * v->coord.z + 
			m->val[0][3] * v->coord.w;
	out.coord.y = m->val[1][0] * v->coord.x + 
			m->val[1][1] * v->coord.y + 
			m->val[1][2] * v->coord.z + 
			m->val[1][3] * v->coord.w;
	out.coord.z = m->val[2][0] * v->coord.x + 
			m->val[2][1] * v->coord.y + 
			m->val[2][2] * v->coord.z + 
			m->val[2][3] * v->coord.w;
	out.coord.w = m->val[3][0] * v->coord.x + 
			m->val[3][1] * v->coord.y + 
			m->val[3][2] * v->coord.z + 
			m->val[3][3] * v->coord.w;
	return out;
}

// Matrix multiply
/*
matrix matrixMul( const matrix* a, const matrix* b ) {
	matrix out;
	for (int m = 0; m < 4; m++ ) {
		for (int n = 0; n < 4; n++ ) {
			out.val[n][m] = a->val[0][m] * b->val[n][0] + 
							a->val[1][m] * b->val[n][1] + 
							a->val[2][m] * b->val[n][2] + 
							a->val[3][m] * b->val[n][3];
		}
	}
	return out;
}
*/

void Set(vector* v, float x, float y, float z, float w) {
	v->coord.x = x;
	v->coord.y = y;
	v->coord.z = z;
	v->coord.w = w;
}

// Set a row in a matrix to a given vector
void matrix_setRow(matrix* m, int row, const vector* v) {
	m->val[0][row] = v->val[0];
	m->val[1][row] = v->val[1];
	m->val[2][row] = v->val[2];
	m->val[3][row] = v->val[3]; }

// Set a column in a matrix to a given vector
void matrix_setColumn(matrix* m, int col, const vector* v) {
	m->cols[col] = *v; }

// Set the translation component of a 4x4 matrix
void matrix_setTranslation(matrix* m, const vector* v) {
//	matrix_setColumn(m, 3, v);
	m->val[3][0] = v->val[0];
	m->val[3][1] = v->val[1];
	m->val[3][2] = v->val[2];
}

// Get the translation component of a 4x4 matrix
const vector* matrix_getTranslation(matrix* m) {
	return &m->cols[3]; }

// Initialise a matrix to the identity
void matrix_setIdentity(matrix* m) {
	memset(m, 0, sizeof(matrix));
	m->val[0][0] = 1.f;
	m->val[1][1] = 1.f;
	m->val[2][2] = 1.f;
	m->val[3][3] = 1.f; }

bool matrix_equal( matrix* a, matrix* b ) {
	for (int i = 0; i < 16; i++)
		if ( ((float*)a)[i] != ((float*)b)[i] )
			return false;
	return true;
}

// Find the determinant of a 4x4 matrix using Laplace Expansion
float matrix_determinant( matrix* src ) {
	// first calculate and cache the determinants of the base 2x2 matrices
	float** m = (float**)src;
	float a = (m[0][2] * m[1][3]) - (m[1][2] * m[0][3]);
	float b = (m[1][2] * m[2][3]) - (m[2][2] * m[1][3]);
	float c = (m[2][2] * m[3][3]) - (m[3][2] * m[2][3]);
	float d = (m[0][2] * m[2][3]) - (m[2][2] * m[0][3]);
	float e = (m[1][2] * m[3][3]) - (m[3][2] * m[1][3]);
	float f = (m[0][2] * m[3][3]) - (m[3][2] * m[0][3]);

	// calculate the determinants of the 4 3x3 sub matrices
	// Each is made from the det of 3 2x2 matrices
	float det00 = ( 				  (m[1][1] * c) - (m[2][1] * e) + (m[3][1] * b) );
	float det01 = ( (m[0][1] * c) - 				  (m[2][1] * f) + (m[3][1] * d) );
	float det02 = ( (m[0][1] * e) - (m[1][1] * f) + 				  (m[3][1] * a) );
	float det03 = ( (m[0][1] * b) - (m[1][1] * d) + (m[2][1] * a)					);

	float det =   ( m[0][0] * det00 )
				- ( m[1][0] * det01 )
				+ ( m[2][0] * det02 )
				- ( m[3][0] * det03 );

	return det;
}

// Matrix inverse
void matrix_inverse( matrix* restrict dst, matrix* src ) {
	// TODO: implement
	printf( "Not Yet Implemented: matrix_inverse \n");
	assert( 0 );
}

// Convert a V matrix to an OGL matrix
const GLfloat* matrix_getGlMatrix( const matrix* m ) {
	return (const GLfloat*)m; }

// Multiply two matrices together
void matrix_mul( matrix* dst, matrix* m1, matrix* m2 ) {
	for ( int i = 0; i < 4; i++ ) {
		for ( int j = 0; j < 4; j++ ) {
			dst->val[i][j] = m1->val[0][j] * m2->val[i][0]
						+ m1->val[1][j] * m2->val[i][1]
						+ m1->val[2][j] * m2->val[i][2]
						+ m1->val[3][j] * m2->val[i][3]; }}}

// Copy one matrix to another
void matrix_cpy( matrix* restrict dst, matrix* src ) { 
	float* restrict a = (float*)dst;
	float* restrict b = (float*)src;
	for (int i = 0; i < 16; i++) {
		*a++ = *b++; } }

void matrix_fromRotTrans( matrix* restrict dst, quaternion* rotation, vector* translation ) {
	// TODO: implement
	printf( "Not Yet Implemented: matrix_fromRotTrans \n");
	assert( 0 );
}

/*
matrix matrix_fromQuat( quaternion q ) {
	matrix m;
	return m;
}

quaternion quat_fromMatrix( matrix* m ) {
	quaternion q;
	return q;
}

matrix matrix_build( quaternion rot, vec trans ) {
	matrix m;
	return m;
}
*/
void matrix_rotY( matrix* dst, float angle ) {
	float sinTheta = sin( angle );
	float cosTheta = cos( angle );
	matrix_setIdentity( dst );
	dst->val[0][0] = cosTheta;
	dst->val[0][2] = sinTheta;
	dst->val[2][0] = -sinTheta;
	dst->val[2][2] = cosTheta;
}

void matrix_rotZ( matrix* dst, float angle ) {
	float sinTheta = sin( angle );
	float cosTheta = cos( angle );
	matrix_setIdentity( dst );
	dst->val[0][0] = cosTheta;
	dst->val[0][1] = -sinTheta;
	dst->val[1][0] = sinTheta;
	dst->val[1][1] = cosTheta;
}


// Create a rotation matrix representing a rotation about the X-axis (Pitch)
// of *angle* radians
// NOTE: the angle must be in radians, not degrees
void matrix_rotX( matrix* dst, float angle ) {
	float sinTheta = sin( angle );
	float cosTheta = cos( angle );
	matrix_setIdentity( dst );
	dst->val[1][1] = cosTheta;
	dst->val[1][2] = -sinTheta;
	dst->val[2][1] = sinTheta;
	dst->val[2][2] = cosTheta;
}

void matrix_print( matrix* src ) {
	printf( "{ %.2f, %.2f, %.2f, %.2f\n ", src->val[0][0], src->val[1][0], src->val[2][0], src->val[3][0] );
	printf( "  %.2f, %.2f, %.2f, %.2f\n ", src->val[0][1], src->val[1][1], src->val[2][1], src->val[3][1] );
	printf( "  %.2f, %.2f, %.2f, %.2f\n ", src->val[0][2], src->val[1][2], src->val[2][2], src->val[3][2] );
	printf( "  %.2f, %.2f, %.2f, %.2f }\n ", src->val[0][3], src->val[1][3], src->val[2][3], src->val[3][3] );
}

// Build a rotation matrix from given Euler Angles
void matrix_fromEuler( matrix* dst, vector* euler_angles ) {
	matrix x, y, z;
	matrix_rotX( &x, euler_angles->coord.x );
	matrix_rotY( &y, euler_angles->coord.y );
	matrix_rotZ( &z, euler_angles->coord.z );

	// Compose angles in the standard YXZ Euler order
	matrix_mul( dst, &y, &x );
	matrix_mul( dst, dst, &z );

//	matrix_print( dst );
}

// Build a rotation quaternion from Euler Angle values
quaternion quaternion_fromEuler( vector* euler_angles ) {
	// TODO: implement
	printf("Not Yet Implemented: quaternion_fromEuler.\n" );
	assert( 0 );
}

void test_matrix() {
	matrix a, b, c;
	matrix_setIdentity( &a );
	matrix_setIdentity( &b );
	vector v = Vector( 3.f, 4.f, 5.f, 1.f );
	matrix_setTranslation( &a, &v );
	matrix_mul( &c, &a, &b );
	assert( matrix_equal( &c, &a ));
	assert( !matrix_equal( &c, &b ));
}
