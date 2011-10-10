// model.h
#pragma once

#include "common.fwd.h"
#include "maths.h"
#include "render/render.h"

#define kMaxSubMeshes 4
#define kMaxHardPoints	16
#define kMaxSubTransforms	16
#define kMaxSubEmitters		16

// *** Mesh ***
/*
   A Mesh contains a list of vertices and a list of triangles (as indices to the vertex array)
   */
struct mesh_s {
	transform*	trans;
	shader*		shader;
	//
	int			vert_count;
	vector*		verts;
	//
	int			index_count;
	uint16_t*	indices;
	//
	int			normal_count;
	vector*		normals;
	//
	uint16_t*	normal_indices;
	//
	vector*		uvs;
	int			uv_count;
	uint16_t*	uv_indices;

	vertex*			vertex_buffer;
	unsigned short*	element_buffer;

	GLuint		texture_diffuse;

	//
	GLuint*		vertex_VBO;
	GLuint*		element_VBO;
};

// *** Model ***
/*
   A Model contains many meshes, each of which use a given shader
   */
struct model_s {
	int			meshCount;
	mesh*		meshes[kMaxSubMeshes];
	// Sub-elements
//	transform*	hardpoints[kMaxHardPoints];
	int	transform_count;
	int emitter_count;
	transform*			transforms[kMaxSubTransforms];
	particleEmitter*	emitters[kMaxSubEmitters];
};


// *** Mesh Functions

// Create an empty mesh with vertCount distinct vertices and index_count vertex indices
mesh* mesh_createMesh( int vertCount, int index_count, int normal_count, int uv_count );

// Draw the verts of a mesh to the openGL buffer
void mesh_render(mesh* m);

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

// Sub-element lookups
int model_transformIndex( model* m, transform* ptr );
