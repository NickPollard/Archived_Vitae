// debugdraw.c

#include "src/common.h"
#include "src/render/debugdraw.h"
//-----------------------


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
