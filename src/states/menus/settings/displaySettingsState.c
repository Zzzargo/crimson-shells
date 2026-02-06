#include "global/debug.h"
#include "states/stateManager.h"

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
    ASSERT(zEngine != NULL, "");

    int count = 0;
    Uint8 insertCount = 0;
    SDL_DisplayMode *modes = getAvailableDisplayModes(zEngine->display, &count);
    if (count < 256) insertCount = (Uint8)count;

    ProviderResult *result = calloc(1, sizeof(ProviderResult));
    ASSERT(result != NULL, "Failed to allocate memory for ProviderResult");

    result->data = modes;
    result->size = insertCount;
    result->type = RESULT_DISPLAYMODE_ARRAY;

    HashMap dataMap = getCurrState(zEngine->stateMng)->stateData;
    MapAddEntry(dataMap, "result:resolutions", (MapEntryVal){ .ptr = result }, ENTRY_PROVIDER_RESULT);

    LOG(DEBUG, "Added %d resolutions to state data map\n", insertCount);
    return result;
}

/**
 * =====================================================================================================================
 */

void changeRes(ZENg zEngine, void *data) {
    if (!data) {
        LOG(ERROR, "No display mode data provided to changeRes\n");
        return;
    }
    SDL_DisplayMode *mode = (SDL_DisplayMode *)data;
    setDisplayMode(zEngine->display, mode);
}

/**
 * =====================================================================================================================
 */

ProviderResult* getWindowModes(ZENg zEngine) {
    ASSERT(zEngine != NULL, "");

    Uint8 *windowModes = calloc(2, sizeof(Uint8));
    ASSERT(windowModes != NULL, "Failed to allocate memory for the window modes");

    windowModes[0] = 0;  // 0 = windowed
    windowModes[1] = 1;  // 1 = fullscreen

    Uint8 windowModeCount = 2;
    
    ProviderResult *result = calloc(1, sizeof(ProviderResult));
    ASSERT(result != NULL, "Failed to allocate memory for ProviderResult");

    result->data = windowModes;
    result->size = windowModeCount;
    result->type = RESULT_WINDOWMODE_ARRAY;

    HashMap map = getCurrState(zEngine->stateMng)->stateData;
    MapAddEntry(map, "result:windowModes", (MapEntryVal){ .ptr = result }, ENTRY_PROVIDER_RESULT);

    LOG(DEBUG, "Added %d window modes to parser map\n", windowModeCount);
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
    LOG(DEBUG, "Current window mode: %d, requested mode: %d\n", currMode, newMode);
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
