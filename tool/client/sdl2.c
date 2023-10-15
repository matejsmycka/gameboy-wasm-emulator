/*
 * SPDX-FileCopyrightText: 2023 David Jolly <majestic53@gmail.com>
 * SPDX-License-Identifier: MIT
 */

#ifdef CLIENT_SDL2

#include <SDL.h>
#include <stdbool.h>

static const uint32_t PALETTE[][4] =
{
    { /* GREEN */
        /* WHITE    LIGHT-GREY  DARK-GREY   BLACK */
        0xFFE0F8D0, 0xFF88C070, 0xFF346856, 0xFF081820,
    }
};

static const SDL_Scancode SCANCODE[] =
{
    /* A            B               SELECT          START */
    SDL_SCANCODE_L, SDL_SCANCODE_K, SDL_SCANCODE_J, SDL_SCANCODE_SPACE,
    /* RIGHT        LEFT            UP              DOWN */
    SDL_SCANCODE_RIGHT, SDL_SCANCODE_LEFT, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN,
};

static struct
{
    uint32_t elapsed;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Window *window;
    struct
    {
        SDL_AudioDeviceID device;
        SDL_AudioSpec spec;
    } audio;
} g_client = {};

static int client_initialize_audio(void)
{
    SDL_AudioSpec desired =
    {
        .freq = 44100, .format = AUDIO_F32SYS, .channels = 1, .samples = 4096
    };
    if (!(g_client.audio.device = SDL_OpenAudioDevice(NULL, false, &desired, &g_client.audio.spec, 0)))
    {
        fprintf(stderr, "SDL_OpenAudioDevice failed -- %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_PauseAudioDevice(g_client.audio.device, false);
    return EXIT_SUCCESS;
}

static int client_initialize_video(const char *const title, uint8_t scale)
{
    if (scale < 1)
    {
        scale = 1;
    }
    else if (scale > 8)
    {
        scale = 8;
    }
    if (!(g_client.window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160 * scale, 144 * scale, 0)))
    {
        fprintf(stderr, "SDL_CreateWindow failed -- %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    if (!(g_client.renderer = SDL_CreateRenderer(g_client.window, -1, SDL_RENDERER_PRESENTVSYNC)))
    {
        fprintf(stderr, "SDL_CreateRenderer failed -- %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    if (SDL_RenderSetLogicalSize(g_client.renderer, 160, 144))
    {
        fprintf(stderr, "SDL_RenderSetLogicalSize failed -- %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    if (SDL_SetRenderDrawColor(g_client.renderer, 0, 0, 0, 0))
    {
        fprintf(stderr, "SDL_SetRenderDrawColor failed -- %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    if (SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1") == SDL_FALSE)
    {
        fprintf(stderr, "SDL_SetHint failed -- %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    if (SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0") == SDL_FALSE)
    {
        fprintf(stderr, "SDL_SetHint failed -- %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    if (!(g_client.texture = SDL_CreateTexture(g_client.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 144)))
    {
        fprintf(stderr, "SDL_CreateTexture failed -- %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static int client_sync_audio(const float (*sample)[735])
{
    if (SDL_QueueAudio(g_client.audio.device, sample, sizeof (*sample)))
    {
        fprintf(stderr, "SDL_QueueAudio failed -- %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static int client_sync_video(const uint8_t (*color)[160][144], uint8_t palette)
{
    uint32_t pixel[144][160] = {};
    if (palette >= sizeof (PALETTE) / sizeof (*PALETTE))
    {
        palette = 0;
    }
    for (uint8_t y = 0; y < 144; ++y)
    {
        for (uint8_t x = 0; x < 160; ++x)
        {
            pixel[y][x] = PALETTE[palette][(*color)[x][y]];
        }
    }
    if (SDL_UpdateTexture(g_client.texture, NULL, pixel, 160 * sizeof (uint32_t)))
    {
        fprintf(stderr, "SDL_UpdateTexture failed -- %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    if (SDL_RenderClear(g_client.renderer))
    {
        fprintf(stderr, "SDL_RenderClear failed -- %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    if (SDL_RenderCopy(g_client.renderer, g_client.texture, NULL, NULL))
    {
        fprintf(stderr, "SDL_RenderCopy failed -- %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_RenderPresent(g_client.renderer);
    return EXIT_SUCCESS;
}

static void client_uninitialize_audio(void)
{
    if (g_client.audio.device)
    {
        SDL_PauseAudioDevice(g_client.audio.device, true);
        SDL_CloseAudioDevice(g_client.audio.device);
    }
}

static void client_uninitialize_video(void)
{
    if (g_client.texture)
    {
        SDL_DestroyTexture(g_client.texture);
    }
    if (g_client.renderer)
    {
        SDL_DestroyRenderer(g_client.renderer);
    }
    if (g_client.window)
    {
        SDL_DestroyWindow(g_client.window);
    }
}

int client_initialize(const char *const title, uint8_t scale)
{
    int result = EXIT_SUCCESS;
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO))
    {
        fprintf(stderr, "SDL_Init failed -- %s", SDL_GetError());
        return EXIT_FAILURE;
    }
    if ((result = client_initialize_video(title, scale)) != EXIT_SUCCESS)
    {
        return result;
    }
    if ((result = client_initialize_audio()) != EXIT_SUCCESS)
    {
        return result;
    }
    g_client.elapsed = SDL_GetTicks();
    return result;
}

uint8_t client_output(uint8_t value)
{
    return 1;
}

int client_poll(bool (*state)[8])
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                if (!event.key.repeat)
                {
                    for (uint8_t button = 0; button < 8; ++button)
                    {
                        if (SCANCODE[button] == event.key.keysym.scancode)
                        {
                            (*state)[button] = (event.type == SDL_KEYDOWN);
                            break;
                        }
                    }
                }
                break;
            case SDL_QUIT:
                return EXIT_FAILURE;
            default:
                break;
        }
    }
    return EXIT_SUCCESS;
}

int client_sync(const uint8_t (*color)[160][144], uint8_t palette, const float (*sample)[735])
{
    uint32_t elapsed = 0;
    int result = EXIT_SUCCESS;
    if ((result = client_sync_audio(sample)) != EXIT_SUCCESS)
    {
        return result;
    }
    if ((result = client_sync_video(color, palette)) != EXIT_SUCCESS)
    {
        return result;
    }
    if ((elapsed = (SDL_GetTicks() - g_client.elapsed)) < (1000 / 60))
    {
        SDL_Delay((1000 / 60) - elapsed);
    }
    g_client.elapsed = SDL_GetTicks();
    return result;
}

void client_uninitialize(void)
{
    client_uninitialize_audio();
    client_uninitialize_video();
    SDL_Quit();
}

#endif /* CLIENT_SDL2 */
