// Allocator.c

// Allocates *size* bytes from the given heapAllocator *heap*
// Will crash if out of memory
void* heap_allocate( heapAllocator* heap, int size ) {
	block* b = heap_findEmptyBlock( heap, size );
	if ( b->size > ( size + sizeof( block ) ) ) {
		block* remaining = block_create( b->data + size, b->size - size );
		block_insertAfter( b, remaining );
		b->size = size;
	}
	b->free = false;
	return b->data;
}

// Find a block of at least *min_size* bytes
// First version will naively use first found block meeting the criteria
block* heap_findEmptyBlock( heapAllocator* heap, int min_size ) {
	block* b = heap->first;
	while ( ( (b->size < min_size) || !b->free ) && b->next )
		b = b->next;
	// Re-check in case we ran out without finding one
	if (!b->free || b->size < min_size)
		b = NULL;
	return b;
}

// Release a block from the heapAllocator
void heap_deallocate( heapAllocator* heap, void* data ) {
	block* b = heap_findBlock( heap, data );
	if ( !b ) {
		printf("Error, trying to free invalid pointer %d.\n", b);
		exit(-1);
	}
	b->free = true;
	// Try to merge blocks
	if ( b->next->free )
		block_merge( b, b->next );
}

void block_merge( block* first, block* second ) {
	assert( second->free );								// Second must be empty (not necessarily first!)
	assert( second == (first->data + first->size) );	// Contiguous

	first->size += second->size + sizeof( block );
	first->next = second->next;
}

// Create a heapAllocator of *size* bytes
// Initialised with one block pointing to the whole memory
heapAllocator* heap_create( int heap_size ) {
	void* data = malloc( sizeof( heapAllocator ) + heap_size );

	heapAllocator* allocator = data;
	data += sizeof( heapAllocator );

	block* first = block_create( data, heap_size );
	allocator->first = first;
	return allocator;
}

// Insert a block *after* into a linked-list after the block *before*
void block_insertAfter( block* before, block* after ) {
	after->next = before->next;
	after->prev = before;
	before->next = after;
}

// Create and initialise a block in a given piece of memory of *size* bytes
block* block_create( void* data, int size ) {
	block* b = (block*)data;
	b->size = size - sizeof( block );
	b->data = data + sizeof( block );
	b->prev = b-> next = NULL;
	return b;
}
