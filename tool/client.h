/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DMGL_CLIENT_H_
#define DMGL_CLIENT_H_

#include <dmgl.h>

int client_initialize(const char *const title, uint8_t scale);
uint8_t client_output(uint8_t value);
int client_poll(bool (*state)[8]);
int client_sync(const uint8_t (*color)[160][144], uint8_t palette, const float (*sample)[735]);
void client_uninitialize(void);

#endif /* DMGL_CLIENT_H_ */
