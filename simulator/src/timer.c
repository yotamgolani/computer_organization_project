#include "timer.h"

void timer_tick(SimState *state) {
    if (state->io_registers[IOREG_TIMERENABLE]) {
        uint32_t current = state->io_registers[IOREG_TIMERCURRENT];
        uint32_t max = state->io_registers[IOREG_TIMERMAX];

        if (current == max) {
            state->io_registers[IOREG_IRQ0STATUS] = 1;
            state->io_registers[IOREG_TIMERCURRENT] = 0;
        } else {
            state->io_registers[IOREG_TIMERCURRENT] = current + 1;
        }
    }
}
