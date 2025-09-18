#include "stateManager.h"

void onEnterGarage(ZENg zEngine) {
    zEngine->uiManager->root = UIparseFromFile(zEngine, "data/states/UIgarageState.json");
    UIapplyLayout(zEngine->uiManager->root);
}

/**
 * =====================================================================================================================
 */

void onExitGarage(ZENg zEngine) {
    UIclear(zEngine->uiManager);
}

/**
 * =====================================================================================================================
 */

Uint8 handleGarageEvents(SDL_Event *event, ZENg zEngine) {
    return handleMenuNavigation(event, zEngine);
}

/**
 * =====================================================================================================================
 */

void garageToMMenu(ZENg zEngine, void *data) {
    popState(zEngine);
}