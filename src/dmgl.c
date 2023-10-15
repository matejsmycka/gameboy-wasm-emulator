/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <audio.h>
#include <input.h>
#include <memory.h>
#include <processor.h>
#include <serial.h>
#include <timer.h>
#include <video.h>

static struct
{
    dmgl_t *context;
    dmgl_audio_t audio;
    dmgl_input_t input;
    dmgl_memory_t memory;
    dmgl_processor_t processor;
    dmgl_serial_t serial;
    dmgl_timer_t timer;
    dmgl_video_t video;
} g_dmgl = {};

static void dmgl_clock(void)
{
    while (!dmgl_video_clock(&g_dmgl.video))
    {
        dmgl_audio_clock(&g_dmgl.audio);
        dmgl_input_clock(&g_dmgl.input);
        dmgl_serial_clock(&g_dmgl.serial);
        dmgl_timer_clock(&g_dmgl.timer);
        dmgl_processor_clock(&g_dmgl.processor);
    }
    dmgl_memory_clock(&g_dmgl.memory);
}

static int dmgl_initialize(dmgl_t *const context)
{
    int result = EXIT_SUCCESS;
    if (!context)
    {
        return DMGL_ERROR("Invalid context -- %p", context);
    }
    if (!context->client.initialize)
    {
        return DMGL_ERROR("Invalid initialize callback -- %p", context->client.initialize);
    }
    if (!context->client.output)
    {
        return DMGL_ERROR("Invalid output callback -- %p", context->client.output);
    }
    if (!context->client.poll)
    {
        return DMGL_ERROR("Invalid poll callback -- %p", context->client.poll);
    }
    if (!context->client.sync)
    {
        return DMGL_ERROR("Invalid sync callback -- %p", context->client.sync);
    }
    if (!context->client.uninitialize)
    {
        return DMGL_ERROR("Invalid uninitialize callback -- %p", context->client.uninitialize);
    }
    g_dmgl.context = context;
    if ((result = dmgl_memory_initialize(&g_dmgl.memory, g_dmgl.context)) != EXIT_SUCCESS)
    {
        return result;
    }
    if ((result = g_dmgl.context->client.initialize(dmgl_memory_title(&g_dmgl.memory), g_dmgl.context->scale)) != EXIT_SUCCESS)
    {
        return DMGL_ERROR("Client initialize failed -- %08X", result);
    }
    g_dmgl.context->client.input = dmgl_input;
    return EXIT_SUCCESS;
}

static int dmgl_poll(void)
{
    int result = EXIT_SUCCESS;
    if ((result = g_dmgl.context->client.poll(dmgl_input_state(&g_dmgl.input))) != EXIT_SUCCESS)
    {
        result = DMGL_ERROR("Client poll failed -- %08X", result);
    }
    return result;
}

static int dmgl_sync(void)
{
    int result = EXIT_SUCCESS;
    if ((result = g_dmgl.context->client.sync(dmgl_video_color(&g_dmgl.video), g_dmgl.context->palette, dmgl_audio_sample(&g_dmgl.audio))) != EXIT_SUCCESS)
    {
        result = DMGL_ERROR("Client sync failed -- %08X", result);
    }
    return result;
}

static void dmgl_uninitialize(void)
{
    if (g_dmgl.context->client.uninitialize)
    {
        g_dmgl.context->client.uninitialize();
    }
}

int dmgl(dmgl_t *const context)
{
    int result = EXIT_SUCCESS;
    if ((result = dmgl_initialize(context)) != EXIT_SUCCESS)
    {
        return result;
    }
    while (dmgl_poll() == EXIT_SUCCESS)
    {
        dmgl_clock();
        if ((result = dmgl_sync()) != EXIT_SUCCESS)
        {
            break;
        }
    }
    dmgl_uninitialize();
    return result;
}

uint8_t dmgl_input(uint8_t value)
{
    return dmgl_serial_input(&g_dmgl.serial, value);
}

void dmgl_interrupt(uint8_t interrupt)
{
    switch (interrupt)
    {
        case 0 ... 4: /* PROCESSOR */
            dmgl_processor_interrupt(&g_dmgl.processor, interrupt);
            break;
        case 5: /* AUDIO */
            dmgl_audio_interrupt(&g_dmgl.audio);
            break;
        default:
            break;
    }
}

uint8_t dmgl_output(uint8_t value)
{
    return g_dmgl.context->client.output(value);
}

uint8_t dmgl_read(uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case 0xFF00: /* INPUT */
            result = dmgl_input_read(&g_dmgl.input, address);
            break;
        case 0xFF01 ... 0xFF02: /* SERIAL */
            result = dmgl_serial_read(&g_dmgl.serial, address);
            break;
        case 0xFF04 ... 0xFF07: /* TIMER */
            result = dmgl_timer_read(&g_dmgl.timer, address);
            break;
        case 0xFF10 ... 0xFF14: /* AUDIO */
        case 0xFF16 ... 0xFF1E:
        case 0xFF20 ... 0xFF26:
        case 0xFF30 ... 0xFF3F:
            result = dmgl_audio_read(&g_dmgl.audio, address);
            break;
        case 0x8000 ... 0x9FFF: /* VIDEO */
        case 0xFE00 ... 0xFE9F:
        case 0xFF40 ... 0xFF4B:
            result = dmgl_video_read(&g_dmgl.video, address);
            break;
        case 0xFF0F: /* PROCESSOR */
        case 0xFFFF:
            result = dmgl_processor_read(&g_dmgl.processor, address);
            break;
        default: /* MEMORY */
            result = dmgl_memory_read(&g_dmgl.memory, address);
            break;
    }
    return result;
}

void dmgl_write(uint16_t address, uint8_t value)
{
    switch (address)
    {
        case 0xFF00: /* INPUT */
            dmgl_input_write(&g_dmgl.input, address, value);
            break;
        case 0xFF01 ... 0xFF02: /* SERIAL */
            dmgl_serial_write(&g_dmgl.serial, address, value);
            break;
        case 0xFF04 ... 0xFF07: /* TIMER */
            dmgl_timer_write(&g_dmgl.timer, address, value);
            break;
        case 0xFF10 ... 0xFF14: /* AUDIO */
        case 0xFF16 ... 0xFF1E:
        case 0xFF20 ... 0xFF26:
        case 0xFF30 ... 0xFF3F:
            dmgl_audio_write(&g_dmgl.audio, address, value);
            break;
        case 0x8000 ... 0x9FFF: /* VIDEO */
        case 0xFE00 ... 0xFE9F:
        case 0xFF40 ... 0xFF4B:
            dmgl_video_write(&g_dmgl.video, address, value);
            break;
        case 0xFF0F: /* PROCESSOR */
        case 0xFFFF:
            dmgl_processor_write(&g_dmgl.processor, address, value);
            break;
        default: /* MEMORY */
            dmgl_memory_write(&g_dmgl.memory, address, value);
            break;
    }
}
