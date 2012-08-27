// Geometry.h
#pragma once

#include "maths/mathstypes.h"

void plane( vector a, vector b, vector c, vector* normal, float* d );

float segment_closestPoint( vector a, vector b, vector point, vector* closest );
