/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifndef DMGL_MEMORY_H_
#define DMGL_MEMORY_H_

#include <system.h>

typedef struct
{
    uint8_t entry[4];
    uint8_t logo[48];
    uint8_t title[11];
    uint8_t manufacturer[4];
    uint8_t cgb;
    uint8_t licensee[2];
    uint8_t sgb;
    uint8_t id;
    uint8_t rom;
    uint8_t ram;
    uint8_t destination;
    uint8_t licensee_old;
    uint8_t version;
    uint8_t checksum;
    uint16_t checksum_global;
} dmgl_cartridge_t;

typedef struct
{
    union
    {
        struct
        {
            uint8_t counter : 6;
        };
        uint8_t raw;
    } second;
    union
    {
        struct
        {
            uint8_t counter : 6;
        };
        uint8_t raw;
    } minute;
    union
    {
        struct
        {
            uint8_t counter : 5;
        };
        uint8_t raw;
    } hour;
    union
    {
        struct
        {
            uint16_t counter : 9;
            uint16_t : 5;
            uint16_t halt : 1;
            uint16_t carry : 1;
        };
        struct
        {
            uint8_t low;
            uint8_t high;
        };
    } day;
} dmgl_clock_t;

typedef struct
{
    uint8_t magic[4];
    uint32_t length;
    union
    {
        struct
        {
            uint32_t major : 4;
            uint32_t minor : 4;
        };
        uint32_t raw;
    } flag;
    dmgl_clock_t clock;
} dmgl_save_t;

typedef struct dmgl_memory_s
{
    char title[12];
    struct
    {
        bool enabled;
    } bootrom;
    struct
    {
        uint16_t delay;
        bool latched;
        dmgl_clock_t *data;
        dmgl_clock_t latch;
    } clock;
    struct
    {
        uint8_t (*read)(const struct dmgl_memory_s *const memory, uint16_t address);
        void (*write)(struct dmgl_memory_s *const memory, uint16_t address, uint8_t value);
        struct
        {
            struct
            {
                uint8_t high;
                uint8_t low;
                uint8_t select;
            } bank;
            struct
            {
                uint32_t bank;
                bool enabled;
            } ram;
            struct
            {
                uint32_t bank[2];
            } rom;
        };
    } mapper;
    struct
    {
        uint32_t high[0x80];
        uint32_t work[0x2000];
        uint8_t *data;
        uint32_t count;
    } ram;
    struct
    {
        const uint8_t *data;
        uint32_t count;
    } rom;
} dmgl_memory_t;

void dmgl_memory_clock(dmgl_memory_t *const memory);
int dmgl_memory_initialize(dmgl_memory_t *const memory, dmgl_t *const context);
uint8_t dmgl_memory_read(const dmgl_memory_t *const memory, uint16_t address);
const char *dmgl_memory_title(const dmgl_memory_t *const memory);
void dmgl_memory_write(dmgl_memory_t *const memory, uint16_t address, uint8_t value);

#endif /* DMGL_MEMORY_H_ */
