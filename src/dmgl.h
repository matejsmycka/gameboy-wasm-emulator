/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DMGL_H_
#define DMGL_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint8_t palette;
    uint8_t scale;
    struct
    {
        int (*initialize)(const char *const title, uint8_t scale);
        uint8_t (*input)(uint8_t value);
        uint8_t (*output)(uint8_t value);
        int (*poll)(bool (*state)[8]);
        int (*sync)(const uint8_t (*color)[160][144], uint8_t palette, const float (*sample)[735]);
        void (*uninitialize)(void);
    } client;
    struct
    {
        uint8_t *data;
        uint32_t length;
    } ram;
    struct
    {
        uint8_t *data;
        uint32_t length;
    } rom;
} dmgl_t;

typedef struct
{
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
} dmgl_version_t;

int dmgl(dmgl_t *const context);
const char *dmgl_error(void);
const dmgl_version_t *dmgl_version(void);

#endif /* DMGL_H_ */
