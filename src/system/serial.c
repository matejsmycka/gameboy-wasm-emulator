/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <serial.h>

void dmgl_serial_clock(dmgl_serial_t *const serial)
{
    ++serial->divider;
    if (serial->control.enabled && serial->control.mode)
    {
        bool overflow = serial->divider & (1 << 8); /* 8 KHz */
        if (serial->overflow && !overflow)
        {
            serial->data = (serial->data << 1) | (dmgl_output((serial->data & 0x80) == 0x80) & 1);
            if (++serial->index >= 8)
            {
                serial->control.enabled = false;
                serial->index = 0;
                dmgl_interrupt(3); /* SERIAL */
            }
        }
        serial->overflow = overflow;
    }
}

uint8_t dmgl_serial_input(dmgl_serial_t *const serial, uint8_t value)
{
    uint8_t result = ((serial->data & 0x80) == 0x80);
    serial->data = (serial->data << 1) | (value & 1);
    if (++serial->index >= 8)
    {
        serial->control.enabled = false;
        serial->index = 0;
        dmgl_interrupt(3); /* SERIAL */
    }
    return result;
}

uint8_t dmgl_serial_read(const dmgl_serial_t *const serial, uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case 0xFF01: /* SB */
            result = serial->data;
            break;
        case 0xFF02: /* SC */
            result = serial->control.raw;
            break;
        default:
            break;
    }
    return result;
}

void dmgl_serial_write(dmgl_serial_t *const serial, uint16_t address, uint8_t value)
{
    switch (address)
    {
        case 0xFF01: /* SB */
            serial->data = value;
            break;
        case 0xFF02: /* SC */
            serial->control.raw = 0x7E | value;
            if (serial->control.enabled)
            {
                serial->index = 0;
            }
            break;
        default:
            break;
    }
}
