// model.c

#include "common.h"
#include "model.h"
//-----------------------
#include "maths.h"

#include <GL/glut.h>

void vitae_glVertexVector(vector* v) {
	glVertex3fv((GLfloat*)v);
}

// Create a test mesh of a cube
mesh* mesh_createTestCube() {
	mesh* m = mesh_createMesh(/*verts*/ 8, /*indices*/ 36);
	Set(&m->verts[0], 1.f,  1.f,  1.f);
	Set(&m->verts[1], 1.f,  1.f, -1.f);
	Set(&m->verts[2], 1.f, -1.f,  1.f);
	Set(&m->verts[3], 1.f, -1.f, -1.f);
	Set(&m->verts[4],-1.f,  1.f,  1.f);
	Set(&m->verts[5],-1.f,  1.f, -1.f);
	Set(&m->verts[6],-1.f, -1.f,  1.f);
	Set(&m->verts[7],-1.f, -1.f, -1.f);
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
// TODO	m->trans = ;
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
		vitae_glVertexVector(&m->verts[m->indices[i]]);
		vitae_glVertexVector(&m->verts[m->indices[i + 1]]);
		vitae_glVertexVector(&m->verts[m->indices[i + 2]]);
	}
	glEnd();
}

// Get the i-th submesh of a given model
mesh* model_getMesh(model* m, int i) {
	return m->meshes[i];
}

// Draw each submesh of a model
void model_draw(model* m) {
	for (int i = 0; i < m->meshCount; i++) {
		mesh_drawVerts(model_getMesh(m, i));
	}
}
