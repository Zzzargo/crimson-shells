#include "states/stateManager.h"

void onEnterControlsSettings(ZENg zEngine) {
    zEngine->uiManager->root = UIparseFromFile(zEngine, "data/states/UIcontrolsSettingsState.json");
    UIapplyLayout(zEngine->uiManager->root);
}

/**
 * =====================================================================================================================
 */

void onExitControlsSettings(ZENg zEngine) {
    UIclear(zEngine->uiManager);
}

/**
 * =====================================================================================================================
 */

Uint8 handleControlsSettingsEvents(SDL_Event *event, ZENg zEngine) {
    return handleMenuNavigation(event, zEngine);
}

/**
 * =====================================================================================================================
 */

void controlsSettingsToSettings(ZENg zEngine, void *data) {
    popState(zEngine);
}
