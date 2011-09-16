// panel.c

#include "common.h"
#include "panel.h"
//-------------------------
#include "mem/allocator.h"
#include "render/render.h"
#include "render/shader.h"
#include "system/hash.h"
#include "model.h"

panel* panel_create() {
	panel* p = mem_alloc( sizeof( panel ));
	memset( p, 0, sizeof( panel ));

	// Default anchoring: tl to tl
	p->local_anchor = kTopLeft;
	p->remote_anchor = kTopLeft;

	return p;
}

static GLushort element_buffer[] = {
	0, 1, 2,
	2, 1, 3
};
static vertex vertex_buffer[4];

static const int element_count = 6;
static const int vert_count = 4;

// The draw function gets passed the current 'cursor' x and y
void panel_draw( panel* p, int x, int y ) {

	// We draw a quad as two triangles
	vertex_buffer[0].position = Vector( x,				y,				0.f, 1.f );
	vertex_buffer[1].position = Vector( x + p->width,	y,				0.f, 1.f );
	vertex_buffer[2].position = Vector( x,				y + p->height,	0.f, 1.f );
	vertex_buffer[3].position = Vector( x + p->width,	y + p->height,	0.f, 1.f );

	// Copy our data to the GPU
	// There are now <index_count> vertices, as we have unrolled them
	GLsizei vertex_buffer_size = vert_count * sizeof( vertex );
	GLsizei element_buffer_size = element_count * sizeof( GLushort );

	/*
	// Textures
	GLint* tex = shader_findConstant( mhash( "tex" ));
	if ( tex )
		render_setUniform_texture( *tex, m->texture_diffuse );
*/
	shader_activate( resources.shader_ui );

	VERTEX_ATTRIBS( VERTEX_ATTRIB_LOOKUP );
	// *** Vertex Buffer
	glBindBuffer( GL_ARRAY_BUFFER, resources.vertex_buffer );
	glBufferData( GL_ARRAY_BUFFER, vertex_buffer_size, vertex_buffer, GL_DYNAMIC_DRAW );// OpenGL ES only supports DYNAMIC_DRAW or STATIC_DRAW
	VERTEX_ATTRIBS( VERTEX_ATTRIB_POINTER );
	// *** Element Buffer
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, resources.element_buffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, element_buffer_size, element_buffer, GL_DYNAMIC_DRAW ); // OpenGL ES only supports DYNAMIC_DRAW or STATIC_DRAW

	// Draw!
	glDrawElements( GL_TRIANGLES, element_count, GL_UNSIGNED_SHORT, (void*)0 );

	// Cleanup
	VERTEX_ATTRIBS( VERTEX_ATTRIB_DISABLE_ARRAY )
}