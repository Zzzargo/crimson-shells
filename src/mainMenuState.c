#include "include/stateManager.h"

void onEnterMainMenu(ZENg zEngine) {
    // Percentages from the top of the (logical) screen
    double titlePos = 0.3;
    double footerPos = 0.8;
    double listStartPos = 0.45;
    double listItemsSpacing = 0.08;

    UINode *titleLabel = UIcreateLabel(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#48"),
        strdup("Crimson Shells"), COLOR_CRIMSON
    );
    UILabel *titleUILabel = (UILabel *)(titleLabel->widget);
    titleUILabel->destRect->x = (LOGICAL_WIDTH - titleUILabel->destRect->w) / 2;
    titleUILabel->destRect->y = LOGICAL_HEIGHT * titlePos;
    UIinsertNode(zEngine->uiManager, zEngine->uiManager->root, titleLabel);

    char* buttonLabels[] = {
        "Play", "Options", "Exit"
    };

    size_t buttonCount = sizeof(buttonLabels) / sizeof(buttonLabels[0]);

    void (*buttonActions[])(ZENg, void *) = {
        &mMenuToPlay, &mMenuToOptions, &prepareExit
    };

    for (Uint8 i = 0; i < buttonCount; i++) {
        UINode *button = UIcreateButton(
            zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#28"),
            strdup(buttonLabels[i]), i == 0 ? UI_STATE_FOCUSED : UI_STATE_NORMAL,
            (SDL_Color[]){ COLOR_WHITE, COLOR_YELLOW, COLOR_WHITE_TRANSPARENT },
            buttonActions[i], NULL
        );

        if (i == 0) {
            // First button gets the focus
            zEngine->uiManager->focusedNode = button;
        }

        UIButton *btnWidget = (UIButton *)(button->widget);
        btnWidget->destRect->x = (LOGICAL_WIDTH - btnWidget->destRect->w) / 2;
        btnWidget->destRect->y = LOGICAL_HEIGHT * (listStartPos + i * listItemsSpacing);
        UIinsertNode(zEngine->uiManager, zEngine->uiManager->root, button);
    }

    UINode *footerLabel = UIcreateLabel(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#28"),
        strdup("2024 © Zargo Games"), COLOR_WHITE_TRANSPARENT
    );
    UILabel *footerUILabel = (UILabel *)(footerLabel->widget);
    footerUILabel->destRect->x = (LOGICAL_WIDTH - footerUILabel->destRect->w) / 2;
    footerUILabel->destRect->y = LOGICAL_HEIGHT * footerPos;
    UIinsertNode(zEngine->uiManager, zEngine->uiManager->root, footerLabel);

    // Enable the needed systems
    zEngine->ecs->depGraph->nodes[SYS_RENDER]->isActive = 1;  // Render system will always be on
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
    return handleMenuNavigation(event, zEngine, "Play", "Exit");
}

Uint8 handleMenuNavigation(SDL_Event *event, ZENg zEngine, char *firstItem, char *lastItem) {
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
                    printf("ANOMALY. Not found a previous focusable sibling for current focused node\n");
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
                    printf("ANOMALY. Not found a next focusable sibling for current focused node\n");
                }
                return 1;
            }
            case INPUT_SELECT: {
                UINode *focused = zEngine->uiManager->focusedNode;
                if (focused && focused->type == UI_BUTTON) {
                    UIButton *btnWidget = (UIButton *)(focused->widget);
                    if (btnWidget->onClick) {
                        btnWidget->onClick(zEngine, btnWidget->data);
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

void mMenuToOptions(ZENg zEngine, void *data) {
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

void prepareExit(ZENg zEngine, void *data) {
    getCurrState(zEngine->stateMng)->type = STATE_EXIT;
}
