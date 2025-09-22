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
    clearStateData(getCurrState(zEngine->stateMng));
}

/**
 * =====================================================================================================================
 */

void getResolutions(ZENg zEngine, HashMap map) {
    if (!zEngine) {
        fprintf(stderr, "Cannot get resolutions from a NULL ZENg\n");
        return;
    }

    int count = 0;
    Uint8 *insertCount = calloc(1, sizeof(Uint8));
    SDL_DisplayMode *modes = getAvailableDisplayModes(zEngine->display, &count);
    if (count < 256) {
        *insertCount = (Uint8)count;
    }

    // addStateData(getCurrState(zEngine->stateMng), (void *)modes, STATE_DATA_PLAIN);
    // addStateData(getCurrState(zEngine->stateMng), (void *)insertCount, STATE_DATA_PLAIN);

    #ifdef DEBUG
        printf("Added %d resolutions to parser map\n", *insertCount);
    #endif
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

void getWindowModes(ZENg zEngine, HashMap map) {
    if (!zEngine) {
        fprintf(stderr, "Cannot get window modes from a NULL ZENg\n");
        return;
    }

    Uint8 *windowModeWindowed = calloc(1, sizeof(Uint8));
    if (!windowModeWindowed) {
        fprintf(stderr, "Failed to allocate memory for the windowed mode\n");
        return;
    }
    Uint8 *windowModeFullscreen = calloc(1, sizeof(Uint8));
    if (!windowModeFullscreen) {
        fprintf(stderr, "Failed to allocate memory for the fullscreen mode\n");
        free(windowModeWindowed);
        return;
    }
    *windowModeWindowed = 0;  // 0 = windowed
    *windowModeFullscreen = 1;  // 1 = fullscreen

    Uint8 *windowModeCount = calloc(1, sizeof(Uint8));
    if (!windowModeCount) {
        fprintf(stderr, "Failed to allocate memory for the window modes count\n");
        free(windowModeWindowed);
        free(windowModeFullscreen);
        return;
    }
    *windowModeCount = 2;  // windowed and fullscreen
    
    // MapAddEntry(map, "windowModes[0]", (MapEntryVal){ .boolean = windowModeWindowed }, MAP_ENTRY_BOOL);
    // MapAddEntry(map, "windowModes[1]", (MapEntryVal){ .boolean = windowModeFullscreen }, MAP_ENTRY_BOOL);
    // MapAddEntry(map, "windowModesCount", (MapEntryVal){ .boolean = windowModeCount }, MAP_ENTRY_BOOL);

    // addStateData(getCurrState(zEngine->stateMng), (void *)windowModeWindowed, STATE_DATA_PLAIN);
    // addStateData(getCurrState(zEngine->stateMng), (void *)windowModeFullscreen, STATE_DATA_PLAIN);
    // addStateData(getCurrState(zEngine->stateMng), (void *)windowModeCount, STATE_DATA_PLAIN);

    #ifdef DEBUG
        printf("Added %d window modes to parser map\n", *windowModeCount);
    #endif
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
    printf("Current window mode: %d, requested mode: %d\n", currMode, newMode);
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