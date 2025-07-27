#include "include/displayManager.h"

void setDefaultDisplaySettings(DisplayManager display) {
    if (!display) {
        printf("Display manager is NULL, cannot set default settings\n");
        return;
    }

    display->width = 1280;  // default width
    display->height = 720;  // default height
    display->wdwFlags = SDL_WINDOW_SHOWN;  // windowed mode by default
}

void setWindowSize(DisplayManager mgr, int width, int height) {
    if (!mgr) return;

    SDL_SetWindowSize(mgr->window, width, height);
    mgr->width = width;
    mgr->height = height;
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

void setFullscreenResolution(DisplayManager mgr, int width, int height) {
    if (!mgr) return;

    mgr->width = width;
    mgr->height = height;

    // First switch to windowed mode to change the size
    SDL_SetWindowFullscreen(mgr->window, SDL_WINDOW_SHOWN);

    // Then set the new size
    SDL_SetWindowSize(mgr->window, mgr->width, mgr->height);

    // Switch back to fullscreen mode with the new resolution
    SDL_SetWindowFullscreen(mgr->window, SDL_WINDOW_FULLSCREEN);
    mgr->wdwFlags |= SDL_WINDOW_FULLSCREEN;

    // SDL has some utility in this case
    SDL_RenderSetLogicalSize(mgr->renderer, mgr->width, mgr->height);
}

void handleDisplayEvent(DisplayManager mgr, SDL_Event *event) {
    if (!mgr || event->type != SDL_WINDOWEVENT) return;

    switch (event->window.event) {
        case SDL_WINDOWEVENT_RESIZED:
            mgr->width = event->window.data1;
            mgr->height = event->window.data2;
            SDL_RenderSetLogicalSize(mgr->renderer, mgr->width, mgr->height);
            break;
    }
}

void saveDisplaySettings(DisplayManager mgr, const char *filePath) {
    if (!mgr || !filePath) return;

    FILE *fout = fopen(filePath, "a");
    if (!fout) {
        printf("Failed to open config file for writing: %s\n", filePath);
        return;
    }

    fprintf(fout, "[DISPLAY]\n");
    fprintf(fout, "WIDTH=%d\n", mgr->width);
    fprintf(fout, "HEIGHT=%d\n", mgr->height);
    fprintf(fout, "FULLSCREEN=%d\n", (mgr->wdwFlags & SDL_WINDOW_FULLSCREEN) ? 1 : 0);
    fprintf(fout, "VSYNC=%d\n", (mgr->wdwFlags & SDL_RENDERER_PRESENTVSYNC) ? 1 : 0);

    fclose(fout);
}