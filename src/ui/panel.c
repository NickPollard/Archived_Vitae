// panel.c

#include "common.h"
#include "panel.h"
//-------------------------
#include "mem/allocator.h"
#include "render/shader.h"
#include "render/texture.h"
#include "system/hash.h"
#include "model.h"

// Create a Panel
panel* panel_create() {
	panel* p = mem_alloc( sizeof( panel ));
	memset( p, 0, sizeof( panel ));

	// Default anchoring: tl to tl
	p->local_anchor = kTopLeft;
	p->remote_anchor = kTopLeft;

	return p;
}

static GLushort element_buffer[] = {
	2, 1, 0,
	1, 2, 3
};
static vertex vertex_buffer[4];

static const int element_count = 6;
static const int vert_count = 4;

// The draw function gets passed the current 'cursor' x and y
void panel_draw( panel* p, float x, float y ) {

//	glDisable(GL_DEPTH_TEST);

	// We draw a quad as two triangles
	vertex_buffer[0].position = Vector( p->x,				p->y,				0.1f, 1.f );
	vertex_buffer[1].position = Vector( p->x + p->width,	p->y,				0.1f, 1.f );
	vertex_buffer[2].position = Vector( p->x,				p->y + p->height,	0.1f, 1.f );
	vertex_buffer[3].position = Vector( p->x + p->width,	p->y + p->height,	0.1f, 1.f );
	vertex_buffer[0].uv = Vector( 0.f, 0.f, 0.f, 0.f );
	vertex_buffer[1].uv = Vector( 1.f, 0.f, 0.f, 0.f );
	vertex_buffer[2].uv = Vector( 0.f, 1.f, 0.f, 0.f );
	vertex_buffer[3].uv = Vector( 1.f, 1.f, 0.f, 0.f );

	// Copy our data to the GPU
	// There are now <index_count> vertices, as we have unrolled them

	// Textures
	GLint* tex = shader_findConstant( mhash( "tex" ));
	if ( tex )
		render_setUniform_texture( *tex, p->texture );

//	shader_activate( resources.shader_ui );

	drawCall* panel_render = drawCall_create( resources.shader_ui, element_count, element_buffer, vertex_buffer, p->texture, modelview );
	render_drawCall( panel_render );
}
