#include "include/mainMenuState.h"

void updateMenuUI(ECS uiEcs, SDL_Renderer *rdr) {
    // Rerender the UI based on the entities' current components' states

    SDL_SetRenderDrawColor(rdr, 100, 50, 0, 200);  // background color - brownish
    SDL_RenderClear(rdr);  // clear the renderer

    for (Uint64 i = 0; i < uiEcs->entityCount; i++) {
        bitset targetFlags = 1 << TEXT_COMPONENT;
        bitset currentFlags = uiEcs->componentsFlags[i];

        if ((currentFlags & targetFlags) == targetFlags) {
            TextComponent *curr = (TextComponent *)(uiEcs->components[TEXT_COMPONENT].dense[i]);

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
                    curr->texture = SDL_CreateTextureFromSurface(rdr, surface);
                    SDL_RenderCopy(rdr, curr->texture, NULL, curr->destRect);
                    SDL_FreeSurface(surface);
                    printf("Updated entity %ld's texture\n", i);
                }
            }
        }
    }
    printf("\n");
}

void handleMainMenuEvents(SDL_Event *event, GameState *currState, SDL_Renderer *rdr, ECS uiEcs, ECS gEcs) {
    // Key press handling
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_UP:
            case SDLK_w: {
                for (Uint64 i = 0; i < uiEcs->entityCount; i++) {
                    bitset targetFlags = 1 << TEXT_COMPONENT;
                    bitset currEntityFlags = uiEcs->componentsFlags[i];

                    if ((targetFlags & currEntityFlags) == targetFlags) {
                        TextComponent *curr = (TextComponent *)(uiEcs->components[TEXT_COMPONENT].dense[i]);
                        if (curr->selected) {
                            curr->selected = 0;
                            if (strcmp(curr->text, "Play") == 0) {
                                // wrap around to `Exit`
                                for (Uint64 j = i; j < uiEcs->entityCount; j++) {
                                    TextComponent *fetch = (TextComponent *)(uiEcs->components[TEXT_COMPONENT].dense[j]);
                                    if (strcmp(fetch->text, "Exit") == 0) {
                                        fetch->selected = 1;  // select "Exit"
                                        break;
                                    }
                                }
                            } else {
                                // select the previous option
                                (*(TextComponent *)(uiEcs->components[TEXT_COMPONENT].dense[i-1])).selected = 1;
                            }
                            updateMenuUI(uiEcs, rdr);
                            break;
                        }
                    }
                }
                break;
            }
            case SDLK_DOWN:
            case SDLK_s: {
                for (Uint64 i = 0; i < uiEcs->entityCount; i++) {
                    bitset targetFlags = 1 << TEXT_COMPONENT;
                    bitset currEntityFlags = uiEcs->componentsFlags[i];

                    if ((targetFlags & currEntityFlags) == targetFlags) {
                        TextComponent *curr = (TextComponent *)(uiEcs->components[TEXT_COMPONENT].dense[i]);
                        if (curr->selected) {
                            curr->selected = 0;
                            if (strcmp(curr->text, "Exit") == 0) {
                                // wrap around to `Play`
                                for (Uint64 j = i; j >= 0; j--) {
                                    TextComponent *fetch = (TextComponent *)(uiEcs->components[TEXT_COMPONENT].dense[j]);
                                    if (strcmp(fetch->text, "Play") == 0) {
                                        fetch->selected = 1;  // select "Play"
                                        break;
                                    }
                                }
                            } else {
                                // select the next option
                                (*(TextComponent *)(uiEcs->components[TEXT_COMPONENT].dense[i+1])).selected = 1;
                            }
                            updateMenuUI(uiEcs, rdr);
                            break;
                        }
                    }
                }
                break;
            }
            case SDLK_RETURN:
            case SDLK_SPACE: {
                for (Uint64 i = 0; i < uiEcs->entityCount; i++) {
                    TextComponent *curr = (TextComponent *)(uiEcs->components[TEXT_COMPONENT].dense[i]);
                    if (curr->selected) {
                        if (strcmp(curr->text, "Play") == 0) {
                            onExitMainMenu(uiEcs, rdr);  // Clear the main menu UI
                            // onEnterPlayState(gEcs, rdr);
                            *currState = STATE_PLAYING;  // Start the game
                        } else if (strcmp(curr->text, "Exit") == 0) {
                            *currState = STATE_EXIT;  // Exit the game
                        }
                    }
                }
                break;
            }
        }
    }
}

void renderMainMenu(SDL_Renderer *rdr, ECS uiEcs) {
    // Clear the screen
    SDL_SetRenderDrawColor(rdr, 100, 50, 0, 200);  // background color - brownish
    SDL_RenderClear(rdr);

    bitset targetMask = 1 << TEXT_COMPONENT;

    for (Uint64 i = 0; i < uiEcs->entityCount; i++) {
        // printf("Component %ld's flag: %d\nTarget flag: %d\n", i, uiEcs->componentsFlags[i], targetMask);
        // printf("uiEcs->componentsFlags[i] (%d) & targetMask (%d) == targetMask (%d): %d\n", uiEcs->componentsFlags[i], targetMask, targetMask, (uiEcs->componentsFlags[i] & targetMask) == targetMask);
        if ((uiEcs->componentsFlags[i] & targetMask) == targetMask) {
            // if the entity has the text component
            SDL_RenderCopy(
                rdr,
                (*(TextComponent *)(uiEcs->components[TEXT_COMPONENT].dense[i])).texture,
                NULL,
                (*(TextComponent *)(uiEcs->components[TEXT_COMPONENT].dense[i])).destRect
            );
        }
    }

    SDL_RenderPresent(rdr);  // render current frame
}