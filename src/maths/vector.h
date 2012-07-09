// vector.h
#pragma once

#include "maths/mathstypes.h"

vector Vector(float x, float y, float z, float w);

void Set(vector* v, float x, float y, float z, float w);

void Add(vector* dst, const vector* srcA, const vector* srcB);
void Sub(vector *dst, const vector* srcA, const vector* srcB);
float Dot( const vector* A, const vector* B );
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

float vector_length( const vector* v );
float vector_distance( const vector* a, const vector* b );

bool vector_equal( const vector* a, const vector* b );

// *** Output
void vector_print( const vector* v );
void vector_printf( const char* label, const vector* v );

#ifdef UNIT_TEST
void test_vector();
#endif // UNIT_TEST
