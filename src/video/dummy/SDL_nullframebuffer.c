/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_DUMMY

#include "../SDL_sysvideo.h"
#include "SDL_nullframebuffer_c.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

#define DUMMY_SURFACE   "_SDL_DummySurface"

int SDL_DUMMY_CreateWindowFramebuffer(_THIS, SDL_Window * window, Uint32 * format, void ** pixels, int *pitch)
{
    SDL_Surface *surface;
    const Uint32 surface_format = SDL_PIXELFORMAT_RGB888;
    int w, h;
    int bpp;
    Uint32 Rmask, Gmask, Bmask, Amask;

    /* Free the old framebuffer surface */
    surface = (SDL_Surface *) SDL_GetWindowData(window, DUMMY_SURFACE);
    SDL_FreeSurface(surface);

    /* Create a new one */
    SDL_PixelFormatEnumToMasks(surface_format, &bpp, &Rmask, &Gmask, &Bmask, &Amask);
    SDL_GetWindowSize(window, &w, &h);
    surface = SDL_CreateRGBSurface(0, w, h, bpp, Rmask, Gmask, Bmask, Amask);
    if (!surface) {
        return -1;
    }

    /* Save the info and return! */
    SDL_SetWindowData(window, DUMMY_SURFACE, surface);
    *format = surface_format;
    *pixels = surface->pixels;
    if (SDL_getenv("SDL_VIDEO_DUMMY_FRAMEBUFFER"))
    {
        char *path = SDL_getenv("SDL_VIDEO_DUMMY_FRAMEBUFFER");
        const size_t allocsize = surface->h * surface->pitch;
        int fd = open(path, O_RDWR | O_CREAT, 0644);
        if (ftruncate(fd, allocsize))
            perror("ftruncate");
        *pixels = mmap(0, allocsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        close(fd);
        printf("Framebuffer: %s, %i x %i\n", path, w, h);
        printf("while convert -size %ix%i -depth 8 -alpha off rgba:%s %s.tmp.png; do mv %s.tmp.png %s.png; done\n", w, h, path, path, path, path);
    }
    *pitch = surface->pitch;
    return 0;
}

int SDL_DUMMY_UpdateWindowFramebuffer(_THIS, SDL_Window * window, const SDL_Rect * rects, int numrects)
{
    static int frame_number;
    SDL_Surface *surface;

    surface = (SDL_Surface *) SDL_GetWindowData(window, DUMMY_SURFACE);
    if (!surface) {
        return SDL_SetError("Couldn't find dummy surface for window");
    }

    if (SDL_getenv("SDL_VIDEO_DUMMY_UPDATES"))
        for (int i = 0; i < numrects; i++)
        {
            printf("x: %i, y: %i, w: %i, h: %i\n", rects[i].x, rects[i].y, rects[i].w, rects[i].h);
        }

    /* Send the data to the display */
    if (SDL_getenv("SDL_VIDEO_DUMMY_SAVE_FRAMES")) {
        char file[128];
        SDL_snprintf(file, sizeof(file), "SDL_window%" SDL_PRIu32 "-%8.8d.bmp",
                     SDL_GetWindowID(window), ++frame_number);
        SDL_SaveBMP(surface, file);
    }
    return 0;
}

void SDL_DUMMY_DestroyWindowFramebuffer(_THIS, SDL_Window * window)
{
    SDL_Surface *surface;

    surface = (SDL_Surface *) SDL_SetWindowData(window, DUMMY_SURFACE, NULL);
    SDL_FreeSurface(surface);
}

#endif /* SDL_VIDEO_DRIVER_DUMMY */

/* vi: set ts=4 sw=4 expandtab: */
