#include "include/stateManager.h"

void onEnterOptionsMenu(ZENg zEngine) {
    Uint8 orderIdx = 0;  // counts the inserted texts

    ButtonComponent *game = createButtonComponent(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf"),
        strdup("Game"), COLOR_YELLOW, &optionsToGameOpt, 1, orderIdx++
    );
    game->destRect->x = (zEngine->display->currentMode.w - game->destRect->w) / 2;
    game->destRect->y = (zEngine->display->currentMode.h - game->destRect->h) / 3;

    Entity id = createEntity(zEngine->uiEcs);
    addComponent(zEngine->uiEcs, id, BUTTON_COMPONENT, (void *)game);

    ButtonComponent *audio = createButtonComponent(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf"),
        strdup("Audio"), COLOR_WHITE, &optionsToAudioOpt, 0, orderIdx++
    );
    audio->destRect->x = (zEngine->display->currentMode.w - audio->destRect->w) / 2;
    audio->destRect->y = (zEngine->display->currentMode.h - audio->destRect->h) / 2.5;

    id = createEntity(zEngine->uiEcs);
    addComponent(zEngine->uiEcs, id, BUTTON_COMPONENT, (void *)audio);

    ButtonComponent *video = createButtonComponent(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf"),
        strdup("Video"), COLOR_WHITE, &optionsToVideoOpt, 0, orderIdx++
    );
    video->destRect->x = (zEngine->display->currentMode.w - video->destRect->w) / 2;
    video->destRect->y = (zEngine->display->currentMode.h - video->destRect->h) / 2;

    id = createEntity(zEngine->uiEcs);
    addComponent(zEngine->uiEcs, id, BUTTON_COMPONENT, (void *)video);

    ButtonComponent *Controls = createButtonComponent(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf"),
        strdup("Controls"), COLOR_WHITE, &optionsToControlsOpt, 0, orderIdx++
    );
    Controls->destRect->x = (zEngine->display->currentMode.w - Controls->destRect->w) / 2;
    Controls->destRect->y = (zEngine->display->currentMode.h - Controls->destRect->h) / 1.5;

    id = createEntity(zEngine->uiEcs);
    addComponent(zEngine->uiEcs, id, BUTTON_COMPONENT, (void *)Controls);
}

/**
 * =====================================================================================================================
 */

void onExitOptionsMenu(ZENg zEngine) {
    while (zEngine->uiEcs->entityCount > 0) {
        deleteEntity(zEngine->uiEcs, zEngine->uiEcs->activeEntities[0]);
    }
}

/**
 * =====================================================================================================================
 */

Uint8 handleOptionsMenuEvents(SDL_Event *event, ZENg zEngine) {
    return handleMenuNavigation(event, zEngine, "Game", "Controls", &updateMenuUI);
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

}

/**
 * =====================================================================================================================
 */

void optionsToControlsOpt(ZENg zEngine) {

}
