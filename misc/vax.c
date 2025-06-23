#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Hardware configuration
#define SEG_MASK 0x3000    // Top 2 bits: 11000000000000 (binary)
#define SEG_SHIFT 12       // Shift right 12 bits to get segment
#define OFFSET_MASK 0x0FFF // Bottom 12 bits: 00111111111111 (binary)

// Segment definitions
#define SEGMENT_CODE 0   // 00
#define SEGMENT_HEAP 1   // 01
#define SEGMENT_UNUSED 2 // 10
#define SEGMENT_STACK 3  // 11

// Simulated hardware registers (per-process)
typedef struct {
    uint32_t base[4];          // Base address for each segment
    uint32_t bounds[4];        // Size limit for each segment
    uint8_t grows_positive[4]; // 1 = grows up, 0 = grows down (stack)
} SegmentTable;

// Simulated physical memory (16MB)
uint8_t physical_memory[16 * 1024 * 1024];

// Current process's segment table (matching book's example)
SegmentTable current_process = {
    // Base addresses for each segment in physical memory
    .base =
        {
            32 * 1024, // Segment 0: Code at 32KB
            34 * 1024, // Segment 1: Heap at 34KB
            0x000000,  // Segment 2: Unused
            28 * 1024  // Segment 3: Stack at 28KB (grows DOWN from here)
        },
    // Size bounds for each segment
    .bounds =
        {
            2 * 1024, // Segment 0: Code 2KB
            2 * 1024, // Segment 1: Heap 2KB
            0x0000,   // Segment 2: Unused 0KB
            2 * 1024  // Segment 3: Stack 2KB
        },
    // Growth direction: 1 = positive (up), 0 = negative (down)
    .grows_positive = {
        1, // Segment 0: Code grows positive
        1, // Segment 1: Heap grows positive
        1, // Segment 2: Unused (doesn't matter)
        0  // Segment 3: Stack grows NEGATIVE (backwards)
    }};

// Hardware address translation function
uint32_t translate_address(uint16_t virtual_address, bool *fault) {
    // Extract segment and offset (exactly like the book's example)
    uint16_t segment = (virtual_address & SEG_MASK) >> SEG_SHIFT;
    uint16_t offset = virtual_address & OFFSET_MASK;

    printf("Virtual Address: 0x%04X (binary: ", virtual_address);
    for (int i = 13; i >= 0; i--) {
        printf("%d", (virtual_address >> i) & 1);
        if (i == 12)
            printf(" "); // Space after segment bits
    }
    printf(")\n");

    printf("  Segment bits: %02d (binary: %s)\n", segment,
           segment == 0   ? "00"
           : segment == 1 ? "01"
           : segment == 2 ? "10"
                          : "11");
    printf("  Raw offset: 0x%03X (%d decimal)\n", offset, offset);

    // Handle positive vs negative growing segments
    int32_t actual_offset;
    uint32_t max_segment_size = 4 * 1024; // 4KB max (2^12 addressable space)

    if (current_process.grows_positive[segment]) {
        // Positive-growing segment (normal case)
        actual_offset = offset;
        printf("  Segment grows POSITIVE, actual offset = %d\n", actual_offset);

        // Bounds check: offset must be < bounds
        if (offset >= current_process.bounds[segment]) {
            printf("  PROTECTION FAULT: Offset %d >= Bounds[%d] = %d\n", offset,
                   segment, current_process.bounds[segment]);
            *fault = true;
            return 0;
        }
    } else {
        // Negative-growing segment (stack)
        // Key insight: negative offset = raw_offset - max_segment_size
        // This makes higher virtual addresses map to LOWER physical addresses
        actual_offset = offset - max_segment_size;
        printf("  Segment grows NEGATIVE, actual offset = %d - %d = %d\n",
               offset, max_segment_size, actual_offset);
        printf("    (Higher virtual addr -> LOWER physical addr within "
               "segment)\n");

        // Bounds check for negative growth: offset must be >= (max_size -
        // bounds)
        uint32_t min_valid_offset =
            max_segment_size - current_process.bounds[segment];
        if (offset < min_valid_offset) {
            printf("  PROTECTION FAULT: Offset %d < minimum valid offset %d\n",
                   offset, min_valid_offset);
            printf("    (Stack can only use top %d bytes of segment space)\n",
                   current_process.bounds[segment]);
            *fault = true;
            return 0;
        }
    }

    // Calculate physical address
    uint32_t physical_addr = current_process.base[segment] + actual_offset;
    printf("  Physical Address: 0x%08X (Base[%d] + actual_offset = %d + %d)\n",
           physical_addr, segment, current_process.base[segment],
           actual_offset);

    *fault = false;
    return physical_addr;
}

// Simulated memory access
void access_memory(uint16_t virtual_addr, uint8_t value, bool is_write) {
    bool fault = false;
    uint32_t physical_addr = translate_address(virtual_addr, &fault);

    if (fault) {
        printf("  SEGMENTATION FAULT!\n\n");
        return;
    }

    if (is_write) {
        physical_memory[physical_addr] = value;
        printf("  WRITE: Stored %d at physical 0x%08X\n", value, physical_addr);
    } else {
        uint8_t data = physical_memory[physical_addr];
        printf("  READ: Loaded %d from physical 0x%08X\n", data, physical_addr);
    }
    printf("  SUCCESS!\n\n");
}

// Print segment information
void print_segment_info() {
    const char *seg_names[] = {"Code", "Heap", "Unused", "Stack"};

    printf("=== SEGMENT TABLE ===\n");
    for (int i = 0; i < 4; i++) {
        if (current_process.bounds[i] > 0) {
            printf("Segment %d (%s): Base=%dKB, Bounds=%dKB, Grows %s\n", i,
                   seg_names[i], current_process.base[i] / 1024,
                   current_process.bounds[i] / 1024,
                   current_process.grows_positive[i] ? "Positive" : "Negative");
        } else {
            printf("Segment %d (%s): UNUSED\n", i, seg_names[i]);
        }
    }
    printf("\nPhysical Memory Layout:\n");
    printf("  Code:  32KB - 34KB (grows up)\n");
    printf("  Heap:  34KB - 36KB (grows up)\n");
    printf("  Stack: 26KB - 28KB (grows DOWN from 28KB base)\n");
    printf("\n");
}

// Helper to show virtual address space layout
void print_virtual_layout() {
    printf("=== VIRTUAL ADDRESS SPACE LAYOUT (14-bit addresses) ===\n");
    printf("Segment 00 (Code):  0x0000 - 0x0FFF (0KB - 4KB)\n");
    printf("Segment 01 (Heap):  0x1000 - 0x1FFF (4KB - 8KB)\n");
    printf("Segment 10 (Unused):0x2000 - 0x2FFF (8KB - 12KB) [NOT USED]\n");
    printf("Segment 11 (Stack): 0x3000 - 0x3FFF (12KB - 16KB) [GROWS "
           "BACKWARDS]\n");
    printf("\nIMPORTANT: Stack segment (11) grows BACKWARDS!\n");
    printf("  Virtual 0x3FFF (16KB-1) -> Physical 28KB-1 (stack base)\n");
    printf(
        "  Virtual 0x3C00 (15KB)   -> Physical 27KB   (1KB down from base)\n");
    printf(
        "  Virtual 0x3800 (14KB)   -> Physical 26KB   (2KB down from base)\n");
    printf("\n");
}

int main() {
    printf("SEGMENTATION WITH NEGATIVE-GROWING STACK SIMULATOR\n");
    printf("=================================================\n\n");

    print_virtual_layout();
    print_segment_info();

    // Book's specific example: virtual 15KB -> physical 27KB
    printf("=== BOOK EXAMPLE: Stack access (virtual 15KB = 0x3C00) ===\n");
    access_memory(0x3C00, 42, true); // Should map to physical 27KB

    printf("=== EXAMPLE 2: Code segment access ===\n");
    access_memory(0x0200, 0, false); // Read from code segment

    printf("=== EXAMPLE 3: Heap segment access ===\n");
    access_memory(0x1400, 99, true); // Write to heap segment

    printf("=== EXAMPLE 4: Stack - highest valid address ===\n");
    access_memory(0x3FFF, 11, true); // Top of stack virtual space

    printf("=== EXAMPLE 5: Stack - lowest valid address ===\n");
    access_memory(0x3800, 22, true); // Bottom of usable stack space

    printf("=== EXAMPLE 6: Stack bounds violation (too low) ===\n");
    access_memory(0x3400, 0, false); // Below valid stack range

    printf("=== EXAMPLE 7: Code bounds violation ===\n");
    access_memory(0x0900, 0, false); // Beyond code segment

    printf("=== ADDRESS MAPPING DEMONSTRATION ===\n");
    printf("Virtual -> Physical address mappings:\n");

    // Show the mapping pattern for stack
    uint16_t stack_addresses[] = {0x3FFF, 0x3E00, 0x3C00, 0x3800};
    for (int i = 0; i < 4; i++) {
        bool fault = false;
        uint32_t phys = translate_address(stack_addresses[i], &fault);
        if (!fault) {
            printf("  Virtual 0x%04X (%dKB) -> Physical 0x%08X (%dKB)\n",
                   stack_addresses[i], stack_addresses[i] / 1024, phys,
                   phys / 1024);
        }
        printf("\n");
    }

    return 0;
}