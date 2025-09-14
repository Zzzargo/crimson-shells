#include "include/stateManager.h"

void onEnterOptionsMenu(ZENg zEngine) {
    // // Percentages from the top of the screen
    // double listStartPos = 0.35;
    // double listItemsSpacing = 0.08;
    // double titlePos = 0.15;
    // double footerPos = 0.85;

    // // Create buttons with evenly spaced positions
    // char* buttonLabels[] = {
    //     "Game", "Audio", "Display", "Controls"
    // };
    // // this is amazing
    // void (*buttonActions[])(ZENg, void *) = {
    //     &optionsToGameOpt, &optionsToAudioOpt, &optionsToVideoOpt, 
    //     &optionsToControlsOpt
    // };

    // Entity id = createEntity(zEngine->ecs, STATE_OPTIONS);
    // Uint8 orderIdx = 0;

    // // Title
    // TextComponent *title = createTextComponent(
    //     zEngine->display->renderer,
    //     getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#48"),
    //     strdup("Options"), COLOR_CRIMSON, 1
    // );
    // title->destRect->x = (LOGICAL_WIDTH - title->destRect->w) / 2;
    // title->destRect->y = LOGICAL_HEIGHT * titlePos;
    // addComponent(zEngine->ecs, id, TEXT_COMPONENT, (void *)title);

    // for (; orderIdx < (sizeof(buttonLabels) / sizeof(buttonLabels[0])); orderIdx++) {
    //     ButtonComponent *button = createButtonComponent(
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
        
    //     id = createEntity(zEngine->ecs, STATE_OPTIONS);
    //     addComponent(zEngine->ecs, id, BUTTON_COMPONENT, (void *)button);
    // }

    // // Back button
    // id = createEntity(zEngine->ecs, STATE_OPTIONS);
    // ButtonComponent *backButton = createButtonComponent(
    //     zEngine->display->renderer,
    //     getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#28"),
    //     strdup("Back"),
    //     COLOR_WHITE,
    //     &optionsToMMenu,
    //     NULL,  // no extra data needed
    //     0,  // not selected
    //     orderIdx  // last in order
    // );
    // backButton->destRect->x = (LOGICAL_WIDTH - backButton->destRect->w) / 2;
    // backButton->destRect->y = LOGICAL_HEIGHT * footerPos;
    // addComponent(zEngine->ecs, id, BUTTON_COMPONENT, (void *)backButton);

    zEngine->ecs->depGraph->nodes[SYS_UI]->isActive = 1;
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