// model.h
#pragma once

#include "common.fwd.h"

#define MAX_SUBMESHES 4

// *** Mesh ***
/*
   A Mesh contains a list of vertices and a list of triangles (as indices to the vertex array)
   */
struct mesh_s {
	transform*	trans;
	//
	int			vertCount;
	vector*		verts;
	//
	int			indexCount;
	int*		indices;
	//
	int			normal_count;
	vector*		normals;
	//
	int*		normal_indices;
};

// *** Model ***
/*
   A Model contains many meshes, each of which use a given shader
   */
struct model_s {
	int			meshCount;
	mesh*		meshes[MAX_SUBMESHES];
};



// *** Mesh Functions

// Create a test mesh of a cube
mesh* mesh_createTestCube();

// Create an empty mesh with vertCount distinct vertices and indexCount vertex indices
mesh* mesh_createMesh(int vertCount, int indexCount, int normal_count );

// Precalculate flat normals for a mesh
void mesh_calculateNormals( mesh* m );

// Draw the verts of a mesh to the openGL buffer
void mesh_drawVerts(mesh* m);



// *** Model Functions

// Create a test model of a cube
model* model_createTestCube( );

// Create an empty model with meshCount submeshes
model* model_createModel(int meshCount);

// Draw each submesh of a model
void model_draw(model* m);

modelHandle model_getHandleFromID( int id );

model* model_fromInstance( modelInstance* instance );

modelHandle model_getHandleFromFilename( const char* filename );
