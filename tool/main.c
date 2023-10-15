
/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <client.h>
#include "pokered.h"

static const char *DESCRIPTION[] =
{
    "Show help information",
    "Set window palette",
    "Set window scaling",
    "Show version information",
};

static const struct option OPTION[] =
{
    { "help", no_argument, NULL, 'h', },
    { "palette", required_argument, NULL, 'p', },
    { "scale", required_argument, NULL, 's', },
    { "version", no_argument, NULL, 'v', },
    { NULL, 0, NULL, 0, },
};

static struct
{
    char *path[2];
    dmgl_t context;
}
g_main =
{
    .context =
    {
        .client =
        {
            .initialize = client_initialize,
            .output = client_output,
            .poll = client_poll,
            .sync = client_sync,
            .uninitialize = client_uninitialize,
        },
    },
};

static void *buffer_allocate(uint32_t length)
{
    void *result = NULL;
    if (!(result = calloc(length, sizeof (uint8_t))))
    {
        fprintf(stderr, "Failed to allocate buffer -- %u bytes\n", length);
    }
    return result;
}

static void buffer_free(void *buffer)
{
    free(buffer);
}

static bool file_exists(const char *const path)
{
    FILE *file = NULL;
    if ((file = fopen(path, "rb")))
    {
        fclose(file);
        return true;
    }
    return false;
}

static bool file_read(const char *const path, uint8_t **data, uint32_t *length)
{
    FILE *file = NULL;
    bool result = false;
    if (!(file = fopen(path, "rb")))
    {
        fprintf(stderr, "Failed to open file -- %s\n", path);
        return result;
    }
    fseek(file, 0, SEEK_END);
    *length = ftell(file);
    fseek(file, 0, SEEK_SET);
    if ((*data = buffer_allocate(*length)))
    {
        if (!(result = (fread(*data, sizeof (**data), *length, file) == *length)))
        {
            fprintf(stderr, "Failed to read file -- %s\n", path);
        }
    }
    fclose(file);
    return result;
}

static bool file_write(const char *const path, const uint8_t *const data, uint32_t length)
{
    FILE *file = NULL;
    bool result = false;
    if (!(file = fopen(path, "wb")))
    {
        fprintf(stderr, "Failed to open file -- %s\n", path);
        return result;
    }
    if (!(result = (fwrite(data, sizeof (*data), length, file) == length)))
    {
        fprintf(stderr, "Failed to write file -- %s\n", path);
    }
    fclose(file);
    return result;
}

static int ram_load(const char *const path, uint8_t **data, uint32_t *length)
{
    if (!file_exists(path))
    {
        *length = 17 * 0x2000;
        if (!(*data = buffer_allocate(*length)))
        {
            return EXIT_FAILURE;
        }
    }
    else if (!file_read(path, data, length))
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static int ram_save(const char *const path, const uint8_t *const data, uint32_t length)
{
    if (!file_write(path, data, length))
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static int rom_load( uint8_t **data, uint32_t *length)
{
   *length = 1048576;
    *data = (uint8_t *)pokered_gb;
    return EXIT_SUCCESS;
}

static int run(void)
{
    int result = EXIT_SUCCESS;
    if ((result = rom_load(&g_main.context.rom.data, &g_main.context.rom.length)) == EXIT_SUCCESS)
    {
        if ((result = ram_load(g_main.path[1], &g_main.context.ram.data, &g_main.context.ram.length)) == EXIT_SUCCESS)
        {
            if ((result = dmgl(&g_main.context)) == EXIT_SUCCESS)
            {
                result = ram_save(g_main.path[1], g_main.context.ram.data, g_main.context.ram.length);
            }
            else
            {
                fprintf(stderr, "%s\n", dmgl_error());
            }
            buffer_free(g_main.context.ram.data);
        }
        buffer_free(g_main.context.rom.data);
    }
    return result;
}

static void usage(void)
{
   fprintf(stdout,"ONLY USE FOR TESTING PURPOSES\n");
}

static void version(void)
{
    const dmgl_version_t *const version = dmgl_version();
    fprintf(stdout, "%u.%u-%x\n", version->major, version->minor, version->patch);
}

int main(int argc, char *argv[])
{   argv[1] = "pokered.gb";
    argc += 1;

    uint32_t length = 0;
    int option = 0, result = EXIT_SUCCESS;
    while ((option = getopt_long(argc, argv, "hp:s:v", OPTION, NULL)) != -1)
    {
        switch (option)
        {
            case 'h': /* HELP */
                usage();
                return EXIT_SUCCESS;
            case 'p': /* PALETTE */
                g_main.context.palette = strtol(optarg, NULL, 10);
                break;
            case 's': /* SCALE */
                g_main.context.scale = strtol(optarg, NULL, 10);
                break;
            case 'v': /* VERSION */
                version();
                return EXIT_SUCCESS;
            case '?':
            default:
                return EXIT_FAILURE;
        }
    }
    
    for (option = optind; option < argc; ++option)
    {
        if (g_main.path[0])
        {
            usage();
            return EXIT_FAILURE;
        }
        g_main.path[0] = argv[option];
    }
    if (!g_main.path[0] || !strlen(g_main.path[0]))
    {
        usage();
        return EXIT_FAILURE;
    }
    length = strlen(g_main.path[0]) + strlen(".sav") + 1;
    if (!(g_main.path[1] = buffer_allocate(length)))
    {
        return EXIT_FAILURE;
    }
    snprintf(g_main.path[1], length, "%s.sav", g_main.path[0]);
    result = run();
    buffer_free(g_main.path[1]);
    return result;
}
