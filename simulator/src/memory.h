#ifndef MEMORY_H
#define MEMORY_H

#include "simulator.h"

void memory_init(SimState *state);
word memory_read(SimState *state, uint32_t address);
void memory_write(SimState *state, uint32_t address, word data);

#endif // MEMORY_H
