#include "include/stateManager.h"

void onEnterOptionsMenu(ZENg zEngine) {
    // Percentages from the top of the screen
    double listStartPos = 0.35;
    double listItemsSpacing = 0.08;

    // Create buttons with evenly spaced positions
    char* buttonLabels[] = {
        "Game", "Audio", "Display", "Controls", "Back"
    };
    // this is amazing
    void (*buttonActions[])(ZENg, void *) = {
        &optionsToGameOpt, &optionsToAudioOpt, &optionsToVideoOpt, 
        &optionsToControlsOpt, &optionsToMMenu
    };

    Entity id;

    for (Uint8 orderIdx = 0; orderIdx < (sizeof(buttonLabels) / sizeof(buttonLabels[0])); orderIdx++) {
        ButtonComponent *button = createButtonComponent(
            zEngine->display->renderer, 
            getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf"),
            strdup(buttonLabels[orderIdx]), 
            orderIdx == 0 ? COLOR_YELLOW : COLOR_WHITE, // First button selected (color)
            buttonActions[orderIdx], 
            NULL, // these buttons have no extra data
            orderIdx == 0 ? 1 : 0,  // First button selected (field flag)
            orderIdx
        );
        
        button->destRect->x = (LOGICAL_WIDTH - button->destRect->w) / 2;
        button->destRect->y = LOGICAL_HEIGHT * (listStartPos + orderIdx * listItemsSpacing);
        
        id = createEntity(zEngine->ecs, STATE_OPTIONS);
        addComponent(zEngine->ecs, id, BUTTON_COMPONENT, (void *)button);
    }

    zEngine->ecs->depGraph->nodes[SYS_BUTTONS]->isActive = 1;
}

/**
 * =====================================================================================================================
 */

void onExitOptionsMenu(ZENg zEngine) {
    while (zEngine->ecs->entityCount > 0) {
        deleteEntity(zEngine->ecs, zEngine->ecs->activeEntities[0]);
    }
}

/**
 * =====================================================================================================================
 */

Uint8 handleOptionsMenuEvents(SDL_Event *event, ZENg zEngine) {
    return handleMenuNavigation(event, zEngine, "Game", "Back");
}

/**
 * =====================================================================================================================
 */

 void optionsToGameOpt(ZENg zEngine, void *data) {

 }

 /**
 * =====================================================================================================================
 */

void optionsToAudioOpt(ZENg zEngin, void *data) {

}

/**
 * =====================================================================================================================
 */

void optionsToVideoOpt(ZENg zEngine, void *data) {
    GameState *videoOpt = calloc(1, sizeof(GameState));
    if (!videoOpt) {
        printf("Failed to allocate memory for the video options state");
        exit(EXIT_FAILURE);
    }
    videoOpt->type = STATE_OPTIONS_VIDEO;
    videoOpt->onEnter = &onEnterVideoOptions;
    videoOpt->onExit = &onExitVideoOptions;
    videoOpt->handleEvents = &handleVideoOptionsEvents;

    pushState(zEngine, videoOpt);
}

/**
 * =====================================================================================================================
 */

void optionsToControlsOpt(ZENg zEngine, void *data) {

}

/**
 * =====================================================================================================================
 */

void optionsToMMenu(ZENg zEngine, void *data) {
    popState(zEngine);  // -> Main menu
}