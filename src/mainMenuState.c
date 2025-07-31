#include "include/stateManager.h"

void onEnterMainMenu(ZENg zEngine) {
    // add the entities(text) with render components to the UI ECS
    Uint8 orderIdx = 0;  // counts the inserted texts

    // Create the title text component
    TextComponent *title = createTextComponent(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf"),
        strdup("Adele's Adventure"), COLOR_WHITE, 1
    );
    title->destRect->x = (zEngine->display->currentMode.w - title->destRect->w) / 2;
    title->destRect->y = (zEngine->display->currentMode.h - title->destRect->h) / 3;

    Entity id = createEntity(zEngine->uiEcs);
    addComponent(zEngine->uiEcs, id, TEXT_COMPONENT, (void *)title);

    // "Play" option
    ButtonComponent *play = createButtonComponent(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf"),
        strdup("Play"), COLOR_YELLOW, &mMenuToPlay, 1, orderIdx++
    );
    play->destRect->x = (zEngine->display->currentMode.w - play->destRect->w) / 2;
    play->destRect->y = (zEngine->display->currentMode.h - play->destRect->h) / 2;

    id = createEntity(zEngine->uiEcs);  // get a new entity's ID
    addComponent(zEngine->uiEcs, id, BUTTON_COMPONENT, (void *)play);

    // "options" option
    ButtonComponent *options = createButtonComponent(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf"),
        strdup("Options"), COLOR_WHITE, &mMenuToOptions, 0, orderIdx++
    );
    options->destRect->x = (zEngine->display->currentMode.w - options->destRect->w) / 2;
    options->destRect->y = (zEngine->display->currentMode.h - options->destRect->h) * 4 / 7;

    id = createEntity(zEngine->uiEcs);  // get a new entity's ID
    addComponent(zEngine->uiEcs, id, BUTTON_COMPONENT, (void *)options);

    // "Exit" option
    ButtonComponent *exitOpt = createButtonComponent(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf"),
        strdup("Exit"), COLOR_WHITE, &prepareExit, 0, orderIdx++
    );
    exitOpt->destRect->x = (zEngine->display->currentMode.w - exitOpt->destRect->w) / 2;
    exitOpt->destRect->y = (zEngine->display->currentMode.h - exitOpt->destRect->h) * 5 / 7;

    id = createEntity(zEngine->uiEcs);  // get a new entity's ID
    addComponent(zEngine->uiEcs, id, BUTTON_COMPONENT, (void *)exitOpt);

    // Instructions
    TextComponent *instructions = createTextComponent(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf"),
        strdup("Use W/S or arrow keys to navigate, Enter/Space to select"), COLOR_WHITE_TRANSPARENT, 1
    );
    instructions->destRect->x = (zEngine->display->currentMode.w - instructions->destRect->w) / 2;
    instructions->destRect->y = (zEngine->display->currentMode.h - instructions->destRect->h) * 6 / 7;

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
        SDL_FreeSurface(surface);
    }

    // rendering in this state is done only when components change
    renderMenu(zEngine);
}

/**
 * =====================================================================================================================
 */

Uint8 handleMainMenuEvents(SDL_Event *event, ZENg zEngine) {
    return handleMenuNavigation(event, zEngine, "Play", "Exit", &updateMenuUI);
}

Uint8 handleMenuNavigation(SDL_Event *event, ZENg zEngine, char *firstItem, char *lastItem, void (*renderFunc)(ZENg)) {
    // Key press handling
    if (event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_F11) {
            // test feature - fullscreen switch
            toggleFullscreen(zEngine->display);
            return 1;
        }

        if (event->key.keysym.sym == SDLK_F10) {
            SDL_DisplayMode mode = {
                .w = 1600,
                .h = 900,
                .format = SDL_PIXELFORMAT_RGBA8888,
                .refresh_rate = 60,
                .driverdata = NULL
            };

            int modeCount = 0;
            SDL_DisplayMode *availableModes = getAvailableDisplayModes(zEngine->display, &modeCount);
            for (int i = 0; i < modeCount; i++) {
                printf("Available mode %d: %dx%d @ %dHz\n", i, availableModes[i].w, availableModes[i].h, availableModes[i].refresh_rate);
            }
            setDisplayMode(zEngine->display, &mode);
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
                for (Uint64 i = 0; i < zEngine->uiEcs->components[BUTTON_COMPONENT].denseSize; i++) {
                    ButtonComponent *curr = (ButtonComponent *)(zEngine->uiEcs->components[BUTTON_COMPONENT].dense[i]);
                    if (curr->selected) {
                        curr->selected = 0;
                        if (strcmp(curr->text, firstItem) == 0) {
                            // wrap around to the last list item
                            // starting at 0 because the ids can be reused
                            for (Uint64 j = 0; j < zEngine->uiEcs->components[BUTTON_COMPONENT].denseSize; j++) {
                                ButtonComponent *fetch = (ButtonComponent *)(zEngine->uiEcs->components[BUTTON_COMPONENT].dense[j]);
                                if (strcmp(fetch->text, lastItem) == 0) {
                                    fetch->selected = 1;  // select the last item
                                    break;
                                }
                            }
                        } else {
                            // select the previous option
                            for (Uint64 j = 0; j < zEngine->uiEcs->components[BUTTON_COMPONENT].denseSize; j++) {
                                ButtonComponent *fetch = (ButtonComponent *)(zEngine->uiEcs->components[BUTTON_COMPONENT].dense[j]);
                                // find the previous item
                                if (fetch->orderIdx == curr->orderIdx - 1) {
                                    fetch->selected = 1;
                                    break;
                                }
                            }
                        }
                        renderFunc(zEngine);
                        return 1;
                    }
                }
                return 1;
            }
            case INPUT_MOVE_DOWN: {
                for (Uint64 i = 0; i < zEngine->uiEcs->components[BUTTON_COMPONENT].denseSize; i++) {
                    ButtonComponent *curr = (ButtonComponent *)(zEngine->uiEcs->components[BUTTON_COMPONENT].dense[i]);
                    if (curr->selected) {
                        curr->selected = 0;
                        if (strcmp(curr->text, lastItem) == 0) {
                            // wrap around to the last list item
                            for (Uint64 j = 0; j < zEngine->uiEcs->components[BUTTON_COMPONENT].denseSize; j++) {
                                ButtonComponent *fetch = (ButtonComponent *)(zEngine->uiEcs->components[BUTTON_COMPONENT].dense[j]);
                                if (strcmp(fetch->text, firstItem) == 0) {
                                    fetch->selected = 1;  // select the first item
                                    break;
                                }
                            }
                        } else {
                            // select the next option
                                for (Uint64 j = 0; j < zEngine->uiEcs->components[BUTTON_COMPONENT].denseSize; j++) {
                                ButtonComponent *fetch = (ButtonComponent *)(zEngine->uiEcs->components[BUTTON_COMPONENT].dense[j]);
                                if (fetch->orderIdx == curr->orderIdx + 1) {
                                    fetch->selected = 1;
                                    break;
                                }
                            }
                        }
                        renderFunc(zEngine);
                        return 1;
                    }
                }
                return 1;
            }
            case INPUT_SELECT: {
                for (Uint64 i = 0; i < zEngine->uiEcs->components[BUTTON_COMPONENT].denseSize; i++) {
                    ButtonComponent *curr = (ButtonComponent *)(zEngine->uiEcs->components[BUTTON_COMPONENT].dense[i]);
                    if (curr->selected) {
                        curr->onClick(zEngine);
                    }
                }
                return 1;
            }
            case INPUT_BACK: {
                if (getCurrState(zEngine->stateMng)->type != STATE_MAIN_MENU) {
                    popState(zEngine);
                    if (getCurrState(zEngine->stateMng)->type == STATE_MAIN_MENU) {
                        renderMenu(zEngine);
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

void renderMenu(ZENg zEngine) {
    // Clear the screen
    SDL_SetRenderDrawColor(zEngine->display->renderer, 100, 50, 0, 200);  // background color - brownish
    SDL_RenderClear(zEngine->display->renderer);

    // Text
    for (Uint64 i = 0; i < zEngine->uiEcs->components[TEXT_COMPONENT].denseSize; i++) {
        TextComponent *curr = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[i]);
        SDL_RenderCopy(
            zEngine->display->renderer,
            curr->texture,
            NULL,
            curr->destRect
        );
    }

    // Then buttons
    for (Uint64 i = 0; i < zEngine->uiEcs->components[BUTTON_COMPONENT].denseSize; i++) {
        ButtonComponent *curr = (ButtonComponent *)(zEngine->uiEcs->components[BUTTON_COMPONENT].dense[i]);
        SDL_RenderCopy(
            zEngine->display->renderer,
            curr->texture,
            NULL,
            curr->destRect
        );
    }
}

/**
 * =====================================================================================================================
 */

void mMenuToPlay(ZENg zEngine) {
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
}

/**
 * =====================================================================================================================
 */

void mMenuToOptions(ZENg zEngine) {
    // push the options state to the gamestate stack
    GameState *optionsState = calloc(1, sizeof(GameState));
    if (!optionsState) {
        printf("Failed to allocate memory for options state\n");
        exit(EXIT_FAILURE);
    }
    optionsState->type = STATE_OPTIONS;
    optionsState->isOverlay = 0;  // this state doesn't use much resources
    optionsState->onEnter = &onEnterOptionsMenu;
    optionsState->onExit = &onExitOptionsMenu;
    optionsState->handleEvents = &handleOptionsMenuEvents;
    optionsState->handleInput = NULL;  // no continuous input
    optionsState->update = NULL;  // no game logic update
    optionsState->render = NULL;  // rendering is done only when needed
    pushState(zEngine, optionsState);
    renderMenu(zEngine);  // render the options menu once
}

/**
 * =====================================================================================================================
 */

void prepareExit(ZENg zEngine) {
    getCurrState(zEngine->stateMng)->type = STATE_EXIT;
}
