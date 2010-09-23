// model.h
#ifndef __MODEL_H__
#define __MODEL_H__

#include "common.fwd.h"

#define MAX_SUBMESHES 4

// *** Mesh ***
/*
   A Mesh contains a list of vertices and a list of triangles (as indices to the vertex array)
   */
struct mesh_s {
	transform*	trans;
	int			vertCount;
	vector*		verts;
	int			indexCount;
	int*		indices;
};

// *** Model ***
/*
   A Model contains many meshes, each of which use a given shader
   */
struct model_s {
	transform*	trans;
	int			meshCount;
	mesh*		meshes[MAX_SUBMESHES];
};

// Create a test mesh of a cube
mesh* mesh_createTestCube();

// Create an empty mesh with vertCount distinct vertices and indexCount vertex indices
mesh* mesh_createMesh(int vertCount, int indexCount);

// Create a test model of a cube
model* model_createTestCube();

// Create an empty model with meshCount submeshes
model* model_createModel(int meshCount);

// Draw the verts of a mesh to the openGL buffer
void mesh_drawVerts(mesh* m);

// Draw each submesh of a model
void model_draw(model* m);

#endif // __MODEL_H__
