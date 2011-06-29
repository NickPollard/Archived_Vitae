// Maths.c

#include "common.h"
#include "maths.h"
//----------------------
#include <assert.h>

static const float epsilon = 0.00000001f;

bool f_eq( float a, float b ) {
	return abs( a - b ) < epsilon;
}

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
vector matrixVecMul( matrix m, const vector* v) {
	vector out;
	out.coord.x = m[0][0] * v->coord.x + 
			m[0][1] * v->coord.y + 
			m[0][2] * v->coord.z + 
			m[0][3] * v->coord.w;
	out.coord.y = m[1][0] * v->coord.x + 
			m[1][1] * v->coord.y + 
			m[1][2] * v->coord.z + 
			m[1][3] * v->coord.w;
	out.coord.z = m[2][0] * v->coord.x + 
			m[2][1] * v->coord.y + 
			m[2][2] * v->coord.z + 
			m[2][3] * v->coord.w;
	out.coord.w = m[3][0] * v->coord.x + 
			m[3][1] * v->coord.y + 
			m[3][2] * v->coord.z + 
			m[3][3] * v->coord.w;
	return out;
}

// Matrix multiply
/*
matrix matrixMul( const matrix a, const matrix b ) {
	matrix out;
	for (int m = 0; m < 4; m++ ) {
		for (int n = 0; n < 4; n++ ) {
			out.val[n][m] = a[0][m] * b[n][0] + 
							a[1][m] * b[n][1] + 
							a[2][m] * b[n][2] + 
							a[3][m] * b[n][3];
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
void matrix_setRow(matrix m, int row, const vector* v) {
	m[0][row] = v->val[0];
	m[1][row] = v->val[1];
	m[2][row] = v->val[2];
	m[3][row] = v->val[3]; }

// Set a column in a matrix to a given vector
void matrix_setColumn(matrix m, int col, const vector* v) {
	m[col][0] = v->val[0];
	m[col][1] = v->val[1];
	m[col][2] = v->val[2];
	m[col][3] = v->val[3];
}

// Set the translation component of a 4x4 matrix
void matrix_setTranslation(matrix m, const vector* v) {
//	matrix_setColumn(m, 3, v);
	m[3][0] = v->val[0];
	m[3][1] = v->val[1];
	m[3][2] = v->val[2];
}

// Get the translation component of a 4x4 matrix
const vector* matrix_getTranslation(matrix m) {
	return (vector*)m[3]; }

// Initialise a matrix to the identity
void matrix_setIdentity(matrix m) {
	memset(m, 0, sizeof(matrix));
	m[0][0] = 1.f;
	m[1][1] = 1.f;
	m[2][2] = 1.f;
	m[3][3] = 1.f; }

bool matrix_equal( matrix a, matrix b ) {
	for (int i = 0; i < 16; i++)
		if ( !f_eq( ((float*)a)[i], ((float*)b)[i] ) )
			return false;
	return true;
}

// Find the determinant of a 4x4 matrix using Laplace Expansion
// This can probably be simplified further if I multiply out the 2x2 and 3x3 factors
float matrix_determinant( matrix src ) {
	// first calculate and cache the determinants of the base 2x2 matrices
	float (*m)[4] = (float(*)[4])src;
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

void matrix_transpose( matrix dst, matrix src ) {
	for ( int i = 0; i < 4; i++ )
		for ( int j = i; j < 4; i++ ) {
			dst[i][j] = src[j][i];
			dst[j][i] = src[i][j];
		}
}

void matrix_scalarMul( matrix dst, matrix src, float scalar ) {
	float* srcf = (float*)src;
	float* dstf = (float*)dst;
	for ( int i = 0; i < 16; i++ )
		dstf[i] = srcf[i] * scalar;
}

// Matrix inverse
void matrix_inverse( matrix dst, matrix src ) {
	/*
	// calulate the matrix of cofactors
	matrix cofactors;

//	float bA = src


	matrix adjugate;
	matrix_transpose( adjugate, cofactors );
	matrix inverse;
	float invDet = 1.f / matrix_determinant( src );
	matrix_scalarMul( inverse, adjugate, invDet );
*/
	// TODO: implement
	printf( "Not Yet Implemented: matrix_inverse \n");
	assert( 0 );
}

// Convert a V matrix to an OGL matrix
const GLfloat* matrix_getGlMatrix( matrix m ) {
	return (const GLfloat*)m; }

// Multiply two matrices together
void matrix_mul( matrix dst, matrix m1, matrix m2 ) {
	for ( int i = 0; i < 4; i++ ) {
		for ( int j = 0; j < 4; j++ ) {
			dst[i][j] = m1[0][j] * m2[i][0]
						+ m1[1][j] * m2[i][1]
						+ m1[2][j] * m2[i][2]
						+ m1[3][j] * m2[i][3]; }}}

// Copy one matrix to another
void matrix_cpy( matrix dst, matrix src ) { 
	float* a = (float*)dst;
	float* b = (float*)src;
	for (int i = 0; i < 16; i++) {
		*a++ = *b++; } }

void matrix_fromRotTrans( matrix dst, quaternion* rotation, vector* translation ) {
	// TODO: implement
	printf( "Not Yet Implemented: matrix_fromRotTrans \n");
	assert( 0 );
}

/*
matrix matrix_fromQuat( quaternion q ) {
	matrix m;
	return m;
}

quaternion quat_fromMatrix( matrix m ) {
	quaternion q;
	return q;
}

matrix matrix_build( quaternion rot, vec trans ) {
	matrix m;
	return m;
}
*/
void matrix_rotY( matrix dst, float angle ) {
	float sinTheta = sin( angle );
	float cosTheta = cos( angle );
	matrix_setIdentity( dst );
	dst[0][0] = cosTheta;
	dst[0][2] = sinTheta;
	dst[2][0] = -sinTheta;
	dst[2][2] = cosTheta;
}

void matrix_rotZ( matrix dst, float angle ) {
	float sinTheta = sin( angle );
	float cosTheta = cos( angle );
	matrix_setIdentity( dst );
	dst[0][0] = cosTheta;
	dst[0][1] = -sinTheta;
	dst[1][0] = sinTheta;
	dst[1][1] = cosTheta;
}


// Create a rotation matrix representing a rotation about the X-axis (Pitch)
// of *angle* radians
// NOTE: the angle must be in radians, not degrees
void matrix_rotX( matrix dst, float angle ) {
	float sinTheta = sin( angle );
	float cosTheta = cos( angle );
	matrix_setIdentity( dst );
	dst[1][1] = cosTheta;
	dst[1][2] = -sinTheta;
	dst[2][1] = sinTheta;
	dst[2][2] = cosTheta;
}

void matrix_print( matrix src ) {
	printf( "{ %.2f, %.2f, %.2f, %.2f\n ", src[0][0], src[1][0], src[2][0], src[3][0] );
	printf( "  %.2f, %.2f, %.2f, %.2f\n ", src[0][1], src[1][1], src[2][1], src[3][1] );
	printf( "  %.2f, %.2f, %.2f, %.2f\n ", src[0][2], src[1][2], src[2][2], src[3][2] );
	printf( "  %.2f, %.2f, %.2f, %.2f }\n ", src[0][3], src[1][3], src[2][3], src[3][3] );
}

// Build a rotation matrix from given Euler Angles
void matrix_fromEuler( matrix dst, vector* euler_angles ) {
	matrix x, y, z;
	matrix_rotX( x, euler_angles->coord.x );
	matrix_rotY( y, euler_angles->coord.y );
	matrix_rotZ( z, euler_angles->coord.z );

	// Compose angles in the standard YXZ Euler order
	matrix_mul( dst, y, x );
	matrix_mul( dst, dst, z );

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
	matrix_setIdentity( a );
	matrix_setIdentity( b );
	vector v = Vector( 3.f, 4.f, 5.f, 1.f );
	matrix_setTranslation( a, &v );
	matrix_mul( c, a, b );
	assert( matrix_equal( c, a ));
	assert( !matrix_equal( c, b ));

	matrix_setIdentity( a );
	float det = matrix_determinant( a );
	printf(" identity det = %.2f\n", det );
	assert( f_eq( det, 1.f ));
	det = matrix_determinant( c );
	printf(" det = %.2f\n", det );
	assert( f_eq( det, 1.f ));
}
