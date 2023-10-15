/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <common.h>

static char g_error[256] = {};

const char *dmgl_error(void)
{
    return g_error;
}

int dmgl_error_set(const char *const file, uint32_t line, const char *const format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    vsnprintf(g_error, sizeof (g_error), format, arguments);
    va_end(arguments);
#ifndef NDEBUG
    snprintf(g_error + strlen(g_error), sizeof (g_error) - strlen(g_error) - 1, " (%s:%u)", file, line);
#endif /* NDEBUG */
    return EXIT_FAILURE;
}
