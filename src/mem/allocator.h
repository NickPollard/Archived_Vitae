#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__
// Allocator.h
#include <stddef.h>

//#define MEM_DEBUG_VERBOSE

typedef struct block_s block;

// A heap allocator struct
// Allocates memory from a defined size heap
// Uses a doubly-linked list of block headers to keep track of used space
// Insertion time is O(n)
// Deallocation is O(n)
struct heapAllocator_s {
	unsigned int total_size;		// in bytes, size of the heap
	unsigned int total_allocated;	// in bytes, currently allocated
	unsigned int total_free;		// in bytes, currently free
	block* first;					// doubly-linked list of blocks
};

// A memory block header for the heapAllocator
// Each heapAllocator has a doubly-linked list of these
// Each heap_allocate call will return one of these
// TODO - can optimise the space here?
struct block_s {
	void*	data;			// the memory location of the actual block
	unsigned int	size;	// in bytes, the block size
	block*	next;			// doubly-linked list pointer
	block*	prev;			// doubly-linked list pointer
	unsigned int	free;	// true (1) if free, false (0) if used
};

#endif // __ALLOCATOR_H__

// Default allocate from the static heap
// Passes straight through to heap_allocate()
void* mem_alloc(size_t bytes);

// Default deallocate from the static heap
// Passes straight through to heap_deallocate()
void mem_free( void* ptr );

// Initialise the memory subsystem
void mem_init(int argc, char** argv);

// Allocates *size* bytes from the given heapAllocator *heap*
// Will crash if out of memory
void* heap_allocate( heapAllocator* heap, int size );

// Find a block of at least *min_size* bytes
// First version will naively use first found block meeting the criteria
block* heap_findEmptyBlock( heapAllocator* heap, int min_size );

// Find a block with a given data pointer to *mem_addr*
// Returns NULL if no such block is found
block* heap_findBlock( heapAllocator* heap, void* mem_addr );

// Release a block from the heapAllocator
void heap_deallocate( heapAllocator* heap, void* data );

// Merge two continous blocks, *first* and *second*
// Afterwards, only *first* will remain valid
// but will have size equal to both plus sizeof( block )
void block_merge( heapAllocator* heap, block* first, block* second );

// Create a heapAllocator of *size* bytes
// Initialised with one block pointing to the whole memory
heapAllocator* heap_create( int heap_size );

// Insert a block *after* into a linked-list after the block *before*
// Both *before* and *after* must be valid
void block_insertAfter( block* before, block* after );

// Create and initialise a block in a given piece of memory of *size* bytes
block* block_create( void* data, int size );

//
// Tests
//

void test_allocator();
