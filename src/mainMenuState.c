#include "include/mainMenuState.h"

void updateMenuUI(ZENg zEngine) {
    // Rerender the UI based on the entities' current components' states

    SDL_SetRenderDrawColor(zEngine->renderer, 100, 50, 0, 200);  // background color - brownish
    SDL_RenderClear(zEngine->renderer);  // clear the renderer

    for (Uint64 i = 0; i < zEngine->uiEcs->entityCount; i++) {
        bitset targetFlags = 1 << TEXT_COMPONENT;
        bitset currentFlags = zEngine->uiEcs->componentsFlags[i];

        if ((currentFlags & targetFlags) == targetFlags) {
            TextComponent *curr = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[i]);

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
                    curr->texture = SDL_CreateTextureFromSurface(zEngine->renderer, surface);
                    SDL_RenderCopy(zEngine->renderer, curr->texture, NULL, curr->destRect);
                    SDL_FreeSurface(surface);
                    printf("Updated entity %ld's texture\n", i);
                }
            }
        }
    }
    printf("\n");
}

void handleMainMenuEvents(SDL_Event *event, ZENg zEngine) {
    // Key press handling
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_UP:
            case SDLK_w: {
                for (Uint64 i = 0; i < zEngine->uiEcs->nextEntityID; i++) {
                    bitset targetFlags = 1 << TEXT_COMPONENT;
                    bitset currEntityFlags = zEngine->uiEcs->componentsFlags[i];

                    if ((targetFlags & currEntityFlags) == targetFlags) {
                        TextComponent *curr = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[i]);
                        if (curr->selected) {
                            curr->selected = 0;
                            if (strcmp(curr->text, "Play") == 0) {
                                // wrap around to `Exit`
                                // starting at 0 because the ids can be reused
                                for (Uint64 j = 0; j < zEngine->uiEcs->nextEntityID; j++) {
                                    TextComponent *fetch = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[j]);
                                    if (strcmp(fetch->text, "Exit") == 0) {
                                        fetch->selected = 1;  // select "Exit"
                                        break;
                                    }
                                }
                            } else {
                                // select the previous option
                                for (Uint64 j = 0; j < zEngine->uiEcs->nextEntityID; j++) {
                                    TextComponent *fetch = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[j]);
                                    // find the previous item
                                    if (fetch->orderIdx == curr->orderIdx - 1) {
                                        fetch->selected = 1;
                                        break;
                                    }
                                }
                            }
                            updateMenuUI(zEngine);
                            break;
                        }
                    }
                }
                break;
            }
            case SDLK_DOWN:
            case SDLK_s: {
                for (Uint64 i = 0; i < zEngine->uiEcs->nextEntityID; i++) {
                    bitset targetFlags = 1 << TEXT_COMPONENT;
                    bitset currEntityFlags = zEngine->uiEcs->componentsFlags[i];

                    if ((targetFlags & currEntityFlags) == targetFlags) {
                        TextComponent *curr = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[i]);
                        if (curr->selected) {
                            curr->selected = 0;
                            if (strcmp(curr->text, "Exit") == 0) {
                                // wrap around to `Play`
                                for (Uint64 j = 0; j < zEngine->uiEcs->nextEntityID; j++) {
                                    TextComponent *fetch = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[j]);
                                    if (strcmp(fetch->text, "Play") == 0) {
                                        fetch->selected = 1;  // select "Play"
                                        break;
                                    }
                                }
                            } else {
                                // select the next option
                                 for (Uint64 j = 0; j < zEngine->uiEcs->nextEntityID; j++) {
                                    TextComponent *fetch = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[j]);
                                    if (fetch->orderIdx == curr->orderIdx + 1) {
                                        fetch->selected = 1;
                                        break;
                                    }
                                }
                            }
                            updateMenuUI(zEngine);
                            break;
                        }
                    }
                }
                break;
            }
            case SDLK_RETURN:
            case SDLK_SPACE: {
                for (Uint64 i = 0; i < zEngine->uiEcs->entityCount; i++) {
                    TextComponent *curr = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[i]);
                    if (curr->selected) {
                        if (strcmp(curr->text, "Play") == 0) {
                            onExitMainMenu(zEngine);  // Clear the main menu UI
                            onEnterPlayState(zEngine);
                            zEngine->state = STATE_PLAYING;  // Start the game
                        } else if (strcmp(curr->text, "Exit") == 0) {
                            zEngine->state = STATE_EXIT;  // Exit the game
                        }
                    }
                }
                break;
            }
        }
    }
}

void renderMainMenu(ZENg zEngine) {
    // Clear the screen
    SDL_SetRenderDrawColor(zEngine->renderer, 100, 50, 0, 200);  // background color - brownish
    SDL_RenderClear(zEngine->renderer);

    bitset targetMask = 1 << TEXT_COMPONENT;

    for (Uint64 i = 0; i < zEngine->uiEcs->entityCount; i++) {
        // printf("Component %ld's flag: %d\nTarget flag: %d\n", i, zEngine->uiEcs->componentsFlags[i], targetMask);
        // printf("zEngine->uiEcs->componentsFlags[i] (%d) & targetMask (%d) == targetMask (%d): %d\n", zEngine->uiEcs->componentsFlags[i], targetMask, targetMask, (zEngine->uiEcs->componentsFlags[i] & targetMask) == targetMask);
        if ((zEngine->uiEcs->componentsFlags[i] & targetMask) == targetMask) {
            TextComponent *curr = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[i]);
            if (curr->state == STATE_MAIN_MENU) {
                // if the entity has the text component
                SDL_RenderCopy(
                    zEngine->renderer,
                    curr->texture,
                    NULL,
                    curr->destRect
                );
            }
        }
    }

    SDL_RenderPresent(zEngine->renderer);  // render current frame
}