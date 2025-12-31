#include "memory.h"
#include <string.h>

// Initialize memory (zero out)
void memory_init(SimState *state) {
    memset(state->memory, 0, sizeof(state->memory));
}

// Read a word from memory (with bounds checking)
word memory_read(SimState *state, uint32_t address) {
    if (address >= MEM_SIZE) {
        // Out of bounds read return 0
        return 0;
    }
    return MASK_WORD(state->memory[address]);
}

// Write a word to memory (with bounds checking)
void memory_write(SimState *state, uint32_t address, word data) {
    if (address >= MEM_SIZE) {
        return;
    }
    // Memory is 20 bits wide
    state->memory[address] = MASK_WORD(data);
}
