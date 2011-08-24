// model.h
#pragma once

#include "common.fwd.h"
#include "maths.h"

#define MAX_SUBMESHES 4

typedef struct vertex_s {
	vector	position;
	vector	normal;
	vector	uv;
} vertex;

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
	int			index_count;
	unsigned short*		indices;
	//
	int			normal_count;
	vector*		normals;
	//
	int*		normal_indices;
	//
	vector*		uvs;
	int			uv_count;
	int*		uv_indices;

	vertex*		vertex_buffer;
	unsigned short*		element_buffer;
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

// Create an empty mesh with vertCount distinct vertices and index_count vertex indices
mesh* mesh_createMesh( int vertCount, int index_count, int normal_count, int uv_count );

// Draw the verts of a mesh to the openGL buffer
void mesh_drawVerts(mesh* m);

// Precalculate flat normals for a mesh
void mesh_calculateNormals( mesh* m );

void mesh_buildBuffers( mesh* m );



// *** Model Functions

// Create a test model of a cube
model* model_createTestCube( );

// Create an empty model with meshCount submeshes
model* model_createModel(int meshCount);

// Draw each submesh of a model
void model_draw(model* m);

model* model_fromInstance( modelInstance* instance );

// Handle lookups
modelHandle model_getHandleFromID( int id );
modelHandle model_getHandleFromFilename( const char* filename );
