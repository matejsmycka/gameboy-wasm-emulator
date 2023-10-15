/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DMGL_SERIAL_H_
#define DMGL_SERIAL_H_

#include <system.h>

typedef struct
{
    uint8_t data;
    uint16_t divider;
    uint8_t index;
    bool overflow;
    union
    {
        struct
        {
            uint8_t mode : 1;
            uint8_t : 6;
            uint8_t enabled : 1;
        };
        uint8_t raw;
    } control;
} dmgl_serial_t;

void dmgl_serial_clock(dmgl_serial_t *const serial);
uint8_t dmgl_serial_input(dmgl_serial_t *const serial, uint8_t value);
uint8_t dmgl_serial_read(const dmgl_serial_t *const serial, uint16_t address);
void dmgl_serial_write(dmgl_serial_t *const serial, uint16_t address, uint8_t value);

#endif /*  DMGL_SERIAL_H_ */
