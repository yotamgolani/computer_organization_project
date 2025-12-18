#include "disk.h"
#include "memory.h"
#include <stdio.h>

#define DISK_DELAY 1024

// ----- DISK FUNCTIONS -----

// Start a disk operation (read or write) if the disk is not busy.
// Reads the command type from the IOREG_DISKCMD register.
void disk_cmd_write(SimState *state) {
    if (state->disk_busy) {
        return;
    }

    uint32_t cmd = state->io_registers[IOREG_DISKCMD];
    if (cmd == DISK_READ || cmd == DISK_WRITE) {
        state->disk_busy = true;
        state->io_registers[IOREG_DISKSTATUS] = 1; // Busy
        state->disk_timer = DISK_DELAY;
    }
}

// Update the disk timer and perform operation on completion
void disk_tick(SimState *state) {
    if (!state->disk_busy) {
        return;
    }

    state->disk_timer--;

    if (state->disk_timer == 0) {
        // Operation complete
        uint32_t cmd = state->io_registers[IOREG_DISKCMD];
        uint32_t sector = state->io_registers[IOREG_DISKSECTOR];
        uint32_t buffer_addr = state->io_registers[IOREG_DISKBUFFER];

        if (sector < DISK_SECTORS) {
            if (cmd == DISK_READ) { // Read sector (Disk -> Memory)
                for (int i = 0; i < DISK_SECTOR_SIZE; i++) {
                    memory_write(state, buffer_addr + i, state->disk[sector][i]);
                }
            } else if (cmd == DISK_WRITE) { // Write sector (Memory -> Disk)
                for (int i = 0; i < DISK_SECTOR_SIZE; i++) {
                    state->disk[sector][i] = memory_read(state, buffer_addr + i);
                }
            }
        }

        // Reset status registers
        state->io_registers[IOREG_DISKCMD] = DISK_NO_CMD;
        state->io_registers[IOREG_DISKSTATUS] = 0; // Idle
        state->disk_busy = false;

        // Trigger IRQ1 (Disk operation complete)
        state->io_registers[IOREG_IRQ1STATUS] = 1;
    }
}
