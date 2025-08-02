#include "include/displayManager.h"

void setDefaultDisplaySettings(DisplayManager display) {
    if (!display) {
        printf("Display manager is NULL, cannot set default settings\n");
        return;
    }

    display->currentMode.w = 1280;  // default width
    display->currentMode.h = 720;  // default height
    display->currentMode.refresh_rate = 60;  // default refresh rate
    display->currentMode.format = SDL_PIXELFORMAT_RGBA8888;  // default pixel format
    display->currentMode.driverdata = NULL;  // should be initialized by SDL
    display->wdwFlags = SDL_WINDOW_SHOWN;  // windowed mode by default
}

void toggleFullscreen(DisplayManager mgr) {
    if (!mgr) return;

    int curr = mgr->wdwFlags & SDL_WINDOW_FULLSCREEN;
    if (curr) {
        SDL_SetWindowFullscreen(mgr->window, SDL_WINDOW_SHOWN);
        mgr->wdwFlags &= ~SDL_WINDOW_FULLSCREEN;
    }
    else {
        SDL_SetWindowFullscreen(mgr->window, SDL_WINDOW_FULLSCREEN);
        mgr->wdwFlags |= SDL_WINDOW_FULLSCREEN;
    }
}

SDL_DisplayMode* getAvailableDisplayModes(DisplayManager mgr, int *count) {
    if (!mgr || !count) return NULL;

    SDL_DisplayMode *availableModes = NULL;
    int modeCount = SDL_GetNumDisplayModes(0);  // 0 for the primary display
    if (modeCount < 0) {
        printf("SDL_GetNumDisplayModes failed: %s\n", SDL_GetError());
        return NULL;
    }

    typedef struct {
        int w, h;
    } Resolution;  // helper struct to store resolutions

    Resolution neededResolutions[] = {
        {3840, 2160},
        {3200, 1200},
        {2560, 1440},
        {1920, 1080},
        {1600, 900},
        {1366, 768},
        {1280, 720},
        {1024, 576}
    };
    int neededCount = sizeof(neededResolutions) / sizeof(Resolution);
    *count = 0;
    SDL_DisplayMode *neededModes = calloc(neededCount, sizeof(SDL_DisplayMode));
    if (!neededModes) {
        printf("Failed to allocate memory for display neededModes\n");
        return NULL;
    }

    availableModes = malloc(modeCount * sizeof(SDL_DisplayMode));
    if (!availableModes) {
        printf("Failed to allocate memory for display availableModes\n");
        return NULL;
    }

    for (int i = 0; i < modeCount; i++) {
        if (SDL_GetDisplayMode(0, i, &availableModes[i]) != 0) {
            printf("SDL_GetDisplayMode failed: %s\n", SDL_GetError());
            free(availableModes);
            free(neededModes);
            return NULL;
        }
        if (
            (double)availableModes[i].w / availableModes[i].h < 1.8
            && (double)availableModes[i].w / availableModes[i].h > 1.7
        ) {
            // 16:9 ratio with a big enough decimal tolerance
            neededModes[(*count)++] = availableModes[i];
        }
        if ((availableModes[i].w <= 1024) && (availableModes[i].h <= 576)) {
            // Skip availableModes that are too small
            break;
        }
    }
    if (neededCount != *count) {
        neededModes = realloc(neededModes, *count * sizeof(SDL_DisplayMode));
        if (!neededModes) {
            printf("Failed to reallocate memory for neededModes\n");
            free(availableModes);
            return NULL;
        }
    }
    free(availableModes);
    return neededModes;
}

void setDisplayMode(DisplayManager mgr, const SDL_DisplayMode *mode) {
    if (!mgr || !mode) return;

    mgr->currentMode = *mode;

    // If fullscreen mode switch to windowed mode to change the size
    Uint8 isFullscreen = mgr->wdwFlags & SDL_WINDOW_FULLSCREEN;
    if (isFullscreen) SDL_SetWindowFullscreen(mgr->window, SDL_WINDOW_SHOWN);

    // Then set the new size
    SDL_SetWindowSize(mgr->window, mgr->currentMode.w, mgr->currentMode.h);

    // Switch back to fullscreen mode with the new resolution if was fullscreen
    if (isFullscreen) {
        SDL_SetWindowDisplayMode(mgr->window, mode);
        SDL_SetWindowFullscreen(mgr->window, SDL_WINDOW_FULLSCREEN);
        mgr->wdwFlags |= SDL_WINDOW_FULLSCREEN;  // just in case
    }

    // SDL has some utility in this case
    SDL_RenderSetLogicalSize(mgr->renderer, mgr->currentMode.w, mgr->currentMode.h);

    printf(
        "Display mode set to %dx%d @ %dHz\n",
        mgr->currentMode.w, mgr->currentMode.h, mgr->currentMode.refresh_rate
    );
}

// void handleDisplayEvent(DisplayManager mgr, SDL_Event *event) {
//     if (!mgr || event->type != SDL_WINDOWEVENT) return;

//     switch (event->window.event) {
//         case SDL_WINDOWEVENT_RESIZED:
//             mgr->width = event->window.data1;
//             mgr->height = event->window.data2;
//             SDL_RenderSetLogicalSize(mgr->renderer, mgr->width, mgr->height);
//             break;
//     }
// }

void saveDisplaySettings(DisplayManager mgr, const char *filePath) {
    if (!mgr || !filePath) return;

    FILE *fout = fopen(filePath, "a");
    if (!fout) {
        printf("Failed to open config file for writing: %s\n", filePath);
        return;
    }

    fprintf(fout, "[DISPLAY]\n");
    fprintf(fout, "WIDTH=%d\n", mgr->currentMode.w);
    fprintf(fout, "HEIGHT=%d\n", mgr->currentMode.h);
    fprintf(fout, "FULLSCREEN=%d\n", (mgr->wdwFlags & SDL_WINDOW_FULLSCREEN) ? 1 : 0);
    fprintf(fout, "VSYNC=%d\n", (mgr->wdwFlags & SDL_RENDERER_PRESENTVSYNC) ? 1 : 0);

    fclose(fout);
}