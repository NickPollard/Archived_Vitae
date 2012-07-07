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

#define kMaxModels 256

int			model_count;
model*		models[kMaxModels];
const char*	modelFiles[kMaxModels];
int 		modelIDs[kMaxModels];

uintptr_t aligned_size( uintptr_t size, uintptr_t alignment ) {
	return ( (size / alignment) + (( size % alignment > 0 ) ? 1 : 0) ) * alignment;
}

void*	advance_align( void* ptr, uintptr_t alignment ) {
	return (void*)aligned_size( (uintptr_t)ptr, alignment );
}

// Create an empty mesh with vertCount distinct vertices and index_count vertex indices
mesh* mesh_createMesh( int vertCount, int index_count, int normal_count, int uv_count ) {
	// We know that the whole data block will be 4-byte aligned
	// We need to ensure each sub-array is also 4-byte aligned
	unsigned int data_size = 
						aligned_size( sizeof( mesh ), 						4 ) +
						aligned_size( sizeof( vector )		* vertCount,	4 ) +	// Verts
						aligned_size( sizeof( uint16_t )	* index_count,	4 ) +	// Indices
						aligned_size( sizeof( vector )		* uv_count,		4 ) +	// UVs
						aligned_size( sizeof( uint16_t )	* index_count,	4 ) +	// UV indices
						aligned_size( sizeof( uint16_t )	* index_count,	4 ) +	// Normal indices
						aligned_size( sizeof( vector )		* normal_count,	4 ) ;	// Normals
	
	void* data = mem_alloc( data_size );
	mesh* m = data;

	data = advance_align( data + sizeof( mesh ), 4 );
	m->verts = data;
	data = advance_align( data + sizeof( m->verts[0] ) * vertCount, 4 );			// Verts
	m->indices = data;
	data = advance_align( data + sizeof( m->indices[0] ) * index_count, 4 );	// Indices
	m->normals = data;
	data = advance_align( data + sizeof( m->normals[0] ) * normal_count, 4 );	// Normals	
	m->normal_indices = data;
	data = advance_align( data + sizeof( m->normal_indices[0] ) * index_count, 4 );	// Normal Indices
	m->uvs = data;
	data = advance_align( data + sizeof( m->uvs[0] ) * uv_count, 4 );		// UVs
	m->uv_indices = data;

	m->vert_count = vertCount;
	m->index_count = index_count;
	m->normal_count = normal_count;
	m->vertex_buffer = NULL;
	m->element_buffer = NULL;

	texture_request( &m->texture_diffuse, "dat/img/ship_hd_2.tga" );
	m->shader = resources.shader_default;
	printf( "shader_default: 0x%x\n", resources.shader_default );
	vAssert( m->shader );

	m->vertex_VBO = 0;
	m->element_VBO = 0;

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
	model* m = model_load( "dat/model/cityscape.obj" );
	return m;
}

// Create an empty model with meshCount submeshes
model* model_createModel(int meshCount) {
	model* m = mem_alloc(	sizeof( model ) +
							sizeof( mesh* ) * meshCount );
	memset( m, 0, sizeof( model ) + sizeof( mesh* ) * meshCount );
	m->meshCount = meshCount;
	return m;
}

// Build a vertex buffer for the mesh, copying out vertices as necessary so that
// each vertex-normal pair is covered
// If fully smooth-shaded, can just use a single copy of each vertex
// If not all smooth-shaded, will have to duplicate vertices
void mesh_buildBuffers( mesh* m ) {
	vAssert( m );
	vAssert( !m->vertex_buffer );
	vAssert( !m->element_buffer );
	unsigned int size_vertex	= sizeof( vertex ) * m->index_count;
	unsigned int size_element	= sizeof( GLushort ) * m->index_count;
	m->vertex_buffer	= mem_alloc( size_vertex );
	m->element_buffer	= mem_alloc( size_element );

	bool all_smooth_shaded = false;
	if ( all_smooth_shaded ) {

	} else {
		// For each element index
		// Unroll the vertex/index bindings
		for ( int i = 0; i < m->index_count; i++ ) {
			// Copy the required vertex position, normal, and uv
			m->vertex_buffer[i].position = m->verts[m->indices[i]];
			m->vertex_buffer[i].normal = m->normals[m->normal_indices[i]];
			m->vertex_buffer[i].uv = m->uvs[m->uv_indices[i]];
			m->element_buffer[i] = i;
		}
	}

	// Now also build the OpenGL VBOs for the static data
	m->vertex_VBO = render_requestBuffer( GL_ARRAY_BUFFER, m->vertex_buffer, size_vertex );
	m->element_VBO = render_requestBuffer( GL_ELEMENT_ARRAY_BUFFER, m->element_buffer, size_element );
	printf( "build buffers - Vertex vbo: %u\n", *m->vertex_VBO );
	printf( "build buffers - Element vbo: %u\n", *m->element_VBO );
}

// Draw the verts of a mesh to the openGL buffer
void mesh_render( mesh* m ) {
	//printf( "Vertex vbo: %u\n", *m->vertex_VBO );
	//printf( "Element vbo: %u\n", *m->element_VBO );
	if (( *m->vertex_VBO != kInvalidBuffer ) && ( *m->element_VBO != kInvalidBuffer )) {
		vAssert( *m->vertex_VBO != 0 );
		vAssert( *m->element_VBO != 0 );
		drawCall* draw = drawCall_create( &renderPass_main, m->shader, m->index_count, m->element_buffer, m->vertex_buffer, m->texture_diffuse, modelview );
		draw->vertex_VBO = *m->vertex_VBO;
		draw->element_VBO = *m->element_VBO;
		(void)draw;
	}
}

// Get the i-th submesh of a given model
mesh* model_getMesh(model* m, int i) {
	return m->meshes[i];
}

// Draw each submesh of a model
void model_draw( model* m ) {
	glDepthMask( GL_TRUE );
	for (int i = 0; i < m->meshCount; i++) {
		mesh_render( model_getMesh( m, i ));
	}
}

void model_initModelStorage() {
	memset( models, 0, sizeof( model* ) * kMaxModels );
	memset( modelFiles, 0, sizeof( const char* ) * kMaxModels );
	model_count = 0;
}

model* model_getByHandle( modelHandle h ) {
	return models[h];
}

// Synchronously load a model from a given file
model* model_loadFromFileSync( const char* filename ) {
	return model_load( filename );
}

const char* model_getFileNameFromID( int id ) {
	(void)id;
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
	assert( model_count < kMaxModels );
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
	assert( model_count < kMaxModels );
	modelHandle handle = (modelHandle)model_count;
	modelFiles[handle] = string_createCopy( filename );
	models[handle] = model_loadFromFileSync( filename );
	model_count++;
	return handle;
}

model* model_fromInstance( modelInstance* instance ) {
	return model_getByHandle( instance->model );
}

int model_transformIndex( model* m, transform* ptr ) {
	for ( int i = 0; i < m->transform_count; i++ ) {
		if ( m->transforms[i] == ptr )
			return i;
	}
	return -1;
}
