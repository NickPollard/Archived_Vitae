// matrix.h
#pragma once
#include "maths/mathstypes.h"
#include "render/vgl.h"

vector matrixVecMul(matrix m, const vector* v);

// Get the inverse of a 4x4 matrix
void matrix_inverse( matrix dst, matrix src );

void matrix_setColumn(matrix m, int col, const vector* v);
void matrix_setRow(matrix m, int row, const vector* v);
void matrix_setRotation( matrix dst, matrix src );
const vector* matrix_getCol( matrix m, int i );

// Set the translation component of a 4x4 matrix
void matrix_setTranslation(matrix m, const vector* v);
void matrix_clearTranslation( matrix m );
const vector* matrix_getTranslation(matrix m);

// Initialise a matrix to the identity
void matrix_setIdentity(matrix m);

// Convert a V matrix to an OpenGL matrix
const GLfloat* matrix_getGlMatrix(matrix m);

// Multiply two matrices together
void matrix_mul(matrix dst, matrix m1, matrix m2);

// Build a matrix from a rotation and translation
void matrix_fromRotTrans( matrix dst, quaternion* rotation, vector* translation );

// Copy one matrix to another
void matrix_cpy(matrix dst, matrix src);

// Build a rotation matrix from given Euler Angles
void matrix_fromEuler( matrix dst, vector* euler_angles );

// Normalize a matrix (so the 3x3 rotation component consists of 3 unit axes)
void matrix_normalize( matrix m );

// Create a rotation matrix (leaving scale and translation as before - unset if it's a new matrix)
void matrix_rotX( matrix dst, float angle );
void matrix_rotY( matrix dst, float angle );
void matrix_rotZ( matrix dst, float angle );

// Print a matrix to the output
void matrix_print( matrix src );

#ifdef UNIT_TEST
void test_matrix();
#endif // UNIT_TEST
