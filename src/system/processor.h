/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DMGL_PROCESSOR_H_
#define DMGL_PROCESSOR_H_

#include <system.h>

typedef union
{
    struct
    {
        union
        {
            struct
            {
                uint8_t : 4;
                uint8_t carry : 1;
                uint8_t half_carry : 1;
                uint8_t negative : 1;
                uint8_t zero : 1;
            };
            uint8_t low;
        };
        uint8_t high;
    };
    uint16_t word;
} dmgl_register_t;

typedef struct
{
    uint8_t delay;
    bool halt_bug;
    bool halted;
    bool stopped;
    dmgl_register_t af;
    dmgl_register_t bc;
    dmgl_register_t de;
    dmgl_register_t hl;
    dmgl_register_t pc;
    dmgl_register_t sp;
    struct
    {
        uint16_t address;
        uint8_t opcode;
    } instruction;
    struct
    {
        uint8_t delay;
        uint8_t enable;
        bool enabled;
        uint8_t flag;
    } interrupt;
} dmgl_processor_t;

typedef void (*dmgl_processor_instruction_t)(dmgl_processor_t *const processor);

void dmgl_processor_clock(dmgl_processor_t *const processor);
void dmgl_processor_interrupt(dmgl_processor_t *const processor, uint8_t interrupt);
uint8_t dmgl_processor_read(const dmgl_processor_t *const processor, uint16_t address);
void dmgl_processor_write(dmgl_processor_t *const processor, uint16_t address, uint8_t value);

#endif /* DMGL_PROCESSOR_H_ */
