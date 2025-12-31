#include "monitor.h"

// Write to monitor
void monitor_cmd_write(SimState *state) {
    uint32_t value = state->io_registers[IOREG_MONITORCMD];

    if (value == 1) {
        uint32_t addr = state->io_registers[IOREG_MONITORADDR];
        uint32_t data = state->io_registers[IOREG_MONITORDATA];

        if (addr < MONITOR_SIZE) {
            state->monitor[addr] = (uint8_t)(data & 0xFF);
        }
    }
}
