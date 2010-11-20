// Maths.c

#include "common.h"
#include "maths.h"
//----------------------

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

vector Mul(matrix* m, vector* v) {
	vector dst;
	dst.coord.x = m->val[0][0] * v->coord.x + 
			m->val[0][1] * v->coord.y + 
			m->val[0][2] * v->coord.z + 
			m->val[0][3] * v->coord.w;
	dst.coord.y = m->val[1][0] * v->coord.x + 
			m->val[1][1] * v->coord.y + 
			m->val[1][2] * v->coord.z + 
			m->val[1][3] * v->coord.w;
	dst.coord.z = m->val[2][0] * v->coord.x + 
			m->val[2][1] * v->coord.y + 
			m->val[2][2] * v->coord.z + 
			m->val[2][3] * v->coord.w;
	dst.coord.w = m->val[3][0] * v->coord.x + 
			m->val[3][1] * v->coord.y + 
			m->val[3][2] * v->coord.z + 
			m->val[3][3] * v->coord.w;
	return dst;
}

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
	m->val[3][row] = v->val[3];
}

// Set a column in a matrix to a given vector
void matrix_setColumn(matrix* m, int col, const vector* v) {
	m->cols[col] = *v;
}

// Set the translation component of a 4x4 matrix
void matrix_setTranslation(matrix* m, const vector* v) {
	matrix_setColumn(m, 3, v);
}

// Get the translation component of a 4x4 matrix
vector* matrix_getTranslation(matrix* m) {
	return &m->cols[3];
}

// Initialise a matrix to the identity
void matrix_setIdentity(matrix* m) {
	m->val[0][0] = 1.f;
	m->val[0][1] = 0.f;
	m->val[0][2] = 0.f;
	m->val[0][3] = 0.f;
	m->val[1][0] = 0.f;
	m->val[1][1] = 1.f;
	m->val[1][2] = 0.f;
	m->val[1][3] = 0.f;
	m->val[2][0] = 0.f;
	m->val[2][1] = 0.f;
	m->val[2][2] = 1.f;
	m->val[2][3] = 0.f;
	m->val[3][0] = 0.f;
	m->val[3][1] = 0.f;
	m->val[3][2] = 0.f;
	m->val[3][3] = 1.f;
}

// Convert a V matrix to an OGL matrix
const GLfloat* matrix_getGlMatrix(const matrix* m) {
	return (const GLfloat*)m;
}

// Multiply two matrices together
matrix matrix_mul(matrix* m1, matrix* m2) {
	matrix dst;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			dst.val[i][j] = m1->val[0][j] * m2->val[i][0]
						+ m1->val[1][j] * m2->val[i][1]
						+ m1->val[2][j] * m2->val[i][2]
						+ m1->val[3][j] * m2->val[i][3];
		}
	}
	return dst;
}
