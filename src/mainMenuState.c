#include "include/stateManager.h"

void onEnterMainMenu(ZENg zEngine) {
    // add the entities(text) with render components to the UI ECS

    // Create the title text component
    TextComponent *title = calloc(1, sizeof(TextComponent));
    if (!title) {
        printf("Failed to allocate memory for title text component\n");
        exit(EXIT_FAILURE);
    }

    // title->state = STATE_MAIN_MENU;  FIX
    title->active = 1;
    title->selected = 0;  // not selectable
    title->orderIdx = 0;  // first item
    title->font = getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf");
    title->text = strdup("GOAT Game");
    title->color = COLOR_WHITE_TRANSPARENT;  // semi-transparent white

    SDL_Surface *titleSurface = TTF_RenderText_Solid(title->font, title->text, COLOR_WHITE_TRANSPARENT);
    if (!titleSurface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    title->texture = SDL_CreateTextureFromSurface(zEngine->display->renderer, titleSurface);
    if (!title->texture) {
        printf("Failed to create text texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    title->destRect = calloc(1, sizeof(SDL_Rect));
    if (!title->destRect) {
        printf("Failed to allocate memory for title rectangle\n");
        exit(EXIT_FAILURE);
    }
    int wW, wH;
    SDL_GetWindowSize(zEngine->display->window, &wW, &wH);  // Get the window size
    // center the title horizontally and position it at the top of the options
    *title->destRect = (SDL_Rect) {
        .x = (wW - titleSurface->w) / 2,  // center horizontally
        .y = (wH - titleSurface->h) / 4,  // position at the top of the options
        .w = titleSurface->w,
        .h = titleSurface->h
    };

    SDL_FreeSurface(titleSurface);  // we don't need the surface anymore

    Entity id = createEntity(zEngine->uiEcs);
    addComponent(zEngine->uiEcs, id, TEXT_COMPONENT, (void *)title);

    // "Play" option
    TextComponent *play = calloc(1, sizeof(TextComponent));
    if (!play) {
        printf("Failed to allocate memory for play text component\n");
        exit(EXIT_FAILURE);
    }
    
    // play->state = STATE_MAIN_MENU;  FIX
    play->active = 1;
    play->selected = 1;  // "Play" is selected by default
    play->orderIdx = 1;
    play->font = getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf");
    play->text = strdup("Play");
    play->color = COLOR_YELLOW;  // highlighted color

    SDL_Surface *playSurface = TTF_RenderText_Solid(play->font, play->text, play->color);
    if (!playSurface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    play->texture = SDL_CreateTextureFromSurface(zEngine->display->renderer, playSurface);
    if (!play->texture) {
        printf("Failed to create text texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    play->destRect = calloc(1, sizeof(SDL_Rect));
    if (!play->destRect) {
        printf("Failed to allocate memory for play rectangle\n");
        exit(EXIT_FAILURE);
    }
    *play->destRect = (SDL_Rect){
        .x = (wW - playSurface->w) / 2,
        .y = (wH - playSurface->h) / 2,
        .w = playSurface->w,
        playSurface->h
    };
    SDL_FreeSurface(playSurface);

    id = createEntity(zEngine->uiEcs);  // get a new entity's ID
    addComponent(zEngine->uiEcs, id, TEXT_COMPONENT, (void *)play);

    // "Exit" option
    TextComponent *exitOpt = calloc(1, sizeof(TextComponent));
    if (!exitOpt) {
        printf("Failed to allocate memory for exit text component\n");
        exit(EXIT_FAILURE);
    }
    // exitOpt->state = STATE_MAIN_MENU;  FIX
    exitOpt->active = 1;
    exitOpt->selected = 0;  // "Exit" is not selected by default
    exitOpt->orderIdx = 2;
    exitOpt->font = getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf");
    exitOpt->text = strdup("Exit");
    exitOpt->color = COLOR_WHITE;  // normal color

    SDL_Surface *exitSurface = TTF_RenderText_Solid(exitOpt->font, exitOpt->text, exitOpt->color);
    if (!exitSurface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    exitOpt->texture = SDL_CreateTextureFromSurface(zEngine->display->renderer, exitSurface);
    if (!exitOpt->texture){
        printf("Failed to create text texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    exitOpt->destRect = calloc(1, sizeof(SDL_Rect));
    if (!exitOpt->destRect) {
        printf("Failed to allocate memory for exit rectangle\n");
        exit(EXIT_FAILURE);
    }
    *exitOpt->destRect = (SDL_Rect){
        .x = (wW - exitSurface->w) / 2,  // center horizontally
        .y = (wH - exitSurface->h) * 4 / 7,
        .w = exitSurface->w,
        .h = exitSurface->h
    };
    SDL_FreeSurface(exitSurface);
    id = createEntity(zEngine->uiEcs);  // get a new entity's ID
    addComponent(zEngine->uiEcs, id, TEXT_COMPONENT, (void *)exitOpt);

    // Instructions
    TextComponent *instructions = calloc(1, sizeof(TextComponent));
    if (!instructions) {
        printf("Failed to allocate memory for instructions text component\n");
        exit(EXIT_FAILURE);
    }
    // instructions->state = STATE_MAIN_MENU;  FIX
    instructions->active = 1;
    instructions->selected = 0;  // not selectable
    instructions->orderIdx = 3;
    instructions->font = getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf");
    instructions->text = strdup("Use W/S or Arrow Keys to navigate, Enter/Space to select");
    instructions->color = COLOR_WHITE_TRANSPARENT;

    SDL_Surface *instrSurface = TTF_RenderText_Solid(instructions->font, instructions->text, instructions->color);
    if (!instrSurface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    instructions->texture = SDL_CreateTextureFromSurface(zEngine->display->renderer, instrSurface);
    if (!instructions->texture) {
        printf("Failed to create text texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    instructions->destRect = calloc(1, sizeof(SDL_Rect));
    if (!instructions->destRect) {
        printf("Failed to allocate memory for instructions rectangle\n");
        exit(EXIT_FAILURE);
    }
    *instructions->destRect = (SDL_Rect){
        .x = (wW - instrSurface->w) / 2,  // center horizontally
        .y = (wH - instrSurface->h) * 5 / 7,  // position below the options
        .w = instrSurface->w,
        .h = instrSurface->h
    };
    SDL_FreeSurface(instrSurface);

    id = createEntity(zEngine->uiEcs);  // get a new entity's ID
    addComponent(zEngine->uiEcs, id, TEXT_COMPONENT, (void *)instructions);
}

void onExitMainMenu(ZENg zEngine) {
    // delete all main menu entities
    while (zEngine->uiEcs->entityCount > 0) {
        deleteEntity(zEngine->uiEcs, zEngine->uiEcs->activeEntities[0]);
    }
}

void updateMenuUI(ZENg zEngine) {
    // Rerender the UI based on the entities' current components' states

    SDL_SetRenderDrawColor(zEngine->display->renderer, 100, 50, 0, 200);  // background color - brownish
    SDL_RenderClear(zEngine->display->renderer);  // clear the renderer

    for (Uint64 i = 0; i < zEngine->uiEcs->components[TEXT_COMPONENT].denseSize; i++) {
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
                curr->texture = SDL_CreateTextureFromSurface(zEngine->display->renderer, surface);
                SDL_RenderCopy(zEngine->display->renderer, curr->texture, NULL, curr->destRect);
                SDL_FreeSurface(surface);
                printf("Updated entity %ld's texture\n", i);
            }
        }
    }
    printf("\n");
}

/**
 * =====================================================================================================================
 */

Uint8 handleMainMenuEvents(SDL_Event *event, ZENg zEngine) {
    // Key press handling
    if (event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_F11) {
            // test feature - fullscreen switch
            toggleFullscreen(zEngine->display);
            return 1;
        }

        if (event->key.keysym.sym == SDLK_F10) {
            setWindowSize(zEngine->display, 1600, 900);
            return 1;
        }

        // pass the pressed key to the input manager
        InputAction action = scancodeToAction(zEngine->inputMng, event->key.keysym.scancode);
        if (action == INPUT_UNKNOWN) {
            printf("Unknown input action for scancode %d\n", event->key.keysym.scancode);
            return 1;
        }

        switch (action) {
            case INPUT_MOVE_UP: {
                for (Uint64 i = 0; i < zEngine->uiEcs->components[TEXT_COMPONENT].denseSize; i++) {
                    TextComponent *curr = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[i]);
                    if (curr->selected) {
                        curr->selected = 0;
                        if (strcmp(curr->text, "Play") == 0) {
                            // wrap around to `Exit`
                            // starting at 0 because the ids can be reused
                            for (Uint64 j = 0; j < zEngine->uiEcs->components[TEXT_COMPONENT].denseSize; j++) {
                                TextComponent *fetch = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[j]);
                                if (strcmp(fetch->text, "Exit") == 0) {
                                    fetch->selected = 1;  // select "Exit"
                                    break;
                                }
                            }
                        } else {
                            // select the previous option
                            for (Uint64 j = 0; j < zEngine->uiEcs->components[TEXT_COMPONENT].denseSize; j++) {
                                TextComponent *fetch = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[j]);
                                // find the previous item
                                if (fetch->orderIdx == curr->orderIdx - 1) {
                                    fetch->selected = 1;
                                    break;
                                }
                            }
                        }
                        updateMenuUI(zEngine);
                        return 1;
                    }
                }
                return 1;
            }
            case INPUT_MOVE_DOWN: {
                for (Uint64 i = 0; i < zEngine->uiEcs->components[TEXT_COMPONENT].denseSize; i++) {
                    TextComponent *curr = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[i]);
                    if (curr->selected) {
                        curr->selected = 0;
                        if (strcmp(curr->text, "Exit") == 0) {
                            // wrap around to `Play`
                            for (Uint64 j = 0; j < zEngine->uiEcs->components[TEXT_COMPONENT].denseSize; j++) {
                                TextComponent *fetch = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[j]);
                                if (strcmp(fetch->text, "Play") == 0) {
                                    fetch->selected = 1;  // select "Play"
                                    break;
                                }
                            }
                        } else {
                            // select the next option
                                for (Uint64 j = 0; j < zEngine->uiEcs->components[TEXT_COMPONENT].denseSize; j++) {
                                TextComponent *fetch = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[j]);
                                if (fetch->orderIdx == curr->orderIdx + 1) {
                                    fetch->selected = 1;
                                    break;
                                }
                            }
                        }
                        updateMenuUI(zEngine);
                        return 1;
                    }
                }
                return 1;
            }
            case INPUT_SELECT: {
                for (Uint64 i = 0; i < zEngine->uiEcs->components[TEXT_COMPONENT].denseSize; i++) {
                    TextComponent *curr = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[i]);
                    if (curr->selected) {
                        if (strcmp(curr->text, "Play") == 0) {
                            // push the play state to the gamestate stack
                            GameState *playState = calloc(1, sizeof(GameState));
                            if (!playState) {
                                printf("Failed to allocate memory for play state\n");
                                exit(EXIT_FAILURE);
                            }
                            playState->type = STATE_PLAYING;
                            playState->onEnter = &onEnterPlayState;
                            playState->onExit = &onExitPlayState;
                            playState->handleEvents = &handlePlayStateEvents;
                            playState->handleInput = &handlePlayStateInput;
                            playState->update = &updatePlayStateLogic;
                            playState->render = &renderPlayState;
                            pushState(zEngine, playState);
                        } else if (strcmp(curr->text, "Exit") == 0) {
                            getCurrState(zEngine->stateMng)->type = STATE_EXIT;  // Exit the game
                            return 0;
                        }
                    }
                }
                return 1;
            }
        }
    }
    return 1;  // no input
}

/**
 * =====================================================================================================================
 */

void renderMainMenu(ZENg zEngine) {
    // Clear the screen
    SDL_SetRenderDrawColor(zEngine->display->renderer, 100, 50, 0, 200);  // background color - brownish
    SDL_RenderClear(zEngine->display->renderer);

    for (Uint64 i = 0; i < zEngine->uiEcs->components[TEXT_COMPONENT].denseSize; i++) {
        TextComponent *curr = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[i]);
        SDL_RenderCopy(
            zEngine->display->renderer,
            curr->texture,
            NULL,
            curr->destRect
        );
    }
}