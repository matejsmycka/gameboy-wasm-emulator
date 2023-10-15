/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <processor.h>

static void dmgl_processor_execute_adc(dmgl_processor_t *const processor)
{
    uint8_t operand = 0;
    uint16_t carry = 0, sum = 0;
    processor->delay = 4;
    switch (processor->instruction.opcode)
    {
        case 0x88: /* B */
            operand = processor->bc.high;
            break;
        case 0x89: /* C */
            operand = processor->bc.low;
            break;
        case 0x8A: /* D */
            operand = processor->de.high;
            break;
        case 0x8B: /* E */
            operand = processor->de.low;
            break;
        case 0x8C: /* H */
            operand = processor->hl.high;
            break;
        case 0x8D: /* L */
            operand = processor->hl.low;
            break;
        case 0x8E: /* (HL) */
            processor->delay += 4;
            operand = dmgl_read(processor->hl.word);
            break;
        case 0x8F: /* A */
            operand = processor->af.high;
            break;
        case 0xCE: /* # */
            processor->delay += 4;
            operand = dmgl_read(processor->pc.word++);
            break;
        default:
            break;
    }
    sum = processor->af.high + operand + processor->af.carry;
    carry = processor->af.high ^ operand ^ processor->af.carry ^ sum;
    processor->af.carry = ((carry & 0x100) == 0x100);
    processor->af.half_carry = ((carry & 0x10) == 0x10);
    processor->af.negative = false;
    processor->af.zero = !(sum & 0xFF);
    processor->af.high = sum;
}

static void dmgl_processor_execute_add(dmgl_processor_t *const processor)
{
    uint8_t operand = 0;
    uint16_t carry = 0, sum = 0;
    processor->delay = 4;
    switch (processor->instruction.opcode)
    {
        case 0x80: /* B */
            operand = processor->bc.high;
            break;
        case 0x81: /* C */
            operand = processor->bc.low;
            break;
        case 0x82: /* D */
            operand = processor->de.high;
            break;
        case 0x83: /* E */
            operand = processor->de.low;
            break;
        case 0x84: /* H */
            operand = processor->hl.high;
            break;
        case 0x85: /* L */
            operand = processor->hl.low;
            break;
        case 0x86: /* (HL) */
            processor->delay += 4;
            operand = dmgl_read(processor->hl.word);
            break;
        case 0x87: /* A */
            operand = processor->af.high;
            break;
        case 0xC6: /* # */
            processor->delay += 4;
            operand = dmgl_read(processor->pc.word++);
            break;
        default:
            break;
    }
    sum = processor->af.high + operand;
    carry = processor->af.high ^ operand ^ sum;
    processor->af.carry = ((carry & 0x100) == 0x100);
    processor->af.half_carry = ((carry & 0x10) == 0x10);
    processor->af.negative = false;
    processor->af.zero = !(sum & 0xFF);
    processor->af.high = sum;
}

static void dmgl_processor_execute_add_hl(dmgl_processor_t *const processor)
{
    uint16_t operand = 0;
    uint32_t carry = 0, sum = 0;
    processor->delay = 8;
    switch (processor->instruction.opcode)
    {
        case 0x09: /* BC */
            operand = processor->bc.word;
            break;
        case 0x19: /* DE */
            operand = processor->de.word;
            break;
        case 0x29: /* HL */
            operand = processor->hl.word;
            break;
        case 0x39: /* SP */
            operand = processor->sp.word;
            break;
        default:
            break;
    }
    sum = processor->hl.word + operand;
    carry = processor->hl.word ^ operand ^ sum;
    processor->af.carry = ((carry & 0x10000) == 0x10000);
    processor->af.half_carry = ((carry & 0x1000) == 0x1000);
    processor->af.negative = false;
    processor->hl.word = sum;
}

static void dmgl_processor_execute_add_sp(dmgl_processor_t *const processor)
{
    uint32_t carry = 0, sum = 0;
    int8_t operand = dmgl_read(processor->pc.word++);
    processor->delay = 16;
    sum = processor->sp.word + operand;
    carry = processor->sp.word ^ operand ^ sum;
    processor->af.carry = ((carry & 0x100) == 0x100);
    processor->af.half_carry = ((carry & 0x10) == 0x10);
    processor->af.negative = false;
    processor->af.zero = false;
    processor->sp.word = sum;
}

static void dmgl_processor_execute_and(dmgl_processor_t *const processor)
{
    processor->delay = 4;
    switch (processor->instruction.opcode)
    {
        case 0xA0: /* B */
            processor->af.high &= processor->bc.high;
            break;
        case 0xA1: /* C */
            processor->af.high &= processor->bc.low;
            break;
        case 0xA2: /* D */
            processor->af.high &= processor->de.high;
            break;
        case 0xA3: /* E */
            processor->af.high &= processor->de.low;
            break;
        case 0xA4: /* H */
            processor->af.high &= processor->hl.high;
            break;
        case 0xA5: /* L */
            processor->af.high &= processor->hl.low;
            break;
        case 0xA6: /* (HL) */
            processor->delay += 4;
            processor->af.high &= dmgl_read(processor->hl.word);
            break;
        case 0xA7: /* A */
            break;
        case 0xE6: /* # */
            processor->delay += 4;
            processor->af.high &= dmgl_read(processor->pc.word++);
            break;
        default:
            break;
    }
    processor->af.carry = false;
    processor->af.half_carry = true;
    processor->af.negative = false;
    processor->af.zero = !processor->af.high;
}

static void dmgl_processor_execute_bit(dmgl_processor_t *const processor)
{
    processor->delay = 8;
    switch (processor->instruction.opcode)
    {
        case 0x40: case 0x48: case 0x50: case 0x58: /* B */
        case 0x60: case 0x68: case 0x70: case 0x78:
            processor->af.zero = !(processor->bc.high & (1 << ((processor->instruction.opcode - 0x40) / 8)));
            break;
        case 0x41: case 0x49: case 0x51: case 0x59: /* C */
        case 0x61: case 0x69: case 0x71: case 0x79:
            processor->af.zero = !(processor->bc.low & (1 << ((processor->instruction.opcode - 0x41) / 8)));
            break;
        case 0x42: case 0x4A: case 0x52: case 0x5A: /* D */
        case 0x62: case 0x6A: case 0x72: case 0x7A:
            processor->af.zero = !(processor->de.high & (1 << ((processor->instruction.opcode - 0x42) / 8)));
            break;
        case 0x43: case 0x4B: case 0x53: case 0x5B: /* E */
        case 0x63: case 0x6B: case 0x73: case 0x7B:
            processor->af.zero = !(processor->de.low & (1 << ((processor->instruction.opcode - 0x43) / 8)));
            break;
        case 0x44: case 0x4C: case 0x54: case 0x5C: /* H */
        case 0x64: case 0x6C: case 0x74: case 0x7C:
            processor->af.zero = !(processor->hl.high & (1 << ((processor->instruction.opcode - 0x44) / 8)));
            break;
        case 0x45: case 0x4D: case 0x55: case 0x5D: /* L */
        case 0x65: case 0x6D: case 0x75: case 0x7D:
            processor->af.zero = !(processor->hl.low & (1 << ((processor->instruction.opcode - 0x45) / 8)));
            break;
        case 0x46: case 0x4E: case 0x56: case 0x5E: /* (HL) */
        case 0x66: case 0x6E: case 0x76: case 0x7E:
            processor->delay += 4;
            processor->af.zero = !(dmgl_read(processor->hl.word) & (1 << ((processor->instruction.opcode - 0x46) / 8)));
            break;
        case 0x47: case 0x4F: case 0x57: case 0x5F: /* A */
        case 0x67: case 0x6F: case 0x77: case 0x7F:
            processor->af.zero = !(processor->af.high & (1 << ((processor->instruction.opcode - 0x47) / 8)));
            break;
        default:
            break;
    }
    processor->af.half_carry = true;
    processor->af.negative = false;
}

static void dmgl_processor_execute_call(dmgl_processor_t *const processor)
{
    bool taken = false;
    dmgl_register_t operand = {};
    processor->delay = 12;
    operand.low = dmgl_read(processor->pc.word++);
    operand.high = dmgl_read(processor->pc.word++);
    switch (processor->instruction.opcode)
    {
        case 0xC4: /* NZ */
            taken = !processor->af.zero;
            break;
        case 0xCC: /* Z */
            taken = processor->af.zero;
            break;
        case 0xCD: /* # */
            taken = true;
            break;
        case 0xD4: /* NC */
            taken = !processor->af.carry;
            break;
        case 0xDC: /* C */
            taken = processor->af.carry;
            break;
        default:
            break;
    }
    if (taken)
    {
        processor->delay += 12;
        dmgl_write(--processor->sp.word, processor->pc.high);
        dmgl_write(--processor->sp.word, processor->pc.low);
        processor->pc.word = operand.word;
    }
}

static void dmgl_processor_execute_ccf(dmgl_processor_t *const processor)
{
    processor->delay = 4;
    processor->af.carry = !processor->af.carry;
    processor->af.half_carry = false;
    processor->af.negative = false;
}

static void dmgl_processor_execute_cp(dmgl_processor_t *const processor)
{
    uint8_t operand = 0;
    processor->delay = 4;
    switch (processor->instruction.opcode)
    {
        case 0xB8: /* B */
            operand = processor->bc.high;
            break;
        case 0xB9: /* C */
            operand = processor->bc.low;
            break;
        case 0xBA: /* D */
            operand = processor->de.high;
            break;
        case 0xBB: /* E */
            operand = processor->de.low;
            break;
        case 0xBC: /* H */
            operand = processor->hl.high;
            break;
        case 0xBD: /* L */
            operand = processor->hl.low;
            break;
        case 0xBE: /* (HL) */
            processor->delay += 4;
            operand = dmgl_read(processor->hl.word);
            break;
        case 0xBF: /* A */
            operand = processor->af.high;
            break;
        case 0xFE: /* # */
            processor->delay += 4;
            operand = dmgl_read(processor->pc.word++);
            break;
        default:
            break;
    }
    processor->af.carry = (processor->af.high < operand);
    processor->af.half_carry = ((processor->af.high & 0x0F) < ((processor->af.high - operand) & 0x0F));
    processor->af.negative = true;
    processor->af.zero = (processor->af.high == operand);
}

static void dmgl_processor_execute_cpl(dmgl_processor_t *const processor)
{
    processor->delay = 4;
    processor->af.high = ~processor->af.high;
    processor->af.half_carry = true;
    processor->af.negative = true;
}

static void dmgl_processor_execute_daa(dmgl_processor_t *const processor)
{
    processor->delay = 4;
    if (!processor->af.negative)
    {
        if (processor->af.carry || (processor->af.high > 0x99))
        {
            processor->af.high += 0x60;
            processor->af.carry = true;
        }
        if (processor->af.half_carry || ((processor->af.high & 0x0F) > 0x09))
        {
            processor->af.high += 0x06;
        }
    }
    else
    {
        if (processor->af.carry)
        {
            processor->af.high -= 0x60;
        }
        if (processor->af.half_carry)
        {
            processor->af.high -= 0x06;
        }
    }
    processor->af.half_carry = false;
    processor->af.zero = !processor->af.high;
}

static void dmgl_processor_execute_dec(dmgl_processor_t *const processor)
{
    uint8_t operand = 0;
    processor->delay = 4;
    switch (processor->instruction.opcode)
    {
        case 0x05: /* B */
            operand = --processor->bc.high;
            break;
        case 0x0D: /* C */
            operand = --processor->bc.low;
            break;
        case 0x15: /* D */
            operand = --processor->de.high;
            break;
        case 0x1D: /* E */
            operand = --processor->de.low;
            break;
        case 0x25: /* H */
            operand = --processor->hl.high;
            break;
        case 0x2D: /* L */
            operand = --processor->hl.low;
            break;
        case 0x35: /* (HL) */
            processor->delay += 8;
            operand = dmgl_read(processor->hl.word) - 1;
            dmgl_write(processor->hl.word, operand);
            break;
        case 0x3D: /* A */
            operand = --processor->af.high;
            break;
        default:
            break;
    }
    processor->af.half_carry = ((operand & 0x0F) == 0x0F);
    processor->af.negative = true;
    processor->af.zero = !operand;
}

static void dmgl_processor_execute_dec_word(dmgl_processor_t *const processor)
{
    processor->delay = 8;
    switch (processor->instruction.opcode)
    {
        case 0x0B: /* BC */
            --processor->bc.word;
            break;
        case 0x1B: /* DE */
            --processor->de.word;
            break;
        case 0x2B: /* HL */
            --processor->hl.word;
            break;
        case 0x3B: /* SP */
            --processor->sp.word;
            break;
        default:
            break;
    }
}

static void dmgl_processor_execute_di(dmgl_processor_t *const processor)
{
    processor->delay = 4;
    processor->interrupt.delay = 0;
    processor->interrupt.enabled = false;
}

static void dmgl_processor_execute_ei(dmgl_processor_t *const processor)
{
    processor->delay = 4;
    if (!processor->interrupt.delay)
    {
        processor->interrupt.delay = 2;
    }
}

static void dmgl_processor_execute_halt(dmgl_processor_t *const processor)
{
    processor->delay = 4;
    processor->halt_bug = (!processor->interrupt.enabled && (processor->interrupt.enable & processor->interrupt.flag & 0x1F));
    processor->halted = true;
}

static void dmgl_processor_execute_inc(dmgl_processor_t *const processor)
{
    uint8_t operand = 0;
    processor->delay = 4;
    switch (processor->instruction.opcode)
    {
        case 0x04: /* B */
            operand = ++processor->bc.high;
            break;
        case 0x0C: /* C */
            operand = ++processor->bc.low;
            break;
        case 0x14: /* D */
            operand = ++processor->de.high;
            break;
        case 0x1C: /* E */
            operand = ++processor->de.low;
            break;
        case 0x24: /* H */
            operand = ++processor->hl.high;
            break;
        case 0x2C: /* L */
            operand = ++processor->hl.low;
            break;
        case 0x34: /* (HL) */
            processor->delay += 8;
            operand = dmgl_read(processor->hl.word) + 1;
            dmgl_write(processor->hl.word, operand);
            break;
        case 0x3C: /* A */
            operand = ++processor->af.high;
            break;
        default:
            break;
    }
    processor->af.half_carry = !(operand & 0x0F);
    processor->af.negative = false;
    processor->af.zero = !operand;
}

static void dmgl_processor_execute_inc_word(dmgl_processor_t *const processor)
{
    processor->delay = 8;
    switch (processor->instruction.opcode)
    {
        case 0x03: /* BC */
            ++processor->bc.word;
            break;
        case 0x13: /* DE */
            ++processor->de.word;
            break;
        case 0x23: /* HL */
            ++processor->hl.word;
            break;
        case 0x33: /* SP */
            ++processor->sp.word;
            break;
        default:
            break;
    }
}

static void dmgl_processor_execute_jp(dmgl_processor_t *const processor)
{
    bool taken = false;
    dmgl_register_t operand = {};
    processor->delay = 12;
    operand.low = dmgl_read(processor->pc.word++);
    operand.high = dmgl_read(processor->pc.word++);
    switch (processor->instruction.opcode)
    {
        case 0xC2: /* NZ */
            taken = !processor->af.zero;
            break;
        case 0xC3: /* # */
            taken = true;
            break;
        case 0xCA: /* Z */
            taken = processor->af.zero;
            break;
        case 0xD2: /* NC */
            taken = !processor->af.carry;
            break;
        case 0xDA: /* C */
            taken = processor->af.carry;
            break;
        default:
            break;
    }
    if (taken)
    {
        processor->delay += 4;
        processor->pc.word = operand.word;
    }
}

static void dmgl_processor_execute_jp_hl(dmgl_processor_t *const processor)
{
    processor->delay = 4;
    processor->pc.word = processor->hl.word;
}

static void dmgl_processor_execute_jr(dmgl_processor_t *const processor)
{
    bool taken = false;
    int8_t operand = dmgl_read(processor->pc.word++);
    processor->delay = 8;
    switch (processor->instruction.opcode)
    {
        case 0x18: /* # */
            taken = true;
            break;
        case 0x20: /* NZ */
            taken = !processor->af.zero;
            break;
        case 0x28: /* Z */
            taken = processor->af.zero;
            break;
        case 0x30: /* NC */
            taken = !processor->af.carry;
            break;
        case 0x38: /* C */
            taken = processor->af.carry;
            break;
        default:
            break;
    }
    if (taken)
    {
        processor->delay += 4;
        processor->pc.word += operand;
    }
}

static void dmgl_processor_execute_ld(dmgl_processor_t *const processor)
{
    dmgl_register_t operand = {};
    processor->delay = 4;
    switch (processor->instruction.opcode)
    {
        case 0x01: /* BC,## */
            processor->delay += 8;
            processor->bc.low = dmgl_read(processor->pc.word++);
            processor->bc.high = dmgl_read(processor->pc.word++);
            break;
        case 0x02: /* (BC),A */
            processor->delay += 4;
            dmgl_write(processor->bc.word, processor->af.high);
            break;
        case 0x06: /* B,# */
            processor->delay += 4;
            processor->bc.high = dmgl_read(processor->pc.word++);
            break;
        case 0x08: /* (##),SP */
            processor->delay += 16;
            operand.low = dmgl_read(processor->pc.word++);
            operand.high = dmgl_read(processor->pc.word++);
            dmgl_write(operand.word, processor->sp.low);
            dmgl_write(operand.word + 1, processor->sp.high);
            break;
        case 0x0A: /* A,(BC) */
            processor->delay += 4;
            processor->af.high = dmgl_read(processor->bc.word);
            break;
        case 0x0E: /* C,# */
            processor->delay += 4;
            processor->bc.low = dmgl_read(processor->pc.word++);
            break;
        case 0x11: /* DE,## */
            processor->delay += 8;
            processor->de.low = dmgl_read(processor->pc.word++);
            processor->de.high = dmgl_read(processor->pc.word++);
            break;
        case 0x12: /* (DE),A */
            processor->delay += 4;
            dmgl_write(processor->de.word, processor->af.high);
            break;
        case 0x16: /* D,# */
            processor->delay += 4;
            processor->de.high = dmgl_read(processor->pc.word++);
            break;
        case 0x1A: /* A,(DE) */
            processor->delay += 4;
            processor->af.high = dmgl_read(processor->de.word);
            break;
        case 0x1E: /* E,# */
            processor->delay += 4;
            processor->de.low = dmgl_read(processor->pc.word++);
            break;
        case 0x21: /* HL,## */
            processor->delay += 8;
            processor->hl.low = dmgl_read(processor->pc.word++);
            processor->hl.high = dmgl_read(processor->pc.word++);
            break;
        case 0x22: /* (HL+),A */
            processor->delay += 4;
            dmgl_write(processor->hl.word++, processor->af.high);
            break;
        case 0x26: /* H,# */
            processor->delay += 4;
            processor->hl.high = dmgl_read(processor->pc.word++);
            break;
        case 0x2A: /* A,(HL+) */
            processor->delay += 4;
            processor->af.high = dmgl_read(processor->hl.word++);
            break;
        case 0x2E: /* L,# */
            processor->delay += 4;
            processor->hl.low = dmgl_read(processor->pc.word++);
            break;
        case 0x31: /* SP,## */
            processor->delay += 8;
            processor->sp.low = dmgl_read(processor->pc.word++);
            processor->sp.high = dmgl_read(processor->pc.word++);
            break;
        case 0x32: /* (HL-),A */
            processor->delay += 4;
            dmgl_write(processor->hl.word--, processor->af.high);
            break;
        case 0x36: /* (HL),# */
            processor->delay += 8;
            dmgl_write(processor->hl.word, dmgl_read(processor->pc.word++));
            break;
        case 0x3A: /* A,(HL-) */
            processor->delay += 4;
            processor->af.high = dmgl_read(processor->hl.word--);
            break;
        case 0x3E: /* A,# */
            processor->delay += 4;
            processor->af.high = dmgl_read(processor->pc.word++);
            break;
        case 0x40: /* B,B */
            break;
        case 0x41: /* B,C */
            processor->bc.high = processor->bc.low;
            break;
        case 0x42: /* B,D */
            processor->bc.high = processor->de.high;
            break;
        case 0x43: /* B,E */
            processor->bc.high = processor->de.low;
            break;
        case 0x44: /* B,H */
            processor->bc.high = processor->hl.high;
            break;
        case 0x45: /* B,L */
            processor->bc.high = processor->hl.low;
            break;
        case 0x46: /* B,(HL) */
            processor->delay += 4;
            processor->bc.high = dmgl_read(processor->hl.word);
            break;
        case 0x47: /* B,A */
            processor->bc.high = processor->af.high;
            break;
        case 0x48: /* C,B */
            processor->bc.low = processor->bc.high;
            break;
        case 0x49: /* C,C */
            break;
        case 0x4A: /* C,D */
            processor->bc.low = processor->de.high;
            break;
        case 0x4B: /* C,E */
            processor->bc.low = processor->de.low;
            break;
        case 0x4C: /* C,H */
            processor->bc.low = processor->hl.high;
            break;
        case 0x4D: /* C,L */
            processor->bc.low = processor->hl.low;
            break;
        case 0x4E: /* C,(HL) */
            processor->delay += 4;
            processor->bc.low = dmgl_read(processor->hl.word);
            break;
        case 0x4F: /* C,A */
            processor->bc.low = processor->af.high;
            break;
        case 0x50: /* D,B */
            processor->de.high = processor->bc.high;
            break;
        case 0x51: /* D,C */
            processor->de.high = processor->bc.low;
            break;
        case 0x52: /* D,D */
            break;
        case 0x53: /* D,E */
            processor->de.high = processor->de.low;
            break;
        case 0x54: /* D,H */
            processor->de.high = processor->hl.high;
            break;
        case 0x55: /* D,L */
            processor->de.high = processor->hl.low;
            break;
        case 0x56: /* D,(HL) */
            processor->delay += 4;
            processor->de.high = dmgl_read(processor->hl.word);
            break;
        case 0x57: /* D,A */
            processor->de.high = processor->af.high;
            break;
        case 0x58: /* E,B */
            processor->de.low = processor->bc.high;
            break;
        case 0x59: /* E,C */
            processor->de.low = processor->bc.low;
            break;
        case 0x5A: /* E,D */
            processor->de.low = processor->de.high;
            break;
        case 0x5B: /* E,E */
            break;
        case 0x5C: /* E,H */
            processor->de.low = processor->hl.high;
            break;
        case 0x5D: /* E,L */
            processor->de.low = processor->hl.low;
            break;
        case 0x5E: /* E,(HL) */
            processor->delay += 4;
            processor->de.low = dmgl_read(processor->hl.word);
            break;
        case 0x5F: /* E,A */
            processor->de.low = processor->af.high;
            break;
        case 0x60: /* H,B */
            processor->hl.high = processor->bc.high;
            break;
        case 0x61: /* H,C */
            processor->hl.high = processor->bc.low;
            break;
        case 0x62: /* H,D */
            processor->hl.high = processor->de.high;
            break;
        case 0x63: /* H,E */
            processor->hl.high = processor->de.low;
            break;
        case 0x64: /* H,H */
            break;
        case 0x65: /* H,L */
            processor->hl.high = processor->hl.low;
            break;
        case 0x66: /* H,(HL) */
            processor->delay += 4;
            processor->hl.high = dmgl_read(processor->hl.word);
            break;
        case 0x67: /* H,A */
            processor->hl.high = processor->af.high;
            break;
        case 0x68: /* L,B */
            processor->hl.low = processor->bc.high;
            break;
        case 0x69: /* L,C */
            processor->hl.low = processor->bc.low;
            break;
        case 0x6A: /* L,D */
            processor->hl.low = processor->de.high;
            break;
        case 0x6B: /* L,E */
            processor->hl.low = processor->de.low;
            break;
        case 0x6C: /* L,H */
            processor->hl.low = processor->hl.high;
            break;
        case 0x6D: /* L,L */
            break;
        case 0x6E: /* L,(HL) */
            processor->delay += 4;
            processor->hl.low = dmgl_read(processor->hl.word);
            break;
        case 0x6F: /* L,A */
            processor->hl.low = processor->af.high;
            break;
        case 0x70: /* (HL),B */
            processor->delay += 4;
            dmgl_write(processor->hl.word, processor->bc.high);
            break;
        case 0x71: /* (HL),C */
            processor->delay += 4;
            dmgl_write(processor->hl.word, processor->bc.low);
            break;
        case 0x72: /* (HL),D */
            processor->delay += 4;
            dmgl_write(processor->hl.word, processor->de.high);
            break;
        case 0x73: /* (HL),E */
            processor->delay += 4;
            dmgl_write(processor->hl.word, processor->de.low);
            break;
        case 0x74: /* (HL),H */
            processor->delay += 4;
            dmgl_write(processor->hl.word, processor->hl.high);
            break;
        case 0x75: /* (HL),L */
            processor->delay += 4;
            dmgl_write(processor->hl.word, processor->hl.low);
            break;
        case 0x77: /* (HL),A */
            processor->delay += 4;
            dmgl_write(processor->hl.word, processor->af.high);
            break;
        case 0x78: /* A,B */
            processor->af.high = processor->bc.high;
            break;
        case 0x79: /* A,C */
            processor->af.high = processor->bc.low;
            break;
        case 0x7A: /* A,D */
            processor->af.high = processor->de.high;
            break;
        case 0x7B: /* A,E */
            processor->af.high = processor->de.low;
            break;
        case 0x7C: /* A,H */
            processor->af.high = processor->hl.high;
            break;
        case 0x7D: /* A,L */
            processor->af.high = processor->hl.low;
            break;
        case 0x7E: /* A,(HL) */
            processor->delay += 4;
            processor->af.high = dmgl_read(processor->hl.word);
            break;
        case 0x7F: /* A,A */
            break;
        case 0xE0: /* (FF00+#),A */
            processor->delay += 8;
            dmgl_write(dmgl_read(processor->pc.word++) + 0xFF00, processor->af.high);
            break;
        case 0xE2: /* (FF00+C),A */
            processor->delay += 4;
            dmgl_write(processor->bc.low + 0xFF00, processor->af.high);
            break;
        case 0xEA: /* (##),A */
            processor->delay += 12;
            operand.low = dmgl_read(processor->pc.word++);
            operand.high = dmgl_read(processor->pc.word++);
            dmgl_write(operand.word, processor->af.high);
            break;
        case 0xF0: /* A,(FF00+#) */
            processor->delay += 8;
            processor->af.high = dmgl_read(dmgl_read(processor->pc.word++) + 0xFF00);
            break;
        case 0xF2: /* A,(FF00+C) */
            processor->delay += 4;
            processor->af.high = dmgl_read(processor->bc.low + 0xFF00);
            break;
        case 0xF9: /* SP,HL */
            processor->delay += 4;
            processor->sp.word = processor->hl.word;
            break;
        case 0xFA: /* A,(##) */
            processor->delay += 12;
            operand.low = dmgl_read(processor->pc.word++);
            operand.high = dmgl_read(processor->pc.word++);
            processor->af.high = dmgl_read(operand.word);
            break;
        default:
            break;
    }
}

static void dmgl_processor_execute_ld_hl(dmgl_processor_t *const processor)
{
    uint32_t carry = 0, sum = 0;
    int8_t operand = dmgl_read(processor->pc.word++);
    processor->delay = 12;
    sum = processor->sp.word + operand;
    carry = processor->sp.word ^ operand ^ sum;
    processor->af.carry = ((carry & 0x100) == 0x100);
    processor->af.half_carry = ((carry & 0x10) == 0x10);
    processor->af.negative = false;
    processor->af.zero = false;
    processor->hl.word = sum;
}

static void dmgl_processor_execute_nop(dmgl_processor_t *const processor)
{
    processor->delay = 4;
}

static void dmgl_processor_execute_or(dmgl_processor_t *const processor)
{
    processor->delay = 4;
    switch (processor->instruction.opcode)
    {
        case 0xB0: /* B */
            processor->af.high |= processor->bc.high;
            break;
        case 0xB1: /* C */
            processor->af.high |= processor->bc.low;
            break;
        case 0xB2: /* D */
            processor->af.high |= processor->de.high;
            break;
        case 0xB3: /* E */
            processor->af.high |= processor->de.low;
            break;
        case 0xB4: /* H */
            processor->af.high |= processor->hl.high;
            break;
        case 0xB5: /* L */
            processor->af.high |= processor->hl.low;
            break;
        case 0xB6: /* (HL) */
            processor->delay += 4;
            processor->af.high |= dmgl_read(processor->hl.word);
            break;
        case 0xB7: /* A */
            break;
        case 0xF6: /* # */
            processor->delay += 4;
            processor->af.high |= dmgl_read(processor->pc.word++);
            break;
        default:
            break;
    }
    processor->af.carry = false;
    processor->af.half_carry = false;
    processor->af.negative = false;
    processor->af.zero = !processor->af.high;
}

static void dmgl_processor_execute_pop(dmgl_processor_t *const processor)
{
    processor->delay = 12;
    switch (processor->instruction.opcode)
    {
        case 0xC1: /* BC */
            processor->bc.low = dmgl_read(processor->sp.word++);
            processor->bc.high = dmgl_read(processor->sp.word++);
            break;
        case 0xD1: /* DE */
            processor->de.low = dmgl_read(processor->sp.word++);
            processor->de.high = dmgl_read(processor->sp.word++);
            break;
        case 0xE1: /* HL */
            processor->hl.low = dmgl_read(processor->sp.word++);
            processor->hl.high = dmgl_read(processor->sp.word++);
            break;
        case 0xF1: /* AF */
            processor->af.low = dmgl_read(processor->sp.word++) & 0xF0;
            processor->af.high = dmgl_read(processor->sp.word++);
            break;
        default:
            break;
    }
}

static void dmgl_processor_execute_push(dmgl_processor_t *const processor)
{
    processor->delay = 16;
    switch (processor->instruction.opcode)
    {
        case 0xC5: /* BC */
            dmgl_write(--processor->sp.word, processor->bc.high);
            dmgl_write(--processor->sp.word, processor->bc.low);
            break;
        case 0xD5: /* DE */
            dmgl_write(--processor->sp.word, processor->de.high);
            dmgl_write(--processor->sp.word, processor->de.low);
            break;
        case 0xE5: /* HL */
            dmgl_write(--processor->sp.word, processor->hl.high);
            dmgl_write(--processor->sp.word, processor->hl.low);
            break;
        case 0xF5: /* AF */
            dmgl_write(--processor->sp.word, processor->af.high);
            dmgl_write(--processor->sp.word, processor->af.low);
            break;
        default:
            break;
    }
}

static void dmgl_processor_execute_res(dmgl_processor_t *const processor)
{
    uint8_t operand = 0;
    processor->delay = 8;
    switch (processor->instruction.opcode)
    {
        case 0x80: case 0x88: case 0x90: case 0x98: /* B */
        case 0xA0: case 0xA8: case 0xB0: case 0xB8:
            processor->bc.high &= ~(1 << ((processor->instruction.opcode - 0x80) / 8));
            break;
        case 0x81: case 0x89: case 0x91: case 0x99: /* C */
        case 0xA1: case 0xA9: case 0xB1: case 0xB9:
            processor->bc.low &= ~(1 << ((processor->instruction.opcode - 0x81) / 8));
            break;
        case 0x82: case 0x8A: case 0x92: case 0x9A: /* D */
        case 0xA2: case 0xAA: case 0xB2: case 0xBA:
            processor->de.high &= ~(1 << ((processor->instruction.opcode - 0x82) / 8));
            break;
        case 0x83: case 0x8B: case 0x93: case 0x9B: /* E */
        case 0xA3: case 0xAB: case 0xB3: case 0xBB:
            processor->de.low &= ~(1 << ((processor->instruction.opcode - 0x83) / 8));
            break;
        case 0x84: case 0x8C: case 0x94: case 0x9C: /* H */
        case 0xA4: case 0xAC: case 0xB4: case 0xBC:
            processor->hl.high &= ~(1 << ((processor->instruction.opcode - 0x84) / 8));
            break;
        case 0x85: case 0x8D: case 0x95: case 0x9D: /* L */
        case 0xA5: case 0xAD: case 0xB5: case 0xBD:
            processor->hl.low &= ~(1 << ((processor->instruction.opcode - 0x85) / 8));
            break;
        case 0x86: case 0x8E: case 0x96: case 0x9E: /* (HL) */
        case 0xA6: case 0xAE: case 0xB6: case 0xBE:
            processor->delay += 8;
            operand = dmgl_read(processor->hl.word);
            operand &= ~(1 << ((processor->instruction.opcode - 0x86) / 8));
            dmgl_write(processor->hl.word, operand);
            break;
        case 0x87: case 0x8F: case 0x97: case 0x9F: /* A */
        case 0xA7: case 0xAF: case 0xB7: case 0xBF:
            processor->af.high &= ~(1 << ((processor->instruction.opcode - 0x87) / 8));
            break;
        default:
            break;
    }
}

static void dmgl_processor_execute_ret(dmgl_processor_t *const processor)
{
    bool taken = false;
    processor->delay = 8;
    switch (processor->instruction.opcode)
    {
        case 0xC0: /* NZ */
            taken = !processor->af.zero;
            break;
        case 0xC8: /* Z */
            taken = processor->af.zero;
            break;
        case 0xC9: /* # */
            taken = true;
            break;
        case 0xD0: /* NC */
            taken = !processor->af.carry;
            break;
        case 0xD8: /* C */
            taken = processor->af.carry;
            break;
        default:
            break;
    }
    if (taken)
    {
        processor->delay += (processor->instruction.opcode == 0xC9) ? 8 : 12;
        processor->pc.low = dmgl_read(processor->sp.word++);
        processor->pc.high = dmgl_read(processor->sp.word++);
    }
}

static void dmgl_processor_execute_reti(dmgl_processor_t *const processor)
{
    processor->delay = 16;
    processor->pc.low = dmgl_read(processor->sp.word++);
    processor->pc.high = dmgl_read(processor->sp.word++);
    processor->interrupt.delay = 0;
    processor->interrupt.enabled = true;
}

static void dmgl_processor_execute_rl(dmgl_processor_t *const processor)
{
    uint8_t carry = processor->af.carry, operand = 0;
    processor->delay = 8;
    switch (processor->instruction.opcode)
    {
        case 0x10: /* B */
            processor->af.carry = ((processor->bc.high & 0x80) == 0x80);
            processor->bc.high = (processor->bc.high << 1) | carry;
            processor->af.zero = !processor->bc.high;
            break;
        case 0x11: /* C */
            processor->af.carry = ((processor->bc.low & 0x80) == 0x80);
            processor->bc.low = (processor->bc.low << 1) | carry;
            processor->af.zero = !processor->bc.low;
            break;
        case 0x12: /* D */
            processor->af.carry = ((processor->de.high & 0x80) == 0x80);
            processor->de.high = (processor->de.high << 1) | carry;
            processor->af.zero = !processor->de.high;
            break;
        case 0x13: /* E */
            processor->af.carry = ((processor->de.low & 0x80) == 0x80);
            processor->de.low = (processor->de.low << 1) | carry;
            processor->af.zero = !processor->de.low;
            break;
        case 0x14: /* H */
            processor->af.carry = ((processor->hl.high & 0x80) == 0x80);
            processor->hl.high = (processor->hl.high << 1) | carry;
            processor->af.zero = !processor->hl.high;
            break;
        case 0x15: /* L */
            processor->af.carry = ((processor->hl.low & 0x80) == 0x80);
            processor->hl.low = (processor->hl.low << 1) | carry;
            processor->af.zero = !processor->hl.low;
            break;
        case 0x16: /* (HL) */
            processor->delay += 8;
            operand = dmgl_read(processor->hl.word);
            processor->af.carry = ((operand & 0x80) == 0x80);
            operand = (operand << 1) | carry;
            processor->af.zero = !operand;
            dmgl_write(processor->hl.word, operand);
            break;
        case 0x17: /* A */
            processor->af.carry = ((processor->af.high & 0x80) == 0x80);
            processor->af.high = (processor->af.high << 1) | carry;
            processor->af.zero = !processor->af.high;
            break;
        default:
            break;
    }
    processor->af.half_carry = false;
    processor->af.negative = false;
}

static void dmgl_processor_execute_rla(dmgl_processor_t *const processor)
{
    uint8_t carry = processor->af.carry;
    processor->delay = 4;
    processor->af.carry = ((processor->af.high & 0x80) == 0x80);
    processor->af.high = (processor->af.high << 1) | carry;
    processor->af.half_carry = false;
    processor->af.negative = false;
    processor->af.zero = false;
}

static void dmgl_processor_execute_rlc(dmgl_processor_t *const processor)
{
    uint8_t operand = 0;
    processor->delay = 8;
    switch (processor->instruction.opcode)
    {
        case 0x00: /* B */
            processor->af.carry = ((processor->bc.high & 0x80) == 0x80);
            processor->bc.high = (processor->bc.high << 1) | processor->af.carry;
            processor->af.zero = !processor->bc.high;
            break;
        case 0x01: /* C */
            processor->af.carry = ((processor->bc.low & 0x80) == 0x80);
            processor->bc.low = (processor->bc.low << 1) | processor->af.carry;
            processor->af.zero = !processor->bc.low;
            break;
        case 0x02: /* D */
            processor->af.carry = ((processor->de.high & 0x80) == 0x80);
            processor->de.high = (processor->de.high << 1) | processor->af.carry;
            processor->af.zero = !processor->de.high;
            break;
        case 0x03: /* E */
            processor->af.carry = ((processor->de.low & 0x80) == 0x80);
            processor->de.low = (processor->de.low << 1) | processor->af.carry;
            processor->af.zero = !processor->de.low;
            break;
        case 0x04: /* H */
            processor->af.carry = ((processor->hl.high & 0x80) == 0x80);
            processor->hl.high = (processor->hl.high << 1) | processor->af.carry;
            processor->af.zero = !processor->hl.high;
            break;
        case 0x05: /* L */
            processor->af.carry = ((processor->hl.low & 0x80) == 0x80);
            processor->hl.low = (processor->hl.low << 1) | processor->af.carry;
            processor->af.zero = !processor->hl.low;
            break;
        case 0x06: /* (HL) */
            processor->delay += 8;
            operand = dmgl_read(processor->hl.word);
            processor->af.carry = ((operand & 0x80) == 0x80);
            operand = (operand << 1) | processor->af.carry;
            processor->af.zero = !operand;
            dmgl_write(processor->hl.word, operand);
            break;
        case 0x07: /* A */
            processor->af.carry = ((processor->af.high & 0x80) == 0x80);
            processor->af.high = (processor->af.high << 1) | processor->af.carry;
            processor->af.zero = !processor->af.high;
            break;
        default:
            break;
    }
    processor->af.half_carry = false;
    processor->af.negative = false;
}

static void dmgl_processor_execute_rlca(dmgl_processor_t *const processor)
{
    processor->delay = 4;
    processor->af.carry = ((processor->af.high & 0x80) == 0x80);
    processor->af.high = (processor->af.high << 1) | processor->af.carry;
    processor->af.half_carry = false;
    processor->af.negative = false;
    processor->af.zero = false;
}

static void dmgl_processor_execute_rr(dmgl_processor_t *const processor)
{
    uint8_t carry = processor->af.carry, operand = 0;
    processor->delay = 8;
    switch (processor->instruction.opcode)
    {
        case 0x18: /* B */
            processor->af.carry = ((processor->bc.high & 1) == 1);
            processor->bc.high = (processor->bc.high >> 1) | (carry ? 0x80 : 0);
            processor->af.zero = !processor->bc.high;
            break;
        case 0x19: /* C */
            processor->af.carry = ((processor->bc.low & 1) == 1);
            processor->bc.low = (processor->bc.low >> 1) | (carry ? 0x80 : 0);
            processor->af.zero = !processor->bc.low;
            break;
        case 0x1A: /* D */
            processor->af.carry = ((processor->de.high & 1) == 1);
            processor->de.high = (processor->de.high >> 1) | (carry ? 0x80 : 0);
            processor->af.zero = !processor->de.high;
            break;
        case 0x1B: /* E */
            processor->af.carry = ((processor->de.low & 1) == 1);
            processor->de.low = (processor->de.low >> 1) | (carry ? 0x80 : 0);
            processor->af.zero = !processor->de.low;
            break;
        case 0x1C: /* H */
            processor->af.carry = ((processor->hl.high & 1) == 1);
            processor->hl.high = (processor->hl.high >> 1) | (carry ? 0x80 : 0);
            processor->af.zero = !processor->hl.high;
            break;
        case 0x1D: /* L */
            processor->af.carry = ((processor->hl.low & 1) == 1);
            processor->hl.low = (processor->hl.low >> 1) | (carry ? 0x80 : 0);
            processor->af.zero = !processor->hl.low;
            break;
        case 0x1E: /* (HL) */
            processor->delay += 8;
            operand = dmgl_read(processor->hl.word);
            processor->af.carry = ((operand & 1) == 1);
            operand = (operand >> 1) | (carry ? 0x80 : 0);
            processor->af.zero = !operand;
            dmgl_write(processor->hl.word, operand);
            break;
        case 0x1F: /* A */
            processor->af.carry = ((processor->af.high & 1) == 1);
            processor->af.high = (processor->af.high >> 1) | (carry ? 0x80 : 0);
            processor->af.zero = !processor->af.high;
            break;
        default:
            break;
    }
    processor->af.half_carry = false;
    processor->af.negative = false;
}

static void dmgl_processor_execute_rra(dmgl_processor_t *const processor)
{
    uint8_t carry = processor->af.carry;
    processor->delay = 4;
    processor->af.carry = ((processor->af.high & 1) == 1);
    processor->af.high = (processor->af.high >> 1) | (carry ? 0x80 : 0);
    processor->af.half_carry = false;
    processor->af.negative = false;
    processor->af.zero = false;
}

static void dmgl_processor_execute_rrc(dmgl_processor_t *const processor)
{
    uint8_t operand = 0;
    processor->delay = 8;
    switch (processor->instruction.opcode)
    {
        case 0x08: /* B */
            processor->af.carry = ((processor->bc.high & 1) == 1);
            processor->bc.high = (processor->bc.high >> 1) | (processor->af.carry ? 0x80 : 0);
            processor->af.zero = !processor->bc.high;
            break;
        case 0x09: /* C */
            processor->af.carry = ((processor->bc.low & 1) == 1);
            processor->bc.low = (processor->bc.low >> 1) | (processor->af.carry ? 0x80 : 0);
            processor->af.zero = !processor->bc.low;
            break;
        case 0x0A: /* D */
            processor->af.carry = ((processor->de.high & 1) == 1);
            processor->de.high = (processor->de.high >> 1) | (processor->af.carry ? 0x80 : 0);
            processor->af.zero = !processor->de.high;
            break;
        case 0x0B: /* E */
            processor->af.carry = ((processor->de.low & 1) == 1);
            processor->de.low = (processor->de.low >> 1) | (processor->af.carry ? 0x80 : 0);
            processor->af.zero = !processor->de.low;
            break;
        case 0x0C: /* H */
            processor->af.carry = ((processor->hl.high & 1) == 1);
            processor->hl.high = (processor->hl.high >> 1) | (processor->af.carry ? 0x80 : 0);
            processor->af.zero = !processor->hl.high;
            break;
        case 0x0D: /* L */
            processor->af.carry = ((processor->hl.low & 1) == 1);
            processor->hl.low = (processor->hl.low >> 1) | (processor->af.carry ? 0x80 : 0);
            processor->af.zero = !processor->hl.low;
            break;
        case 0x0E: /* (HL) */
            processor->delay += 8;
            operand = dmgl_read(processor->hl.word);
            processor->af.carry = ((operand & 1) == 1);
            operand = (operand >> 1) | (processor->af.carry ? 0x80 : 0);
            processor->af.zero = !operand;
            dmgl_write(processor->hl.word, operand);
            break;
        case 0x0F: /* A */
            processor->af.carry = ((processor->af.high & 1) == 1);
            processor->af.high = (processor->af.high >> 1) | (processor->af.carry ? 0x80 : 0);
            processor->af.zero = !processor->af.high;
            break;
        default:
            break;
    }
    processor->af.half_carry = false;
    processor->af.negative = false;
}

static void dmgl_processor_execute_rrca(dmgl_processor_t *const processor)
{
    processor->delay = 4;
    processor->af.carry = ((processor->af.high & 1) == 1);
    processor->af.high = (processor->af.high >> 1) | (processor->af.carry ? 0x80 : 0);
    processor->af.half_carry = false;
    processor->af.negative = false;
    processor->af.zero = false;
}

static void dmgl_processor_execute_rst(dmgl_processor_t *const processor)
{
    processor->delay = 16;
    dmgl_write(--processor->sp.word, processor->pc.high);
    dmgl_write(--processor->sp.word, processor->pc.low);
    processor->pc.word = processor->instruction.opcode - 0xC7;
}

static void dmgl_processor_execute_sbc(dmgl_processor_t *const processor)
{
    uint8_t operand = 0;
    uint16_t carry = 0, sum = 0;
    processor->delay = 4;
    switch (processor->instruction.opcode)
    {
        case 0x98: /* B */
            operand = processor->bc.high;
            break;
        case 0x99: /* C */
            operand = processor->bc.low;
            break;
        case 0x9A: /* D */
            operand = processor->de.high;
            break;
        case 0x9B: /* E */
            operand = processor->de.low;
            break;
        case 0x9C: /* H */
            operand = processor->hl.high;
            break;
        case 0x9D: /* L */
            operand = processor->hl.low;
            break;
        case 0x9E: /* (HL) */
            processor->delay += 4;
            operand = dmgl_read(processor->hl.word);
            break;
        case 0x9F: /* A */
            operand = processor->af.high;
            break;
        case 0xDE: /* # */
            processor->delay += 4;
            operand = dmgl_read(processor->pc.word++);
            break;
        default:
            break;
    }
    sum = processor->af.high - operand - processor->af.carry;
    carry = processor->af.high ^ operand ^ processor->af.carry ^ sum;
    processor->af.carry = ((carry & 0x100) == 0x100);
    processor->af.half_carry = ((carry & 0x10) == 0x10);
    processor->af.negative = true;
    processor->af.zero = !(sum & 0xFF);
    processor->af.high = sum;
}

static void dmgl_processor_execute_scf(dmgl_processor_t *const processor)
{
    processor->delay = 4;
    processor->af.carry = true;
    processor->af.half_carry = false;
    processor->af.negative = false;
}

static void dmgl_processor_execute_set(dmgl_processor_t *const processor)
{
    uint8_t operand = 0;
    processor->delay = 8;
    switch (processor->instruction.opcode)
    {
        case 0xC0: case 0xC8: case 0xD0: case 0xD8: /* B */
        case 0xE0: case 0xE8: case 0xF0: case 0xF8:
            processor->bc.high |= (1 << ((processor->instruction.opcode - 0xC0) / 8));
            break;
        case 0xC1: case 0xC9: case 0xD1: case 0xD9: /* C */
        case 0xE1: case 0xE9: case 0xF1: case 0xF9:
            processor->bc.low |= (1 << ((processor->instruction.opcode - 0xC1) / 8));
            break;
        case 0xC2: case 0xCA: case 0xD2: case 0xDA: /* D */
        case 0xE2: case 0xEA: case 0xF2: case 0xFA:
            processor->de.high |= (1 << ((processor->instruction.opcode - 0xC2) / 8));
            break;
        case 0xC3: case 0xCB: case 0xD3: case 0xDB: /* E */
        case 0xE3: case 0xEB: case 0xF3: case 0xFB:
            processor->de.low |= (1 << ((processor->instruction.opcode - 0xC3) / 8));
            break;
        case 0xC4: case 0xCC: case 0xD4: case 0xDC: /* H */
        case 0xE4: case 0xEC: case 0xF4: case 0xFC:
            processor->hl.high |= (1 << ((processor->instruction.opcode - 0xC4) / 8));
            break;
        case 0xC5: case 0xCD: case 0xD5: case 0xDD: /* L */
        case 0xE5: case 0xED: case 0xF5: case 0xFD:
            processor->hl.low |= (1 << ((processor->instruction.opcode - 0xC5) / 8));
            break;
        case 0xC6: case 0xCE: case 0xD6: case 0xDE: /* (HL) */
        case 0xE6: case 0xEE: case 0xF6: case 0xFE:
            processor->delay += 8;
            operand = dmgl_read(processor->hl.word);
            operand |= (1 << ((processor->instruction.opcode - 0xC6) / 8));
            dmgl_write(processor->hl.word, operand);
            break;
        case 0xC7: case 0xCF: case 0xD7: case 0xDF: /* A */
        case 0xE7: case 0xEF: case 0xF7: case 0xFF:
            processor->af.high |= (1 << ((processor->instruction.opcode - 0xC7) / 8));
            break;
        default:
            break;
    }
}

static void dmgl_processor_execute_sla(dmgl_processor_t *const processor)
{
    uint8_t operand = 0;
    processor->delay = 8;
    switch (processor->instruction.opcode)
    {
        case 0x20: /* B */
            processor->af.carry = ((processor->bc.high & 0x80) == 0x80);
            processor->bc.high <<= 1;
            processor->af.zero = !processor->bc.high;
            break;
        case 0x21: /* C */
            processor->af.carry = ((processor->bc.low & 0x80) == 0x80);
            processor->bc.low <<= 1;
            processor->af.zero = !processor->bc.low;
            break;
        case 0x22: /* D */
            processor->af.carry = ((processor->de.high & 0x80) == 0x80);
            processor->de.high <<= 1;
            processor->af.zero = !processor->de.high;
            break;
        case 0x23: /* E */
            processor->af.carry = ((processor->de.low & 0x80) == 0x80);
            processor->de.low <<= 1;
            processor->af.zero = !processor->de.low;
            break;
        case 0x24: /* H */
            processor->af.carry = ((processor->hl.high & 0x80) == 0x80);
            processor->hl.high <<= 1;
            processor->af.zero = !processor->hl.high;
            break;
        case 0x25: /* L */
            processor->af.carry = ((processor->hl.low & 0x80) == 0x80);
            processor->hl.low <<= 1;
            processor->af.zero = !processor->hl.low;
            break;
        case 0x26: /* (HL) */
            processor->delay += 8;
            operand = dmgl_read(processor->hl.word);
            processor->af.carry = ((operand & 0x80) == 0x80);
            operand <<= 1;
            processor->af.zero = !operand;
            dmgl_write(processor->hl.word, operand);
            break;
        case 0x27: /* A */
            processor->af.carry = ((processor->af.high & 0x80) == 0x80);
            processor->af.high <<= 1;
            processor->af.zero = !processor->af.high;
            break;
        default:
            break;
    }
    processor->af.half_carry = false;
    processor->af.negative = false;
}

static void dmgl_processor_execute_sra(dmgl_processor_t *const processor)
{
    uint8_t operand = 0;
    processor->delay = 8;
    switch (processor->instruction.opcode)
    {
        case 0x28: /* B */
            processor->af.carry = ((processor->bc.high & 1) == 1);
            processor->bc.high = (processor->bc.high >> 1) | (processor->bc.high & 0x80);
            processor->af.zero = !processor->bc.high;
            break;
        case 0x29: /* C */
            processor->af.carry = ((processor->bc.low & 1) == 1);
            processor->bc.low = (processor->bc.low >> 1) | (processor->bc.low & 0x80);
            processor->af.zero = !processor->bc.low;
            break;
        case 0x2A: /* D */
            processor->af.carry = ((processor->de.high & 1) == 1);
            processor->de.high = (processor->de.high >> 1) | (processor->de.high & 0x80);
            processor->af.zero = !processor->de.high;
            break;
        case 0x2B: /* E */
            processor->af.carry = ((processor->de.low & 1) == 1);
            processor->de.low = (processor->de.low >> 1) | (processor->de.low & 0x80);
            processor->af.zero = !processor->de.low;
            break;
        case 0x2C: /* H */
            processor->af.carry = ((processor->hl.high & 1) == 1);
            processor->hl.high = (processor->hl.high >> 1) | (processor->hl.high & 0x80);
            processor->af.zero = !processor->hl.high;
            break;
        case 0x2D: /* L */
            processor->af.carry = ((processor->hl.low & 1) == 1);
            processor->hl.low = (processor->hl.low >> 1) | (processor->hl.low & 0x80);
            processor->af.zero = !processor->hl.low;
            break;
        case 0x2E: /* (HL) */
            processor->delay += 8;
            operand = dmgl_read(processor->hl.word);
            processor->af.carry = ((operand & 1) == 1);
            operand = (operand >> 1) | (operand & 0x80);
            processor->af.zero = !operand;
            dmgl_write(processor->hl.word, operand);
            break;
        case 0x2F: /* A */
            processor->af.carry = ((processor->af.high & 1) == 1);
            processor->af.high = (processor->af.high >> 1) | (processor->af.high & 0x80);
            processor->af.zero = !processor->af.high;
            break;
        default:
            break;
    }
    processor->af.half_carry = false;
    processor->af.negative = false;
}

static void dmgl_processor_execute_srl(dmgl_processor_t *const processor)
{
    uint8_t operand = 0;
    processor->delay = 8;
    switch (processor->instruction.opcode)
    {
        case 0x38: /* B */
            processor->af.carry = ((processor->bc.high & 1) == 1);
            processor->bc.high >>= 1;
            processor->af.zero = !processor->bc.high;
            break;
        case 0x39: /* C */
            processor->af.carry = ((processor->bc.low & 1) == 1);
            processor->bc.low >>= 1;
            processor->af.zero = !processor->bc.low;
            break;
        case 0x3A: /* D */
            processor->af.carry = ((processor->de.high & 1) == 1);
            processor->de.high >>= 1;
            processor->af.zero = !processor->de.high;
            break;
        case 0x3B: /* E */
            processor->af.carry = ((processor->de.low & 1) == 1);
            processor->de.low >>= 1;
            processor->af.zero = !processor->de.low;
            break;
        case 0x3C: /* H */
            processor->af.carry = ((processor->hl.high & 1) == 1);
            processor->hl.high >>= 1;
            processor->af.zero = !processor->hl.high;
            break;
        case 0x3D: /* L */
            processor->af.carry = ((processor->hl.low & 1) == 1);
            processor->hl.low >>= 1;
            processor->af.zero = !processor->hl.low;
            break;
        case 0x3E: /* (HL) */
            processor->delay += 8;
            operand = dmgl_read(processor->hl.word);
            processor->af.carry = ((operand & 1) == 1);
            operand >>= 1;
            processor->af.zero = !operand;
            dmgl_write(processor->hl.word, operand);
            break;
        case 0x3F: /* A */
            processor->af.carry = ((processor->af.high & 1) == 1);
            processor->af.high >>= 1;
            processor->af.zero = !processor->af.high;
            break;
        default:
            break;
    }
    processor->af.half_carry = false;
    processor->af.negative = false;
}

static void dmgl_processor_execute_stop(dmgl_processor_t *const processor)
{
    processor->delay = 4;
    processor->stopped = true;
    dmgl_read(processor->pc.word++);
}

static void dmgl_processor_execute_sub(dmgl_processor_t *const processor)
{
    uint8_t operand = 0;
    uint16_t carry = 0, sum = 0;
    processor->delay = 4;
    switch (processor->instruction.opcode)
    {
        case 0x90: /* B */
            operand = processor->bc.high;
            break;
        case 0x91: /* C */
            operand = processor->bc.low;
            break;
        case 0x92: /* D */
            operand = processor->de.high;
            break;
        case 0x93: /* E */
            operand = processor->de.low;
            break;
        case 0x94: /* H */
            operand = processor->hl.high;
            break;
        case 0x95: /* L */
            operand = processor->hl.low;
            break;
        case 0x96: /* (HL) */
            processor->delay += 4;
            operand = dmgl_read(processor->hl.word);
            break;
        case 0x97: /* A */
            operand = processor->af.high;
            break;
        case 0xD6: /* # */
            processor->delay += 4;
            operand = dmgl_read(processor->pc.word++);
            break;
        default:
            break;
    }
    sum = processor->af.high - operand;
    carry = processor->af.high ^ operand ^ sum;
    processor->af.carry = ((carry & 0x100) == 0x100);
    processor->af.half_carry = ((carry & 0x10) == 0x10);
    processor->af.negative = true;
    processor->af.zero = !(sum & 0xFF);
    processor->af.high = sum;
}

static void dmgl_processor_execute_swap(dmgl_processor_t *const processor)
{
    uint8_t operand = 0;
    processor->delay = 8;
    switch (processor->instruction.opcode)
    {
        case 0x30: /* B */
            processor->bc.high = (processor->bc.high << 4) | (processor->bc.high >> 4);
            processor->af.zero = !processor->bc.high;
            break;
        case 0x31: /* C */
            processor->bc.low = (processor->bc.low << 4) | (processor->bc.low >> 4);
            processor->af.zero = !processor->bc.low;
            break;
        case 0x32: /* D */
            processor->de.high = (processor->de.high << 4) | (processor->de.high >> 4);
            processor->af.zero = !processor->de.high;
            break;
        case 0x33: /* E */
            processor->de.low = (processor->de.low << 4) | (processor->de.low >> 4);
            processor->af.zero = !processor->de.low;
            break;
        case 0x34: /* H */
            processor->hl.high = (processor->hl.high << 4) | (processor->hl.high >> 4);
            processor->af.zero = !processor->hl.high;
            break;
        case 0x35: /* L */
            processor->hl.low = (processor->hl.low << 4) | (processor->hl.low >> 4);
            processor->af.zero = !processor->hl.low;
            break;
        case 0x36: /* (HL) */
            processor->delay += 8;
            operand = dmgl_read(processor->hl.word);
            operand = (operand << 4) | (operand >> 4);
            processor->af.zero = !operand;
            dmgl_write(processor->hl.word, operand);
            break;
        case 0x37: /* A */
            processor->af.high = (processor->af.high << 4) | (processor->af.high >> 4);
            processor->af.zero = !processor->af.high;
            break;
        default:
            break;
    }
    processor->af.carry = false;
    processor->af.half_carry = false;
    processor->af.negative = false;
}

static void dmgl_processor_execute_xor(dmgl_processor_t *const processor)
{
    processor->delay = 4;
    switch (processor->instruction.opcode)
    {
        case 0xA8: /* B */
            processor->af.high ^= processor->bc.high;
            break;
        case 0xA9: /* C */
            processor->af.high ^= processor->bc.low;
            break;
        case 0xAA: /* D */
            processor->af.high ^= processor->de.high;
            break;
        case 0xAB: /* E */
            processor->af.high ^= processor->de.low;
            break;
        case 0xAC: /* H */
            processor->af.high ^= processor->hl.high;
            break;
        case 0xAD: /* L */
            processor->af.high ^= processor->hl.low;
            break;
        case 0xAE: /* (HL) */
            processor->delay += 4;
            processor->af.high ^= dmgl_read(processor->hl.word);
            break;
        case 0xAF: /* A */
            processor->af.high = 0;
            break;
        case 0xEE: /* # */
            processor->delay += 4;
            processor->af.high ^= dmgl_read(processor->pc.word++);
            break;
        default:
            break;
    }
    processor->af.carry = false;
    processor->af.half_carry = false;
    processor->af.negative = false;
    processor->af.zero = !processor->af.high;
}

static const dmgl_processor_instruction_t INSTRUCTION[] =
{
    /* 00 */
    dmgl_processor_execute_nop, dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_inc_word,
    dmgl_processor_execute_inc, dmgl_processor_execute_dec, dmgl_processor_execute_ld, dmgl_processor_execute_rlca,
    /* 08 */
    dmgl_processor_execute_ld, dmgl_processor_execute_add_hl, dmgl_processor_execute_ld, dmgl_processor_execute_dec_word,
    dmgl_processor_execute_inc, dmgl_processor_execute_dec, dmgl_processor_execute_ld, dmgl_processor_execute_rrca,
    /* 10 */
    dmgl_processor_execute_stop, dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_inc_word,
    dmgl_processor_execute_inc, dmgl_processor_execute_dec, dmgl_processor_execute_ld, dmgl_processor_execute_rla,
    /* 18 */
    dmgl_processor_execute_jr, dmgl_processor_execute_add_hl, dmgl_processor_execute_ld, dmgl_processor_execute_dec_word,
    dmgl_processor_execute_inc, dmgl_processor_execute_dec, dmgl_processor_execute_ld, dmgl_processor_execute_rra,
    /* 20 */
    dmgl_processor_execute_jr, dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_inc_word,
    dmgl_processor_execute_inc, dmgl_processor_execute_dec, dmgl_processor_execute_ld, dmgl_processor_execute_daa,
    /* 28 */
    dmgl_processor_execute_jr, dmgl_processor_execute_add_hl, dmgl_processor_execute_ld, dmgl_processor_execute_dec_word,
    dmgl_processor_execute_inc, dmgl_processor_execute_dec, dmgl_processor_execute_ld, dmgl_processor_execute_cpl,
    /* 30 */
    dmgl_processor_execute_jr, dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_inc_word,
    dmgl_processor_execute_inc, dmgl_processor_execute_dec, dmgl_processor_execute_ld, dmgl_processor_execute_scf,
    /* 38 */
    dmgl_processor_execute_jr, dmgl_processor_execute_add_hl, dmgl_processor_execute_ld, dmgl_processor_execute_dec_word,
    dmgl_processor_execute_inc, dmgl_processor_execute_dec, dmgl_processor_execute_ld, dmgl_processor_execute_ccf,
    /* 40 */
    dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld,
    dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld,
    /* 48 */
    dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld,
    dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld,
    /* 50 */
    dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld,
    dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld,
    /* 58 */
    dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld,
    dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld,
    /* 60 */
    dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld,
    dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld,
    /* 68 */
    dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld,
    dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld,
    /* 70 */
    dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld,
    dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_halt, dmgl_processor_execute_ld,
    /* 78 */
    dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld,
    dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ld,
    /* 80 */
    dmgl_processor_execute_add, dmgl_processor_execute_add, dmgl_processor_execute_add, dmgl_processor_execute_add,
    dmgl_processor_execute_add, dmgl_processor_execute_add, dmgl_processor_execute_add, dmgl_processor_execute_add,
    /* 88 */
    dmgl_processor_execute_adc, dmgl_processor_execute_adc, dmgl_processor_execute_adc, dmgl_processor_execute_adc,
    dmgl_processor_execute_adc, dmgl_processor_execute_adc, dmgl_processor_execute_adc, dmgl_processor_execute_adc,
    /* 90 */
    dmgl_processor_execute_sub, dmgl_processor_execute_sub, dmgl_processor_execute_sub, dmgl_processor_execute_sub,
    dmgl_processor_execute_sub, dmgl_processor_execute_sub, dmgl_processor_execute_sub, dmgl_processor_execute_sub,
    /* 98 */
    dmgl_processor_execute_sbc, dmgl_processor_execute_sbc, dmgl_processor_execute_sbc, dmgl_processor_execute_sbc,
    dmgl_processor_execute_sbc, dmgl_processor_execute_sbc, dmgl_processor_execute_sbc, dmgl_processor_execute_sbc,
    /* A0 */
    dmgl_processor_execute_and, dmgl_processor_execute_and, dmgl_processor_execute_and, dmgl_processor_execute_and,
    dmgl_processor_execute_and, dmgl_processor_execute_and, dmgl_processor_execute_and, dmgl_processor_execute_and,
    /* A8 */
    dmgl_processor_execute_xor, dmgl_processor_execute_xor, dmgl_processor_execute_xor, dmgl_processor_execute_xor,
    dmgl_processor_execute_xor, dmgl_processor_execute_xor, dmgl_processor_execute_xor, dmgl_processor_execute_xor,
    /* B0 */
    dmgl_processor_execute_or, dmgl_processor_execute_or, dmgl_processor_execute_or, dmgl_processor_execute_or,
    dmgl_processor_execute_or, dmgl_processor_execute_or, dmgl_processor_execute_or, dmgl_processor_execute_or,
    /* B8 */
    dmgl_processor_execute_cp, dmgl_processor_execute_cp, dmgl_processor_execute_cp, dmgl_processor_execute_cp,
    dmgl_processor_execute_cp, dmgl_processor_execute_cp, dmgl_processor_execute_cp, dmgl_processor_execute_cp,
    /* C0 */
    dmgl_processor_execute_ret, dmgl_processor_execute_pop, dmgl_processor_execute_jp, dmgl_processor_execute_jp,
    dmgl_processor_execute_call, dmgl_processor_execute_push, dmgl_processor_execute_add, dmgl_processor_execute_rst,
    /* C8 */
    dmgl_processor_execute_ret, dmgl_processor_execute_ret, dmgl_processor_execute_jp, dmgl_processor_execute_stop,
    dmgl_processor_execute_call, dmgl_processor_execute_call, dmgl_processor_execute_adc, dmgl_processor_execute_rst,
    /* D0 */
    dmgl_processor_execute_ret, dmgl_processor_execute_pop, dmgl_processor_execute_jp, dmgl_processor_execute_stop,
    dmgl_processor_execute_call, dmgl_processor_execute_push, dmgl_processor_execute_sub, dmgl_processor_execute_rst,
    /* D8 */
    dmgl_processor_execute_ret, dmgl_processor_execute_reti, dmgl_processor_execute_jp, dmgl_processor_execute_stop,
    dmgl_processor_execute_call, dmgl_processor_execute_stop, dmgl_processor_execute_sbc, dmgl_processor_execute_rst,
    /* E0 */
    dmgl_processor_execute_ld, dmgl_processor_execute_pop, dmgl_processor_execute_ld, dmgl_processor_execute_stop,
    dmgl_processor_execute_stop, dmgl_processor_execute_push, dmgl_processor_execute_and, dmgl_processor_execute_rst,
    /* E8 */
    dmgl_processor_execute_add_sp, dmgl_processor_execute_jp_hl, dmgl_processor_execute_ld, dmgl_processor_execute_stop,
    dmgl_processor_execute_stop, dmgl_processor_execute_stop, dmgl_processor_execute_xor, dmgl_processor_execute_rst,
    /* F0 */
    dmgl_processor_execute_ld, dmgl_processor_execute_pop, dmgl_processor_execute_ld, dmgl_processor_execute_di,
    dmgl_processor_execute_stop, dmgl_processor_execute_push, dmgl_processor_execute_or, dmgl_processor_execute_rst,
    /* F8 */
    dmgl_processor_execute_ld_hl, dmgl_processor_execute_ld, dmgl_processor_execute_ld, dmgl_processor_execute_ei,
    dmgl_processor_execute_stop, dmgl_processor_execute_stop, dmgl_processor_execute_cp, dmgl_processor_execute_rst,
    /* CB 00 */
    dmgl_processor_execute_rlc, dmgl_processor_execute_rlc, dmgl_processor_execute_rlc, dmgl_processor_execute_rlc,
    dmgl_processor_execute_rlc, dmgl_processor_execute_rlc, dmgl_processor_execute_rlc, dmgl_processor_execute_rlc,
    /* CB 08 */
    dmgl_processor_execute_rrc, dmgl_processor_execute_rrc, dmgl_processor_execute_rrc, dmgl_processor_execute_rrc,
    dmgl_processor_execute_rrc, dmgl_processor_execute_rrc, dmgl_processor_execute_rrc, dmgl_processor_execute_rrc,
    /* CB 10 */
    dmgl_processor_execute_rl, dmgl_processor_execute_rl, dmgl_processor_execute_rl, dmgl_processor_execute_rl,
    dmgl_processor_execute_rl, dmgl_processor_execute_rl, dmgl_processor_execute_rl, dmgl_processor_execute_rl,
    /* CB 18 */
    dmgl_processor_execute_rr, dmgl_processor_execute_rr, dmgl_processor_execute_rr, dmgl_processor_execute_rr,
    dmgl_processor_execute_rr, dmgl_processor_execute_rr, dmgl_processor_execute_rr, dmgl_processor_execute_rr,
    /* CB 20 */
    dmgl_processor_execute_sla, dmgl_processor_execute_sla, dmgl_processor_execute_sla, dmgl_processor_execute_sla,
    dmgl_processor_execute_sla, dmgl_processor_execute_sla, dmgl_processor_execute_sla, dmgl_processor_execute_sla,
    /* CB 28 */
    dmgl_processor_execute_sra, dmgl_processor_execute_sra, dmgl_processor_execute_sra, dmgl_processor_execute_sra,
    dmgl_processor_execute_sra, dmgl_processor_execute_sra, dmgl_processor_execute_sra, dmgl_processor_execute_sra,
    /* CB 30 */
    dmgl_processor_execute_swap, dmgl_processor_execute_swap, dmgl_processor_execute_swap, dmgl_processor_execute_swap,
    dmgl_processor_execute_swap, dmgl_processor_execute_swap, dmgl_processor_execute_swap, dmgl_processor_execute_swap,
    /* CB 38 */
    dmgl_processor_execute_srl, dmgl_processor_execute_srl, dmgl_processor_execute_srl, dmgl_processor_execute_srl,
    dmgl_processor_execute_srl, dmgl_processor_execute_srl, dmgl_processor_execute_srl, dmgl_processor_execute_srl,
    /* CB 40 */
    dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit,
    dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit,
    /* CB 48 */
    dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit,
    dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit,
    /* CB 50 */
    dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit,
    dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit,
    /* CB 58 */
    dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit,
    dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit,
    /* CB 60 */
    dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit,
    dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit,
    /* CB 68 */
    dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit,
    dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit,
    /* CB 70 */
    dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit,
    dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit,
    /* CB 78 */
    dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit,
    dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit, dmgl_processor_execute_bit,
    /* CB 80 */
    dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res,
    dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res,
    /* CB 88 */
    dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res,
    dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res,
    /* CB 90 */
    dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res,
    dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res,
    /* CB 98 */
    dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res,
    dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res,
    /* CB A0 */
    dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res,
    dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res,
    /* CB A8 */
    dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res,
    dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res,
    /* CB B0 */
    dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res,
    dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res,
    /* CB B8 */
    dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res,
    dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res, dmgl_processor_execute_res,
    /* CB C0 */
    dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set,
    dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set,
    /* CB C8 */
    dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set,
    dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set,
    /* CB D0 */
    dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set,
    dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set,
    /* CB D8 */
    dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set,
    dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set,
    /* CB E0 */
    dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set,
    dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set,
    /* CB E8 */
    dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set,
    dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set,
    /* CB F0 */
    dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set,
    dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set,
    /* CB F8 */
    dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set,
    dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set, dmgl_processor_execute_set,
};

static void dmgl_processor_execute(dmgl_processor_t *const processor)
{
    processor->instruction.address = processor->pc.word;
    processor->instruction.opcode = dmgl_read(processor->pc.word++);
    if (processor->halt_bug)
    {
        processor->halt_bug = false;
        --processor->pc.word;
    }
    if (processor->instruction.opcode == 0xCB)
    {
        processor->instruction.opcode = dmgl_read(processor->pc.word++);
        INSTRUCTION[processor->instruction.opcode + 256](processor);
    }
    else
    {
        INSTRUCTION[processor->instruction.opcode](processor);
    }
}

static void dmgl_processor_service(dmgl_processor_t *const processor)
{
    for (uint8_t interrupt = 0; interrupt < 5; ++interrupt)
    {
        uint8_t mask = 1 << interrupt;
        if (processor->interrupt.enable & processor->interrupt.flag & mask)
        {
            processor->delay = 20;
            processor->interrupt.flag &= ~mask;
            if (!processor->halt_bug)
            {
                dmgl_write(--processor->sp.word, processor->pc.high);
                dmgl_write(--processor->sp.word, processor->pc.low);
                processor->pc.word = (interrupt * 8) + 0x0040;
                processor->interrupt.delay = 0;
                processor->interrupt.enabled = false;
            }
            else
            {
                processor->halt_bug = false;
            }
            break;
        }
    }
}

void dmgl_processor_clock(dmgl_processor_t *const processor)
{
    if (!processor->delay)
    {
        if (processor->interrupt.delay && !--processor->interrupt.delay)
        {
            processor->interrupt.enabled = true;
        }
        if (processor->interrupt.enable & processor->interrupt.flag & 0x1F)
        {
            processor->halted = false;
            if (processor->interrupt.enabled)
            {
                dmgl_processor_service(processor);
            }
            else if (!processor->stopped)
            {
                dmgl_processor_execute(processor);
            }
            else
            {
                processor->delay = 4;
            }
        }
        else if (!processor->halted && !processor->stopped)
        {
            dmgl_processor_execute(processor);
        }
        else
        {
            processor->delay = 4;
        }
    }
    --processor->delay;
}

void dmgl_processor_interrupt(dmgl_processor_t *const processor, uint8_t interrupt)
{
    dmgl_processor_write(processor, 0xFF0F, dmgl_processor_read(processor, 0xFF0F) | (1 << interrupt));
}

uint8_t dmgl_processor_read(const dmgl_processor_t *const processor, uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case 0xFF0F: /* IF */
            return processor->interrupt.flag;
            break;
        case 0xFFFF: /* IE */
            return processor->interrupt.enable;
            break;
        default:
            break;
    }
    return result;
}

void dmgl_processor_write(dmgl_processor_t *const processor, uint16_t address, uint8_t value)
{
    switch (address)
    {
        case 0xFF0F: /* IF */
            processor->interrupt.flag = 0xE0 | value;
            if (processor->interrupt.flag & (1 << 4))
            { /* INPUT */
                processor->stopped = false;
            }
            break;
        case 0xFFFF: /* IE */
            processor->interrupt.enable = value;
            break;
        default:
            break;
    }
}
