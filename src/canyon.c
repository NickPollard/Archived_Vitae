// canyon.c
#include "common.h"
#include "canyon.h"
//-----------------------
#include "vtime.h"
#include "maths/geometry.h"
#include "maths/maths.h"
#include "maths/vector.h"
#include "render/debugdraw.h"
#include "render/render.h"

#include <float.h>

// *** New Terrain Canyon

/*
   The new canyon is made from a quadratic Bezier-spline. We (randomly) generate piecewise linear segments
   and then make each contiguous trio into a quadratic Bezier curve. By splitting every segment in half, we
   can ensure that the control points for each consecutive quadratic curve lie upon the same line so that we
   do not have a discontinuity in the first derivative - so the canyon will be smooth.
   */

#define kMaxCanyonPoints 128
vector	canyon_points[kMaxCanyonPoints];
#define kCanyonSegmentLength 20.f

float canyon_v( int segment_index, float segment_prog ) {
	return segment_prog + (float)segment_index * kCanyonSegmentLength;
}

// Convert world-space X and Z coords into canyon space U and V
void terrain_canyonSpaceFromWorld( float x, float z, float* u, float* v ) {
	vector point = Vector( x, 0.f, z, 1.f );
	int closest_i = 0;
	float closest_d = FLT_MAX;
	// We need to find the closest segment
	for ( int i = 1; i + 1 < kMaxCanyonPoints; ++i ) {
		vector displacement = vector_sub( point, canyon_points[i] );
		float d = vector_length( &displacement );
		if ( d < closest_d ) {
			closest_d = d;
			closest_i = i;
		}
	}
	// find closest points on the two segments using that point
	vector closest_a, closest_b;
	float seg_pos_a = segment_closestPoint( canyon_points[closest_i], canyon_points[closest_i+1], point, &closest_a );
	float seg_pos_b = segment_closestPoint( canyon_points[closest_i-1], canyon_points[closest_i], point, &closest_b );
	// use the closest
	float length_a = vector_lengthI( vector_sub( point, closest_a ));
	float length_b = vector_lengthI( vector_sub( point, closest_b ));
	if ( length_a < length_b ) {
		*u = length_a;
		*v = canyon_v( closest_i, seg_pos_a );
	} else {
		*u = length_a;
		*v = canyon_v( closest_i - 1, seg_pos_b );
	}
	//printf( "terrain_canyonSpaceFromWorld: x,z ( %.2f, %.2f ) to u, v ( %.2f, %.2f )\n", x, z, *u, *v );
}

int terrainCanyon_segmentAtDistance( float v ) {
	return (float)floorf(v / kCanyonSegmentLength);
}

// Convert canyon-space U and V coords into world space X and Z
void terrain_worldSpaceFromCanyon( float u, float v, float* x, float* z ) {
	int i = terrainCanyon_segmentAtDistance( v );
	float segment_position = v - (float)i * kCanyonSegmentLength;

	vector canyon_position = vector_lerp( &canyon_points[i+1], &canyon_points[i], segment_position );
	vector segment = normalized( vector_sub( canyon_points[i+1], canyon_points[i] ));
	vector normal = Vector( segment.coord.z, 0.f, segment.coord.x, 0.f );
	vAssert( isNormalized( &normal ));
	vector u_offset = vector_scaled( normal, u );
	vector position = vector_add( canyon_position, u_offset );
	*x = position.coord.x;
	*z = position.coord.z;
}

const float new_base_radius = 20.f;
const float new_canyon_width = 20.f;
const float new_canyon_height = 40.f;

// Sample the canyon height (Y) at a given world X and Z
float terrain_newCanyonHeight( float x, float z ) {
	float u, v;
	terrain_canyonSpaceFromWorld( x, z, &u, &v );

	u = ( u < 0.f ) ? fminf( u + new_base_radius, 0.f ) : fmaxf( u - new_base_radius, 0.f );
	
	// Turn the canyon-space U into a height
	float mask = cos( fclamp( u / new_canyon_width, -PI/2.f, PI/2.f ));
	return (1.f - fclamp( powf( u / new_canyon_width, 4.f ), 0.f, 1.f )) * mask * new_canyon_height;
}

vector terrain_newCanyonPoint( vector current, vector previous ) {
	// We want to generate a point in front
	// TODO

	(void)previous;

	// Generate a forward facing angle
	vector direction = vector_sub( current, previous );
	Normalize( &direction, &direction );
	float previous_angle = acosf( direction.coord.x );

	//float previous_angle = PI / 2.f;
	const float max_angle_delta = PI / 8.f;

	float min_angle = fmaxf( 0.f, previous_angle - max_angle_delta );
	float max_angle = fminf( PI, previous_angle + max_angle_delta );
	float angle = frand( min_angle, max_angle );
	vector offset = Vector( cosf( angle ) * kCanyonSegmentLength, 0.f, sinf( angle ) * kCanyonSegmentLength, 0.f );

	/*
	// Temp just walk forward
	vector next = current;
	next.coord.z = next.coord.z + kCanyonSegmentLength;
	*/
	vector next;
	Add( &next, &current, &offset );
	
	return next;
}


void terrain_updateCanyonPoints( vector active_position ) {
	(void)active_position;
	/*
	int old_min, old_max;
	// Calculate a new range
	const int segments_behind = 10;
	float u, v;
	terrain_canyonSpaceFromWorld( active_position.coord.x, active_position.coord.z, &u, &v );
	int active_segment = terrainCanyon_segmentAtDistance( v );
	new_min = active_segment - segments_behind;
	new_max = new_min + kMaxCanyonPoints;
	*/
}

// Generate X points of the canyon
void terrain_generatePoints() {
	// Each point is a given distance and angle from the previous point/segment
	canyon_points[0] = Vector( 0.f, 0.f, 0.f, 1.f );
	canyon_points[1] = Vector( 0.f, 0.f, kCanyonSegmentLength, 1.f );
	for ( int i = 2; i < kMaxCanyonPoints; ++i ) {
		canyon_points[i] = terrain_newCanyonPoint( canyon_points[i-1], canyon_points[i-2] );
	}
}

vector  terrainSegment_toScreen( window* w, vector world ) {
	const float visible_width = 200.f;
	const float visible_length = 200.f;
	const float x_pos = 400.f;
	const float y_pos = 50.f;
	vector screen = Vector( x_pos + ( world.coord.x / visible_width ) * w->width,
		   	y_pos + ( world.coord.z / visible_length ) * w->height,
		   	0.f,
		   	1.f );
	return screen;
}

void terrain_drawSegment( window* w, vector a, vector b ) {
	vector a_2d = terrainSegment_toScreen( w, a );
	vector b_2d = terrainSegment_toScreen( w, b );
	debugdraw_line2d( a_2d, b_2d, color_green );
}

// Draw debug lines on the screen showing the path of the canyon we have generated
void terrain_debugDraw( window* w ) {
	int total_points = kMaxCanyonPoints;

	for ( int i = 0; i+1 < total_points; ++i ) {
		// draw line segment from (i) to (i+1)
		terrain_drawSegment( w, canyon_points[i], canyon_points[i+1] );
	}
}
