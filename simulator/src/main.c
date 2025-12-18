#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simulator.h"
#include "memory.h"
#include "files.h"
#include "core.h"

#define NUM_ARGS 13

int main(int argc, char *argv[]) {
    if (argc != NUM_ARGS + 1) {
        fprintf(stderr, "Usage: %s memin diskin irq2in memout regout trace hwregtrace cycles leds display7seg diskout monitor(txt) monitor(yuv)\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Input files
    const char *memin_path = argv[1];
    const char *diskin_path = argv[2];
    const char *irq2in_path = argv[3];

    // Output files
    const char *memout_path = argv[4];
    const char *regout_path = argv[5];
    const char *trace_path = argv[6];
    const char *hwregtrace_path = argv[7];
    const char *cycles_path = argv[8];
    const char *leds_path = argv[9];
    const char *display7seg_path = argv[10];
    const char *diskout_path = argv[11];
    const char *monitor_path = argv[12];
    const char *monitor_yuv_path = argv[13];

    // Initialize State
    SimState state;
    memset(&state, 0, sizeof(SimState));
    // memory_init(&state); // memset handled it

    // Read Inputs
    if (!files_read_memin(&state, memin_path)) {
        fprintf(stderr, "Error reading memin file: %s\n", memin_path);
        return EXIT_FAILURE;
    }
    files_read_diskin(&state, diskin_path);
    files_read_irq2in(&state, irq2in_path);

    // Initialize Trace Files
    files_init_traces(trace_path, hwregtrace_path, leds_path, display7seg_path);

    // Simulation Loop
    while (core_step(&state)) {
        // Continue until halted
    }

    // Close Trace Files
    files_close_traces();

    // Write Outputs
    files_write_memout(&state, memout_path);
    files_write_regout(&state, regout_path);
    files_write_diskout(&state, diskout_path);
    files_write_cycles(&state, cycles_path);
    files_write_monitor(&state, monitor_path, monitor_yuv_path);

    // Cleanup
    if (state.irq2_cycles) {
        free(state.irq2_cycles);
    }

    return EXIT_SUCCESS;
}
