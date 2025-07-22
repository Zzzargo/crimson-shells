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
