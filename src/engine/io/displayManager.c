#include "displayManager.h"

#include "global/debug.h"

void setDefaultDisplaySettings(DisplayManager display) {
    ASSERT(display != NULL, "Display manager is NULL, cannot set default settings\n");

    display->currentMode.w = 1280;  // default width
    display->currentMode.h = 720;  // default height
    display->currentMode.refresh_rate = 60;  // default refresh rate
    display->currentMode.format = SDL_PIXELFORMAT_RGBA8888;  // default pixel format
    display->currentMode.driverdata = NULL;  // should be initialized by SDL
    display->wdwFlags = SDL_WINDOW_SHOWN;  // windowed mode by default
    display->fullscreen = 0;
    display->vsync = 0;
}

/**
 * =====================================================================================================================
 */

void toggleFullscreen(DisplayManager mgr) {
    ASSERT(mgr != NULL, "Display manager reference was lost somehow");

    int isFullscreen = mgr->wdwFlags & SDL_WINDOW_FULLSCREEN;
    if (isFullscreen) {
        SDL_SetWindowFullscreen(mgr->window, 0);
        mgr->wdwFlags &= ~SDL_WINDOW_FULLSCREEN;
        mgr->fullscreen = 0;
    }
    else {
        SDL_SetWindowFullscreen(mgr->window, SDL_WINDOW_FULLSCREEN);
        mgr->wdwFlags |= SDL_WINDOW_FULLSCREEN;
        mgr->fullscreen = 1;
    }
}

/**
 * =====================================================================================================================
 */

SDL_DisplayMode* getAvailableDisplayModes(DisplayManager mgr, int *count) {
    ASSERT(mgr && count, "mgr = %p, count = %p\n", mgr, count);

    SDL_DisplayMode *availableModes = NULL;
    int modeCount = SDL_GetNumDisplayModes(0);  // 0 for the primary display
    if (modeCount < 0) {
        LOG(ERROR, "SDL_GetNumDisplayModes failed: %s\n", SDL_GetError());
        return NULL;
    }

    typedef struct Resolution {
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
    ASSERT(neededModes != NULL, "Failed to allocate memory for needed modes");

    availableModes = calloc(modeCount, sizeof(SDL_DisplayMode));
    ASSERT(availableModes != NULL, "Failed to allocate memory for available modes");

    for (int i = 0; i < modeCount; i++) {
        int successVal = SDL_GetDisplayMode(0, i, &availableModes[i]);
        if (successVal != 0) {
            LOG(ERROR, "SDL_GetDisplayMode failed: '%s'\n", SDL_GetError());
            return NULL;
        }

        SDL_DisplayMode availableMode = availableModes[i];

        if ((double)availableMode.w / availableMode.h < 1.8
            && (double)availableMode.w / availableMode.h > 1.7)
            neededModes[(*count)++] = availableMode;  // 16:9 ratio with a big enough decimal tolerance
        if ((availableMode.w <= 1024) && (availableMode.h <= 576)) {
            LOG(DEBUG, "Skipping unsatisfying resolution %dx%d @%dHz\n",
                availableMode.w, availableMode.h, availableMode.refresh_rate);
            break;
        }
    }
    if (neededCount != *count) {
        neededModes = realloc(neededModes, *count * sizeof(SDL_DisplayMode));
        ASSERT(neededModes != NULL, "Failed to resize needed modes");
    }
    free(availableModes);
    return neededModes;
}

/**
 * =====================================================================================================================
 */

void setDisplayMode(DisplayManager mgr, const SDL_DisplayMode *mode) {
    ASSERT(mgr && mode, "mgr = %p, node = %p\n", mgr, mode);
    mgr->currentMode = *mode;

    // If fullscreen mode switch to windowed mode to change the size
    Uint8 isFullscreen = mgr->wdwFlags & SDL_WINDOW_FULLSCREEN;
    if (isFullscreen) SDL_SetWindowFullscreen(mgr->window, 0);

    // Then set the new size
    SDL_SetWindowDisplayMode(mgr->window, mode);
    SDL_SetWindowSize(mgr->window, mgr->currentMode.w, mgr->currentMode.h);

    // Switch back to fullscreen mode with the new resolution if was fullscreen
    if (isFullscreen) {
        SDL_SetWindowFullscreen(mgr->window, SDL_WINDOW_FULLSCREEN);
        mgr->wdwFlags |= SDL_WINDOW_FULLSCREEN;  // just in case
    }

    LOG(INFO, "Display mode set to %dx%d @%dHz\n",
        mgr->currentMode.w, mgr->currentMode.h, mgr->currentMode.refresh_rate);
}

/**
 * =====================================================================================================================
 */

void saveDisplaySettings(DisplayManager mgr, const char *filePath) {
    if (!mgr || !filePath) {
        LOG(ERROR, "Display manager or filePath is NULL");
        return;
    }

    FILE *fout = fopen(filePath, "a");
    if (!fout) {
        LOG(ERROR, "Failed to open config file for writing: %s\n", filePath);
        return;
    }

    fprintf(fout, "[DISPLAY]\n");
    fprintf(fout, "WIDTH=%d\n", mgr->currentMode.w);
    fprintf(fout, "HEIGHT=%d\n", mgr->currentMode.h);
    fprintf(fout, "FULLSCREEN=%d\n", (mgr->wdwFlags & SDL_WINDOW_FULLSCREEN) ? 1 : 0);
    // TODO: This looks sus
    fprintf(fout, "VSYNC=%d\n", (mgr->wdwFlags & SDL_RENDERER_PRESENTVSYNC) ? 1 : 0);

    fclose(fout);
}