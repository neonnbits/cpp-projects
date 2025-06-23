#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

// Alignment to 8 bytes
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

// Page size for mmap (typically 4KB)
#define PAGE_SIZE 4096
#define ALIGN_TO_PAGE(size) (((size) + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1))

// Block header structure
typedef struct block {
    size_t size;        // Size of the user data area (not including header)
    int is_free;        // 1 if free, 0 if allocated
    struct block *next; // Next free block (only used when is_free = 1)
} block_t;

// Global variables
static block_t *free_list_head = NULL; // Head of free list

// Function declarations
void *c_malloc(size_t size);
void c_free(void *ptr);
block_t *find_free_block(size_t size);
block_t *grow_heap(size_t size);
void print_heap_status(void);

// Find a free block that's big enough
block_t *find_free_block(size_t size) {
    block_t *current = free_list_head;

    while (current != NULL) {
        if (current->size >= size) {
            return current; // Found a suitable block
        }
        current = current->next;
    }

    return NULL; // No suitable block found
}

// Remove a block from the free list
void remove_from_free_list(block_t *block) {
    if (free_list_head == block) {
        free_list_head = block->next;
        return;
    }

    block_t *current = free_list_head;
    while (current && current->next != block) {
        current = current->next;
    }

    if (current) {
        current->next = block->next;
    }
}

// Add a block to the free list
void add_to_free_list(block_t *block) {
    block->is_free = 1;
    block->next = free_list_head;
    free_list_head = block;
}

// Grow the heap by requesting more memory from OS using mmap
block_t *grow_heap(size_t size) {
    size_t total_size = sizeof(block_t) + size;

    // Round up to page size for efficiency
    size_t mmap_size = ALIGN_TO_PAGE(total_size);

    // Request memory from OS using mmap
    void *new_memory =
        mmap(NULL, mmap_size,
             PROT_READ | PROT_WRITE,      // Readable and writable
             MAP_PRIVATE | MAP_ANONYMOUS, // Private, not backed by file
             -1, 0);                      // No file descriptor, offset 0

    if (new_memory == MAP_FAILED) {
        return NULL; // mmap failed
    }

    // Initialize the new block
    block_t *new_block = (block_t *)new_memory;
    new_block->size = size;
    new_block->is_free = 0; // Will be marked as allocated
    new_block->next = NULL;

    // If we allocated more than needed, create a free block with the remainder
    size_t remaining = mmap_size - total_size;
    if (remaining >= sizeof(block_t) + ALIGNMENT) {
        block_t *remainder_block = (block_t *)((char *)new_memory + total_size);
        remainder_block->size = remaining - sizeof(block_t);
        remainder_block->is_free = 1;
        remainder_block->next = NULL;

        // Add the remainder to free list
        add_to_free_list(remainder_block);
    }

    return new_block;
}

// Our malloc implementation
void *c_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    // Align the size
    size = ALIGN(size);

    // Try to find a free block
    block_t *block = find_free_block(size);

    if (block) {
        // Found a free block, remove it from free list
        remove_from_free_list(block);
        block->is_free = 0;

        // TODO: Handle splitting large blocks (for now, just use the whole
        // block)

        // Return pointer to user data (skip the header)
        return (char *)block + sizeof(block_t);
    }

    // No suitable free block found, grow heap
    block = grow_heap(size);
    if (!block) {
        return NULL; // Out of memory
    }

    // Return pointer to user data
    return (char *)block + sizeof(block_t);
}

// Our free implementation
void c_free(void *ptr) {
    if (!ptr) {
        return;
    }

    // Find the block header (it's right before the user data)
    block_t *block = (block_t *)((char *)ptr - sizeof(block_t));

    // Add it back to the free list
    add_to_free_list(block);

    // TODO: Implement coalescing later
}

// Debug function to print heap status
void print_heap_status(void) {
    printf("Free List: ");
    block_t *current = free_list_head;

    if (!current) {
        printf("(empty)\n");
        return;
    }

    int count = 0;
    while (current) {
        printf("[size=%zu] -> ", current->size);
        current = current->next;
        count++;
        if (count > 10) { // Prevent infinite loops in debugging
            printf("...");
            break;
        }
    }
    printf("NULL\n");
}

/*
 * Why mmap instead of sbrk?
 *
 * 1. sbrk grows a single contiguous heap - limited and can fragment
 * 2. mmap can allocate anywhere in virtual memory - more flexible
 * 3. mmap works with page-sized chunks - more efficient with OS
 * 4. sbrk is deprecated on many modern systems
 * 5. mmap allows us to return large chunks back to OS (with munmap)
 */

int main() {
    printf("Simple Malloc Implementation (using mmap)\n");
    printf("==========================================\n");

    // Test basic allocation
    printf("\n1. Allocating 20 bytes...\n");
    char *ptr1 = (char *)c_malloc(20);
    printf("   Allocated at: %p\n", ptr1);
    print_heap_status();

    printf("\n2. Allocating 50 bytes...\n");
    char *ptr2 = (char *)c_malloc(50);
    printf("   Allocated at: %p\n", ptr2);
    print_heap_status();

    printf("\n3. Freeing first block...\n");
    c_free(ptr1);
    print_heap_status();

    printf("\n4. Allocating 15 bytes (should reuse freed space)...\n");
    char *ptr3 = (char *)c_malloc(15);
    printf("   Allocated at: %p\n", ptr3);
    print_heap_status();

    printf("\n5. Allocating large block (3000 bytes)...\n");
    char *ptr4 = (char *)c_malloc(3000);
    printf("   Allocated at: %p\n", ptr4);
    print_heap_status();

    printf("\nNote: mmap allocates in 4KB pages, so you might see large "
           "remainder blocks!\n");

    return 0;
}