/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <input.h>

void dmgl_input_clock(dmgl_input_t *const input)
{
    if (!input->delay)
    {
        bool changed = false;
        for (uint8_t button = 0; button < 8; ++button)
        {
            if (input->state[0][button] && !input->state[1][button])
            {
                changed = true;
            }
            input->state[1][button] = input->state[0][button];
        }
        if (changed)
        {
            dmgl_interrupt(4); /* INPUT */
        }
        input->delay = 1 << 15; /* 128 Hz */
    }
    --input->delay;
}

uint8_t dmgl_input_read(const dmgl_input_t *const input, uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case 0xFF00: /* P1 */
            result = input->control.raw;
            break;
        default:
            break;
    }
    return result;
}

bool (*dmgl_input_state(dmgl_input_t *const input))[8]
{
    return &input->state[0];
}

void dmgl_input_write(dmgl_input_t *const input, uint16_t address, uint8_t value)
{
    switch (address)
    {
        case 0xFF00: /* P1 */
            input->control.raw = 0xCF | (value & 0x30);
            if (!input->control.button)
            { /* A,B,SELECT,START */
                for (uint8_t button = 0; button < 4; ++button)
                {
                    if (input->state[1][button])
                    {
                        input->control.state &= ~(1 << button);
                    }
                }
            }
            if (!input->control.direction)
            { /* RIGHT,LEFT,UP,DOWN */
                for (uint8_t button = 4; button < 8; ++button)
                {
                    if (input->state[1][button])
                    {
                        input->control.state &= ~(1 << (button - 4));
                    }
                }
            }
            break;
        default:
            break;
    }
}
