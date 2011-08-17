// Allocator.c
#include "common.h"
#include "allocator.h"
//---------------------
#include <assert.h>
#include <stdio.h>

#define static_heap_size (64 << 20) // In MegaBytes
heapAllocator* static_heap = NULL;

void heap_dumpBlocks( heapAllocator* heap );

// Default allocate from the static heap
// Passes straight through to heap_allocate()
void* mem_alloc( size_t bytes ) {
	return heap_allocate( static_heap, bytes );
}

// Default deallocate from the static heap
// Passes straight through to heap_deallocate()
void mem_free( void* ptr ) {
	heap_deallocate( static_heap, ptr );
}

// Initialise the memory subsystem
void mem_init(int argc, char** argv) {
	static_heap = heap_create(static_heap_size);
}

// Allocates *size* bytes from the given heapAllocator *heap*
// Will crash if out of memory
void* heap_allocate( heapAllocator* heap, int size ) {
#ifdef MEM_DEBUG_VERBOSE
	printf( "HeapAllocator request for %d bytes.\n", size );
#endif
	block* b = heap_findEmptyBlock( heap, size );
	if ( !b ) {
		heap_dumpBlocks( heap );
		printError( "HeapAllocator out of memory on request for %d bytes. Total size: %d bytes, Used size: %d bytes\n", size, heap->total_size, heap->total_allocated );
		assert( 0 );
	}
	if ( b->size > ( size + sizeof( block ) ) ) {
//		printf( "Allocator: Splitting Block of size %d into sizes %d ad %d\n", b->size, size, b->size-size );
		block* remaining = block_create( b->data + size, b->size - size );
		block_insertAfter( b, remaining );
		b->size = size;
		heap->total_allocated += sizeof( block );
		heap->total_free -= sizeof( block );
		// Try to merge blocks
		if ( remaining->next && remaining->next->free )
			block_merge( heap, remaining, remaining->next );
	}
	b->free = false;

	heap->total_allocated += size;
	heap->total_free -= size;

#ifdef MEM_DEBUG_VERBOSE
	printf("Allocator returned address: %x.\n", (unsigned int)b->data );
#endif
	return b->data;
}

void heap_dumpBlocks( heapAllocator* heap ) {
	block* b = heap->first;
	while ( b ) {
		printf( "Block: ptr 0x%x, data: 0x%x, size %d, free %d\n", (unsigned int)b, (unsigned int)b->data, b->size, b->free );
		b = b->next;
	}
}

// Find a block of at least *min_size* bytes
// First version will naively use first found block meeting the criteria
block* heap_findEmptyBlock( heapAllocator* heap, int min_size ) {
	block* b = heap->first;
	while ( ( ( b->size < min_size ) || !b->free ) && b->next )
		b = b->next;
	// Re-check in case we ran out without finding one
	if ( !b->free || ( b->size < min_size ) )
		b = NULL;
	return b;
}

// Find a block with a given data pointer to *mem_addr*
// Returns NULL if no such block is found
block* heap_findBlock( heapAllocator* heap, void* mem_addr ) {
	block* b = heap->first;
	while ( (b->data != mem_addr) && b->next ) b = b->next;
	if ( b->data != mem_addr ) b = NULL;
	return b;
}

// Release a block from the heapAllocator
void heap_deallocate( heapAllocator* heap, void* data ) {
	block* b = heap_findBlock( heap, data );
	assert( b );
#ifdef MEM_DEBUG_VERBOSE
	printf("Allocator freed address: %x.\n", (unsigned int)b->data );
#endif
	b->free = true;

	heap->total_free += b->size;
	heap->total_allocated -= b->size;

	// Try to merge blocks
	if ( b->next && b->next->free )
		block_merge( heap, b, b->next );
	if ( b->prev && b->prev->free )
		block_merge( heap, b->prev, b );
}

// Merge two continous blocks, *first* and *second*
// Both *first* and *second* must be valid
// Afterwards, only *first* will remain valid
// but will have size equal to both plus sizeof( block )
void block_merge( heapAllocator* heap, block* first, block* second ) {
//	printf( "Allocator: Merging Blocks\n" );
	assert( first );
	assert( second );
	assert( first->free );								// Both must be empty
	assert( second == (first->data + first->size) );	// Contiguous
	assert( first->next == second );
	assert( second->prev == first );

	heap->total_free += sizeof( block );
	heap->total_allocated -= sizeof( block );

	first->size += second->size + sizeof( block );
	first->next = second->next;
	if ( second->next )
		second->next->prev = first;
	first->free = true;
}

// Create a heapAllocator of *size* bytes
// Initialised with one block pointing to the whole memory
heapAllocator* heap_create( int heap_size ) {
	// We add space for the first block header, so we do get the correct total size
	// ie. this means that heap_create (size), followed by heap_Allocate( size ) should work
	void* data = malloc( sizeof( heapAllocator ) + sizeof( block ) + heap_size );

	heapAllocator* allocator = (heapAllocator*)data;
	data += sizeof( heapAllocator );
	allocator->total_size = heap_size;
	allocator->total_free = heap_size;
	allocator->total_allocated = 0;
	
	// Should not be possible to fail creating the first block header
	block* first = block_create( data, heap_size );
	assert( first ); 
	allocator->first = first;

	return allocator;
}

// Insert a block *after* into a linked-list after the block *before*
// Both *before* and *after* must be valid
void block_insertAfter( block* before, block* after ) {
	assert( before );
	assert( after );
	after->next = before->next;
	after->prev = before;
	before->next = after;
	if ( after->next )
		after->next->prev = after;
}

// Create and initialise a block in a given piece of memory of *size* bytes
block* block_create( void* data, int size ) {
	block* b = (block*)data;
	b->size = size - sizeof( block );
	b->data = data + sizeof( block );
	b->free = true;
	b->prev = b->next = NULL;
	return b;
}

//
// Tests
//

void test_allocator() {
	printf( "%s--- Beginning Unit Test: Heap Allocator ---\n", TERM_WHITE );
	heapAllocator* heap = heap_create( 4096 );

	if (!heap) {
		printf("[ Failed ]\tAllocator not created.\n");
	} else {
		printf("[ %sPassed%s ]\tAllocator created succesfully.\n", TERM_GREEN, TERM_WHITE );
	}
//	if (heap->total_size == 4096) {
//		printf("[ Passed: Create Allocator of 4096 byte heap.\n");
//	} else {
//		printf("[ Failed ]\tAllocator has incorrect size (should be 4096).\n");
//	}

	void* a = heap_allocate( heap, 2048 );
	memset( a, 0, 2048 );
	printf( "[ %sPassed%s ]\tAllocated 2048 bytes succesfully.\n", TERM_GREEN, TERM_WHITE );

	void* b = heap_allocate( heap, 1024 );
	memset( b, 0, 1024 );
	printf( "[ %sPassed%s ]\tAllocated 2048 + 1024 bytes succesfully.\n", TERM_GREEN, TERM_WHITE );

	heap_deallocate( heap, a );
	printf( "[ %sPassed%s ]\tDeallocated 2048 bytes succesfully.\n", TERM_GREEN, TERM_WHITE );

	heap_deallocate( heap, b );
	printf( "[ %sPassed%s ]\tDeallocated 1024 bytes succesfully.\n", TERM_GREEN, TERM_WHITE );

	a = heap_allocate( heap, 3072 );
	memset( a, 0, 3072 );
	printf( "[ %sPassed%s ]\tAllocated 3072 bytes succesfully.\n", TERM_GREEN, TERM_WHITE );

	b = heap_allocate( heap, 512 );
	memset( b, 0, 512 );
	printf( "[ %sPassed%s ]\tAllocated 512 bytes succesfully.\n", TERM_GREEN, TERM_WHITE );
}
