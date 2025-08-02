#include "include/stateManager.h"

void updatePauseUI(ZENg zEngine) {
    SDL_SetRenderDrawColor(zEngine->display->renderer, 100, 50, 0, 200);  // background color - brownish
    SDL_RenderClear(zEngine->display->renderer);  // clear the renderer

    for (Uint64 i = 0; i < zEngine->uiEcs->components[BUTTON_COMPONENT].denseSize; i++) {
        ButtonComponent *curr = (ButtonComponent *)(zEngine->uiEcs->components[BUTTON_COMPONENT].dense[i]);

        // Update the texture
        SDL_DestroyTexture(curr->texture);
        SDL_Surface *surface = TTF_RenderText_Solid(
            curr->font,
            curr->text,
            curr->selected ? COLOR_YELLOW : COLOR_WHITE
        );
        curr->texture = SDL_CreateTextureFromSurface(zEngine->display->renderer, surface);
        SDL_RenderCopy(zEngine->display->renderer, curr->texture, NULL, curr->destRect);
        SDL_FreeSurface(surface);
    }
    renderPauseState(zEngine);
}

/**
 * =====================================================================================================================
 */

Uint8 handlePauseStateEvents(SDL_Event *e, ZENg zEngine) {
    return handleMenuNavigation(e, zEngine, "Continue", "Exit to main menu", &updatePauseUI);
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
    for (Uint64 i = 0; i < zEngine->uiEcs->components[BUTTON_COMPONENT].denseSize; i++) {
        ButtonComponent *buttonComp = (ButtonComponent *)(zEngine->uiEcs->components[BUTTON_COMPONENT].dense[i]);
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