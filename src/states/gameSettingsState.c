#include "stateManager.h"

void onEnterGameSettings(ZENg zEngine) {
    zEngine->uiManager->root = UIparseFromFile(zEngine, "data/states/UIgameSettingsState.json");
    UIapplyLayout(zEngine->uiManager->root);
}

/**
 * =====================================================================================================================
 */

void onExitGameSettings(ZENg zEngine) {
    UIclear(zEngine->uiManager);
}

/**
 * =====================================================================================================================
 */

Uint8 handleGameSettingsEvents(SDL_Event *event, ZENg zEngine) {
    return handleMenuNavigation(event, zEngine);
}

/**
 * =====================================================================================================================
 */

void gameSettingsToSettings(ZENg zEngine, void *data) {
    popState(zEngine);
}