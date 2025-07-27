#include "include/displayManager.h"

void setDefaultDisplaySettings(DisplayManager display) {
    if (!display) {
        printf("Display manager is NULL, cannot set default settings\n");
        return;
    }

    display->width = 1280;  // default width
    display->height = 720;  // default height
    display->wdwFlags = SDL_WINDOW_SHOWN;  // default window flags
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