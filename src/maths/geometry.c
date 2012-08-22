// Geometry.c
#include "common.h"
#include "geometry.h"
//----------------------
#include "maths/maths.h"
#include "maths/vector.h"

// Calculate the normal and distance of a plane containing 3 points
void plane( vector a, vector b, vector c, vector* normal, float* d ) {
	// Calculate the plane of the 3 points
	vector ab, ac;
	Sub( &ab, &b, &a );
	Sub( &ac, &c, &a );
	Cross( normal, &ab, &ac );
	Normalize( normal, normal );
	*d = Dot( &a, normal );
}
