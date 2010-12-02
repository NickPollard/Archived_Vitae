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

typedef union matrix_u {
	vector cols[4];
	float val[4][4];
} matrix;

vector Vector(float x, float y, float z, float w);

// Vector Addition
void Add(vector* dst, vector* srcA, vector* srcB);

// Vector subtraction
void Sub(vector *dst, vector* srcA, vector* srcB);

// Vector dot product
float Dot(vector* A, vector* B);

// Vector cross product
void Cross(vector* dst, vector* srcA, vector* srcB);

// Matrix Vector multiply
void matrixVecMul(vector* out, matrix* m, vector* in);

void Set(vector* v, float x, float y, float z, float w);

// Set a column in a matrix to a given vector
void matrix_setColumn(matrix* m, int col, const vector* v);

// Set a row in a matrix to a given vector
void matrix_setRow(matrix* m, int row, const vector* v);

// Set the translation component of a 4x4 matrix
void matrix_setTranslation(matrix* m, const vector* v);

// Get the translation component of a 4x4 matrix
vector* matrix_getTranslation(matrix* m);

// Initialise a matrix to the identity
void matrix_setIdentity(matrix* m);

// Convert a V matrix to an OGL matrix
const GLfloat* matrix_getGlMatrix(const matrix* m);

// Multiply two matrices together
void matrix_mul(matrix* dst, matrix* m1, matrix* m2);

// Copy one matrix to another
void matrix_cpy(matrix* dst, matrix* src);

#endif // __MATHS_H__
