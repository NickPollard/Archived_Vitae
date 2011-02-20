// debugdraw.c

#include "src/common.h"
#include "src/render/debugdraw.h"
//-----------------------
#include "src/maths.h"
#include "src/render/render.h"
// GLFW Libraries
#include <GL/glfw.h>

// Draw a debug cross at the point *center*
void debugdraw_cross(vector* center, float radius) {
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

	glTexCoord2f( 0.f, 0.f );
	glVertex3f( tl->coord.x, tl->coord.y, -0.5f );	

	glTexCoord2f( 1.f, 0.f );
	glVertex3f( tl->coord.x, br->coord.y, -0.5f );	

	glTexCoord2f( 0.f, 1.f );
	glVertex3f( br->coord.x, tl->coord.y, -0.5f );	

	glTexCoord2f( 0.f, 1.f );
	glVertex3f( br->coord.x, tl->coord.y, -0.5f );	

	glTexCoord2f( 1.f, 0.f );
	glVertex3f( tl->coord.x, br->coord.y, -0.5f );	

	glTexCoord2f( 1.f, 1.f );
	glVertex3f( br->coord.x, br->coord.y, -0.5f );	

	glEnd();
	glPopMatrix();
}
