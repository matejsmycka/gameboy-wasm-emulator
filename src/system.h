/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DMGL_SYSTEM_H_
#define DMGL_SYSTEM_H_

#include <common.h>

uint8_t dmgl_input(uint8_t value);
void dmgl_interrupt(uint8_t interrupt);
uint8_t dmgl_output(uint8_t value);
uint8_t dmgl_read(uint16_t address);
void dmgl_write(uint16_t address, uint8_t value);

#endif /* DMGL_SYSTEM_H_ */
