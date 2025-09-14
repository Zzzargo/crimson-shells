#include "include/stateManager.h"

void onEnterPauseState(ZENg zEngine) {
    // // Percentages from the top of the screen
    // double titlePos = 0.3;
    // double listStartPos = 0.45;
    // double listItemsSpacing = 0.08;

    // // Create buttons with evenly spaced positions
    // char* buttonLabels[] = {
    //     "Resume", "Exit to main menu"
    // };
    // // this is amazing
    // void (*buttonActions[])(ZENg, void *data) = {
    //     &pauseToPlay, &pauseToMMenu
    // };

    // Entity id;
    // for (Uint8 orderIdx = 0; orderIdx < (sizeof(buttonLabels) / sizeof(buttonLabels[0])); orderIdx++) {
    //     ButtonComponent *button = createButtonComponent (
    //         zEngine->display->renderer, 
    //         getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#28"),
    //         strdup(buttonLabels[orderIdx]), 
    //         orderIdx == 0 ? COLOR_YELLOW : COLOR_WHITE, // First button selected (color)
    //         buttonActions[orderIdx],
    //         NULL, // these buttons have no extra data
    //         orderIdx == 0 ? 1 : 0,  // First button selected (field flag)
    //         orderIdx
    //     );

    //     button->destRect->x = (LOGICAL_WIDTH - button->destRect->w) / 2;
    //     button->destRect->y = LOGICAL_HEIGHT * (listStartPos + orderIdx * listItemsSpacing);

    //     id = createEntity(zEngine->ecs, STATE_PAUSED);
    //     addComponent(zEngine->ecs, id, BUTTON_COMPONENT, (void *)button);
    // }

    SystemNode **systems = zEngine->ecs->depGraph->nodes;
    systems[SYS_LIFETIME]->isActive = 0;
    systems[SYS_WEAPONS]->isActive = 0;
    systems[SYS_VELOCITY]->isActive = 0;
    systems[SYS_WORLD_COLLISIONS]->isActive = 0;
    systems[SYS_ENTITY_COLLISIONS]->isActive = 0;
    systems[SYS_POSITION]->isActive = 0;
    systems[SYS_HEALTH]->isActive = 0;
    systems[SYS_TRANSFORM]->isActive = 0;

    systems[SYS_UI]->isActive = 1;
}

/**
 * =====================================================================================================================
 */

void onExitPauseState(ZENg zEngine) {
    // Delete pause state entities
    sweepState(zEngine->ecs, STATE_PAUSED);

    // Disable the pause state systems
    SystemNode **systems = zEngine->ecs->depGraph->nodes;
    systems[SYS_UI]->isActive = 0;

    // And enable the game's
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
    return handleMenuNavigation(e, zEngine, "Resume", "Exit to main menu");
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