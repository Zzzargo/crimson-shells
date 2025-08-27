#include "include/stateManager.h"

void onEnterPauseState(ZENg zEngine) {
    int screenH = zEngine->display->currentMode.h;
    int screenW = zEngine->display->currentMode.w;

    // Percentages from the top of the screen
    double titlePos = 0.3;
    double listStartPos = 0.45;
    double listItemsSpacing = 0.08;

    // Create buttons with evenly spaced positions
    char* buttonLabels[] = {
        "Resume", "Exit to main menu"
    };
    // this is amazing
    void (*buttonActions[])(ZENg) = {
        &pauseToPlay, &pauseToMMenu
    };

    Entity id;
    for (Uint8 orderIdx = 0; orderIdx < (sizeof(buttonLabels) / sizeof(buttonLabels[0])); orderIdx++) {
        ButtonComponent *button = createButtonComponent (
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

void onExitPauseState(ZENg zEngine) {
    // delete game entities
    while (zEngine->ecs->entityCount > 0) {
        deleteEntity(zEngine->ecs, zEngine->ecs->activeEntities[0]);
    }

    // Disable the pause state systems
    zEngine->ecs->depGraph->nodes[SYS_BUTTONS]->isActive = 0;
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

void renderPauseState(ZENg zEngine) {
    renderPlayState(zEngine);

    // transparent overlay
    SDL_SetRenderDrawBlendMode(zEngine->display->renderer, SDL_BLENDMODE_BLEND);  // Enable blending for transparency
    SDL_SetRenderDrawColor(zEngine->display->renderer, 0, 0, 0, 150);  // semi-transparent black
    SDL_RenderFillRect(zEngine->display->renderer, NULL);  // Fill the entire screen with the semi-transparent color

    // Render the options
    for (Uint64 i = 0; i < zEngine->ecs->components[BUTTON_COMPONENT].denseSize; i++) {
        ButtonComponent *buttonComp = (ButtonComponent *)(zEngine->ecs->components[BUTTON_COMPONENT].dense[i]);
        SDL_RenderCopy(zEngine->display->renderer, buttonComp->texture, NULL, buttonComp->destRect);
    }
    SDL_SetRenderDrawBlendMode(zEngine->display->renderer, SDL_BLENDMODE_NONE);  // Disable blending
}

/**
 * =====================================================================================================================
 */

void pauseToPlay(ZENg zEngine) {
    popState(zEngine);
}

/**
 * =====================================================================================================================
 */

void pauseToMMenu(ZENg zEngine) {
    popState(zEngine);  // -> play
    popState(zEngine);  // -> Menu
}