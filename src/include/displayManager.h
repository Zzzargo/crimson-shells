#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <global.h>

typedef struct displaymng {
    SDL_Window *window;  // pointer to the game's window
    SDL_Renderer *renderer;  // pointer to the renderer

    Uint32 width;  // current width of the window
    Uint32 height;  // current height of the window
    Uint32 wdwFlags;  // fullscreen, borderless, etc.
} *DisplayManager;

void setDefaultDisplaySettings(DisplayManager display);

void setWindowSize(DisplayManager mgr, int width, int height);

void toggleFullscreen(DisplayManager mgr);

void setFullscreenResolution(DisplayManager mgr, int width, int height);

void handleDisplayEvent(DisplayManager mgr, SDL_Event *event);

void saveDisplaySettings(DisplayManager mgr, const char *filePath);

#endif // DISPLAY_MANAGER_H