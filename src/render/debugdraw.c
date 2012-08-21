// debugdraw.c

#include "common.h"
#include "render/debugdraw.h"
//-----------------------
#include "maths/maths.h"
#include "render/render.h"

#include "render/vgl.h"

// The total number of verts per frame we can draw using the debug draw system
#define kMaxDebugDrawVerts 1024

// Static buffers used to store debug draw verts and indices - these are wiped each frame
vertex debugDraw_vertex_buffer[kMaxDebugDrawVerts];
GLushort debugDraw_element_buffer[kMaxDebugDrawVerts];
int debugDraw_verts_used;

void debugdraw_line2d( vector from, vector to, vector color ) {
	// Grab a vertex and element buffer from our static ones
	int vert_count = 2;
	vertex* vertex_buffer = &debugDraw_vertex_buffer[debugDraw_verts_used];
	GLushort* element_buffer = &debugDraw_element_buffer[debugDraw_verts_used];
	debugDraw_verts_used += vert_count;

	vertex_buffer[0].position = from;
	vertex_buffer[1].position = to;
	vertex_buffer[0].color = color;
	vertex_buffer[1].color = color;

	element_buffer[0] = 0;
	element_buffer[1] = 1;

	matrix modelview;

	// Make a drawcall
	const GLuint no_texture = 0;
	drawCall* draw = drawCall_create( &renderPass_debug, resources.shader_debug_2d, vert_count, element_buffer, vertex_buffer, no_texture, modelview );
	draw->elements_mode = GL_LINES;
}

void debugdraw_preTick( float dt ) {
	(void)dt;
	debugDraw_verts_used = 0;
}

	/*
// Draw a debug cross at the point *center*
void debugdraw_cross( const vector* center, float radius ) {
	glBegin(GL_LINES);
	// horizonal
	glVertex3f(center->coord.x - radius, center->coord.y, center->coord.z);
	glVertex3f(center->coord.x + radius, center->coord.y, center->coord.z);
	// vertical
	glVertex3f(center->coord.x, center->coord.y - radius, center->coord.z);
	glVertex3f(center->coord.x, center->coord.y + radius, center->coord.z);
	// directional
	glVertex3f(center->coord.x, center->coord.y, center->coord.z - radius);
	glVertex3f(center->coord.x, center->coord.y, center->coord.z + radius);
	glEnd();
}

void debugdraw_drawRect2D( vector* tl, vector* br ) {
	render_set2D();
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glPushMatrix();

	glBegin( GL_TRIANGLES );

	glColor4f( 1.f, 1.f, 1.f, 1.f );
	glNormal3f( 0.f, 0.f, 1.f );

	glTexCoord2f( tl->coord.z, tl->coord.w );
	glVertex3f( tl->coord.x, tl->coord.y, -0.5f );	

	glTexCoord2f( tl->coord.z, br->coord.w );
	glVertex3f( tl->coord.x, br->coord.y, -0.5f );	

	glTexCoord2f( br->coord.z, tl->coord.w );
	glVertex3f( br->coord.x, tl->coord.y, -0.5f );	

	glTexCoord2f( br->coord.z, tl->coord.w );
	glVertex3f( br->coord.x, tl->coord.y, -0.5f );	

	glTexCoord2f( tl->coord.z, br->coord.w );
	glVertex3f( tl->coord.x, br->coord.y, -0.5f );	

	glTexCoord2f( br->coord.z, br->coord.w );
	glVertex3f( br->coord.x, br->coord.y, -0.5f );	

	glEnd();
	glPopMatrix();
}
	*/
