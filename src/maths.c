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


int max( int a, int b ) {
	return a > b ? a : b;
}
int min( int a, int b ) {
	return a < b ? a : b;
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

// *** Vectors

vector Vector(float x, float y, float z, float w) {
	vector v;
	v.coord.x = x;
	v.coord.y = y;
	v.coord.z = z;
	v.coord.w = w;
	return v;
}

// Vector Addition
void Add(vector* dst, const vector* srcA, const vector* srcB) {
	for (int i = 0; i < 4; i++)
		dst->val[i] = srcA->val[i] + srcB->val[i];
}

// Vector &subtraction
void Sub(vector* dst, const vector* srcA, const vector* srcB) {
	for (int i = 0; i < 4; i++)
		dst->val[i] = srcA->val[i] - srcB->val[i];
}

// Vector dot product
float Dot( const vector* A, const vector* B ) {
	return (A->coord.x * B->coord.x + A->coord.y + B->coord.y + A->coord.z + B->coord.z);
}

// Vector cross product
void Cross(vector* dst, const vector* srcA, const vector* srcB) {
	vAssert( dst != srcA );
	vAssert( dst != srcB );
	dst->coord.x = (srcA->coord.y * srcB->coord.z) - (srcA->coord.z * srcB->coord.y);
	dst->coord.y = (srcA->coord.z * srcB->coord.x) - (srcA->coord.x * srcB->coord.z);
	dst->coord.z = (srcA->coord.x * srcB->coord.y) - (srcA->coord.y * srcB->coord.x);
	dst->coord.w = 1.f;
}

void vector_scale( vector* dst, vector* src, float scale ) {
	dst->coord.x = src->coord.x * scale;
	dst->coord.y = src->coord.y * scale;
	dst->coord.z = src->coord.z * scale;
	dst->coord.w = src->coord.w;
}

float vector_length( const vector* v ) {
	float length = sqrt( v->coord.x * v->coord.x + v->coord.y * v->coord.y + v->coord.z * v->coord.z );
	return length;
}
// Normalise a vector
// No use of restrict; dst *can* alias src
void Normalize( vector* dst, const vector* src ) {
	float length = vector_length( src );
	float invLength = 1.f / length;
	dst->coord.x = src->coord.x * invLength;
	dst->coord.y = src->coord.y * invLength;
	dst->coord.z = src->coord.z * invLength;
	dst->coord.w = src->coord.w; // Preserve the W coord? This seems right to me
}

bool isNormalized( const vector* v ) {
#if 0
	printf( "IsNormalized: Vector v: " );
	vector_print( v );
	printf( ", length: %.10f\n", vector_length( v ));
#endif
	return f_eq( 1.f, vector_length( v ));
}

vector vector_lerp( vector* from, vector* to, float amount ) {
	vector v;
	float inv = 1.f - amount;
	v.coord.x = from->coord.x * inv + to->coord.x * amount;
	v.coord.y = from->coord.y * inv + to->coord.y * amount;
	v.coord.z = from->coord.z * inv + to->coord.z * amount;
	v.coord.w = from->coord.w * inv + to->coord.w * amount;
	return v;
}

vector vector_mul( vector* a, vector* b ) {
	vector v;
	v.coord.x = a->coord.x * b->coord.x;
	v.coord.y = a->coord.y * b->coord.y;
	v.coord.z = a->coord.z * b->coord.z;
	v.coord.w = a->coord.w * b->coord.w;
	return v;
}

// Matrix Vector multiply
vector matrixVecMul( matrix m, const vector* v) {
	vector out;
	out.coord.x = m[0][0] * v->coord.x + 
					m[1][0] * v->coord.y + 
					m[2][0] * v->coord.z + 
					m[3][0] * v->coord.w;
	out.coord.y = m[0][1] * v->coord.x + 
					m[1][1] * v->coord.y + 
					m[2][1] * v->coord.z + 
					m[3][1] * v->coord.w;
	out.coord.z = m[0][2] * v->coord.x + 
					m[1][2] * v->coord.y + 
					m[2][2] * v->coord.z + 
					m[3][2] * v->coord.w;
	out.coord.w = m[0][3] * v->coord.x + 
					m[1][3] * v->coord.y + 
					m[2][3] * v->coord.z + 
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

void matrix_clearTranslation( matrix m ) {
	m[3][0] = 0.f;
	m[3][1] = 0.f;
	m[3][2] = 0.f;
}

void matrix_setRotation( matrix dst, matrix src ) {
	for ( int i = 0; i < 3; i++ ) {
		for ( int j = 0; j < 3; j++ ) {
			dst[i][j] = src[i][j];
		}
	}
}

// Get the translation component of a 4x4 matrix
const vector* matrix_getTranslation(matrix m) {
	return (vector*)m[3]; }

const vector* matrix_getCol( matrix m, int i ) {
	return (vector*)m[i]; }

// Initialise a matrix to the identity
void matrix_setIdentity(matrix m) {
	memset(m, 0, sizeof(matrix));
	m[0][0] = 1.f;
	m[1][1] = 1.f;
	m[2][2] = 1.f;
	m[3][3] = 1.f; }

void vector_print( const vector* v ) {
	printf( "%.4f, %.4f, %.4f, %.4f", v->val[0], v->val[1], v->val[2], v->val[3] );
}

bool vector_equal( const vector* a, const vector* b ) {
	for (int i = 0; i < 4; i++)
		if ( !f_eq( ((float*)a)[i], ((float*)b)[i] ) ) {
			printf( "vector_equal: vectors not equal.\nA: " );
			vector_print( a );
			printf( "\nB: " );
			vector_print( b );
			printf( "\n" );
			return false;
		}
	return true;
}

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

float matrix_determinantFromCofactors( matrix m, matrix cofactors ) {

	float det = ( m[0][0] * cofactors[0][0] ) +
				( m[1][0] * cofactors[1][0] ) +
				( m[2][0] * cofactors[2][0] ) +
				( m[3][0] * cofactors[3][0] );

	return det;
}



void matrix_transpose( matrix dst, matrix src ) {
	for ( int i = 0; i < 4; i++ ) {
		for ( int j = i; j < 4; j++ ) {
			dst[i][j] = src[j][i];
			dst[j][i] = src[i][j];
		}
	}
}

void test_create_matrix( matrix m ) {
	float k = 0.f;
	for ( int i = 0; i < 4; i++ ) {
		for ( int j = 0; j < 4; j++ ) {
			m[i][j] = k;
			k += 1.f;
		}
	}
}

void test_matrix_transpose( ) {
	matrix dst, src;
	test_create_matrix( src );
	matrix_transpose( dst, src );
//	matrix_print( src );
//	matrix_print( dst );
	for ( int i = 0; i < 4; i++ ) {
		for ( int j = 0; j < 4; j++ ) {
			assert( f_eq( dst[i][j], src[j][i] ));
		}
	}
}

void matrix_scalarMul( matrix dst, matrix src, float scalar ) {
	float* srcf = (float*)src;
	float* dstf = (float*)dst;
	for ( int i = 0; i < 16; i++ )
		dstf[i] = srcf[i] * scalar;
}

void test_matrix_scalarMul( ) {
	matrix dst, src;
	float scale = 0.5f;
	test_create_matrix( src );
	matrix_scalarMul( dst, src, scale );
	for ( int i = 0; i < 4; i++ ) {
		for ( int j = 0; j < 4; j++ ) {
			assert( f_eq( dst[i][j], src[i][j]*scale ));
		}
	}
}

// Matrix inverse
// Calculate the matrix of cofactors
// Take the tranpose of this, which is the adjugate
// Calculate the determinant using Laplacian expansion from the cofactors
// the inverse is 1/det * adjugate
void matrix_inverse( matrix inverse, matrix src ) {
	// calulate the matrix of cofactors
	matrix cofactors;

	float bA = src[0][2]*src[1][3] - src[0][3]*src[1][2];
	float bB = src[1][2]*src[2][3] - src[1][3]*src[2][2];
	float bC = src[2][2]*src[3][3] - src[2][3]*src[3][2];
	float bD = src[0][2]*src[2][3] - src[0][3]*src[2][2];
	float bE = src[1][2]*src[3][3] - src[1][3]*src[3][2];
	float bF = src[0][2]*src[3][3] - src[0][3]*src[3][2];

	float tA = src[0][0]*src[1][1] - src[0][1]*src[1][0];
	float tB = src[1][0]*src[2][1] - src[1][1]*src[2][0];
	float tC = src[2][0]*src[3][1] - src[2][1]*src[3][0];
	float tD = src[0][0]*src[2][1] - src[0][1]*src[2][0];
	float tE = src[1][0]*src[3][1] - src[1][1]*src[3][0];
	float tF = src[0][0]*src[3][1] - src[0][1]*src[3][0];

	// Top Row
	cofactors[0][0] = src[1][1]*bC - src[2][1]*bE + src[3][1]*bB;
	cofactors[1][0] = -(src[0][1]*bC - src[2][1]*bF + src[3][1]*bD);
	cofactors[2][0] = src[0][1]*bE - src[1][1]*bF + src[3][1]*bA;
	cofactors[3][0] = -(src[0][1]*bB - src[2][1]*bD + src[2][1]*bA);
	// Second row
	cofactors[0][1] = -(src[1][0]*bC - src[2][0]*bE + src[3][0]*bB);
	cofactors[1][1] = src[0][0]*bC - src[2][0]*bF + src[3][0]*bD;
	cofactors[2][1] = -(src[0][0]*bE - src[1][0]*bF + src[3][0]*bA);
	cofactors[3][1] = src[0][0]*bB - src[2][0]*bD + src[2][0]*bA;

	// Third Row
	cofactors[0][2] = src[1][3]*tC - src[2][3]*tE + src[3][3]*tB;
	cofactors[1][2] = -(src[0][3]*tC - src[2][3]*tF + src[3][3]*tD);
	cofactors[2][2] = src[0][3]*tE - src[1][3]*tF + src[3][3]*tA;
	cofactors[3][2] = -(src[0][3]*tB - src[2][3]*tD + src[2][3]*tA);
	// Fourth row
	cofactors[0][3] = -(src[1][2]*tC - src[2][2]*tE + src[3][2]*tB);
	cofactors[1][3] = src[0][2]*tC - src[2][2]*tF + src[3][2]*tD;
	cofactors[2][3] = -(src[0][2]*tE - src[1][2]*tF + src[3][2]*tA);
	cofactors[3][3] = src[0][2]*tB - src[1][2]*tD + src[2][2]*tA;

	float other_det = + tA * bC
						- tD * bE
						+ tF * bB
						+ tB * bF
						- tE * bD
						+ tC * bA;

	/*
	float det = matrix_determinantFromCofactors( src, cofactors );
	printf( "det: %.6f, other_det: %.6f\n", det, other_det  );
	assert( f_eq( det, other_det ));
	*/

	//float invDet = 1.f / matrix_determinantFromCofactors( src, cofactors );
	float invDet = 1.f / other_det;

	matrix adjugate;
	matrix_transpose( adjugate, cofactors );
	matrix_scalarMul( inverse, adjugate, invDet );

#if 0
	vector a, b, c;
	a = Vector( 0.5f, 1.2f, 3.0f, 1.0 );
	b = matrixVecMul( src, &a );
	c = matrixVecMul( inverse, &b );

	printf( "det: %.6f\n", other_det );
	printf( "Inverse:\n" );
	matrix_print( src );
	matrix_print( inverse );

	/*
	printf( "Inverse testing vectors: " );
	vector_print( &a );
	printf( "\n" );
	vector_print( &c );
	printf( "\n" );
	*/
	assert( vector_equal( &a, &c ));
#endif
}

// Convert a V matrix to an OGL matrix
const GLfloat* matrix_getGlMatrix( matrix m ) {
	return (const GLfloat*)m; }

// Multiply two matrices together
void matrix_mul( matrix dst, matrix m1, matrix m2 ) {
	matrix m;
	for ( int i = 0; i < 4; i++ ) {
		for ( int j = 0; j < 4; j++ ) {
			m[i][j] = m1[0][j] * m2[i][0]
						+ m1[1][j] * m2[i][1]
						+ m1[2][j] * m2[i][2]
						+ m1[3][j] * m2[i][3]; }}
	matrix_cpy( dst, m );
}

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

// Normalize the axes to be unit axes
void matrix_normalize( matrix m ) {
	vector v;
	for ( int i = 0; i < 4; i++ ) {
	   	Normalize( &v, matrix_getCol( m, i ));
		matrix_setColumn( m, i, &v );
	}
}

/*
matrix matrix_fromQuat( quaternion q ) {
	matrix m;
	return m;
}
*/

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

/*
matrix matrix_build( quaternion rot, vec trans ) {
	matrix m;
	return m;
}
*/

// Create a rotation matrix representing a rotation about the Y-axis (Yaw)
// of *angle* radians
// NOTE: the angle must be in radians, not degrees
// Axes are aligned as if +Z is into the screen, +Y is up, +X is right
// A Positive Yaw rotation means to yaw right, ie. as if turning a corner to the right
void matrix_rotY( matrix dst, float angle ) {
	float sinTheta = sin( angle );
	float cosTheta = cos( angle );
	matrix_setIdentity( dst );
	dst[0][0] = cosTheta;
	dst[0][2] = -sinTheta;
	dst[2][0] = sinTheta;
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
// Axes are aligned as if +Z is into the screen, +Y is up, +X is right
// A positive rotation (pitch) means to pitch up (ie. lean back)
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
	printf( "{ %.6f, %.6f, %.6f, %.6f\n ", src[0][0], src[1][0], src[2][0], src[3][0] );
	printf( "  %.6f, %.6f, %.6f, %.6f\n ", src[0][1], src[1][1], src[2][1], src[3][1] );
	printf( "  %.6f, %.6f, %.6f, %.6f\n ", src[0][2], src[1][2], src[2][2], src[3][2] );
	printf( "  %.6f, %.6f, %.6f, %.6f }\n ", src[0][3], src[1][3], src[2][3], src[3][3] );
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
	printf("TEST: Maths.c: identity det = %.2f\n", det );
	assert( f_eq( det, 1.f ));
	det = matrix_determinant( c );
	printf("TEST: Maths.c: det = %.2f\n", det );
	assert( f_eq( det, 1.f ));

	test_matrix_transpose();
	test_matrix_scalarMul();

	// Test Inverse
	// Inverse of Identity should be identity 
	matrix_setIdentity( a );
	matrix_setIdentity( b );
	matrix_inverse( c, b );
	assert( matrix_equal( c, a ) );

	a[0][0] = 0.f;
	a[1][0] = 0.5f;
	a[0][1] = -2.f;
	a[1][1] = 0.f;
	matrix_inverse( c, a );
	//matrix_print( a );
	//matrix_print( c );
	assert( f_eq( c[1][0], -0.5f ) );

	v = Vector( 0.4f, 0.2f, -3.f, 1.f );
	vector v2 = matrixVecMul( a, &v );
	v2 = matrixVecMul( c, &v2 );
	assert( vector_equal( &v, &v2 ));


}
