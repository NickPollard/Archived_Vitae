// Maths.c

#include "maths.h"

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

void Set(vector* v, float x, float y, float z) {
	v->coord.x = x;
	v->coord.y = y;
	v->coord.z = z;
}

// Set a row in a matrix to a given vector
void matrix_setRow(matrix* m, int row, vector* v) {
	m->val[0][row] = v->val[0];
	m->val[1][row] = v->val[1];
	m->val[2][row] = v->val[2];
	m->val[3][row] = v->val[3];
}

// Set a column in a matrix to a given vector
void matrix_setColumn(matrix* m, int col, vector* v) {
	m->cols[col] = *v;
}

// Set the translation component of a 4x4 matrix
void matrix_setTranslation(matrix* m, vector* v) {
	matrix_setColumn(m, 3, v);
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
