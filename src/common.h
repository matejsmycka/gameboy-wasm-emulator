/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DMGL_COMMON_H_
#define DMGL_COMMON_H_

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dmgl.h>

#define DMGL_MAJOR 0
#define DMGL_MINOR 1
#define DMGL_PATCH 0xe895851

#define DMGL_ERROR(_FORMAT_, ...) \
    dmgl_error_set(__FILE__, __LINE__, _FORMAT_, ##__VA_ARGS__)

int dmgl_error_set(const char *const file, uint32_t line, const char *const format, ...);

#endif /* DMGL_COMMON_H_ */
