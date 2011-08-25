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
#include "render/shader.h"
#include "render/texture.h"
#include "render/vgl.h"
#include "system/hash.h"
#include "system/string.h"

#define maxModels 256

int model_count;
model* models[maxModels];
const char* modelFiles[maxModels];
int modelIDs[maxModels];

void vgl_vertexDraw(vector* v) {
	glVertex3fv((GLfloat*)v);
}

// Create an empty mesh with vertCount distinct vertices and index_count vertex indices
mesh* mesh_createMesh( int vertCount, int index_count, int normal_count, int uv_count ) {
	void* data = mem_alloc( sizeof( mesh ) +
						sizeof( vector ) * vertCount +		// Verts
						sizeof( int )	 * index_count +	// Indices
						sizeof( vector )	 * uv_count +	// UVs
						sizeof( int )	 * index_count +	// UV indices
						sizeof( int )	 * index_count +	// Normal indices
						sizeof( vector ) * normal_count );	// Normals
	mesh* m = data;
	data += sizeof(mesh);
	m->verts = data;
	data += sizeof(vector) * vertCount;
	m->indices = data;
	data += sizeof( int ) * index_count;
	m->normals = data;
	data += sizeof( vector ) * normal_count;
	m->normal_indices = data;
	data += sizeof( int ) * index_count;
	m->uvs = data;
	data += sizeof( vector ) * uv_count;
	m->uv_indices = data;

	m->vertCount = vertCount;
	m->index_count = index_count;
	m->normal_count = normal_count;

	m->vertex_buffer = NULL;
	m->element_buffer = NULL;

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
			// Copy the required vertex position
			m->vertex_buffer[i].position = m->verts[m->indices[i]];
			// Copy the required vertex normal
			m->vertex_buffer[i].normal = m->normals[m->normal_indices[i]];
			m->vertex_buffer[i].uv = m->uvs[m->uv_indices[i]];
			m->element_buffer[i] = i;
		}
	}
}

#define VERTEX_ATTRIBS( f ) \
	f( position ) \
	f( normal ) \
	f( uv )

#define VERTEX_ATTRIB_DISABLE_ARRAY( attrib ) \
	glDisableVertexAttribArray( attrib );

#define VERTEX_ATTRIB_LOOKUP( attrib ) \
	GLint attrib = *(shader_findConstant( mhash( #attrib )));

#define VERTEX_ATTRIB_POINTER( attrib ) \
	glVertexAttribPointer( attrib, /*vec4*/ 4, GL_FLOAT, /*Normalized?*/GL_FALSE, sizeof( vertex ), (void*)offsetof( vertex, attrib) ); \
	glEnableVertexAttribArray( attrib );

// Draw the verts of a mesh to the openGL buffer
void mesh_drawVerts( mesh* m ) {
	glDepthMask( GL_TRUE );
	// Copy our data to the GPU
	// There are now <index_count> vertices, as we have unrolled them
	GLsizei vertex_buffer_size = m->index_count * sizeof( vertex );
	GLsizei element_buffer_size = m->index_count * sizeof( GLushort );

	// Textures
	GLint* tex = shader_findConstant( mhash( "tex" ));
	if ( tex )
		render_setUniform_texture( *tex, g_texture_default );

	VERTEX_ATTRIBS( VERTEX_ATTRIB_LOOKUP );
	// *** Vertex Buffer
	glBindBuffer( GL_ARRAY_BUFFER, resources.vertex_buffer );
	glBufferData( GL_ARRAY_BUFFER, vertex_buffer_size, m->vertex_buffer, GL_STREAM_DRAW );
	VERTEX_ATTRIBS( VERTEX_ATTRIB_POINTER );
	// *** Element Buffer
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, resources.element_buffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, element_buffer_size, m->element_buffer, GL_STREAM_DRAW );

	// Draw!
	glDrawElements( GL_TRIANGLES, m->index_count, GL_UNSIGNED_SHORT, (void*)0 );

	// Cleanup
	VERTEX_ATTRIBS( VERTEX_ATTRIB_DISABLE_ARRAY )
}

// Get the i-th submesh of a given model
mesh* model_getMesh(model* m, int i) {
	return m->meshes[i];
}

// Draw each submesh of a model
void model_draw( model* m ) {
//	vglBindTexture( g_texture_default );
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
