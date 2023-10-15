/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <memory.h>

static const uint8_t BOOTROM[] =
{
    0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
    0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
    0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
    0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
    0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
    0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
    0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
    0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
    0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
    0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
    0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
    0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
    0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50,
};

static const uint32_t RAM[] =
{
    1, 1, 1, 4, 16, 8,
};

static const uint32_t ROM[] =
{
    2, 4, 8, 16, 32, 64, 128, 256, 512,
};

static const dmgl_cartridge_t *dmgl_memory_cartridge(const uint8_t *const data)
{
    return (const dmgl_cartridge_t *)&data[0x0100];
}

static uint8_t dmgl_memory_checksum(const uint8_t *const data, uint16_t begin, uint16_t end)
{
    uint8_t result = 0;
    for (uint32_t index = begin; index <= end; ++index)
    {
        result = result - data[index] - 1;
    }
    return result;
}

static uint8_t dmgl_memory_mapper_mbc0_read(const dmgl_memory_t *const memory, uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case 0x0000 ... 0x7FFF: /* ROM 0-1 */
            result = memory->rom.data[address];
            break;
        case 0xA000 ... 0xBFFF: /* RAM 0 */
            if (memory->ram.data)
            {
                result = memory->ram.data[address - 0xA000];
            }
            break;
        default:
            break;
    }
    return result;
}

static void dmgl_memory_mapper_mbc0_write(dmgl_memory_t *const memory, uint16_t address, uint8_t value)
{
    switch (address)
    {
        case 0xA000 ... 0xBFFF: /* RAM 0 */
            if (memory->ram.data)
            {
                memory->ram.data[address - 0xA000] = value;
            }
            break;
        default:
            break;
    }
}

static uint8_t dmgl_memory_mapper_mbc1_read(const dmgl_memory_t *const memory, uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case 0x0000 ... 0x3FFF: /* ROM 0,32,64,96 */
            result = memory->rom.data[(memory->mapper.rom.bank[0] * 0x4000) + address];
            break;
        case 0x4000 ... 0x7FFF: /* ROM 1-31,33-63,65-95,97-127 */
            result = memory->rom.data[(memory->mapper.rom.bank[1] * 0x4000) + (address - 0x4000)];
            break;
        case 0xA000 ... 0xBFFF: /* RAM 0-3 */
            if (memory->mapper.ram.enabled)
            {
                result = memory->ram.data[(memory->mapper.ram.bank * 0x2000) + (address - 0xA000)];
            }
            break;
        default:
            break;
    }
    return result;
}

static void dmgl_memory_mapper_mbc1_update(dmgl_memory_t *const memory)
{
    if (memory->rom.count >= 64)
    { /* >=1MB */
        memory->mapper.ram.bank = 0;
        memory->mapper.rom.bank[0] = (memory->mapper.bank.select & 1) ? (memory->mapper.bank.high & 3) << 5 : 0;
        memory->mapper.rom.bank[1] = ((memory->mapper.bank.high & 3) << 5) | (memory->mapper.bank.low & 31);
    }
    else
    { /* <=512KB */
        memory->mapper.ram.bank = (memory->mapper.bank.select & 1) ? memory->mapper.bank.high & 3 : 0;
        memory->mapper.rom.bank[0] = 0;
        memory->mapper.rom.bank[1] = memory->mapper.bank.low & 31;
    }
    switch (memory->mapper.rom.bank[1])
    {
        case 0: /* BANK 0->1 */
        case 32: /* BANK 32->33 */
        case 64: /* BANK 64->65 */
        case 96: /* BANK 96->97 */
            ++memory->mapper.rom.bank[1];
            break;
        default:
            break;
    }
    memory->mapper.ram.bank &= memory->ram.count - 1;
    memory->mapper.rom.bank[0] &= memory->rom.count - 1;
    memory->mapper.rom.bank[1] &= memory->rom.count - 1;
}

static void dmgl_memory_mapper_mbc1_write(dmgl_memory_t *const memory, uint16_t address, uint8_t value)
{
    switch (address)
    {
        case 0x0000 ... 0x1FFF: /* RAM ENABLE */
            memory->mapper.ram.enabled = ((value & 0x0F) == 0x0A);
            break;
        case 0x2000 ... 0x3FFF: /* LOW BANK */
            memory->mapper.bank.low = value;
            dmgl_memory_mapper_mbc1_update(memory);
            break;
        case 0x4000 ... 0x5FFF: /* HIGH BANK */
            memory->mapper.bank.high = value;
            dmgl_memory_mapper_mbc1_update(memory);
            break;
        case 0x6000 ... 0x7FFF: /* BANK SELECT */
            memory->mapper.bank.select = value;
            dmgl_memory_mapper_mbc1_update(memory);
            break;
        case 0xA000 ... 0xBFFF: /* RAM 0-3 */
            if (memory->mapper.ram.enabled)
            {
                memory->ram.data[(memory->mapper.ram.bank * 0x2000) + (address - 0xA000)] = value;
            }
            break;
        default:
            break;
    }
}

static uint8_t dmgl_memory_mapper_mbc2_read(const dmgl_memory_t *const memory, uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case 0x0000 ... 0x3FFF: /* ROM 0 */
            result = memory->rom.data[address];
            break;
        case 0x4000 ... 0x7FFF: /* ROM 1-15 */
            result = memory->rom.data[(memory->mapper.rom.bank[1] * 0x4000) + (address - 0x4000)];
            break;
        case 0xA000 ... 0xBFFF: /* RAM 0 */
            if (memory->mapper.ram.enabled)
            {
                result = 0xF0 | memory->ram.data[(address - 0xA000) & 511];
            }
            break;
        default:
            break;
    }
    return result;
}

static void dmgl_memory_mapper_mbc2_update(dmgl_memory_t *const memory)
{
    if (!memory->mapper.rom.bank[1])
    { /* BANK 0->1 */
        ++memory->mapper.rom.bank[1];
    }
    memory->mapper.rom.bank[1] &= memory->rom.count - 1;
}

static void dmgl_memory_mapper_mbc2_write(dmgl_memory_t *const memory, uint16_t address, uint8_t value)
{
    switch (address)
    {
        case 0x0000 ... 0x3FFF:
            if (address & 0x0100)
            { /* ROM BANK */
                memory->mapper.rom.bank[1] = value & 15;
                dmgl_memory_mapper_mbc2_update(memory);
            }
            else
            { /* RAM ENABLE */
                memory->mapper.ram.enabled = ((value & 0x0F) == 0x0A);
            }
            break;
        case 0xA000 ... 0xBFFF: /* RAM 0 */
            if (memory->mapper.ram.enabled)
            {
                memory->ram.data[(address - 0xA000) & 511] = 0xF0 | value;
            }
            break;
        default:
            break;
    }
}

static uint8_t dmgl_memory_mapper_mbc3_read(const dmgl_memory_t *const memory, uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case 0x0000 ... 0x3FFF: /* ROM 0 */
            result = memory->rom.data[(memory->mapper.rom.bank[0] * 0x4000) + address];
            break;
        case 0x4000 ... 0x7FFF: /* ROM 1-127 */
            result = memory->rom.data[(memory->mapper.rom.bank[1] * 0x4000) + (address - 0x4000)];
            break;
        case 0xA000 ... 0xBFFF: /* RAM/CLOCK */
            if (memory->mapper.ram.enabled)
            {
                switch (memory->mapper.bank.select)
                {
                    case 0x08: /* SECOND */
                        result = memory->clock.latch.second.raw;
                        break;
                    case 0x09: /* MINUTE */
                        result = memory->clock.latch.minute.raw;
                        break;
                    case 0x0A: /* HOUR */
                        result = memory->clock.latch.hour.raw;
                        break;
                    case 0x0B: /* DAY LOW */
                        result = memory->clock.latch.day.low;
                        break;
                    case 0x0C: /* DAY HIGH */
                        result = memory->clock.latch.day.high;
                        break;
                    default: /* RAM 0-3 */
                        result = memory->ram.data[(memory->mapper.ram.bank * 0x2000) + (address - 0xA000)];
                        break;
                }
            }
            break;
        default:
            break;
    }
    return result;
}

static void dmgl_memory_mapper_mbc3_update(dmgl_memory_t *const memory)
{
    if (!memory->mapper.rom.bank[1])
    { /* BANK 0->1 */
        ++memory->mapper.rom.bank[1];
    }
    memory->mapper.ram.bank &= memory->ram.count - 1;
    memory->mapper.rom.bank[1] &= memory->rom.count - 1;
}

static void dmgl_memory_mapper_mbc3_write(dmgl_memory_t *const memory, uint16_t address, uint8_t value)
{
    switch (address)
    {
        case 0x0000 ... 0x1FFF: /* RAM/CLOCK ENABLE */
            memory->mapper.ram.enabled = ((value & 0x0F) == 0x0A);
            break;
        case 0x2000 ... 0x3FFF: /* ROM BANK */
            memory->mapper.rom.bank[1] = value & 127;
            dmgl_memory_mapper_mbc3_update(memory);
            break;
        case 0x4000 ... 0x5FFF: /* BANK SELECT */
            switch (value)
            {
                case 0x08 ... 0x0C: /* CLOCK BANK */
                    memory->mapper.bank.select = value;
                    break;
                default: /* RAM BANK */
                    memory->mapper.ram.bank = value & 3;
                    memory->mapper.bank.select = 0;
                    dmgl_memory_mapper_mbc3_update(memory);
                    break;
            }
            break;
        case 0x6000 ... 0x7FFF: /* CLOCK LATCH */
            if (!value && !memory->clock.latched)
            {
                memory->clock.latched = true;
            }
            else if (value && memory->clock.latched)
            {
                memory->clock.latched = false;
                memcpy(&memory->clock.latch, memory->clock.data, sizeof (*memory->clock.data));
            }
            break;
        case 0xA000 ... 0xBFFF: /* RAM/CLOCK */
            if (memory->mapper.ram.enabled)
            {
                switch (memory->mapper.bank.select)
                {
                    case 0x08: /* SECOND */
                        memory->clock.data->second.counter = value;
                        break;
                    case 0x09: /* MINUTE */
                        memory->clock.data->minute.counter = value;
                        break;
                    case 0x0A: /* HOUR */
                        memory->clock.data->hour.counter = value;
                        break;
                    case 0x0B: /* DAY LOW */
                        memory->clock.data->day.low = value;
                        break;
                    case 0x0C: /* DAY HIGH */
                        memory->clock.data->day.high = value & 193;
                        break;
                    default: /* RAM 0-3 */
                        memory->ram.data[(memory->mapper.ram.bank * 0x2000) + (address - 0xA000)] = value;
                        break;
                }
            }
            break;
        default:
            break;
    }
}

static uint8_t dmgl_memory_mapper_mbc5_read(const dmgl_memory_t *const memory, uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case 0x0000 ... 0x3FFF: /* ROM 0 */
            result = memory->rom.data[(memory->mapper.rom.bank[0] * 0x4000) + address];
            break;
        case 0x4000 ... 0x7FFF: /* ROM 0-511 */
            result = memory->rom.data[(memory->mapper.rom.bank[1] * 0x4000) + (address - 0x4000)];
            break;
        case 0xA000 ... 0xBFFF: /* RAM 0-15 */
            if (memory->mapper.ram.enabled)
            {
                result = memory->ram.data[(memory->mapper.ram.bank * 0x2000) + (address - 0xA000)];
            }
            break;
        default:
            break;
    }
    return result;
}

static void dmgl_memory_mapper_mbc5_update(dmgl_memory_t *const memory)
{
    memory->mapper.rom.bank[1] = ((memory->mapper.bank.high & 1) << 8) | memory->mapper.bank.low;
    memory->mapper.ram.bank &= memory->ram.count - 1;
    memory->mapper.rom.bank[0] &= memory->rom.count - 1;
    memory->mapper.rom.bank[1] &= memory->rom.count - 1;
}

static void dmgl_memory_mapper_mbc5_write(dmgl_memory_t *const memory, uint16_t address, uint8_t value)
{
    switch (address)
    {
        case 0x0000 ... 0x1FFF: /* RAM ENABLE */
            memory->mapper.ram.enabled = ((value & 0x0F) == 0x0A);
            break;
        case 0x2000 ... 0x2FFF: /* ROM LOW BANK */
            memory->mapper.bank.low = value;
            dmgl_memory_mapper_mbc5_update(memory);
            break;
        case 0x3000 ... 0x3FFF: /* ROM HIGH BANK */
            memory->mapper.bank.high = value;
            dmgl_memory_mapper_mbc5_update(memory);
            break;
        case 0x4000 ... 0x5FFF: /* RAM BANK */
            memory->mapper.ram.bank = value & 15;
            dmgl_memory_mapper_mbc5_update(memory);
            break;
        case 0xA000 ... 0xBFFF: /* RAM 0-15 */
            if (memory->mapper.ram.enabled)
            {
                memory->ram.data[(memory->mapper.ram.bank * 0x2000) + (address - 0xA000)] = value;
            }
            break;
        default:
            break;
    }
}

static int dmgl_memory_initialize_mapper(dmgl_memory_t *const memory, const dmgl_t *const context)
{
    int result = EXIT_SUCCESS;
    uint8_t id = dmgl_memory_cartridge(context->rom.data)->id;
    switch (id)
    {
        case 0: /* MBC0 */
        case 8 ... 9:
            memory->mapper.read = dmgl_memory_mapper_mbc0_read;
            memory->mapper.write = dmgl_memory_mapper_mbc0_write;
            break;
        case 1 ... 3: /* MBC1 */
            memory->mapper.read = dmgl_memory_mapper_mbc1_read;
            memory->mapper.write = dmgl_memory_mapper_mbc1_write;
            break;
        case 5 ... 6: /* MBC2 */
            memory->mapper.read = dmgl_memory_mapper_mbc2_read;
            memory->mapper.write = dmgl_memory_mapper_mbc2_write;
            break;
        case 15 ... 19: /* MBC3 */
            memory->mapper.read = dmgl_memory_mapper_mbc3_read;
            memory->mapper.write = dmgl_memory_mapper_mbc3_write;
            break;
        case 25 ... 30: /* MBC5 */
            memory->mapper.read = dmgl_memory_mapper_mbc5_read;
            memory->mapper.write = dmgl_memory_mapper_mbc5_write;
            break;
        default:
            result = DMGL_ERROR("Unsupported mapper id -- %u", id);
            break;
    }
    memory->mapper.ram.bank = 0;
    memory->mapper.ram.enabled = false;
    memory->mapper.rom.bank[0] = 0;
    memory->mapper.rom.bank[1] = 1;
    return result;
}

static int dmgl_memory_initialize_ram(dmgl_memory_t *const memory, dmgl_t *const context)
{
    dmgl_save_t *save = NULL;
    uint32_t expected = sizeof (RAM) / sizeof (*RAM);
    const dmgl_cartridge_t *cartridge = dmgl_memory_cartridge(context->rom.data);
    if (cartridge->ram >= expected)
    {
        return DMGL_ERROR("Invalid ram type -- %u (expecting < %u)", cartridge->ram, expected);
    }
    expected = 0;
    memory->ram.count = RAM[cartridge->ram];
    if (!context->ram.data)
    {
        return DMGL_ERROR("Invalid ram data -- %p", context->ram.data);
    }
    expected = (memory->ram.count * 0x2000) + sizeof (dmgl_save_t);
    if (context->ram.length < expected)
    {
        return DMGL_ERROR("Invalid ram length -- %u bytes (expecting >= %u bytes)", context->ram.length, expected);
    }
    save = (dmgl_save_t *)context->ram.data;
    strcpy((char *)save->magic, "sav");
    save->length = expected;
    save->flag.major = DMGL_MAJOR;
    save->flag.minor = DMGL_MINOR;
    memory->clock.data = &save->clock;
    memory->ram.data = ((uint8_t *)save) + sizeof (dmgl_save_t);
    context->ram.length = expected;
    if (!cartridge->ram)
    {
        memset(memory->ram.data, 0xFF, 0x2000);
    }
    return EXIT_SUCCESS;
}

static int dmgl_memory_initialize_rom(dmgl_memory_t *const memory, const dmgl_t *const context)
{
    uint8_t checksum = 0;
    uint32_t expected = 0x4000;
    const dmgl_cartridge_t *cartridge = NULL;
    if (!context->rom.data)
    {
        return DMGL_ERROR("Invalid rom data -- %p", context->rom.data);
    }
    if (context->rom.length < expected)
    {
        return DMGL_ERROR("Invalid rom length -- %u bytes (expecting >= %u bytes)", context->rom.length, expected);
    }
    cartridge = dmgl_memory_cartridge(context->rom.data);
    if ((checksum = dmgl_memory_checksum(context->rom.data, 0x0134, 0x014C)) != cartridge->checksum)
    {
        return DMGL_ERROR("Mismatched rom checksum -- %02X (expecting %02X)", checksum, cartridge->checksum);
    }
    expected = sizeof (ROM) / sizeof (*ROM);
    if (cartridge->rom >= expected)
    {
        return DMGL_ERROR("Invalid rom type -- %u (expecting < %u)", cartridge->rom, expected);
    }
    memory->rom.count = ROM[cartridge->rom];
    expected = memory->rom.count * 0x4000;
    if (context->rom.length != expected)
    {
        return DMGL_ERROR("Invalid rom length -- %u bytes (expecting %u bytes)", context->rom.length, expected);
    }
    memory->rom.data = context->rom.data;
    return EXIT_SUCCESS;
}

static void dmgl_memory_initialize_title(dmgl_memory_t *const memory, const dmgl_t *const context)
{
    const dmgl_cartridge_t *cartridge = dmgl_memory_cartridge(context->rom.data);
    for (uint32_t index = 0; index < sizeof (cartridge->title); ++index)
    {
        uint8_t value = cartridge->title[index];
        if (!isprint(value))
        {
            break;
        }
        memory->title[index] = !isspace(value) ? value : ' ';
    }
}

void dmgl_memory_clock(dmgl_memory_t *const memory)
{
    if (!memory->clock.data->day.halt)
    {
        if (!memory->clock.delay)
        {
            if (++memory->clock.data->second.counter == 60)
            {
                memory->clock.data->second.counter = 0;
                if (++memory->clock.data->minute.counter == 60)
                {
                    memory->clock.data->minute.counter = 0;
                    if (++memory->clock.data->hour.counter == 24)
                    {
                        memory->clock.data->hour.counter = 0;
                        if ((memory->clock.data->day.carry = (memory->clock.data->day.counter == 511)))
                        {
                            memory->clock.data->day.counter = 0;
                        }
                        else
                        {
                            ++memory->clock.data->day.counter;
                        }
                    }
                }
            }
            memory->clock.delay = 60; /* 1 Hz */
        }
        --memory->clock.delay;
    }
}

int dmgl_memory_initialize(dmgl_memory_t *const memory, dmgl_t *const context)
{
    int result = EXIT_SUCCESS;
    if ((result = dmgl_memory_initialize_rom(memory, context)) != EXIT_SUCCESS)
    {
        return result;
    }
    if ((result = dmgl_memory_initialize_ram(memory, context)) != EXIT_SUCCESS)
    {
        return result;
    }
    if ((result = dmgl_memory_initialize_mapper(memory, context)) != EXIT_SUCCESS)
    {
        return result;
    }
    dmgl_memory_initialize_title(memory, context);
    memory->bootrom.enabled = true;
    return result;
}

uint8_t dmgl_memory_read(const dmgl_memory_t *const memory, uint16_t address)
{
    uint8_t result = 0xFF;
    switch (address)
    {
        case 0x0000 ... 0x00FF: /* BOOTROM/ROM */
            if (memory->bootrom.enabled)
            {
                result = BOOTROM[address];
            }
            else
            {
                result = memory->mapper.read(memory, address);
            }
            break;
        case 0xC000 ... 0xDFFF: /* WORK RAM */
            result = memory->ram.work[address - 0xC000];
            break;
        case 0xE000 ... 0xFDFF: /* WORK RAM (MIRROR) */
            result = memory->ram.work[address - 0xE000];
            break;
        case 0xFEA0 ... 0xFEFF: /* UNUSED */
            result = 0;
            break;
        case 0xFF80 ... 0xFFFE: /* HIGH RAM */
            result = memory->ram.high[address - 0xFF80];
            break;
        default: /* RAM/ROM */
            result = memory->mapper.read(memory, address);
            break;
    }
    return result;
}

const char *dmgl_memory_title(const dmgl_memory_t *const memory)
{
    return strlen(memory->title) ? memory->title : "UNTITLED";
}

void dmgl_memory_write(dmgl_memory_t *const memory, uint16_t address, uint8_t value)
{
    switch (address)
    {
        case 0xC000 ... 0xDFFF: /* WORK RAM */
            memory->ram.work[address - 0xC000] = value;
            break;
        case 0xE000 ... 0xFDFF: /* WORK RAM (MIRROR) */
            memory->ram.work[address - 0xE000] = value;
            break;
        case 0xFEA0 ... 0xFEFF: /* UNUSED */
            break;
        case 0xFF50: /* BOOTROM DISABLE */
            if (value)
            {
                memory->bootrom.enabled = false;
            }
            break;
        case 0xFF80 ... 0xFFFE: /* HIGH RAM */
            memory->ram.high[address - 0xFF80] = value;
            break;
        default: /* RAM/ROM */
            memory->mapper.write(memory, address, value);
            break;
    }
}
