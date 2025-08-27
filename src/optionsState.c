#include "include/stateManager.h"

void onEnterOptionsMenu(ZENg zEngine) {
    int screenW = zEngine->display->currentMode.w;
    int screenH = zEngine->display->currentMode.h;

    // Percentages from the top of the screen
    double listStartPos = 0.35;
    double listItemsSpacing = 0.08;

    // Create buttons with evenly spaced positions
    char* buttonLabels[] = {
        "Game", "Audio", "Display", "Controls", "Back"
    };
    // this is amazing
    void (*buttonActions[])(ZENg) = {
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
            orderIdx == 0 ? 1 : 0,  // First button selected (field flag)
            orderIdx
        );
        
        button->destRect->x = (screenW - button->destRect->w) / 2;
        button->destRect->y = screenH * (listStartPos + orderIdx * listItemsSpacing);
        
        id = createEntity(zEngine->ecs);
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

 void optionsToGameOpt(ZENg zEngine) {

 }

 /**
 * =====================================================================================================================
 */

void optionsToAudioOpt(ZENg zEngine) {

}

/**
 * =====================================================================================================================
 */

void optionsToVideoOpt(ZENg zEngine) {
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

void optionsToControlsOpt(ZENg zEngine) {

}

/**
 * =====================================================================================================================
 */

void optionsToMMenu(ZENg zEngine) {
    popState(zEngine);  // -> Main menu
}