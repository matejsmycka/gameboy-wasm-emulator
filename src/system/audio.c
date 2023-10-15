/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <audio.h>

static float dmgl_audio_channel_1(dmgl_audio_t *const audio)
{
    float result = audio->channel_1.sample;
    if (audio->channel_1.envelope.enabled)
    {
        /* TODO: UPDATE CHANNEL 1 SAMPLE */
        audio->channel_1.sample = 0.f;
    }
    else
    {
        audio->channel_1.sample = 0.f;
    }
    return result;
}

static uint16_t dmgl_audio_count(dmgl_audio_t *const audio)
{
    uint16_t result = 0;
    if (audio->buffer.read < audio->buffer.write)
    {
        result = audio->buffer.write - audio->buffer.read;
    }
    else if (audio->buffer.read > audio->buffer.write)
    {
        result = (sizeof (audio->buffer.data) / sizeof (*audio->buffer.data)) - audio->buffer.read;
        if (audio->buffer.write)
        {
            result += audio->buffer.write;
        }
    }
    else if (!audio->buffer.full)
    {
        result = sizeof (audio->buffer.data) / sizeof (*audio->buffer.data);
    }
    return result;
}

static float dmgl_audio_dequeue(dmgl_audio_t *const audio)
{
    float result = audio->buffer.data[audio->buffer.read++];
    audio->buffer.read %= sizeof (audio->buffer.data) / sizeof (*audio->buffer.data);
    audio->buffer.full = false;
    return result;
}

static void dmgl_audio_enqueue(dmgl_audio_t *const audio, float sample)
{
    if (!audio->buffer.full)
    {
        audio->buffer.data[audio->buffer.write++] = sample;
        audio->buffer.write %= sizeof (audio->buffer.data) / sizeof (*audio->buffer.data);
        audio->buffer.full = (audio->buffer.write == audio->buffer.read);
    }
}

void dmgl_audio_clock(dmgl_audio_t *const audio)
{
    if (!audio->delay.clock)
    {
        if (audio->control.enabled)
        {
            float channel[4] = {}, sample = 0.f;
            if (audio->control.channel_1_enabled && (audio->mixer.channel_1_right || audio->mixer.channel_1_left))
            {
                channel[0] = dmgl_audio_channel_1(audio);
            }
            /* TODO: SAMPLE CHANNEL 2-4 */
            sample = ((channel[0] + channel[1] + channel[2] + channel[3]) / 4.f) * ((audio->volume.right + audio->volume.left) / 14.f);
            dmgl_audio_enqueue(audio, sample);
        }
        audio->delay.clock = 95; /* 44.1 KHz */
    }
    --audio->delay.clock;
}

void dmgl_audio_interrupt(dmgl_audio_t *const audio)
{
    if (audio->control.channel_1_enabled)
    {
        if(!++audio->channel_1.length.timer)
        {
            audio->control.channel_1_enabled = false;
        }
    }
    /* CHANNEL 2-4 LENGTH UPDATE */
    if (!audio->delay.interrupt)
    {
        /* TODO: VOLUME/ENVELOPE UPDATE */
        audio->delay.interrupt = 4; /* 64 Hz */
    }
    --audio->delay.interrupt;
}

uint8_t dmgl_audio_read(const dmgl_audio_t *const audio, uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case 0xFF10: /* NR10 */
            result = audio->channel_1.sweep.raw;
            break;
        case 0xFF11: /* NR11 */
            result = audio->channel_1.length.raw & 0xC0;
            break;
        case 0xFF12: /* NR12 */
            result = audio->channel_1.envelope.raw;
            break;
        case 0xFF14: /* NR14 */
            result = audio->channel_1.frequency.high & 0x40;
            break;
        case 0xFF16: /* NR21 */
            result = audio->channel_2.length.raw & 0xC0;
            break;
        case 0xFF17: /* NR22 */
            result = audio->channel_2.envelope.raw;
            break;
        case 0xFF19: /* NR24 */
            result = audio->channel_2.frequency.high & 0x40;
            break;
        case 0xFF1A: /* NR30 */
            result = audio->channel_3.control.raw & 0x80;
            break;
        case 0xFF1B: /* NR31 */
            result = audio->channel_3.length;
            break;
        case 0xFF1C: /* NR32 */
            result = audio->channel_3.level.raw & 0x60;
            break;
        case 0xFF1E: /* NR34 */
            result = audio->channel_3.frequency.high & 0x40;
            break;
        case 0xFF21: /* NR42 */
            result = audio->channel_4.envelope.raw;
            break;
        case 0xFF22: /* NR43 */
            result = audio->channel_4.frequency.raw;
            break;
        case 0xFF23: /* NR44 */
            result = audio->channel_4.control.raw & 0x40;
            break;
        case 0xFF24: /* NR50 */
            result = audio->volume.raw;
            break;
        case 0xFF25: /* NR51 */
            result = audio->mixer.raw;
            break;
        case 0xFF26: /* NR52 */
            result = audio->control.raw;
            break;
        case 0xFF30 ... 0xFF3F: /* WAVE RAM */
            result = audio->channel_3.ram[address - 0xFF30];
            break;
        default:
            break;
    }
    return result;
}

const float (*dmgl_audio_sample(dmgl_audio_t *const audio))[735]
{
    if (dmgl_audio_count(audio) >= sizeof (audio->buffer.sample) / sizeof (*audio->buffer.sample))
    {
        for (uint16_t index = 0; index < sizeof (audio->buffer.sample) / sizeof (*audio->buffer.sample); ++index)
        {
            audio->buffer.sample[index] = dmgl_audio_dequeue(audio);
        }
    }
    else
    {
        memset(audio->buffer.sample, 0, sizeof (audio->buffer.sample));
    }
    return &audio->buffer.sample;
}

void dmgl_audio_write(dmgl_audio_t *const audio, uint16_t address, uint8_t value)
{
    switch (address)
    {
        case 0xFF10: /* NR10 */
            if (audio->control.enabled)
            {
                audio->channel_1.sweep.raw = value;
            }
            break;
        case 0xFF11: /* NR11 */
            if (audio->control.enabled)
            {
                audio->channel_1.length.raw = value;
            }
            break;
        case 0xFF12: /* NR12 */
            if (audio->control.enabled)
            {
                audio->channel_1.envelope.raw = value;
                if (!audio->channel_1.envelope.enabled)
                {
                    audio->channel_1.frequency.enabled = false;
                }
                audio->control.channel_1_enabled = audio->channel_1.envelope.enabled && audio->channel_1.frequency.enabled;
            }
            break;
        case 0xFF13: /* NR13 */
            if (audio->control.enabled)
            {
                audio->channel_1.frequency.low = value;
            }
            break;
        case 0xFF14: /* NR14 */
            if (audio->control.enabled)
            {
                audio->channel_1.frequency.high = value;
                if (!audio->channel_1.envelope.enabled)
                {
                    audio->channel_1.frequency.enabled = false;
                }
                audio->control.channel_1_enabled = audio->channel_1.envelope.enabled && audio->channel_1.frequency.enabled;
            }
            break;
        case 0xFF16: /* NR21 */
            if (audio->control.enabled)
            {
                audio->channel_2.length.raw = value;
            }
            break;
        case 0xFF17: /* NR22 */
            if (audio->control.enabled)
            {
                audio->channel_2.envelope.raw = value;
                /* TODO: SET CHANNEL 2 ENVELOPE/FREQUENCY ENABLE FLAGS */
            }
            break;
        case 0xFF18: /* NR23 */
            if (audio->control.enabled)
            {
                audio->channel_2.frequency.low = value;
            }
            break;
        case 0xFF19: /* NR24 */
            if (audio->control.enabled)
            {
                audio->channel_2.frequency.high = value;
                /* TODO: SET CHANNEL 2 ENVELOPE/FREQUENCY ENABLE FLAGS */
            }
            break;
        case 0xFF1A: /* NR30 */
            if (audio->control.enabled)
            {
                audio->channel_3.control.raw = value;
                /* TODO: SET CHANNEL 3 ENVELOPE/FREQUENCY ENABLE FLAGS */
            }
            break;
        case 0xFF1B: /* NR31 */
            if (audio->control.enabled)
            {
                audio->channel_3.length = value;
            }
            break;
        case 0xFF1C: /* NR32 */
            if (audio->control.enabled)
            {
                audio->channel_3.level.raw = value;
            }
            break;
        case 0xFF1D: /* NR33 */
            if (audio->control.enabled)
            {
                audio->channel_3.frequency.low = value;
            }
            break;
        case 0xFF1E: /* NR34 */
            if (audio->control.enabled)
            {
                audio->channel_3.frequency.high = value;
                /* TODO: SET CHANNEL 3 ENVELOPE/FREQUENCY ENABLE FLAGS */
            }
            break;
        case 0xFF20: /* NR41 */
            if (audio->control.enabled)
            {
                audio->channel_4.length.raw = value;
            }
            break;
        case 0xFF21: /* NR42 */
            if (audio->control.enabled)
            {
                audio->channel_4.envelope.raw = value;
                /* TODO: SET CHANNEL 4 ENVELOPE/FREQUENCY ENABLE FLAGS */
            }
            break;
        case 0xFF22: /* NR43 */
            if (audio->control.enabled)
            {
                audio->channel_4.frequency.raw = value;
            }
            break;
        case 0xFF23: /* NR44 */
            if (audio->control.enabled)
            {
                audio->channel_4.control.raw = value;
                /* TODO: SET CHANNEL 4 ENVELOPE/FREQUENCY ENABLE FLAGS */
            }
            break;
        case 0xFF24: /* NR50 */
            if (audio->control.enabled)
            {
                audio->volume.raw = value;
            }
            break;
        case 0xFF25: /* NR51 */
            if (audio->control.enabled)
            {
                audio->mixer.raw = value;
            }
            break;
        case 0xFF26: /* NR52 */
            audio->control.raw = value & 0x80;
            if (!audio->control.enabled)
            {
                memset(&audio->buffer, 0, sizeof (audio->buffer));
                memset(&audio->channel_1, 0, sizeof (audio->channel_1));
                memset(&audio->channel_2, 0, sizeof (audio->channel_2));
                memset(&audio->channel_3, 0, sizeof (audio->channel_3));
                memset(&audio->channel_4, 0, sizeof (audio->channel_4));
                audio->mixer.raw = 0;
                audio->volume.raw = 0;
            }
            break;
        case 0xFF30 ... 0xFF3F: /* WAVE RAM */
            audio->channel_3.ram[address - 0xFF30] = value;
            break;
        default:
            break;
    }
}
