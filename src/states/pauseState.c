#include "stateManager.h"

void onEnterPauseState(ZENg zEngine) {
    zEngine->uiManager->root = UIparseFromFile(zEngine, "data/states/UIpauseState.json");

    UIapplyLayout(zEngine->uiManager->root);

    SystemNode **systems = zEngine->ecs->depGraph->nodes;
    systems[SYS_LIFETIME]->isActive = 0;
    systems[SYS_WEAPONS]->isActive = 0;
    systems[SYS_VELOCITY]->isActive = 0;
    systems[SYS_WORLD_COLLISIONS]->isActive = 0;
    systems[SYS_ENTITY_COLLISIONS]->isActive = 0;
    systems[SYS_POSITION]->isActive = 0;
    systems[SYS_HEALTH]->isActive = 0;
    systems[SYS_TRANSFORM]->isActive = 0;
}

/**
 * =====================================================================================================================
 */

void onExitPauseState(ZENg zEngine) {
    // Delete pause state entities
    sweepState(zEngine->ecs, STATE_PAUSED);
    UIclear(zEngine->uiManager);

    SystemNode **systems = zEngine->ecs->depGraph->nodes;
    // Enable the game's systems
    systems[SYS_LIFETIME]->isActive = 1;
    systems[SYS_WEAPONS]->isActive = 1;
    systems[SYS_VELOCITY]->isActive = 1;
    systems[SYS_WORLD_COLLISIONS]->isActive = 1;
    systems[SYS_ENTITY_COLLISIONS]->isActive = 1;
    systems[SYS_POSITION]->isActive = 1;
    systems[SYS_HEALTH]->isActive = 1;
    systems[SYS_TRANSFORM]->isActive = 1;
    // Force a frame
    systems[SYS_VELOCITY]->isDirty = 1;
}

/**
 * =====================================================================================================================
 */

Uint8 handlePauseStateEvents(SDL_Event *e, ZENg zEngine) {
    return handleMenuNavigation(e, zEngine);
}

/**
 * =====================================================================================================================
 */

void pauseToPlay(ZENg zEngine, void *data) {
    popState(zEngine);
}

/**
 * =====================================================================================================================
 */

void pauseToMMenu(ZENg zEngine, void *data) {
    popState(zEngine);  // -> play
    popState(zEngine);  // -> Menu
}