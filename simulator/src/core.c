#include "core.h"
#include "memory.h"
#include "disk.h"
#include "timer.h"
#include "monitor.h"
#include "files.h"
#include <stdlib.h>
#include <string.h>

static int32_t sign_extend(uint32_t val, int bits) {
    if (val & (1 << (bits - 1))) {
        return (int32_t)(val | (~((1 << bits) - 1)));
    }
    return (int32_t)val;
}

bool core_step(SimState *state) {
    if (state->halted) return false;

    uint32_t pc = state->pc;
    word inst_word = memory_read(state, pc);
    
    // Decode
    uint32_t opcode = (inst_word >> 12) & 0xFF;
    uint32_t rd = (inst_word >> 8) & 0xF;
    uint32_t rs = (inst_word >> 4) & 0xF;
    uint32_t rt = (inst_word) & 0xF;

    bool is_imm = (rs == REG_IMM || rt == REG_IMM);
    int32_t imm_val = 0;
    
    // Log instruction trace
    files_log_trace_step(state, inst_word);

    if (is_imm) {
        imm_val = memory_read(state, pc + 1);
        state->registers[REG_IMM] = sign_extend(imm_val, 20);
    }
    
    uint32_t next_pc = pc + (is_imm ? 2 : 1);
    
    int32_t src_s = state->registers[rs];
    int32_t src_t = state->registers[rt];
    int32_t res = 0;
    bool write_reg = false;
    
    switch (opcode) {
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
            res = (uint32_t)src_s >> src_t; 
            write_reg = true;
            break;
        case 9: // beq
            if (src_s == src_t) { next_pc = state->registers[rd]; }
            break;
        case 10: // bne
            if (src_s != src_t) { next_pc = state->registers[rd]; }
            break;
        case 11: // blt
            if (src_s < src_t) { next_pc = state->registers[rd]; }
            break;
        case 12: // bgt
            if (src_s > src_t) { next_pc = state->registers[rd]; }
            break;
        case 13: // ble
            if (src_s <= src_t) { next_pc = state->registers[rd]; }
            break;
        case 14: // bge
            if (src_s >= src_t) { next_pc = state->registers[rd]; }
            break;
        case 15: // jal
            state->registers[rd] = next_pc; 
            next_pc = state->registers[rs];
            break;
        case 16: // lw
            {
                uint32_t addr = src_s + src_t;
                res = memory_read(state, addr);
                res = sign_extend(res, 20);
                write_reg = true;
            }
            break;
        case 17: // sw
            {
                uint32_t addr = src_s + src_t;
                uint32_t val = state->registers[rd];
                memory_write(state, addr, val);
            }
            break;
        case 18: // reti
            next_pc = state->io_registers[IOREG_IRQRETURN];
            state->in_interrupt = false; // Return from interrupt
            break;
        case 19: // in
            {
                uint32_t reg_idx = src_s + src_t;
                if (reg_idx < NUM_IO_REGISTERS) {
                    res = state->io_registers[reg_idx];
                    files_log_hwreg(state, reg_idx, HWREG_READ, res);
                }
                write_reg = true;
            }
            break;
        case 20: // out
            {
                uint32_t reg_idx = src_s + src_t;
                uint32_t val = state->registers[rd];
                if (reg_idx < NUM_IO_REGISTERS) {
                    state->io_registers[reg_idx] = val;
                    files_log_hwreg(state, reg_idx, HWREG_WRITE, val);

                    if (reg_idx == IOREG_DISKCMD) {
                        disk_cmd_write(state);
                    } else if (reg_idx == IOREG_MONITORCMD) {
                        monitor_cmd_write(state);
                    } else if (reg_idx == IOREG_LEDS) {
                         files_log_leds(state);
                    } else if (reg_idx == IOREG_DISPLAY7SEG) {
                         files_log_display7seg(state);
                    }
                }
            }
            break;
        case 21: // halt
            state->halted = true;
            return false;
    }

    if (write_reg && rd != REG_ZERO && rd != REG_IMM) {
        state->registers[rd] = res;
    }
    
    state->registers[REG_ZERO] = 0;
    state->pc = next_pc & 0xFFF;

    // Check IRQ2
    bool irq2_active = false;
    if (state->irq2_cycles && state->irq2_index < state->irq2_count) {
        if (state->irq2_cycles[state->irq2_index] == state->total_cycles) {
            irq2_active = true;
            state->irq2_index++; 
        } else if (state->irq2_cycles[state->irq2_index] < state->total_cycles) {
             state->irq2_index++;
        }
    }
    
    if (irq2_active) {
        state->io_registers[IOREG_IRQ2STATUS] = 1;
    } else {
        state->io_registers[IOREG_IRQ2STATUS] = 0;
    }

    timer_tick(state);
    disk_tick(state);
    
    bool irq0 = (state->io_registers[IOREG_IRQ0ENABLE] & state->io_registers[IOREG_IRQ0STATUS]);
    bool irq1 = (state->io_registers[IOREG_IRQ1ENABLE] & state->io_registers[IOREG_IRQ1STATUS]);
    bool irq2 = (state->io_registers[IOREG_IRQ2ENABLE] & state->io_registers[IOREG_IRQ2STATUS]);
    
    bool irq = irq0 || irq1 || irq2;
    
    if (irq && !state->in_interrupt) {
        state->in_interrupt = true;
        state->io_registers[IOREG_IRQRETURN] = state->pc;
        state->pc = state->io_registers[IOREG_IRQHANDLER] & 0xFFF;
    }
    
    state->io_registers[IOREG_CLKS]++;
    state->total_cycles++;
    
    return true;
}
