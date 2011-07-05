// model.c

#include "common.h"
#include "model.h"
//-----------------------
#include "maths.h"
#include "model_loader.h"
#include "engine.h"
#include "mem/allocator.h"
#include "render/modelinstance.h"
#include "render/texture.h"
#include "render/vgl.h"
#include "system/string.h"

#define maxModels 256

int model_count;
model* models[maxModels];
const char* modelFiles[maxModels];
int modelIDs[maxModels];

void vgl_vertexDraw(vector* v) {
	glVertex3fv((GLfloat*)v);
}

// Create a test mesh of a cube
mesh* mesh_createTestCube() {
	mesh* m = mesh_createMesh(/*verts*/ 8, /*indices*/ 36, /*normals*/ 0);
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
mesh* mesh_createMesh( int vertCount, int indexCount, int normal_count ) {
	void* data = mem_alloc( sizeof( mesh ) +
						sizeof( vector ) * vertCount +
						sizeof( int )	 * indexCount +
						sizeof( int )	 * indexCount +	// Normal indices
						sizeof( vector ) * normal_count );
	mesh* m = data;
	data += sizeof(mesh);
	m->verts = data;
	data += sizeof(vector) * vertCount;
	m->indices = data;
	data += sizeof( int ) * indexCount;
	m->normals = data;
	data += sizeof( vector ) * normal_count;
	m->normal_indices = data;

	m->vertCount = vertCount;
	m->indexCount = indexCount;
	m->normal_count = normal_count;

	return m;
}

// Precalculate flat normals for a mesh
void mesh_calculateNormals( mesh* m ) {
	int j = 0;
	for ( int i = 0; i < m->indexCount; i+=3 ) {
		// For now, calculate the normals at runtime from the three points of the triangle
		vector a, b, normal;
		Sub( &a, &m->verts[m->indices[i]], &m->verts[m->indices[i + 1]] );
		Sub( &b, &m->verts[m->indices[i]], &m->verts[m->indices[i + 2]] );
		Cross( &normal, &a, &b );
		Normalize( &normal, &normal );
		m->normal_indices[i+0] = j;
		m->normal_indices[i+1] = j;
		m->normal_indices[i+2] = j;
		m->normals[j++] = normal;
	}
}

// Create a test model of a cube
model* model_createTestCube( ) {
//	model* m = model_createModel(/* meshCount */ 1);
//	mesh* me = mesh_createTestCube();
//	m->meshes[0] = me;


	model* m = LoadObj( "dat/model/cityscape.obj" );
	return m;
}

// Create an empty model with meshCount submeshes
model* model_createModel(int meshCount) {
	model* m = mem_alloc(sizeof(model) +
						sizeof(mesh*) * meshCount);
	m->meshCount = meshCount;
	return m;
}

// Draw the verts of a mesh to the openGL buffer
void mesh_drawVerts( mesh* m ) {
	// Apply material properties
	vector diffuse = Vector( 1.0, 1.0, 1.0, 1.0);
	vector specular= Vector( 1.0, 1.0, 1.0, 1.0);
	glMaterialfv( GL_FRONT, GL_DIFFUSE, (GLfloat*)&diffuse );
	glMaterialfv( GL_FRONT, GL_SPECULAR, (GLfloat*)&specular );


	glBegin( GL_TRIANGLES );
	// Draw a triangle at a time
	for ( int i = 0; i < m->indexCount; i += 3 ) {
		glTexCoord2f( 0.f, 0.f ); // TODO - UVs in model format
		glNormal3fv( (GLfloat*)&m->normals[m->normal_indices[i + 0]] );
		vgl_vertexDraw( &m->verts[m->indices[i + 0]] );
		glTexCoord2f( 1.f, 0.f );
		glNormal3fv( (GLfloat*)&m->normals[m->normal_indices[i + 1]] );
		vgl_vertexDraw( &m->verts[m->indices[i + 1]] );
		glTexCoord2f( 0.f, 1.f );
		glNormal3fv( (GLfloat*)&m->normals[m->normal_indices[i + 2]] );
		vgl_vertexDraw( &m->verts[m->indices[i + 2]] );
	}
	glEnd();
}

// Get the i-th submesh of a given model
mesh* model_getMesh(model* m, int i) {
	return m->meshes[i];
}

// Draw each submesh of a model
void model_draw( model* m ) {
	vglBindTexture( g_texture_default );
	for (int i = 0; i < m->meshCount; i++) {
		mesh_drawVerts( model_getMesh( m, i ));
	}
}

void model_initModelStorage() {
	memset( models, 0, sizeof( model* ) * maxModels );
	memset( modelFiles, 0, sizeof( const char* ) * maxModels );
	model_count = 0;
}

model* model_getByHandle( modelHandle h ) {
	return models[h];
}

// Synchronously load a model from a given file
model* model_loadFromFileSync( const char* filename ) {
	// TODO: Implement
//	return model_createTestCube();
	return LoadObj( filename );
}

const char* model_getFileNameFromID( int id ) {
	// TODO: Implement
	return NULL;
}

modelHandle model_getHandleFromID( int id ) {
	// If the model is already in the array, return it
	for ( int i = 0; i < model_count; i++ ) {
		if ( modelIDs[i] == id ) {
			return (modelHandle)i;
		}
	}
	// Otherwise add it and return
	assert( model_count < maxModels );
	modelHandle handle = (modelHandle)model_count;
	modelIDs[handle] = id;
	models[handle] = model_loadFromFileSync( model_getFileNameFromID( id ) );
	model_count++;
	return handle;
}

// TODO - debug; should be replaced with hashed ID
modelHandle model_getHandleFromFilename( const char* filename ) {
	// If the model is already in the array, return it
	for ( int i = 0; i < model_count; i++ ) {
		if ( string_equal( filename, modelFiles[i] )) {
			return (modelHandle)i;
		}
	}
	// Otherwise add it and return
	assert( model_count < maxModels );
	modelHandle handle = (modelHandle)model_count;
	modelFiles[handle] = string_createCopy( filename );
	models[handle] = model_loadFromFileSync( filename );
	model_count++;
	return handle;
}

model* model_fromInstance( modelInstance* instance ) {
	return model_getByHandle( instance->model );
}
