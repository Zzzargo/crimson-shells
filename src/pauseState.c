#include "include/stateManager.h"

void updatePauseUI(ZENg zEngine) {
    SDL_SetRenderDrawColor(zEngine->display->renderer, 100, 50, 0, 200);  // background color - brownish
    SDL_RenderClear(zEngine->display->renderer);  // clear the renderer

    for (Uint64 i = 0; i < zEngine->uiEcs->components[TEXT_COMPONENT].denseSize; i++) {
        TextComponent *curr = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[i]);
        // if (curr->state == STATE_PAUSED) {  // if it's part of the pause menu
            // if the entity has selectable color
            if (
                CMP_COLORS(curr->color, COLOR_WHITE)
                || CMP_COLORS(curr->color, COLOR_YELLOW)
            ) {
                // and it's active
                if (curr->active) {
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
                    printf("Updated entity %ld's texture\n", i);
                }
            }
        // }
    }
    printf("\n");
}

/**
 * =====================================================================================================================
 */

Uint8 handlePauseStateEvents(SDL_Event *e, ZENg zEngine) {
    // Key press handling
    if (e->type == SDL_KEYDOWN) {
        InputAction action = scancodeToAction(zEngine->inputMng, e->key.keysym.scancode);
        if (action == INPUT_UNKNOWN) {
            printf("Unknown input action for scancode %d\n", e->key.keysym.scancode);
            return 1;
        }

        switch (action) {
            case INPUT_BACK: {
                // go back to the game
                popState(zEngine);
                return 1;
            }
            case INPUT_MOVE_UP: {
                Uint64 maxComp = zEngine->uiEcs->components[TEXT_COMPONENT].denseSize;
                for (Uint64 i = 0; i < maxComp; i++) {
                    TextComponent *curr = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[i]);
                    if (curr && curr->selected) {
                        curr->selected = 0;
                        if (strcmp(curr->text, "Continue") == 0) {
                            // wrap around to `Exit`
                            for (Uint64 j = 0; j < maxComp; j++) {
                                TextComponent *fetch = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[j]);
                                if (strcmp(fetch->text, "Exit to main menu") == 0) {
                                    fetch->selected = 1;  // select "Exit"
                                    break;
                                }
                            }
                        } else {
                            // select the previous option
                            for (Uint64 j = 0; j < maxComp; j++) {
                                TextComponent *fetch = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[j]);
                                if (fetch->orderIdx == curr->orderIdx - 1) {
                                    fetch->selected = 1;
                                    break;
                                }
                            }
                        }
                        updatePauseUI(zEngine);
                        return 1;
                    }
                }
                break;
            }
            case INPUT_MOVE_DOWN: {
                Uint64 maxComp = zEngine->uiEcs->components[TEXT_COMPONENT].denseSize;
                for (Uint64 i = 0; i < maxComp; i++) {
                    TextComponent *curr = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[i]);
                    if (curr && curr->selected) {
                        curr->selected = 0;
                        if (strcmp(curr->text, "Exit to main menu") == 0) {
                            // wrap around to `Play`
                            for (Uint64 j = 0; j < maxComp; j++) {
                                TextComponent *fetch = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[j]);
                                if (strcmp(fetch->text, "Continue") == 0) {
                                    fetch->selected = 1;  // select "Continue"
                                    break;
                                }
                            }
                        } else {
                            // select the next option
                            for (Uint64 j = 0; j < maxComp; j++) {
                                TextComponent *fetch = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[j]);
                                if (fetch->orderIdx == curr->orderIdx + 1) {
                                    fetch->selected = 1;
                                    break;
                                }
                            }
                        }
                        updatePauseUI(zEngine);
                        return 1;
                    }
                }
                break;
            }
            case INPUT_SELECT: {
                Uint64 maxComp = zEngine->uiEcs->components[TEXT_COMPONENT].denseSize;
                for (Uint64 i = 0; i < maxComp; i++) {
                    TextComponent *curr = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[i]);
                    if (curr && curr->selected) {
                        if (strcmp(curr->text, "Continue") == 0) {
                            popState(zEngine);  // go back to the game
                        } else if (strcmp(curr->text, "Exit to main menu") == 0) {
                            popState(zEngine);  // exit to game
                            popState(zEngine);  // exit to main menu
                        }
                    }
                }
                break;
            }
        }
    }
    return 1;
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
    for (Uint64 i = 0; i < zEngine->uiEcs->components[TEXT_COMPONENT].denseSize; i++) {
        TextComponent *textComp = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[i]);
        SDL_RenderCopy(zEngine->display->renderer, textComp->texture, NULL, textComp->destRect);
    }
    SDL_SetRenderDrawBlendMode(zEngine->display->renderer, SDL_BLENDMODE_NONE);  // Disable blending
}