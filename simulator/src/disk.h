#ifndef DISK_H
#define DISK_H

#include "simulator.h"

#define DISK_DELAY 1024

typedef enum {
    DISK_NO_CMD = 0,
    DISK_READ = 1,
    DISK_WRITE = 2
} DiskCommand;

void disk_cmd_write(SimState *state);
void disk_tick(SimState *state);

#endif // DISK_H
