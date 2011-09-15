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

/*
void vgl_vertexDraw(vector* v) {
	glVertex3fv((GLfloat*)v);
}
*/

unsigned int aligned_size( unsigned int size, unsigned int alignment ) {
	return ( (size / alignment) + (( size % alignment > 0 ) ? 1 : 0) ) * alignment;
}

void*	advance_align( void* ptr, unsigned int alignment ) {
	return (void*)aligned_size( (unsigned int)ptr, alignment );
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

	// TODO: Align (4-byte);
//	data += sizeof(mesh);
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

	/*
	void* data = mem_alloc( sizeof( mesh ) +
						sizeof( vector ) * vertCount +		// Verts
						sizeof( uint16_t )	 * index_count +	// Indices
						sizeof( vector )	 * uv_count +	// UVs
						sizeof( uint16_t )	 * index_count +	// UV indices
						sizeof( uint16_t )	 * index_count +	// Normal indices
						sizeof( vector ) * normal_count );	// Normals
	mesh* m = data;

	// TODO: Align (4-byte);
	data += sizeof(mesh);
	m->verts = data;
	data += sizeof( m->verts[0] ) * vertCount;			// Verts
	m->indices = data;
	data += sizeof( m->indices[0] ) * index_count;	// Indices
	m->normals = data;
	data += sizeof( m->normals[0] ) * normal_count;	// Normals	
	m->normal_indices = data;
	data += sizeof( m->normal_indices[0] ) * index_count;	// Normal Indices
	m->uvs = data;
	data += sizeof( m->uvs[0] ) * uv_count;		// UVs
	m->uv_indices = data;
*/

	m->vert_count = vertCount;
	m->index_count = index_count;
	m->normal_count = normal_count;

	m->vertex_buffer = NULL;
	m->element_buffer = NULL;

	//m->texture_diffuse = texture_loadTGA( "assets/img/test64rgba.tga" );
	m->texture_diffuse = texture_loadTGA( "assets/img/ship_hd_2.tga" );

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
	vAssert( !m->vertex_buffer );
	vAssert( !m->element_buffer );
	m->vertex_buffer = mem_alloc( sizeof( vertex ) * m->index_count );
	m->element_buffer = mem_alloc( sizeof( GLushort ) * m->index_count );

	bool all_smooth_shaded = false;
	if ( all_smooth_shaded ) {

	} else {
		printf( "MODEL: Build Buffers. Index count: %d\n", m->index_count );
		/*
		printf( "MODEL: Alignment vertex: %d", ((unsigned int)m->vertex_buffer) % sizeof( vertex ) );
		printf( "MODEL: Alignment vector: %d", ((unsigned int)m->vertex_buffer) % sizeof( vector ) );
		printf( "MODEL: Alignment verts: %d", ((unsigned int)m->verts) % sizeof( vector ) );
		printf( "MODEL: Alignment normals: %d", ((unsigned int)m->normals) % sizeof( vector ) );
		printf( "MODEL: Alignment uvs: %d", ((unsigned int)m->uvs) % sizeof( vector ) );
		*/
		// For each element index
		// Unroll the vertex/index bindings
		for ( int i = 0; i < m->index_count; i++ ) {
			// Copy the required vertex position, normal, and uv
			m->vertex_buffer[i].position = m->verts[m->indices[i]];
			m->vertex_buffer[i].normal = m->normals[m->normal_indices[i]];
			m->vertex_buffer[i].uv = m->uvs[m->uv_indices[i]];
			m->element_buffer[i] = i;
		}
		printf( "MODEL: Buffers built successfully.\n" );
	}
}

// Draw the verts of a mesh to the openGL buffer
void mesh_drawVerts( mesh* m ) {
	// Copy our data to the GPU
	// There are now <index_count> vertices, as we have unrolled them
	GLsizei vertex_buffer_size = m->index_count * sizeof( vertex );
	GLsizei element_buffer_size = m->index_count * sizeof( GLushort );

	// Textures
	GLint* tex = shader_findConstant( mhash( "tex" ));
	if ( tex )
		render_setUniform_texture( *tex, m->texture_diffuse );

	VERTEX_ATTRIBS( VERTEX_ATTRIB_LOOKUP );
	// *** Vertex Buffer
	glBindBuffer( GL_ARRAY_BUFFER, resources.vertex_buffer );
	glBufferData( GL_ARRAY_BUFFER, vertex_buffer_size, m->vertex_buffer, GL_DYNAMIC_DRAW );// OpenGL ES only supports DYNAMIC_DRAW or STATIC_DRAW
	VERTEX_ATTRIBS( VERTEX_ATTRIB_POINTER );
	// *** Element Buffer
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, resources.element_buffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, element_buffer_size, m->element_buffer, GL_DYNAMIC_DRAW ); // OpenGL ES only supports DYNAMIC_DRAW or STATIC_DRAW

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
	glDepthMask( GL_TRUE );
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
