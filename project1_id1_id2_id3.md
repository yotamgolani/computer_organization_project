This PDF explains the code structure of the simulator and the assembler programs.
# Simulator
Each section describes the role of a `*.c/*.h`file pair within the project.
## `simulator.h`
### System constants
- Define system-wide constants such as memory size, disk sectors, and monitor dimensions.
- Define the `word` type as a 20-bit unsigned integer stored in a 32-bit unsigned int, along with relevant constants and macros for bit masking.
### General purpose / IO register constants
Define number of registers and constants mapping register names to indices for both general-purpose and IO registers.
### Simulator state structure
Define the `SimState` struct that stores the entire state of the simulator (GP and IO registers, program counter, memory, disk, monitor, timer, and interrupt flags)
### Instruction structure
Define `Instruction` struct to represent decoded instructions. (opcode, destination/source/target registers, immediate values) as well as function prototype for instruction decoding.

## `main.c`
- Entry point of the simulator (`main` function). Parses the CLI arguments (file paths), initializes simulator state, reads input files, and starts the simulation loop. When finished, writes output files and cleans up resources.

## `core.c` / `core.h`
Define the core simulation step: 
- Read the next instction from memory using the program counter. 
- Decode the instruction into its components, reading the immediate values from the next instruction as needed.
- Execute the instruction (handle each opcode case), updating registers, memory, and IO components as required.
- Update the program counter. 
- Handle interrupts and timer updates.

## `memory.c` / `memory.h`
Functions for initializing, reading, and writing the simulator's memory.

## `disk.c` / `disk.h`
Functions for initializing a disk operation (reading or writing) and a 'disk tick'. After inializtion, the disk operation will complete after a fixed delay (defined by `DISK_DELAY` constant). The disk tick function is called each simulation cycle to update the disk state and complete operations when the delay has completed.

## `timer.c` / `timer.h`
`timer_tick` function to update the simulator's timer register and handle timer interrupts.

## `monitor.c` / `monitor.h`
`monitor_cmd_write` function to update the monitor display memory when a write command is issued to the monitor IO register.

## `files.c` / `files.h`
- "Reading" functions to load the initial memory, disk state, and irqin counters from the input files. 
- "Writing" functions to save the final memory, register state, disk state, total cycles, and monitor state to the output files. 
- "Tracing" functions that run at each simulation step to log the current instruction state, IO register access, LED registers, and 7 segment display to the trace files. 
# Assembler 
The assembler's code is all contained within a single `main.c` file. each section describes a functionality within the assembler. 
## Assembler entry point
- Implements the assembler’s `main` function.
- Parses command-line arguments (assembly input file path and output `memin.txt` path).
- Manages file opening, execution of the assembly passes, output generation, and cleanup.

## Two-pass assembly process
- **Pass 1 – Symbol collection**
  - Reads the assembly source line by line.
  - Removes and validates comments.
  - Identifies label definitions and stores them in a symbol table.
  - Tracks the program counter (PC) to compute label addresses.
  - Determines instruction size, including extra memory words for immediate values.
- **Pass 2 – Code generation**
  - Re-reads the assembly source.
  - Assembles instructions into a memory array of fixed size.
  - Resolves label references used as immediates.
  - Applies `.word` directives to explicitly write values into memory.
  - Produces the final memory image.

## Symbol table
- Stores all labels defined in the program along with their corresponding memory addresses.
- Supports:
  - Adding new labels
  - Detecting duplicate label definitions
  - Resolving label addresses during the second pass

## Parsing and validation
- **Tokenization**
  - Splits input lines into tokens, treating commas as separators.
- **Label validation**
  - Ensures labels follow the required syntax and are case-sensitive.
- **Numeric parsing**
  - Supports decimal and hexadecimal values (`0x...`).
  - Masks values to the 20-bit word width.
- **Comment handling**
  - Uses `#` as the comment delimiter.
  - Ensures comments only appear after the required operands for each line type.

## Instruction handling
- Maps opcode mnemonics to numeric opcode values.
- Maps register names to register indices.
- Encodes instructions into 20-bit words.
- Detects instructions requiring an immediate value and emits an additional memory word when needed.

## Memory image generation
- Uses a fixed-size memory array representing the simulator memory.
- Ensures the output contains exactly one word per memory address.
- Writes the final memory image to `memin.txt` using fixed-width hexadecimal formatting.

## Output
- Produces a `memin.txt` file containing:
  - Exactly `MEM_SIZE` lines
  - One 20-bit word per line
  - Each word printed as a zero-padded hexadecimal value
