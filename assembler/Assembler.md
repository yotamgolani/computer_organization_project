# Assembler Documentation
This document documents the code structure and implementation of the assembler. Each section describes the role of a `*.c/*.h` file pair (or single source file) within the project.

## Project File Structure
### `main.c`
#### Assembler entry point
- Implements the assembler’s `main` function.
- Parses command-line arguments (assembly input file path and output `memin.txt` path).
- Manages file opening, execution of the assembly passes, output generation, and cleanup.

#### Two-pass assembly process
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

#### Symbol table
- Stores all labels defined in the program along with their corresponding memory addresses.
- Supports:
  - Adding new labels
  - Detecting duplicate label definitions
  - Resolving label addresses during the second pass

#### Parsing and validation
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

#### Instruction handling
- Maps opcode mnemonics to numeric opcode values.
- Maps register names to register indices.
- Encodes instructions into 20-bit words.
- Detects instructions requiring an immediate value and emits an additional memory word when needed.

#### Memory image generation
- Uses a fixed-size memory array representing the simulator memory.
- Ensures the output contains exactly one word per memory address.
- Writes the final memory image to `memin.txt` using fixed-width hexadecimal formatting.

#### Output
- Produces a `memin.txt` file containing:
  - Exactly `MEM_SIZE` lines
  - One 20-bit word per line
  - Each word printed as a zero-padded hexadecimal value
