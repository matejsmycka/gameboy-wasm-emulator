/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <common.h>

static const dmgl_version_t VERSION =
{
    DMGL_MAJOR, DMGL_MINOR, DMGL_PATCH
};

const dmgl_version_t *dmgl_version(void)
{
    return &VERSION;
}
