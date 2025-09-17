#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include "../global/global.h"

typedef struct displaymng {
    SDL_Window *window;  // pointer to the game's window
    SDL_Renderer *renderer;  // pointer to the renderer

    SDL_DisplayMode currentMode;  // thanks SDL for making this easy
    Uint32 wdwFlags;  // fullscreen, borderless, etc.
    Uint8 fullscreen;
    Uint8 vsync;
} *DisplayManager;

/**
 * Sets the default display settings for the screen
 * @note default is 1280x720 @60Hz, windowed mode
 * @param display pointer to the display manager
 */
void setDefaultDisplaySettings(DisplayManager display);

/**
 * Toggles fullscreen mode for the display
 * @param mgr pointer to the display manager
 */
void toggleFullscreen(DisplayManager mgr);

/**
 * Gets the available display modes for the primary display
 * @param mgr pointer to the display manager
 * @param count pointer to an integer to store the number of modes
 * @return array of SDL_DisplayMode structs or NULL on failure
 * @note the memory for the returned array is allocated inside the function
 */
SDL_DisplayMode* getAvailableDisplayModes(DisplayManager mgr, int *count);

/**
 * Sets the display mode for the window
 * @param mgr pointer to the display manager
 * @param mode pointer to the SDL_DisplayMode to set
 */
void setDisplayMode(DisplayManager mgr, const SDL_DisplayMode *mode);

/**
 * Saves the current display settings to a file
 * @param mgr pointer to the display manager
 * @param filePath path to the settings file
 */
void saveDisplaySettings(DisplayManager mgr, const char *filePath);

#endif // DISPLAY_MANAGER_H