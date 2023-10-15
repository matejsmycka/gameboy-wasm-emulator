/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DMGL_AUDIO_H_
#define DMGL_AUDIO_H_

#include <system.h>

typedef union
{
    struct
    {
        uint8_t : 3;
        uint8_t enabled : 5;
    };
    struct
    {
        uint8_t pace : 3;
        uint8_t direction : 1;
        uint8_t volume : 4;
    };
    uint8_t raw;
} dmgl_audio_envelope_t;

typedef union
{
    struct
    {
        uint16_t period : 11;
        uint16_t : 3;
        uint16_t length_enabled : 1;
        uint16_t enabled : 1;
    };
    struct
    {
        uint8_t low;
        uint8_t high;
    };
} dmgl_audio_frequency_t;

typedef union
{
    struct
    {
        uint8_t timer : 6;
        uint8_t duty : 2;
    };
    uint8_t raw;
} dmgl_audio_length_t;

typedef union
{
    struct
    {
        uint8_t slope : 3;
        uint8_t direction : 1;
        uint8_t pace : 3;
    };
    uint8_t raw;
} dmgl_audio_sweep_t;

typedef struct
{
    struct
    {
        bool full;
        uint16_t read;
        uint16_t write;
        float data[44100];
        float sample[735];
    } buffer;
    struct
    {
        float sample;
        dmgl_audio_envelope_t envelope;
        dmgl_audio_frequency_t frequency;
        dmgl_audio_length_t length;
        dmgl_audio_sweep_t sweep;

    } channel_1;
    struct
    {
        float sample;
        dmgl_audio_envelope_t envelope;
        dmgl_audio_frequency_t frequency;
        dmgl_audio_length_t length;
    } channel_2;
    struct
    {
        float sample;
        uint8_t length;
        uint8_t ram[16];
        dmgl_audio_frequency_t frequency;
        union
        {
            struct
            {
                uint8_t : 7;
                uint8_t enabled : 1;
            };
            uint8_t raw;
        } control;
        union
        {
            struct
            {
                uint8_t : 5;
                uint8_t volume : 2;
            };
            uint8_t raw;
        } level;
    } channel_3;
    struct
    {
        float sample;
        dmgl_audio_envelope_t envelope;
        dmgl_audio_length_t length;
        union
        {
            struct
            {
                uint8_t : 6;
                uint8_t length_enabled : 1;
                uint8_t enabled : 1;
            };
            uint8_t raw;
        } control;
        union
        {
            struct
            {
                uint8_t divider : 3;
                uint8_t width : 1;
                uint8_t shift : 4;
            };
            uint8_t raw;
        } frequency;
    } channel_4;
    union
    {
        struct
        {
            uint8_t channel_1_enabled : 1;
            uint8_t channel_2_enabled : 1;
            uint8_t channel_3_enabled : 1;
            uint8_t channel_4_enabled : 1;
            uint8_t : 3;
            uint8_t enabled : 1;
        };
        uint8_t raw;
    } control;
    struct
    {
        uint16_t clock;
        uint8_t interrupt;
    } delay;
    union
    {
        struct
        {
            uint8_t channel_1_right : 1;
            uint8_t channel_2_right : 1;
            uint8_t channel_3_right : 1;
            uint8_t channel_4_right : 1;
            uint8_t channel_1_left : 1;
            uint8_t channel_2_left : 1;
            uint8_t channel_3_left : 1;
            uint8_t channel_4_left : 1;
        };
        uint8_t raw;
    } mixer;
    union
    {
        struct
        {
            uint8_t right : 3;
            uint8_t vin_right_enabled : 1;
            uint8_t left : 3;
            uint8_t vin_left_enabled : 1;
        };
        uint8_t raw;
    } volume;
} dmgl_audio_t;

void dmgl_audio_clock(dmgl_audio_t *const audio);
void dmgl_audio_interrupt(dmgl_audio_t *const audio);
uint8_t dmgl_audio_read(const dmgl_audio_t *const audio, uint16_t address);
const float (*dmgl_audio_sample(dmgl_audio_t *const audio))[735];
void dmgl_audio_write(dmgl_audio_t *const audio, uint16_t address, uint8_t value);

#endif /* DMGL_AUDIO_H_ */
