#include "files.h"
#include <stdlib.h>
#include <string.h>

// Static instance of TraceHandles to manage open file pointers for logging
static TraceHandles g_traces = { NULL, NULL, NULL, NULL };

static const char* g_io_reg_names[] = IO_REG_NAMES;

// ----- READING FUNCTIONS (Called once at the beginning) -----

// Read memin to SimState memory
// Returns false on failure
bool files_read_memin(SimState *state, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return false;
    char line[LINE_BUFFER_SIZE];
    int addr = 0;
    while (fgets(line, sizeof(line), f) && addr < MEM_SIZE) { // Read a line
        uint32_t val;
        // Parse the hex value and mask it to 20 bits
        if (sscanf(line, "%x", &val) == 1) {
            state->memory[addr++] = MASK_WORD(val);
        }
    }

    // the rest is zero-initialized by the caller
    
    fclose(f);
    return true;
}

// Read diskin to SimState disk
// Returns false on failure
bool files_read_diskin(SimState *state, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return false;
    char line[LINE_BUFFER_SIZE];
    int sector = 0;
    int word_idx = 0;
    
    while (fgets(line, sizeof(line), f)) { // Read a line
        uint32_t val;
        if (sscanf(line, "%x", &val) == 1) { // Parse hex value 
            state->disk[sector][word_idx] = MASK_WORD(val); // Store 20-bit word
            word_idx++;
            if (word_idx >= DISK_SECTOR_SIZE) { // Move to next sector
                word_idx = 0;
                sector++;
            }
        }
    }
    
    fclose(f);
    return true;
}

// Read irq2in to SimState irq2_cycles
// Returns false on failure
bool files_read_irq2in(SimState *state, const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return false;
    
    // Count lines first to allocate
    int count = 0;
    char line[LINE_BUFFER_SIZE];
    while (fgets(line, sizeof(line), f)) count++;
    
    rewind(f);

    if (count > 0) {
        state->irq2_cycles = (uint32_t*)malloc(sizeof(uint32_t) * count);
        state->irq2_count = 0;
        
        while (fgets(line, sizeof(line), f)) { // Read a line
            uint32_t val;
            if (sscanf(line, "%u", &val) == 1) { // Parse Decimal
                state->irq2_cycles[state->irq2_count++] = val; // store cycle index in next position
            }
        }
    }
    // else (count == 0): 
    // state->irq2_cycles remains NULL (from zero-init)
    // state->irq2_count remains 0

    fclose(f);
    return true;
}

// ----- SNAPSHOT FUNCTIONS (Called once at the end) -----

// Write SimState memory to memout
void files_write_memout(SimState *state, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return;
    for (int i = 0; i < MEM_SIZE; i++) {
        fprintf(f, "%05X\n", state->memory[i]); // write each word as 5-digit hex
    }
    fclose(f);
}

// Write SimState registers to regout
void files_write_regout(SimState *state, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return;
    // Write registers R2-R15, one per line
    for (int i = 2; i < 16; i++) {
        fprintf(f, "%08X\n", state->registers[i]);
    }
    fclose(f);
}

// Write SimState disk to diskout
void files_write_diskout(SimState *state, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return;
    // Write all sectors and words
    for (int s = 0; s < DISK_SECTORS; s++) {
        for (int w = 0; w < DISK_SECTOR_SIZE; w++) {
            fprintf(f, "%05X\n", state->disk[s][w]); // Write each word as 5-digit hex 
        }
    }
    fclose(f);
}

// Write SimState total cycles to cycles out
void files_write_cycles(SimState *state, const char *path) {
    FILE *f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "%llu\n", state->total_cycles); // Write total cycles as decimal
    fclose(f);
}

// Write SimState monitor to monitor out (text and binary)
void files_write_monitor(SimState *state, const char *path_txt, const char *path_yuv) {
    FILE *ft = fopen(path_txt, "w");  // Text for .txt
    FILE *fy = fopen(path_yuv, "wb"); // Binary for .yuv
    
    if (ft) {
        for (int i = 0; i < MONITOR_SIZE; i++) {
            fprintf(ft, "%02X\n", state->monitor[i]); // write each byte as 2-digit hex
        }
        fclose(ft);
    }
    
    if (fy) {
        fwrite(state->monitor, 1, MONITOR_SIZE, fy); // write raw bytes
        fclose(fy);
    }
}

// ----- RUNTIME LOGGING FUNCTIONS (runs during simulation) -----

// Initialize trace files.
// Opens files for writing (logging) throughout the simulation.
void files_init_traces(const char *trace_path, const char *hwreg_path, const char *leds_path, const char *disp_path) {
    g_traces.trace = fopen(trace_path, "w");
    g_traces.hwregtrace = fopen(hwreg_path, "w");
    g_traces.leds = fopen(leds_path, "w");
    g_traces.display7seg = fopen(disp_path, "w");
}

// Close trace files.
// Called at the end of simulation to close log files.
void files_close_traces(void) {
    if (g_traces.trace) { fclose(g_traces.trace); g_traces.trace = NULL; }
    if (g_traces.hwregtrace) { fclose(g_traces.hwregtrace); g_traces.hwregtrace = NULL; }
    if (g_traces.leds) { fclose(g_traces.leds); g_traces.leds = NULL; }
    if (g_traces.display7seg) { fclose(g_traces.display7seg); g_traces.display7seg = NULL; }
}

// Log the current instruction step
void files_log_trace_step(SimState *state, word inst_word) {
    if (!g_traces.trace) return;
    fprintf(g_traces.trace, "%03X %05X", state->pc, inst_word); // 3 hex digits for PC, 5 hex digits for instruction
    for (int i = 0; i < 16; i++) {
        fprintf(g_traces.trace, " %08X", state->registers[i]); // 8 hex digits per register
    }
    fprintf(g_traces.trace, "\n");
}

// Log hardware register access
void files_log_hwreg(SimState *state, HwRegAction action, uint32_t reg_idx, uint32_t val) {
    if (!g_traces.hwregtrace) return;
    const char *action_str = (action == HWREG_READ) ? "READ" : "WRITE";
    const char *name = (reg_idx < NUM_IO_REGISTERS) ? g_io_reg_names[reg_idx] : "unknown";
    fprintf(g_traces.hwregtrace, "%llu %s %s %08X\n", state->total_cycles, action_str, name, val);
}

// Log LED register changes
void files_log_leds(SimState *state) {
    if (!g_traces.leds) return;
    uint32_t val = state->io_registers[IOREG_LEDS];
    fprintf(g_traces.leds, "%llu %08X\n", state->total_cycles, val);
}

// Log 7-segment display register changes
void files_log_display7seg(SimState *state) {
    if (!g_traces.display7seg) return;
    uint32_t val = state->io_registers[IOREG_DISPLAY7SEG];
    fprintf(g_traces.display7seg, "%llu %08X\n", state->total_cycles, val);
}
