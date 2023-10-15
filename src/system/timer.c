/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <timer.h>

static const uint16_t MODE[] =
{
    1 << 9, /* 4 KHz */
    1 << 3, /* 256 KHz */
    1 << 5, /* 64 KHz */
    1 << 7, /* 16 KHz */
};

void dmgl_timer_clock(dmgl_timer_t *const timer)
{
    bool overflow = false;
    ++timer->divider;
    if (timer->control.enabled)
    {
        overflow = timer->divider & MODE[timer->control.mode];
        if (timer->overflow[0] && !overflow && !++timer->counter)
        {
            timer->counter = timer->modulo;
            dmgl_interrupt(2); /* TIMER */
        }
        timer->overflow[0] = overflow;
    }
    overflow = timer->divider & (1 << 13); /* 256 Hz */
    if (timer->overflow[1] && !overflow)
    {
        dmgl_interrupt(5); /* AUDIO */
    }
    timer->overflow[1] = overflow;
}

uint8_t dmgl_timer_read(const dmgl_timer_t *const timer, uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case 0xFF04: /* DIV */
            result = timer->divider >> 8;
            break;
        case 0xFF05: /* TIMA */
            result = timer->counter;
            break;
        case 0xFF06: /* TMA */
            result = timer->modulo;
            break;
        case 0xFF07: /* TAC */
            result = timer->control.raw;
            break;
        default:
            break;
    }
    return result;
}

void dmgl_timer_write(dmgl_timer_t *const timer, uint16_t address, uint8_t value)
{
    switch (address)
    {
        case 0xFF04: /* DIV */
            timer->divider = 0;
            break;
        case 0xFF05: /* TIMA */
            timer->counter = value;
            break;
        case 0xFF06: /* TMA */
            timer->modulo = value;
            break;
        case 0xFF07: /* TAC */
            timer->control.raw = value;
            break;
        default:
            break;
    }
}
