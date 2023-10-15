/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DMGL_INPUT_H_
#define DMGL_INPUT_H_

#include <system.h>

typedef struct
{
    uint16_t delay;
    bool state[2][8];
    union
    {
        struct
        {
            uint8_t state : 4;
            uint8_t direction : 1;
            uint8_t button : 1;
        };
        uint8_t raw;
    } control;
} dmgl_input_t;

void dmgl_input_clock(dmgl_input_t *const input);
uint8_t dmgl_input_read(const dmgl_input_t *const input, uint16_t address);
bool (*dmgl_input_state(dmgl_input_t *const input))[8];
void dmgl_input_write(dmgl_input_t *const input, uint16_t address, uint8_t value);

#endif /*  DMGL_INPUT_H_ */
