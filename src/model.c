// model.c

#include "common.h"
#include "model.h"
//-----------------------
#include "maths.h"
#include "model_loader.h"
#include "engine.h"
#include "mem/allocator.h"
#include "render/modelinstance.h"
#include "render/render.h"
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

// Create an empty mesh with vertCount distinct vertices and index_count vertex indices
mesh* mesh_createMesh( int vertCount, int index_count, int normal_count ) {
	void* data = mem_alloc( sizeof( mesh ) +
						sizeof( vector ) * vertCount +
						sizeof( int )	 * index_count +
						sizeof( int )	 * index_count +	// Normal indices
						sizeof( vector ) * normal_count );
	mesh* m = data;
	data += sizeof(mesh);
	m->verts = data;
	data += sizeof(vector) * vertCount;
	m->indices = data;
	data += sizeof( int ) * index_count;
	m->normals = data;
	data += sizeof( vector ) * normal_count;
	m->normal_indices = data;

	m->vertCount = vertCount;
	m->index_count = index_count;
	m->normal_count = normal_count;

	return m;
}

// Precalculate flat normals for a mesh
void mesh_calculateNormals( mesh* m ) {
	int j = 0;
	for ( int i = 0; i < m->index_count; i+=3 ) {
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

GLfloat vertex_buffer_data_m[] = { 1.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 1.f, -1.f, 0.f, 0.f, 1.f, 0.f, -1.f, -0.f, 1.f };
GLushort element_buffer_data_m[] = { 0, 1, 3, 1, 3, 2 };

// Build a vertex buffer for the mesh, copying out vertices as necessary so that
// each vertex-normal pair is covered
// If fully smooth-shaded, can just use a single copy of each vertex
// If not all smooth-shaded, will have to duplicate vertices
void mesh_buildBuffers( mesh* m ) {
	assert( !m->vertex_buffer );
	assert( !m->element_buffer );
	m->vertex_buffer = mem_alloc( sizeof( vertex ) * m->index_count );
	m->element_buffer = mem_alloc( sizeof( GLushort ) * m->index_count );

	bool all_smooth_shaded = false;
	if ( all_smooth_shaded ) {

	} else {
		printf( "Build Buffers.\n" );
		// allocate space

		// For each element index
		// Unroll the vertex/index bindings
		for ( int i = 0; i < m->index_count; i++ ) {
			printf( "Vert num: %d, index %d.\n", i, m->indices[i] );
			// Copy the required vertex position
			m->vertex_buffer[i].position = m->verts[m->indices[i]];
			// Copy the required vertex normal
			m->vertex_buffer[i].normal = m->normals[m->normal_indices[i]];
			m->element_buffer[i] = i;
			printf( "Vert: " );
			vector_print( &m->vertex_buffer[i].position );
			printf( "\n" );
		}
	}
}

// Draw the verts of a mesh to the openGL buffer
void mesh_drawVerts( mesh* m ) {
	// Copy our data to the GPU
	// There are now <index_count> vertices, as we have unrolled them
	GLsizei vertex_buffer_size = m->index_count * sizeof( vertex );
	render_setBuffers( (GLfloat*)m->vertex_buffer, vertex_buffer_size, (int*)m->element_buffer, m->index_count * sizeof( GLushort ) );

	// Activate our buffers
	glBindBuffer( GL_ARRAY_BUFFER, resources.vertex_buffer );
	// Set up position data
	glVertexAttribPointer( resources.attributes.position, /*vec4*/ 4, GL_FLOAT, /*Normalized?*/GL_FALSE, sizeof( vertex ), (void*)offsetof( vertex, position) );
	glEnableVertexAttribArray( resources.attributes.position );
	// Set up normal data
	glVertexAttribPointer( resources.attributes.normal, /*vec4*/ 4, GL_FLOAT, /*Normalized?*/GL_FALSE, sizeof( vertex ), (void*)offsetof( vertex, normal ) );
	glEnableVertexAttribArray( resources.attributes.normal );

	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, resources.element_buffer );

	// Draw!
	glDrawElements( GL_TRIANGLES, m->index_count, GL_UNSIGNED_SHORT, (void*)0 );

	// Cleanup
	glDisableVertexAttribArray( resources.attributes.position );
	glDisableVertexAttribArray( resources.attributes.normal );
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
