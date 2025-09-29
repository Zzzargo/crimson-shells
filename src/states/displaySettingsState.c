#include "stateManager.h"

void onEnterVideoSettings(ZENg zEngine) {
    getCurrState(zEngine->stateMng)->stateData = MapInit(13, MAP_STATE_DATA);
    zEngine->uiManager->root = UIparseFromFile(zEngine, "data/states/UIdisplaySettingsState.json");

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

ProviderResult* getResolutions(ZENg zEngine) {
    if (!zEngine) THROW_ERROR_AND_RETURN("Engine is NULL in getResolutions", NULL);

    int count = 0;
    Uint8 insertCount = 0;
    SDL_DisplayMode *modes = getAvailableDisplayModes(zEngine->display, &count);
    if (count < 256) insertCount = (Uint8)count;

    ProviderResult *result = calloc(1, sizeof(ProviderResult));
    if (!result) THROW_ERROR_AND_EXIT("Failed to allocate memory for ProviderResult in getResolutions");
    result->data = modes;
    result->size = insertCount;
    result->type = RESULT_DISPLAYMODE_ARRAY;

    HashMap dataMap = getCurrState(zEngine->stateMng)->stateData;
    MapAddEntry(dataMap, "result:resolutions", (MapEntryVal){ .ptr = result }, ENTRY_PROVIDER_RESULT);

    #ifdef DEBUG
        printf("Added %d resolutions to state data map\n", insertCount);
    #endif
    return result;
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

ProviderResult* getWindowModes(ZENg zEngine) {
    if (!zEngine) THROW_ERROR_AND_RETURN("Engine is NULL in getWindowModes", NULL);

    Uint8 *windowModes = calloc(2, sizeof(Uint8));
    if (!windowModes) THROW_ERROR_AND_EXIT("Failed to allocate memory for the window modes");
    windowModes[0] = 0;  // 0 = windowed
    windowModes[1] = 1;  // 1 = fullscreen

    Uint8 windowModeCount = 2;
    
    ProviderResult *result = calloc(1, sizeof(ProviderResult));
    if (!result) THROW_ERROR_AND_EXIT("Failed to allocate memory for ProviderResult in getWindowModes");
    result->data = windowModes;
    result->size = windowModeCount;
    result->type = RESULT_WINDOWMODE_ARRAY;

    HashMap map = getCurrState(zEngine->stateMng)->stateData;
    MapAddEntry(map, "result:windowModes", (MapEntryVal){ .ptr = result }, ENTRY_PROVIDER_RESULT);

    #ifdef DEBUG
        printf("Added %d window modes to parser map\n", windowModeCount);
    #endif
    return result;
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