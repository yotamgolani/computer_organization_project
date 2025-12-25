#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef uint32_t word;

// Register Names (Indices)
#define REG_ZERO 0
#define REG_IMM 1 // sign-extended immediate value
#define REG_V0 2
#define REG_A0 3
#define REG_A1 4
#define REG_A2 5
#define REG_A3 6
#define REG_T0 7
#define REG_T1 8
#define REG_T2 9
#define REG_S0 10
#define REG_S1 11
#define REG_S2 12
#define REG_GP 13
#define REG_SP 14
#define REG_RA 15

// Macro to check if a register is writable (not ZERO or IMM)
#define WRITABLE_REGISTER(reg) ((reg) != REG_ZERO && (reg) != REG_IMM)

#define WORD_LEN 20
#define WORD_MASK 0xFFFFF
#define MASK_WORD(w) ((w) & WORD_MASK)

// System Constants
#define MEM_SIZE 4096
#define NUM_REGISTERS 16
#define NUM_IO_REGISTERS 23
#define DISK_SECTORS 128
#define DISK_SECTOR_SIZE 128
#define MONITOR_WIDTH 256
#define MONITOR_HEIGHT 256
#define MONITOR_SIZE (MONITOR_WIDTH * MONITOR_HEIGHT)

// IO Register Indices
#define IOREG_IRQ0ENABLE 0     // irq0 enable (single-bit)
#define IOREG_IRQ1ENABLE 1     // irq1 enable (single-bit)
#define IOREG_IRQ2ENABLE 2     // irq2 enable (single-bit)
#define IOREG_IRQ0STATUS 3     // irq0 status (single-bit)
#define IOREG_IRQ1STATUS 4     // irq1 status (single-bit)
#define IOREG_IRQ2STATUS 5     // irq2 status (single-bit)
#define IOREG_IRQHANDLER 6     // PC of irq handler (12-bits)
#define IOREG_IRQRETURN 7      // PC to return to after irq (12-bits)
#define IOREG_CLKS 8           // clock cycles since boot (32-bits)
#define IOREG_LEDS 9           // 32-bit LED array
#define IOREG_DISPLAY7SEG 10   // 8-digit 7-segment display (32-bits)
#define IOREG_TIMERENABLE 11   // timer enable (single-bit)
#define IOREG_TIMERCURRENT 12  // current timer value (32-bits)
#define IOREG_TIMERMAX 13      // timer max value (32-bits)
#define IOREG_DISKCMD 14       // disk command (2-bits) - 0: none, 1: read, 2: write
#define IOREG_DISKSECTOR 15    // disk sector number (7-bits)
#define IOREG_DISKBUFFER 16    // disk buffer address in memory (12-bits)
#define IOREG_DISKSTATUS 17    // disk status (single-bit) - 0: idle, 1: busy
// 18-19 Reserved
#define IOREG_MONITORADDR 20   // monitor address (16-bits)
#define IOREG_MONITORDATA 21   // monitor data (8-bits)
#define IOREG_MONITORCMD 22    // monitor command (1-bit) - 0: none, 1: write pixel

// Simulator State
typedef struct {
    // --- CPU registers ---
    uint32_t registers[NUM_REGISTERS]; // 32-bit wide registers
    uint32_t pc;                       // Program Counter (12-bit address)
    
    // --- IO registers ---
    uint32_t io_registers[NUM_IO_REGISTERS]; // IO registers
    
    // --- Memory ---
    word memory[MEM_SIZE]; // Main memory (4096 words of 20-bits)
    
    // --- Peripheral Data ---
    word disk[DISK_SECTORS][DISK_SECTOR_SIZE]; // Disk storage (128 sectors of 128 words)
    uint8_t monitor[MONITOR_SIZE]; // Frame buffer
    
    // --- Simulation Control & Status ---
    uint64_t total_cycles; // Global clock cycle counter
    uint64_t disk_timer;   // Cycles remaining for the current disk operation
    bool disk_busy;        // True if a disk operation is in progress
    bool halted;          // True if the simulator has executed the HALT instruction
    bool in_interrupt;    // True if the CPU is currently inside an interrupt service routine
    
    // --- IRQ2 External Interrupt Data ---
    uint32_t *irq2_cycles; // Array of cycle indices for IRQ2 interrupt
    int irq2_count;        // Total number of IRQ2 interrupts in the input file
    int irq2_index;        // Index of the next IRQ2 interrupts to process
} SimState;

// Instruction Struct
typedef struct {
    uint32_t opcode;
    uint32_t rd;
    uint32_t rs;
    uint32_t rt;
    bool is_imm;
} Instruction;

static inline Instruction decode_instruction(word inst_word) {
    Instruction inst;
    inst.opcode = (inst_word >> 12) & 0xFF; // 8-bit opcode
    inst.rd = (inst_word >> 8) & 0xF; // 4-bit rd
    inst.rs = (inst_word >> 4) & 0xF; // 4-bit rs
    inst.rt = inst_word & 0xF; // 4-bit rt
    inst.is_imm = (inst.rs == REG_IMM || inst.rt == REG_IMM || inst.rd == REG_IMM); // Immediate if rs or rt is REG_IMM
    return inst;
}


#endif // SIMULATOR_H
