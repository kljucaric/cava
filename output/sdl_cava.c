#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#include "sdl_cava.h"//"output/sdl_cava.h"
#include <stdbool.h>
#include <stdlib.h>

#include "../util.h"

SDL_Window *gWindow = NULL;

SDL_Renderer *gRenderer = NULL;

SDL_Event e;

struct colors {
    uint16_t R;
    uint16_t G;
    uint16_t B;
};

struct colors fg_color = {0};
struct colors bg_color = {0};

int bar_sweep = 0;

static void parse_color(char *color_string, struct colors *color) {
    if (color_string[0] == '#') {
        sscanf(++color_string, "%02hx%02hx%02hx", &color->R, &color->G, &color->B);
    }
}

void init_sdl_window(int width, int height, int x, int y) {
    if (x == -1)
        x = SDL_WINDOWPOS_UNDEFINED;

    if (y == -1)
        y = SDL_WINDOWPOS_UNDEFINED;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    } else {
        gWindow =
            SDL_CreateWindow("onion", x, y, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP);
        if (gWindow == NULL) {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        } else {
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
            if (gRenderer == NULL) {
                printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
            }
        }
    }
}

void init_sdl_surface(int *w, int *h, char *const fg_color_string, char *const bg_color_string) {
    SDL_GetWindowSize(gWindow, w, h);

    parse_color(bg_color_string, &bg_color);
    SDL_SetRenderDrawColor(gRenderer, bg_color.R, bg_color.G, bg_color.B, 0xFF);
    SDL_RenderClear(gRenderer);

    parse_color(fg_color_string, &fg_color);
    SDL_SetRenderDrawColor(gRenderer, fg_color.R, fg_color.G, fg_color.B, 0xFF);
}

int draw_sdl(int bars_count, int bar_width, int bar_spacing, int remainder, int height,
             const int bars[], int previous_frame[], int frame_time, enum orientation orientation) {

    bool update = false;
    int rc = 0, color = 0, pos = 0, intense = 0x00;
    SDL_Rect fillRect;

    for (int bar = 0; bar < bars_count; bar++) {
        if (bars[bar] != previous_frame[bar]) {
            update = true;
            break;
        }
    }

    color = (bar_sweep + bars[bar_sweep])  % 3;

    // high freq increase color, low freq decrease color
    if (bar_sweep > bars_count / 2) {
        color = 2 - color;  // inverse color for right channel
        if (bar_sweep > (3 * (bar_sweep / 4))) {
            pos = 0;
        } else {
            pos = 1;
        }
    } else {
        if (bar_sweep > (bar_sweep / 4)) {
            pos = 1;
        } else {
            pos = 0;
        }
    }

    if (bars[bar_sweep] < (height / 4) || bars[bar_sweep] > (3 * (height / 4))) {
        intense = 0x02;
    } else {
        intense = 0x01;
    }

    // color shift smoothing
    switch (color) { 
        case 1:
        if (pos == 1) {
            if (fg_color.G < 0xFE) {
                fg_color.G = fg_color.G + intense;
            }
        } else {
            if (fg_color.G > 0x01) {
                fg_color.G = fg_color.G - intense;
            }
        }
        break;
        case 2:
        if (pos == 1) {
            if (fg_color.B < 0xFE) {
                fg_color.B = fg_color.B + intense;
            }
        } else {
            if (fg_color.B > 0x01) {
                fg_color.B = fg_color.B - intense;
            }
        }
        break;
        default:
        if (pos == 1) {
            if (fg_color.R < 0xFE) {
                fg_color.R = fg_color.R + intense;
            }
        } else {
            if (fg_color.R > 0x01) {
                fg_color.R = fg_color.R - intense;
            }
        }
        break;
    }

    bar_sweep = (bar_sweep + 1) % bars_count;

    if (update) {
        SDL_SetRenderDrawColor(gRenderer, bg_color.R, bg_color.G, bg_color.B, 0xFF);
        SDL_RenderClear(gRenderer);
        for (int bar = 0; bar < bars_count; bar++) {
            switch (orientation) {
            case ORIENT_LEFT:
                fillRect.x = 0;
                fillRect.y = bar * (bar_width + bar_spacing) + remainder;
                fillRect.w = bars[bar];
                fillRect.h = bar_width;
                break;
            case ORIENT_RIGHT:
                fillRect.x = height - bars[bar];
                fillRect.y = bar * (bar_width + bar_spacing) + remainder;
                fillRect.w = bars[bar];
                fillRect.h = bar_width;
                break;
            case ORIENT_TOP:
                fillRect.x = bar * (bar_width + bar_spacing) + remainder;
                fillRect.y = 0;
                fillRect.w = bar_width;
                fillRect.h = bars[bar];
                break;
            default:
                fillRect.x = bar * (bar_width + bar_spacing) + remainder;
                fillRect.y = height - bars[bar];
                fillRect.w = bar_width;
                fillRect.h = bars[bar];
                break;
            }

            SDL_SetRenderDrawColor(gRenderer, fg_color.R, fg_color.G, fg_color.B, 0xFF);
            SDL_RenderFillRect(gRenderer, &fillRect);
        }
        SDL_RenderPresent(gRenderer);
    }

    SDL_Delay(frame_time);

    SDL_PollEvent(&e);
    if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
        rc = -1;
    if (e.type == SDL_QUIT)
        rc = -2;

    return rc;
}

// general: cleanup
void cleanup_sdl(void) {
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
}
