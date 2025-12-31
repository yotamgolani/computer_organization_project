#include "timer.h"

// Increment the timer and handle timer interrupts (IRQ0) if enabled
void timer_tick(SimState *state) {
    if (state->io_registers[IOREG_TIMERENABLE]) {
        uint32_t current = state->io_registers[IOREG_TIMERCURRENT];
        uint32_t max = state->io_registers[IOREG_TIMERMAX];

        if (current == max) { // Timer reached max value
            state->io_registers[IOREG_IRQ0STATUS] = 1;
            state->io_registers[IOREG_TIMERCURRENT] = 0;
        } else { // Increment timer
            state->io_registers[IOREG_TIMERCURRENT] = current + 1;
        }
    }
}
