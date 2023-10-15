/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DMGL_TIMER_H_
#define DMGL_TIMER_H_

#include <system.h>

typedef struct
{
    uint8_t counter;
    uint16_t divider;
    uint8_t modulo;
    bool overflow[2];
    union
    {
        struct
        {
            uint8_t mode : 2;
            uint8_t enabled : 1;
        };
        uint8_t raw;
    } control;
} dmgl_timer_t;

void dmgl_timer_clock(dmgl_timer_t *const timer);
uint8_t dmgl_timer_read(const dmgl_timer_t *const timer, uint16_t address);
void dmgl_timer_write(dmgl_timer_t *const timer, uint16_t address, uint8_t value);

#endif /*  DMGL_TIMER_H_ */
