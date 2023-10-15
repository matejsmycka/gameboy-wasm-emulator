/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <video.h>

static uint8_t dmgl_video_background_color(dmgl_video_t *const video, uint8_t map, uint8_t x, uint8_t y)
{
    uint16_t address = (map ? 0x1C00 : 0x1800) + (32 * ((y / 8) & 31)) + ((x / 8) & 31);
    if (video->control.background_data)
    {
        address = (16 * video->ram[address]) + (2 * (y & 7));
    }
    else
    {
        address = (16 * (int8_t)video->ram[address]) + (2 * (y & 7)) + 0x1000;
    }
    x = 1 << (7 - (x & 7));
    return ((video->ram[address + 1] & x) ? 2 : 0) + ((video->ram[address] & x) ? 1 : 0);
}

static void dmgl_video_coincidence(dmgl_video_t *const video)
{
    bool coincidence = video->status.coincidence;
    video->status.coincidence = (video->line.coincidence == video->line.y);
    if (video->status.coincidence_interrupt && !coincidence && video->status.coincidence)
    {
        dmgl_interrupt(1); /* STATUS */
    }
}

static uint8_t dmgl_video_object_color(dmgl_video_t *const video, const dmgl_object_t *object, uint8_t x, uint8_t y)
{
    uint16_t address = 0;
    uint8_t index = object->index;
    if (video->control.object_size)
    {
        if (object->attribute.y_flip)
        {
            if ((y - (object->y - 16)) < 8)
            {
                index |= 1;
            }
            else
            {
                index &= 0xFE;
            }
        }
        else if ((y - (object->y - 16)) < 8)
        {
            index &= 0xFE;
        }
        else
        {
            index |= 1;
        }
    }
    y = (y - object->y) & 7;
    if (object->attribute.x_flip)
    {
        x = 7 - x;
    }
    if (object->attribute.y_flip)
    {
        y = 7 - y;
    }
    address = (16 * index) + (2 * y);
    x = 1 << (7 - x);
    return ((video->ram[address + 1] & x) ? 2 : 0) + ((video->ram[address] & x) ? 1 : 0);
}

static int dmgl_video_object_comparator(const void *first, const void *second)
{
    int result = 0;
    const dmgl_object_entry_t *entry[] = { first, second };
    if (entry[0]->object->x < entry[1]->object->x)
    {
        result = 1;
    }
    else if (entry[0]->object->x == entry[1]->object->x)
    {
        result = (entry[0]->index < entry[1]->index) ? 1 : 0;
    }
    else
    {
        result = -1;
    }
    return result;
}

static uint8_t dmgl_video_palette_color(const dmgl_palette_t *const palette, uint8_t color)
{
    uint8_t result = 0;
    switch (color)
    {
        case 0: /* WHITE */
            result = palette->white;
            break;
        case 1: /* LIGHT GREY */
            result = palette->light_grey;
            break;
        case 2: /* DARK GREY */
            result = palette->dark_grey;
            break;
        case 3: /* BLACK */
            result = palette->black;
            break;
        default:
            break;
    }
    return result;
}

static void dmgl_video_render_background(dmgl_video_t *const video)
{
    for (uint8_t pixel = 0; pixel < 160; ++pixel)
    {
        uint8_t color = 0, map = 0, x = pixel, y = video->line.y;
        if (video->control.window_enabled && (video->window.x <= 166) && (video->window.y <= 143) && ((video->window.x - 7) <= x) && (video->window.y <= y))
        {
            map = video->control.window_map;
            x -= (video->window.x - 7);
            y = video->window.counter - video->window.y;
        }
        else
        {
            map = video->control.background_map;
            x += video->scroll.x;
            y += video->scroll.y;
        }
        color = dmgl_video_palette_color(&video->background.palette, dmgl_video_background_color(video, map, x, y));
        video->color[pixel][video->line.y] = color;
    }
}

static void dmgl_video_render_objects(dmgl_video_t *const video)
{
    uint8_t color = 0, y = video->line.y;
    for (uint32_t index = 0; index < video->object.shown.count; ++index)
    {
        const dmgl_object_t *object = video->object.shown.entry[index].object;
        for (uint8_t x = 0; x < 8; ++x)
        {
            if ((object->x < 8) && (x < (8 - object->x)))
            {
                continue;
            }
            else if ((object->x >= 160) && (x >= (8 - (object->x - 160))))
            {
                break;
            }
            if ((color = dmgl_video_object_color(video, object, x, y)))
            {
                if (!object->attribute.priority || !video->color[object->x + x - 8][y])
                {
                    color = dmgl_video_palette_color(&video->object.palette[object->attribute.palette], color);
                    video->color[object->x + x - 8][y] = color;
                }
            }
        }
    }
}

static void dmgl_video_sort_objects(dmgl_video_t *const video)
{
    video->object.shown.count = 0;
    uint8_t y = video->line.y, size = video->control.object_size ? 16 : 8;
    for (uint8_t index = 0; index < 40; ++index)
    {
        const dmgl_object_t *object = &video->object.ram[index];
        if ((y >= (object->y - 16)) && (y < (object->y - 16 + size)))
        {
            dmgl_object_entry_t *entry = &video->object.shown.entry[video->object.shown.count++];
            entry->object = object;
            entry->index = index;
        }
        if (video->object.shown.count >= 10)
        {
            break;
        }
    }
    if (video->object.shown.count)
    {
        qsort(video->object.shown.entry, video->object.shown.count, sizeof (*video->object.shown.entry), dmgl_video_object_comparator);
    }
}

static void dmgl_video_mode_hblank(dmgl_video_t *const video)
{
    if (video->control.background_enabled)
    {
        dmgl_video_render_background(video);
    }
    if (video->control.object_enabled)
    {
        dmgl_video_render_objects(video);
    }
    if (video->status.hblank_interrupt)
    {
        dmgl_interrupt(1); /* STATUS */
    }
}

static void dmgl_video_mode_search(dmgl_video_t *const video)
{
    if (video->status.search_interrupt)
    {
        dmgl_interrupt(1); /* STATUS */
    }
}

static void dmgl_video_mode_transfer(dmgl_video_t *const video)
{
    if (video->control.object_enabled)
    {
        dmgl_video_sort_objects(video);
    }
}

static void dmgl_video_mode_vblank(dmgl_video_t *const video)
{
    if (video->status.vblank_interrupt)
    {
        dmgl_interrupt(1); /* STATUS */
    }
    dmgl_interrupt(0); /* VBLANK */
}

static void dmgl_video_transfer(dmgl_video_t *const video)
{
    if (!video->transfer.delay)
    {
        ((uint8_t *)video->object.ram)[video->transfer.destination++ & 0xFF] = dmgl_read(video->transfer.source++);
        if (video->transfer.destination == 0xFEA0)
        {
            video->transfer.destination = 0;
            video->transfer.source = 0;
            return;
        }
        video->transfer.delay = 4;
    }
    --video->transfer.delay;
}

bool dmgl_video_clock(dmgl_video_t *const video)
{
    bool result = false;
    if (video->control.enabled)
    {
        dmgl_video_coincidence(video);
    }
    if (video->transfer.destination)
    {
        dmgl_video_transfer(video);
    }
    if (video->line.y < 144)
    {
        if (!video->line.x)
        {
            if (video->control.enabled)
            {
                dmgl_video_mode_search(video);
            }
            video->status.mode = 2; /* SEARCH */
        }
        else if (video->line.x == 80)
        {
            if (video->control.enabled)
            {
                dmgl_video_mode_transfer(video);
            }
            video->status.mode = 3; /* TRANSFER */
        }
        else if (video->line.x == 260)
        {
            if (video->control.enabled)
            {
                dmgl_video_mode_hblank(video);
            }
            video->status.mode = 0; /* HBLANK */
        }
    }
    else if (!video->line.x)
    {
        if (video->line.y == 144)
        {
            if (video->control.enabled)
            {
                dmgl_video_mode_vblank(video);
            }
            result = true;
        }
        video->status.mode = 1; /* VBLANK */
    }
    if (++video->line.x == 456)
    {
        if ((video->window.x <= 166) && (video->window.y <= 143))
        {
            ++video->window.counter;
        }
        if (++video->line.y == 154)
        {
            video->line.y = 0;
            video->window.counter = 0;
        }
        video->line.x = 0;
    }
    return result;
}

const uint8_t (*dmgl_video_color(dmgl_video_t *const video))[160][144]
{
    return &video->color;
}

uint8_t dmgl_video_read(const dmgl_video_t *const video, uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case 0x8000 ... 0x9FFF: /* VIDEO RAM */
            if (!video->control.enabled || (video->status.mode < 3)) /* HBLANK-SEARCH */
            {
                result = video->ram[address - 0x8000];
            }
            break;
        case 0xFE00 ... 0xFE9F: /* OBJECT RAM */
            if (!video->control.enabled || (video->status.mode < 2)) /* HBLANK-VBLANK */
            {
                result = ((uint8_t *)video->object.ram)[address - 0xFE00];
            }
            break;
        case 0xFF40: /* LCDC */
            result = video->control.raw;
            break;
        case 0xFF41: /* STAT */
            result = video->status.raw;
            break;
        case 0xFF42: /* SCY */
            result = video->scroll.y;
            break;
        case 0xFF43: /* SCX */
            result = video->scroll.x;
            break;
        case 0xFF44: /* LY */
            result = video->line.y;
            break;
        case 0xFF45: /* LYC */
            result = video->line.coincidence;
            break;
        case 0xFF47: /* BGP */
            result = video->background.palette.raw;
            break;
        case 0xFF48: /* OBP0 */
            result = video->object.palette[0].raw;
            break;
        case 0xFF49: /* OBP1 */
            result = video->object.palette[1].raw;
            break;
        case 0xFF4A: /* WY */
            result = video->window.y;
            break;
        case 0xFF4B: /* WX */
            result = video->window.x;
            break;
        default:
            break;
    }
    return result;
}

void dmgl_video_write(dmgl_video_t *const video, uint16_t address, uint8_t value)
{
    switch (address)
    {
        case 0x8000 ... 0x9FFF: /* VIDEO RAM */
            if (!video->control.enabled || (video->status.mode < 3)) /* HBLANK-SEARCH */
            {
                video->ram[address - 0x8000] = value;
            }
            break;
        case 0xFE00 ... 0xFE9F: /* OBJECT RAM */
            if (!video->control.enabled || (video->status.mode < 2)) /* HBLANK-VBLANK */
            {
                ((uint8_t *)video->object.ram)[address - 0xFE00] = value;
            }
            break;
        case 0xFF40: /* LCDC */
            video->control.raw = value;
            if (!video->control.enabled)
            {
                memset(video->color, 0, sizeof (video->color));
            }
            break;
        case 0xFF41: /* STAT */
            video->status.raw = 0x80 | (value & 0x78);
            break;
        case 0xFF42: /* SCY */
            video->scroll.y = value;
            break;
        case 0xFF43: /* SCX */
            video->scroll.x = value;
            break;
        case 0xFF45: /* LYC */
            video->line.coincidence = value;
            break;
        case 0xFF46: /* DMA */
            video->transfer.delay = 4;
            video->transfer.destination = 0xFE00;
            video->transfer.source = value << 8;
            break;
        case 0xFF47: /* BGP */
            video->background.palette.raw = value;
            break;
        case 0xFF48: /* OBP0 */
            video->object.palette[0].raw = value;
            break;
        case 0xFF49: /* OBP1 */
            video->object.palette[1].raw = value;
            break;
        case 0xFF4A: /* WY */
            video->window.y = value;
            break;
        case 0xFF4B: /* WX */
            video->window.x = value;
            break;
        default:
            break;
    }
}
