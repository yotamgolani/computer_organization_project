#ifndef CORE_H
#define CORE_H

#include "simulator.h"
#include <stdio.h>

// Perform one cycle of simulation
// Returns false if halted
bool core_step(SimState *state);

#endif // CORE_H
