#include "stateManager.h"

void onEnterMainMenu(ZENg zEngine) {
    zEngine->uiManager->root = UIparseFromFile(zEngine, "data/states/UImainMenuState.json");

    UIapplyLayout(zEngine->uiManager->root);

    // Enable the needed systems
    // Those two will remain active during the whole game
    zEngine->ecs->depGraph->nodes[SYS_RENDER]->isActive = 1;
    zEngine->ecs->depGraph->nodes[SYS_UI]->isActive = 1;
}

void onExitMainMenu(ZENg zEngine) {
    // Delete all main menu entities
    sweepState(zEngine->ecs, STATE_MAIN_MENU);
    UIclear(zEngine->uiManager);
}

/**
 * =====================================================================================================================
 */

Uint8 handleMainMenuEvents(SDL_Event *event, ZENg zEngine) {
    return handleMenuNavigation(event, zEngine);
}

Uint8 handleMenuNavigation(SDL_Event *event, ZENg zEngine) {
    // Key press handling
    if (event->type == SDL_KEYDOWN) {

        // pass the pressed key to the input manager
        InputAction action = scancodeToAction(zEngine->inputMng, event->key.keysym.scancode);
        if (action == INPUT_UNKNOWN) {
            printf("Unknown input action for scancode %d\n", event->key.keysym.scancode);
            return 1;
        }

        switch (action) {
            case INPUT_MOVE_UP: {
                UINode *focused = zEngine->uiManager->focusedNode;
                if (focused && focused->parent && focused->parent->childrenCount > 0) {
                    // Search for the previous focusable node in the focused node's siblings
                    UINode **siblings = focused->parent->children;
                    size_t start = focused->siblingIndex;
                    size_t childCount = focused->parent->childrenCount;
                    for (size_t step = 1; step < childCount; step++) {
                        size_t nextIdx = (start + childCount - step) % childCount;
                        if (UIisNodeFocusable(siblings[nextIdx])) {
                            UIrefocus(zEngine->uiManager, siblings[nextIdx]);
                            return 1;
                        }
                    }
                    #ifdef DEBUG
                        printf("Not found a previous focusable sibling for current focused node\n");
                    #endif
                }
                return 1;
            }
            case INPUT_MOVE_DOWN: {
                UINode *focused = zEngine->uiManager->focusedNode;
                if (focused && focused->parent && focused->parent->childrenCount > 0) {
                    // Search for the next focusable node in the focused node's siblings
                    UINode **siblings = focused->parent->children;
                    size_t start = focused->siblingIndex;
                    size_t childCount = focused->parent->childrenCount;
                    for (size_t step = 1; step < childCount; step++) {
                        size_t nextIdx = (start + step) % childCount;
                        if (UIisNodeFocusable(siblings[nextIdx])) {
                            UIrefocus(zEngine->uiManager, siblings[nextIdx]);
                            return 1;
                        }
                    }
                    #ifdef DEBUG
                        printf("Not found a next focusable sibling for current focused node\n");
                    #endif
                }
                return 1;
            }
            case INPUT_MOVE_RIGHT: {
                UINode *focused = zEngine->uiManager->focusedNode;
                if (focused && focused->type == UI_OPTION_CYCLE) {
                    UIOptionCycle *optCycle = (UIOptionCycle *)(focused->widget);
                    if (optCycle->currOption && optCycle->currOption->next) {
                        // Remove the current option from the node's children
                        UIremoveChild(focused, (UINode *)optCycle->currOption->data.ptr);

                        // And add the next option as child
                        UIaddChild(focused, (UINode *)optCycle->currOption->next->data.ptr);

                        optCycle->currOption = optCycle->currOption->next;
                        // Arrange the new node
                        UIapplyLayout(focused);
                    }
                }
                return 1;
            }
            case INPUT_MOVE_LEFT: {
                UINode *focused = zEngine->uiManager->focusedNode;
                if (focused && focused->type == UI_OPTION_CYCLE) {
                    UIOptionCycle *optCycle = (UIOptionCycle *)(focused->widget);
                    if (optCycle->currOption && optCycle->currOption->prev) {
                        // Remove the current option from the node's children
                        UIremoveChild(focused, (UINode *)optCycle->currOption->data.ptr);

                        // And add the previous option as child
                        UIaddChild(focused, (UINode *)optCycle->currOption->prev->data.ptr);

                        optCycle->currOption = optCycle->currOption->prev;
                        // Arrange the new node
                        UIapplyLayout(focused);
                    }
                }
                return 1;
            }
            case INPUT_SELECT: {
                UINode *focused = zEngine->uiManager->focusedNode;
                if (!focused) {
                    printf("No focused node to select\n");
                    return 1;
                }
                switch (focused->type) {
                    case UI_BUTTON: {
                        UIButton *btnWidget = (UIButton *)(focused->widget);
                        if (btnWidget->onClick) {
                            btnWidget->onClick(zEngine, btnWidget->data);
                        }
                        break;
                    }
                    case UI_OPTION_CYCLE: {
                        UIOptionCycle *optCycle = (UIOptionCycle *)(focused->widget);
                        UIButton *btn = (UIButton *)(optCycle->selector->widget);

                        // The cycle buttons contain the needed data
                        UINode *optNode = (UINode *)optCycle->currOption->data.ptr;
                        switch (optNode->type) {
                            case UI_BUTTON: {
                                UIButton *opt = (UIButton *)optNode->widget;
                                if (btn && btn->onClick && opt->data) {
                                    btn->onClick(zEngine, opt->data);
                                }
                                break;
                            }
                            case UI_IMAGE: {
                                UIImage *opt = (UIImage *)optNode->widget;
                                if (btn && btn->onClick && opt->data) {
                                    btn->onClick(zEngine, opt->data);
                                }
                                break;
                            }
                        }
                        break;
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

void mMenuToPlay(ZENg zEngine, void *data) {
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

void mMenuToSettings(ZENg zEngine, void *data) {
    // push the settings state to the gamestate stack
    GameState *settingsState = calloc(1, sizeof(GameState));
    if (!settingsState) {
        printf("Failed to allocate memory for settings state\n");
        exit(EXIT_FAILURE);
    }
    settingsState->type = STATE_SETTINGS;
    settingsState->isOverlay = 0;  // this state doesn't use much resources
    settingsState->onEnter = &onEnterSettingsMenu;
    settingsState->onExit = &onExitSettingsMenu;
    settingsState->handleEvents = &handleSettingsMenuEvents;
    settingsState->handleInput = NULL;  // no continuous input
    pushState(zEngine, settingsState);
}

/**
 * =====================================================================================================================
 */

void mMenuToGarage(ZENg zEngine, void *data) {
    GameState *garageState = calloc(1, sizeof(GameState));
    if (!garageState) {
        printf("Failed to allocate memory for garage state\n");
        exit(EXIT_FAILURE);
    }
    garageState->type = STATE_GARAGE;
    garageState->onEnter = &onEnterGarage;
    garageState->onExit = &onExitGarage;
    garageState->handleEvents = &handleGarageEvents;
    pushState(zEngine, garageState);
}

/**
 * =====================================================================================================================
 */

void prepareExit(ZENg zEngine, void *data) {
    getCurrState(zEngine->stateMng)->type = STATE_EXIT;
}
