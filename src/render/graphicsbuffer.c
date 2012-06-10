// graphicsbuffer.c

graphicsBuffer* graphicsBuffer_createStatic() {
	glGenBuffers();
	glBindBuffer( TARGET );
	// Create a buffer of the correct size
	// Not dependent on anything, should not sync
	// Just takes time to allocate (which is unavoidable)
	glBufferData( TARGET, size, NULL );
	// We know we're not using this buffer yet
	// So we can do an unsynchronised copy using mapBufferRange
	glMapBufferRange( UNSYNCHED );
	memcpy();
	glUnmap();
}

graphicsBuffer* graphicsBuffer_createDynamic() {
}

graphicsBuffer* graphicsBuffer_createStream() {
}

void graphicsBuffer_fill( void* data ) {
	glBindBuffer( buffer->target );
	glBufferSubData( buffer->size, data );

	glBindBuffer( buffer->target );
	data = glMapBuffer( buffer->target );
	memcpy( data, buffer->data, buffer->size );
	glUnmapBuffer( buffer->target );
}

void graphicsBuffer_bind( graphicsBuffer* buffer ) {
	glBindBuffer( buffer->target, buffer->object );
}

void graphicsBuffer_unbind( graphicsBuffer* buffer ) {
	glBindBuffer( buffer->target, INVALID_BUFFER );
}

/* Example usage: Models

init:
   model_buffer = graphicsBuffer_createStatic( buffer_size, buffer_data );

draw:
   draw_call->buffer = block_buffer;

render:
	graphicsBuffer_bind( draw_call->buffer );


   */

/* Example usage: Terrain

init:
   block_buffer = graphicsBuffer_createStatic( buffer_size, NULL );
   graphicsBuffer_fill( buffer_data );

draw:
   draw_call->buffer = block_buffer;

render:
	graphicsBuffer_bind( draw_call->buffer );

   */

/* Example usage: Particles

   */
