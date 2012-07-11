// quaternion.h
#pragma once

#include "maths/mathstypes.h"

// Constructor
quaternion Quaternion( float s, float x, float y, float z );

// Are two quaternions equal value?
bool quaternion_equal( quaternion a, quaternion b );

// Quaternion Multiplication a * b - Hamilton product
quaternion quaternion_multiply( quaternion a, quaternion b );

// Conjugate quaternion b with quaternion a, that is (a.b.a')
quaternion quaternion_conjugation( quaternion a, quaternion b );

// Get the inverse q' of quaternion q
quaternion quaternion_inverse( quaternion q );

// Build a rotation quaternion for a rotation of ANGLE about AXIS
quaternion quaternion_fromAxisAngle( vector axis, float angle );
// Get the AXIS and ANGLE for the rotation described by quaternion Q
void quaternion_getAxisAngle( quaternion q, vector* axis, float* angle );

// Build a rotation quaternion from Euler Angle values
quaternion quaternion_fromEuler( vector* euler_angles );

#ifdef UNIT_TEST
void test_quaternion();
#endif // UNIT_TEST
