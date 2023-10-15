/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DMGL_VIDEO_H_
#define DMGL_VIDEO_H_

#include <system.h>

typedef struct
{
    uint8_t y;
    uint8_t x;
    uint8_t index;
    union
    {
        struct
        {
            uint8_t : 4;
            uint8_t palette : 1;
            uint8_t x_flip : 1;
            uint8_t y_flip : 1;
            uint8_t priority : 1;
        };
        uint8_t raw;
    } attribute;
} dmgl_object_t;

typedef struct
{
    uint8_t index;
    const dmgl_object_t *object;
} dmgl_object_entry_t;

typedef union
{
    struct
    {
        uint8_t white : 2;
        uint8_t light_grey : 2;
        uint8_t dark_grey : 2;
        uint8_t black : 2;
    };
    uint8_t raw;
} dmgl_palette_t;

typedef struct
{
    uint8_t ram[0x2000];
    uint8_t color[160][144];
    struct
    {
        dmgl_palette_t palette;
    } background;
    union
    {
        struct
        {
            uint8_t background_enabled : 1;
            uint8_t object_enabled : 1;
            uint8_t object_size : 1;
            uint8_t background_map : 1;
            uint8_t background_data : 1;
            uint8_t window_enabled : 1;
            uint8_t window_map : 1;
            uint8_t enabled : 1;
        };
        uint8_t raw;
    } control;
    struct
    {
        uint8_t coincidence;
        uint16_t x;
        uint8_t y;
    } line;
    struct
    {
        dmgl_palette_t palette[2];
        dmgl_object_t ram[40];
        struct
        {
            uint8_t count;
            dmgl_object_entry_t entry[10];
        } shown;
    } object;
    struct
    {
        uint8_t x;
        uint8_t y;
    } scroll;
    union
    {
        struct
        {
            uint8_t mode : 2;
            uint8_t coincidence : 1;
            uint8_t hblank_interrupt : 1;
            uint8_t vblank_interrupt : 1;
            uint8_t search_interrupt : 1;
            uint8_t coincidence_interrupt : 1;
        };
        uint8_t raw;
    } status;
    struct
    {
        uint8_t delay;
        uint16_t destination;
        uint16_t source;
    } transfer;
    struct
    {
        uint8_t counter;
        uint8_t x;
        uint8_t y;
    } window;
} dmgl_video_t;

bool dmgl_video_clock(dmgl_video_t *const video);
const uint8_t (*dmgl_video_color(dmgl_video_t *const video))[160][144];
uint8_t dmgl_video_read(const dmgl_video_t *const video, uint16_t address);
void dmgl_video_write(dmgl_video_t *const video, uint16_t address, uint8_t value);

#endif /* DMGL_VIDEO_H_ */
