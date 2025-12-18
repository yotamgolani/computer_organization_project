#ifndef FILES_H
#define FILES_H

#include "simulator.h"
#include <stdio.h>

#define LINE_BUFFER_SIZE 100 // for reading our input files

// IO Register Name Strings
#define IO_REG_NAMES {                                   \
    "irq0enable", "irq1enable", "irq2enable",            \
    "irq0status", "irq1status", "irq2status",            \
    "irqhandler", "irqreturn",                           \
    "clks", "leds", "display7seg",                       \
    "timerenable", "timercurrent", "timermax",           \
    "diskcmd", "disksector", "diskbuffer", "diskstatus", \
    "reserved", "reserved",                              \
    "monitoraddr", "monitordata", "monitorcmd"}

typedef enum {
    HWREG_READ,
    HWREG_WRITE
} HwRegAction;

// Internal Trace Handles Structure
typedef struct {
    FILE *trace;
    FILE *hwregtrace;
    FILE *leds;
    FILE *display7seg;
} TraceHandles;

// Reading Functions (returns false on failure)
bool files_read_memin(SimState *state, const char *path);
bool files_read_diskin(SimState *state, const char *path);
bool files_read_irq2in(SimState *state, const char *path);

// Writing functions (snapshots)
void files_write_memout(SimState *state, const char *path);
void files_write_regout(SimState *state, const char *path);
void files_write_diskout(SimState *state, const char *path);
void files_write_cycles(SimState *state, const char *path);
void files_write_monitor(SimState *state, const char *path_txt, const char *path_yuv);

// Trace functions
void files_init_traces(const char *trace_path, const char *hwreg_path, const char *leds_path, const char *disp_path);
void files_close_traces(void);

void files_log_trace_step(SimState *state, word inst_word);
void files_log_hwreg(SimState *state, uint32_t reg_idx, HwRegAction action, uint32_t val);
void files_log_leds(SimState *state, uint32_t val);
void files_log_display7seg(SimState *state, uint32_t val);

#endif // FILES_H
