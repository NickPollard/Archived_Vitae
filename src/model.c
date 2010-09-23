// model.c

#include "common.h"
#include "model.h"
//-----------------------
#include "maths.h"
#include "transform.h"
#include "engine.h"

#include <GL/glut.h>

void vgl_vertexDraw(vector* v) {
	glVertex3fv((GLfloat*)v);
}

// Create a test mesh of a cube
mesh* mesh_createTestCube() {
	mesh* m = mesh_createMesh(/*verts*/ 8, /*indices*/ 36);
	Set(&m->verts[0], 1.f,  1.f,  1.f, 1.f);
	Set(&m->verts[1], 1.f,  1.f, -1.f, 1.f);
	Set(&m->verts[2], 1.f, -1.f,  1.f, 1.f);
	Set(&m->verts[3], 1.f, -1.f, -1.f, 1.f);
	Set(&m->verts[4],-1.f,  1.f,  1.f, 1.f);
	Set(&m->verts[5],-1.f,  1.f, -1.f, 1.f);
	Set(&m->verts[6],-1.f, -1.f,  1.f, 1.f);
	Set(&m->verts[7],-1.f, -1.f, -1.f, 1.f);

	m->indices[0] = 0;
	m->indices[1] = 1;
	m->indices[2] = 2;
	m->indices[3] = 1;
	m->indices[4] = 2;
	m->indices[5] = 3;

	m->indices[6] = 4;
	m->indices[7] = 5;
	m->indices[8] = 6;
	m->indices[9] = 5;
	m->indices[10] = 6;
	m->indices[11] = 7;

	m->indices[12] = 0;
	m->indices[13] = 1;
	m->indices[14] = 4;
	m->indices[15] = 1;
	m->indices[16] = 4;
	m->indices[17] = 5;

	m->indices[18] = 2;
	m->indices[19] = 3;
	m->indices[20] = 6;
	m->indices[21] = 3;
	m->indices[22] = 6;
	m->indices[23] = 7;

	m->indices[24] = 0;
	m->indices[25] = 2;
	m->indices[26] = 4;
	m->indices[27] = 2;
	m->indices[28] = 4;
	m->indices[29] = 6;

	m->indices[30] = 1;
	m->indices[31] = 3;
	m->indices[32] = 5;
	m->indices[33] = 3;
	m->indices[34] = 5;
	m->indices[35] = 7;

	return m;
}

// Create an empty mesh with vertCount distinct vertices and indexCount vertex indices
mesh* mesh_createMesh(int vertCount, int indexCount) {
	void* data = malloc(sizeof(mesh) + 
						(sizeof(vector) * vertCount) + 
						(sizeof(int) * indexCount));
	mesh* m = data;
	data += sizeof(mesh);
	m->verts = data;
	data += sizeof(vector) * vertCount;
	m->indices = data;
	m->vertCount = vertCount;
	m->indexCount = indexCount;
	return m;
}

// Create a test model of a cube
model* model_createTestCube() {
	model* m = model_createModel(/* meshCount */ 1);
	mesh* me = mesh_createTestCube();
	m->trans = transform_createTransform(theScene);
	m->meshes[0] = me;
	return m;
}

// Create an empty model with meshCount submeshes
model* model_createModel(int meshCount) {
	model* m = malloc(sizeof(model) +
						sizeof(mesh*) * meshCount);
	m->meshCount = meshCount;
	return m;
}

// Draw the verts of a mesh to the openGL buffer
void mesh_drawVerts(mesh* m) {
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < m->indexCount; i += 3) {
		vgl_vertexDraw(&m->verts[m->indices[i]]);
		vgl_vertexDraw(&m->verts[m->indices[i + 1]]);
		vgl_vertexDraw(&m->verts[m->indices[i + 2]]);
	}
	glEnd();
}

// Get the i-th submesh of a given model
mesh* model_getMesh(model* m, int i) {
	return m->meshes[i];
}

// Draw each submesh of a model
void model_draw(model* m) {
	glPushMatrix();
	glMultMatrixf(matrix_getGlMatrix(&m->trans->world));
	for (int i = 0; i < m->meshCount; i++) {
		mesh_drawVerts(model_getMesh(m, i));
	}
	glPopMatrix();
}
