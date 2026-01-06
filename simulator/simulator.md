# Simulator Documentation
This document documents the code structure and implementation of the simulator. Each section describes the role of a `*.c/*.h`file pair within the project.

## Project File Structure
### `simulator.h`
#### System constants
- Define system-wide constants such as memory size, disk sectors, and monitor dimensions.
- Define the `word` type as a 20-bit unsigned integer stored in a 32-bit unsigned int, along with relevant constants and macros for bit masking.
#### General purpose / IO register constants
Define number of registers and constants mapping register names to indices for both general-purpose and IO registers.
#### Simulator state structure
Define the `SimState` struct that stores the entire state of the simulator (GP and IO registers, program counter, memory, disk, monitor, timer, and interrupt flags)
#### Instruction structure
Define `Instruction` struct to represent decoded instructions. (opcode, destination/source/target registers, immediate values) as well as function prototype for instruction decoding.

### `main.c`
- Entry point of the simulator (`main` function). Parses the CLI arguments (file paths), initializes simulator state, reads input files, and starts the simulation loop. When finished, writes output files and cleans up resources.

### `core.c` / `core.h`
Define the core simulation step: 
- Read the next instction from memory using the program counter. 
- Decode the instruction into its components, reading the immediate values from the next instruction as needed.
- Execute the instruction (handle each opcode case), updating registers, memory, and IO components as required.
- Update the program counter. 
- Handle interrupts and timer updates.

### `memory.c` / `memory.h`
Functions for initializing, reading, and writing the simulator's memory.

### `disk.c` / `disk.h`
Functions for initializing a disk operation (reading or writing) and a 'disk tick'. After inializtion, the disk operation will complete after a fixed delay (defined by `DISK_DELAY` constant). The disk tick function is called each simulation cycle to update the disk state and complete operations when the delay has completed.

### `timer.c` / `timer.h`
`timer_tick` function to update the simulator's timer register and handle timer interrupts.

### `monitor.c` / `monitor.h`
`monitor_cmd_write` function to update the monitor display memory when a write command is issued to the monitor IO register.

### `files.c` / `files.h`
- "Reading" functions to load the initial memory, disk state, and irqin counters from the input files. 
- "Writing" functions to save the final memory, register state, disk state, total cycles, and monitor state to the output files. 
- "Tracing" functions that run at each simulation step to log the current instruction state, IO register access, LED registers, and 7 segment display to the trace files. 