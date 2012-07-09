// vector.c
#include "common.h"
#include "vector.h"
//----------------------
#include "maths/maths.h"

vector Vector(float x, float y, float z, float w) {
	vector v;
	v.coord.x = x;
	v.coord.y = y;
	v.coord.z = z;
	v.coord.w = w;
	return v;
}

void Set(vector* v, float x, float y, float z, float w) {
	v->coord.x = x;
	v->coord.y = y;
	v->coord.z = z;
	v->coord.w = w;
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
	return (A->coord.x * B->coord.x + A->coord.y * B->coord.y + A->coord.z * B->coord.z);
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


vector vector_max( vector* a, vector* b ) {
	vector m;
	m.coord.x = fmaxf( a->coord.x, b->coord.x );
	m.coord.y = fmaxf( a->coord.y, b->coord.y );
	m.coord.z = fmaxf( a->coord.z, b->coord.z );
	m.coord.w = fmaxf( a->coord.w, b->coord.w );
	return m;
}

vector vector_min( vector* a, vector* b ) {
	vector m;
	m.coord.x = fminf( a->coord.x, b->coord.x );
	m.coord.y = fminf( a->coord.y, b->coord.y );
	m.coord.z = fminf( a->coord.z, b->coord.z );
	m.coord.w = fminf( a->coord.w, b->coord.w );
	return m;
}

float vector_distance( const vector* a, const vector* b ) {
	vector displacement;
	Sub( &displacement, a, b );
	return vector_length( &displacement );
}

bool vector_equal( const vector* a, const vector* b ) {
	// TODO: could this be optimized by just calculating all four in parallel
	// and then doing an &&?
	for (int i = 0; i < 4; i++)
		if ( !f_eq( ((float*)a)[i], ((float*)b)[i] ) ) {
			return false;
		}
	return true;
}

// *** Output
void vector_print( const vector* v ) {
	printf( "%.4f, %.4f, %.4f, %.4f", v->val[0], v->val[1], v->val[2], v->val[3] );
}

void vector_printf( const char* label, const vector* v ) {
	printf( "%s", label );
	vector_print( v );
	printf( "\n" );
}

#ifdef UNIT_TEST
void test_vector() {
	// Vector tests
}
#endif // UNIT_TEST
