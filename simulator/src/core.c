#include "core.h"
#include "memory.h"
#include "disk.h"
#include "timer.h"
#include "monitor.h"
#include "files.h"
#include <stdlib.h>
#include <string.h>

// Sign-extend a value with given bit-width to 32-bits
static int32_t sign_extend(uint32_t val, int bits) {
    if (val & (1 << (bits - 1))) { // MSB is set => negative number
        return (int32_t)(val | (~((1 << bits) - 1))); // Fill upper bits with 1s
    }
    return (int32_t)val; // For positive values, casting to int32_t will sign-extend correctly
}

// Simulate one CPU step
bool core_step(SimState *state) {
    if (state->halted) return false; // Stop if already halted

    uint32_t pc = state->pc;
    word inst_word = memory_read(state, pc); // Read instruction from memory
    
    // Decode instruction fields
    Instruction inst = decode_instruction(inst_word);

    int32_t imm_val = 0;
    
    // Set REG_ZERO to 0
    state->registers[REG_ZERO] = 0;

    // Set REG_IMM if needed
    if (inst.is_imm) {
        imm_val = memory_read(state, pc + 1);
        state->registers[REG_IMM] = sign_extend(imm_val, WORD_LEN);
    }
    else // Clear REG_IMM if not used
    {
        state->registers[REG_IMM] = 0;
    }
    
    // Log instruction trace
    files_log_trace_step(state, inst_word);
    
    // Calculate next PC based on instruction type
    uint32_t next_pc = pc + (inst.is_imm ? 2 : 1);
    
    int32_t src_s = state->registers[inst.rs];
    int32_t src_t = state->registers[inst.rt];
    int32_t res = 0;
    bool write_reg = false;

    // Handle each opcode
    switch (inst.opcode) {
        case 0: // add
            res = src_s + src_t;
            write_reg = true;
            break;
        case 1: // sub
            res = src_s - src_t;
            write_reg = true;
            break;
        case 2: // mul
            res = src_s * src_t;
            write_reg = true;
            break;
        case 3: // and
            res = src_s & src_t;
            write_reg = true;
            break;
        case 4: // or
            res = src_s | src_t;
            write_reg = true;
            break;
        case 5: // xor
            res = src_s ^ src_t;
            write_reg = true;
            break;
        case 6: // sll
            res = src_s << src_t;
            write_reg = true;
            break;
        case 7: // sra
            res = src_s >> src_t;
            write_reg = true;
            break;
        case 8: // srl
            res = (uint32_t)src_s >> src_t; // casting to uint32_t for logical right shift
            write_reg = true;
            break;
        case 9: // beq
            if (src_s == src_t) { next_pc = state->registers[inst.rd]; }
            break;
        case 10: // bne
            if (src_s != src_t) { next_pc = state->registers[inst.rd]; }
            break;
        case 11: // blt
            if (src_s < src_t) { next_pc = state->registers[inst.rd]; }
            break;
        case 12: // bgt
            if (src_s > src_t) { next_pc = state->registers[inst.rd]; }
            break;
        case 13: // ble
            if (src_s <= src_t) { next_pc = state->registers[inst.rd]; }
            break;
        case 14: // bge
            if (src_s >= src_t) { next_pc = state->registers[inst.rd]; }
            break;
        case 15: // jal
            state->registers[inst.rd] = next_pc; 
            next_pc = state->registers[inst.rs];
            break;
        case 16: // lw
            {
                uint32_t addr = src_s + src_t;
                res = memory_read(state, addr); // read 20-bit word from memory
                res = sign_extend(res, WORD_LEN); // sign-extend to 32-bits
                write_reg = true;
            }
            break;
        case 17: // sw
            {
                uint32_t addr = src_s + src_t;
                uint32_t val = state->registers[inst.rd]; // value to store
                memory_write(state, addr, val); // write 20-bit word to memory (masking happens inside)
            }
            break;
        case 18: // reti
            next_pc = state->io_registers[IOREG_IRQRETURN];
            state->in_interrupt = false; // Return from interrupt
            break;
        case 19: // in
            {
                uint32_t reg_idx = src_s + src_t;
                if (reg_idx < NUM_IO_REGISTERS) { // valid IOReg index
                    res = state->io_registers[reg_idx];
                    files_log_hwreg(state, HWREG_READ, reg_idx, res);
                }
                write_reg = true;
            }
            break;
        case 20: // out
            {
                uint32_t reg_idx = src_s + src_t;
                uint32_t val = state->registers[inst.rd];
                if (reg_idx < NUM_IO_REGISTERS) {
                    state->io_registers[reg_idx] = val;
                    files_log_hwreg(state, HWREG_WRITE, reg_idx, val);
                    // Handle different IO Operations
                    switch (reg_idx) {
                        case IOREG_LEDS:
                            files_log_leds(state);
                            break;
                        case IOREG_DISPLAY7SEG:
                            files_log_display7seg(state);
                            break;
                        case IOREG_DISKCMD:
                            disk_cmd_write(state);
                            break;
                        case IOREG_MONITORCMD:
                            monitor_cmd_write(state);
                            break;
                    }
                }
            }
            break;
        case 21: // halt
            state->halted = true;
            state->io_registers[IOREG_CLKS]++;
            state->total_cycles++;
            return false; // Stop execution
    }

    if (write_reg && WRITABLE_REGISTER(inst.rd)) {
        state->registers[inst.rd] = res; // Write result to destination register
    }
    
    state->pc = next_pc & 0xFFF; // Ensure PC is 12-bits

    // Check IRQ2
    bool irq2_active = false;
    if (state->irq2_cycles && state->irq2_index < state->irq2_count) { // IRQ2 cycles left to process
        if (state->irq2_cycles[state->irq2_index] == state->total_cycles) { // Set IRQ2 to high if cycle matches
            irq2_active = true;
            state->irq2_index++; 
        }
    }
    
    if (irq2_active) {
        state->io_registers[IOREG_IRQ2STATUS] = 1;
    }

    timer_tick(state); // Update timer
    disk_tick(state); // Update disk
    
    bool irq0 = (state->io_registers[IOREG_IRQ0ENABLE] & state->io_registers[IOREG_IRQ0STATUS]);
    bool irq1 = (state->io_registers[IOREG_IRQ1ENABLE] & state->io_registers[IOREG_IRQ1STATUS]);
    bool irq2 = (state->io_registers[IOREG_IRQ2ENABLE] & state->io_registers[IOREG_IRQ2STATUS]);
    
    bool irq = irq0 || irq1 || irq2;
    
    if (irq && !state->in_interrupt) { //  If any IRQ is active and not already in interrupt
        state->in_interrupt = true; // Enter interrupt
        state->io_registers[IOREG_IRQRETURN] = state->pc; // Save return PC
        state->pc = state->io_registers[IOREG_IRQHANDLER] & 0xFFF; // Jump to IRQ handler (12-bits)
    }
    // Disabling IRQ status bits are handled in the IRQHandler above
    
    state->io_registers[IOREG_CLKS]++; // Increment clock cycles
    state->total_cycles++; // Increment total cycles
    
    return true;
}
