#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__
// Allocator.h

typedef struct heapAllocator_s {
	unsigned int total_size;		// in bytes, size of the heap
	unsigned int total_allocated;	// in bytes, currently allocated
	unsigned int total_free;		// in bytes, currently free
	block* first;					// doubly-linked list of blocks
} heapAllocator;

typedef struct block_s {
	void*	data;			// the memory location of the actual block
	unsigned int	size;	// in bytes, the block size
	block*	next;			// doubly-linked list pointer
	block*	prev;			// doubly-linked list pointer
	unsigned int	free;	// true (1) if free, false (0) if used
} block;

#endif // __ALLOCATOR_H__
