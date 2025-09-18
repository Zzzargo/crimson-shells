#include "stateManager.h"

void onEnterVideoSettings(ZENg zEngine) {
    zEngine->uiManager->root = UIparseFromFile(zEngine, "data/states/UIdisplaySettingsState.json");

    // Thought of making a post-process function to insert needed data to the nodes

    UIapplyLayout(zEngine->uiManager->root);
}

/**
 * =====================================================================================================================
 */

void onExitVideoSettings(ZENg zEngine) {
    UIclear(zEngine->uiManager);
}

/**
 * =====================================================================================================================
 */

void changeRes(ZENg zEngine, void *data) {
    if (!data) {
        printf("No display mode data provided to changeRes\n");
        return;
    }
    SDL_DisplayMode *mode = (SDL_DisplayMode *)data;
    setDisplayMode(zEngine->display, mode);
}

/**
 * =====================================================================================================================
 */

void changeWindowMode(ZENg zEngine, void *data) {
    if (!data) {
        printf("No window mode data provided to changeWindowMode\n");
        return;
    }
    Uint8 currMode = zEngine->display->fullscreen;
    Uint8 newMode = *((Uint8 *)data);
    if (currMode != newMode) {
        toggleFullscreen(zEngine->display);
    }
}

/**
 * =====================================================================================================================
 */

Uint8 handleVideoSettingsEvents(SDL_Event *event, ZENg zEngine) {
    return handleMenuNavigation(event, zEngine);
}

/**
 * =====================================================================================================================
 */

void videoSettingsToSettings(ZENg zEngine, void *data) {
    popState(zEngine);  // -> Settings menu
}