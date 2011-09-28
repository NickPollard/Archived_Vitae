// skybox.c

#include "common.h"
#include "skybox.h"
//-----------------------
#include "model.h"
#include "model_loader.h"
#include "render/render.h"
#include "render/shader.h"
#include "render/texture.h"
#include "system/hash.h"

model*			skybox_model = NULL;
GLint			skybox_texture = -1;
GLushort		skybox_index_count = 36;
skybox_vertex*	skybox_static_vertex_buffer = NULL;
GLushort		skybox_static_element_buffer[] = {
	// TOP
	0, 1, 2,
	1, 3, 2,
	// BOTTOM,
	4, 6, 5,
	5, 6, 7,
	// FRONT
	2, 3, 6,
	3, 7, 6,
	// BACK
	0, 4, 1,
	1, 4, 5,
	// LEFT
	1, 5, 3,
	3, 5, 7,
	// RIGHT
	0, 2, 4,
	2, 6, 4
};

// Initialise static data for the skybox system
void skybox_init( ) {
	const float distance = 100.f;
	assert( skybox_static_vertex_buffer == NULL );
	skybox_static_vertex_buffer = mem_alloc( sizeof( skybox_vertex ) * 8 );
	skybox_static_vertex_buffer[0].position = Vector(  distance,  distance,  distance, 1.0 );
	skybox_static_vertex_buffer[1].position = Vector( -distance,  distance,  distance, 1.0 );
	skybox_static_vertex_buffer[2].position = Vector(  distance,  distance, -distance, 1.0 );
	skybox_static_vertex_buffer[3].position = Vector( -distance,  distance, -distance, 1.0 );
	skybox_static_vertex_buffer[4].position = Vector(  distance, -distance,  distance, 1.0 );
	skybox_static_vertex_buffer[5].position = Vector( -distance, -distance,  distance, 1.0 );
	skybox_static_vertex_buffer[6].position = Vector(  distance, -distance, -distance, 1.0 );
	skybox_static_vertex_buffer[7].position = Vector( -distance, -distance, -distance, 1.0 );

	skybox_texture = texture_loadTGA( "assets/3rdparty/img/grimmnight_medium.tga" );

	skybox_model = model_load( "dat/model/inverse_cube.s" );
	skybox_model->meshes[0]->texture_diffuse = skybox_texture;
}

#define SKYBOX_VERTEX_ATTRIB_POINTER( attrib ) \
	glVertexAttribPointer( attrib, /*vec4*/ 4, GL_FLOAT, /*Normalized?*/GL_FALSE, sizeof( skybox_vertex ), (void*)offsetof( skybox_vertex, attrib) ); \
	glEnableVertexAttribArray( attrib );

// Render the skybox
void skybox_render( void* data ) {
	// Skybox does not write to the depth buffer
	glDepthMask( GL_FALSE );

	// Switch to terrain shader
	shader_activate( resources.shader_skybox );

	render_resetModelView();
	vector v = Vector( 0.f, 0.f, 0.f, 1.f );
	matrix_setTranslation( modelview, &v );
	// Set up uniforms
	render_setUniform_matrix( *resources.uniforms.projection, perspective );
	render_setUniform_matrix( *resources.uniforms.modelview, modelview );
	render_setUniform_matrix( *resources.uniforms.worldspace, modelview );

#if 0
	// Copy our data to the GPU
	// There are now <index_count> vertices, as we have unrolled them
	GLsizei vertex_buffer_size = skybox_index_count * sizeof( skybox_vertex );
	GLsizei element_buffer_size = skybox_index_count * sizeof( GLushort );

	// Textures
	GLint* tex = shader_findConstant( mhash( "tex" ));
	if ( tex )
		render_setUniform_texture( *tex, skybox_texture );

	SKYBOX_VERTEX_ATTRIBS( VERTEX_ATTRIB_LOOKUP );
	// *** Vertex Buffer
	glBindBuffer( GL_ARRAY_BUFFER, resources.vertex_buffer );
	glBufferData( GL_ARRAY_BUFFER, vertex_buffer_size, skybox_static_vertex_buffer, GL_DYNAMIC_DRAW );// OpenGL ES only supports DYNAMIC_DRAW or STATIC_DRAW
	SKYBOX_VERTEX_ATTRIBS( SKYBOX_VERTEX_ATTRIB_POINTER );
	// *** Element Buffer
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, resources.element_buffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, element_buffer_size, skybox_static_element_buffer, GL_DYNAMIC_DRAW ); // OpenGL ES only supports DYNAMIC_DRAW or STATIC_DRAW

	// Draw!
	glDrawElements( GL_TRIANGLES, skybox_index_count, GL_UNSIGNED_SHORT, (void*)0 );

	// Cleanup
	SKYBOX_VERTEX_ATTRIBS( VERTEX_ATTRIB_DISABLE_ARRAY )
#endif

	mesh_drawVerts( skybox_model->meshes[0] );
}
