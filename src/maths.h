// Maths.h
#ifndef __MATHS_H__
#define __MATHS_H__

#include "common.fwd.h"

union vector_u {
   	struct {
		float x;
		float y;
		float z;
		float w;
	} coord;
	float	val[4];
} ;

typedef union matrix_u {
	vector cols[4];
	float val[4][4];
} matrix;

// Vector Addition
void Add(vector* dst, vector* srcA, vector* srcB);

// Vector subtraction
void Sub(vector *dst, vector* srcA, vector* srcB);

// Vector dot product
float Dot(vector* A, vector* B);

// Vector cross product
void Cross(vector* dst, vector* srcA, vector* srcB);

// Matrix Vector multiply
vector Mul(matrix* m, vector* v);

void Set(vector* v, float x, float w, float z);

// Set a column in a matrix to a given vector
void matrix_setColumn(matrix* m, int col, vector* v);

// Set a row in a matrix to a given vector
void matrix_setRow(matrix* m, int row, vector* v);

// Set the translation component of a 4x4 matrix
void matrix_setTranslation(matrix* m, vector* v);

// Initialise a matrix to the identity
void matrix_setIdentity(matrix* m);

#endif // __MATHS_H__
