#include "include/stateManager.h"

void onEnterMainMenu(ZENg zEngine) {
    int screenW = zEngine->display->currentMode.w;
    int screenH = zEngine->display->currentMode.h;

    // Percentages from the top of the screen
    double titlePos = 0.3;
    double footerPos = 0.8;
    double listStartPos = 0.45;
    double listItemsSpacing = 0.08;

    // Create buttons with evenly spaced positions
    char* buttonLabels[] = {
        "Play", "Options", "Exit"
    };
    // this is amazing
    void (*buttonActions[])(ZENg) = {
        &mMenuToPlay, &mMenuToOptions, &prepareExit
    };

    // Create the title text component first
    TextComponent *title = createTextComponent(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf"),
        strdup("Adele's Adventure"), COLOR_WHITE, 1
    );
    title->destRect->x = (screenW - title->destRect->w) / 2;
    title->destRect->y = screenH * titlePos;

    Entity id = createEntity(zEngine->ecs, STATE_MAIN_MENU);
    addComponent(zEngine->ecs, id, TEXT_COMPONENT, (void *)title);

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
        
        id = createEntity(zEngine->ecs, STATE_MAIN_MENU);
        addComponent(zEngine->ecs, id, BUTTON_COMPONENT, (void *)button);
    }

    char *instrText = calloc(64, sizeof(char));
    if (!instrText) {
        printf("Failed to allocate memory for the instructions text\n");
        exit(EXIT_FAILURE);
    }
    snprintf(
        instrText, 64, "Use %s/%s to navigate, %s to select",
        getHRKeyFromInputAction(zEngine->inputMng, INPUT_MOVE_UP),
        getHRKeyFromInputAction(zEngine->inputMng, INPUT_MOVE_DOWN),
        getHRKeyFromInputAction(zEngine->inputMng, INPUT_SELECT)
    );

    // Instructions cause I want to flex the input manager
    TextComponent *instructions = createTextComponent(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf"),
        instrText, COLOR_WHITE_TRANSPARENT, 1
    );
    instructions->destRect->x = (screenW - instructions->destRect->w) / 2;
    instructions->destRect->y = screenH * footerPos;

    id = createEntity(zEngine->ecs, STATE_MAIN_MENU);  // get a new entity's ID
    addComponent(zEngine->ecs, id, TEXT_COMPONENT, (void *)instructions);

    // Enable the needed systems
    zEngine->ecs->depGraph->nodes[SYS_RENDER]->isActive = 1;  // Render system will always be on
    zEngine->ecs->depGraph->nodes[SYS_BUTTONS]->isActive = 1;
}

void onExitMainMenu(ZENg zEngine) {
    // Delete all main menu entities
    sweepState(zEngine->ecs, STATE_MAIN_MENU);

    // Disable the buttons system
    zEngine->ecs->depGraph->nodes[SYS_BUTTONS]->isActive = 0;
}

/**
 * =====================================================================================================================
 */

Uint8 handleMainMenuEvents(SDL_Event *event, ZENg zEngine) {
    return handleMenuNavigation(event, zEngine, "Play", "Exit");
}

Uint8 handleMenuNavigation(SDL_Event *event, ZENg zEngine, char *firstItem, char *lastItem) {
    // Key press handling
    if (event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_F11) {
            // test feature - fullscreen switch
            toggleFullscreen(zEngine->display);
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
                for (Uint64 i = 0; i < zEngine->ecs->components[BUTTON_COMPONENT].denseSize; i++) {
                    ButtonComponent *curr = (ButtonComponent *)(zEngine->ecs->components[BUTTON_COMPONENT].dense[i]);
                    if (curr->selected) {
                        curr->selected = 0;
                        if (strcmp(curr->text, firstItem) == 0) {
                            // wrap around to the last list item
                            // starting at 0 because the ids can be reused
                            for (Uint64 j = 0; j < zEngine->ecs->components[BUTTON_COMPONENT].denseSize; j++) {
                                ButtonComponent *fetch = (ButtonComponent *)(zEngine->ecs->components[BUTTON_COMPONENT].dense[j]);
                                if (strcmp(fetch->text, lastItem) == 0) {
                                    fetch->selected = 1;  // select the last item

                                    Entity currOwner = zEngine->ecs->components[BUTTON_COMPONENT].denseToEntity[i];
                                    Entity fetchOwner = zEngine->ecs->components[BUTTON_COMPONENT].denseToEntity[j];
                                    markComponentDirty(zEngine->ecs, currOwner, BUTTON_COMPONENT);
                                    markComponentDirty(zEngine->ecs, fetchOwner, BUTTON_COMPONENT);
                                    break;
                                }
                            }
                        } else {
                            // select the previous option
                            for (Uint64 j = 0; j < zEngine->ecs->components[BUTTON_COMPONENT].denseSize; j++) {
                                ButtonComponent *fetch = (ButtonComponent *)(zEngine->ecs->components[BUTTON_COMPONENT].dense[j]);
                                // find the previous item
                                if (fetch->orderIdx == curr->orderIdx - 1) {
                                    fetch->selected = 1;

                                    Entity currOwner = zEngine->ecs->components[BUTTON_COMPONENT].denseToEntity[i];
                                    Entity fetchOwner = zEngine->ecs->components[BUTTON_COMPONENT].denseToEntity[j];
                                    markComponentDirty(zEngine->ecs, currOwner, BUTTON_COMPONENT);
                                    markComponentDirty(zEngine->ecs, fetchOwner, BUTTON_COMPONENT);
                                    break;
                                }
                            }
                        }
                        return 1;
                    }
                }
                return 1;
            }
            case INPUT_MOVE_DOWN: {
                for (Uint64 i = 0; i < zEngine->ecs->components[BUTTON_COMPONENT].denseSize; i++) {
                    ButtonComponent *curr = (ButtonComponent *)(zEngine->ecs->components[BUTTON_COMPONENT].dense[i]);
                    if (curr->selected) {
                        curr->selected = 0;
                        if (strcmp(curr->text, lastItem) == 0) {
                            // wrap around to the last list item
                            for (Uint64 j = 0; j < zEngine->ecs->components[BUTTON_COMPONENT].denseSize; j++) {
                                ButtonComponent *fetch = (ButtonComponent *)(zEngine->ecs->components[BUTTON_COMPONENT].dense[j]);
                                if (strcmp(fetch->text, firstItem) == 0) {
                                    fetch->selected = 1;  // select the first item

                                    Entity currOwner = zEngine->ecs->components[BUTTON_COMPONENT].denseToEntity[i];
                                    Entity fetchOwner = zEngine->ecs->components[BUTTON_COMPONENT].denseToEntity[j];
                                    markComponentDirty(zEngine->ecs, currOwner, BUTTON_COMPONENT);
                                    markComponentDirty(zEngine->ecs, fetchOwner, BUTTON_COMPONENT);
                                    break;
                                }
                            }
                        } else {
                            // select the next option
                                for (Uint64 j = 0; j < zEngine->ecs->components[BUTTON_COMPONENT].denseSize; j++) {
                                ButtonComponent *fetch = (ButtonComponent *)(zEngine->ecs->components[BUTTON_COMPONENT].dense[j]);
                                if (fetch->orderIdx == curr->orderIdx + 1) {
                                    fetch->selected = 1;

                                    Entity currOwner = zEngine->ecs->components[BUTTON_COMPONENT].denseToEntity[i];
                                    Entity fetchOwner = zEngine->ecs->components[BUTTON_COMPONENT].denseToEntity[j];
                                    markComponentDirty(zEngine->ecs, currOwner, BUTTON_COMPONENT);
                                    markComponentDirty(zEngine->ecs, fetchOwner, BUTTON_COMPONENT);
                                    break;
                                }
                            }
                        }
                        return 1;
                    }
                }
                return 1;
            }
            case INPUT_SELECT: {
                for (Uint64 i = 0; i < zEngine->ecs->components[BUTTON_COMPONENT].denseSize; i++) {
                    ButtonComponent *curr = (ButtonComponent *)(zEngine->ecs->components[BUTTON_COMPONENT].dense[i]);
                    if (curr->selected) {
                        curr->onClick(zEngine);
                    }
                }
                return 1;
            }
            case INPUT_BACK: {
                if (getCurrState(zEngine->stateMng)->type != STATE_MAIN_MENU) {
                    popState(zEngine);
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
    pushState(zEngine, optionsState);
}

/**
 * =====================================================================================================================
 */

void prepareExit(ZENg zEngine) {
    getCurrState(zEngine->stateMng)->type = STATE_EXIT;
}
