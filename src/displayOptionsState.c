#include "include/stateManager.h"

void onEnterVideoOptions(ZENg zEngine) {
    double titleHeight = 0.3;  // 30% of the screen height
    
    double listHeight = 0.7;  // 70% of the screen height
    double listPaddingTop = 0.1;  // 20% of the list height
    double listSpacing = 0.08;

    double windowModesHeight = 0.2;  // 30% of the list height
    double windowModesPaddingH = 0.1;  // 20% of the div's width
    double windowModesSpacing = 0.5;  // 50% of the div's width

    double resolutionsHeight = 0.2; // 20% of the list height
    double resolutionsPaddingH = 0.1;  // 20% of the div's width
    double resolutionsSpacing = 0.5;  // 50% of the div's width

    Entity id = createEntity(zEngine->ecs, STATE_OPTIONS_VIDEO);
    Uint8 orderIdx = 0;

    UINode *titleDiv = UIcreateContainer(
        (SDL_Rect){.x = 0, .y = 0, .w = LOGICAL_WIDTH, .h = (int)(LOGICAL_HEIGHT * titleHeight)},
        UIcreateLayout(
            UI_LAYOUT_VERTICAL, (UIPadding){0.0, 0.0, 0.0, 0.0},
            (UIAlignment){.h = UI_ALIGNMENT_CENTER, .v = UI_ALIGNMENT_END}, 0.0
        )
    );
    UIinsertNode(zEngine->uiManager, zEngine->uiManager->root, titleDiv);

    UINode *titleLabel = UIcreateLabel(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#48"),
        strdup("Display Options"), COLOR_CRIMSON
    );
    UIinsertNode(zEngine->uiManager, titleDiv, titleLabel);


    UINode *listDiv = UIcreateContainer(
        (SDL_Rect) {
            .x = 0,
            .y = (int)(LOGICAL_HEIGHT * titleHeight),
            .w = LOGICAL_WIDTH,
            .h = (int)(LOGICAL_HEIGHT * listHeight)
        },
        UIcreateLayout(
            UI_LAYOUT_VERTICAL, (UIPadding){.top = listPaddingTop, .bottom = 0.0, .left = 0.0, .right = 0.0},
            (UIAlignment){.h = UI_ALIGNMENT_CENTER, .v = UI_ALIGNMENT_START}, listSpacing
        )
    );
    UIinsertNode(zEngine->uiManager, zEngine->uiManager->root, listDiv);

    char* optionCyclesLabels[] = {
        "Display Mode", "Resolution"
    };
    void (*optionCyclesActions[])(ZENg, void *) = {
        &changeWindowMode, &changeRes
    };
    size_t optionCyclesCount = sizeof(optionCyclesLabels) / sizeof(optionCyclesLabels[0]);

    char* windowModes[] = {
        "Windowed", "Fullscreen"
    };
    size_t windowModesCount = sizeof(windowModes) / sizeof(windowModes[0]);

    UINode *windowModeButton = UIcreateButton(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#28"),
        strdup(optionCyclesLabels[0]), UI_STATE_FOCUSED, (SDL_Color[]) {
            COLOR_WHITE, COLOR_YELLOW, COLOR_WITH_ALPHA(COLOR_WHITE, OPACITY_MEDIUM)
        }, optionCyclesActions[0], NULL  // Data is fetched from the current option in navigation
    );

    // Populate the options list
    CDLLNode *windowModesOptions = NULL;
    for (Uint8 i = 0; i < windowModesCount; i++) {
        Uint8 *modeData = calloc(1, sizeof(Uint8));
        if (!modeData) {
            printf("Failed to allocate memory for window mode data\n");
            exit(EXIT_FAILURE);
        }
        *modeData = i;  // 0 = windowed, 1 = fullscreen
        UINode *button = UIcreateButton(
            zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#28"),
            strdup(windowModes[i]), UI_STATE_NORMAL, (SDL_Color[]) {
                COLOR_WHITE, COLOR_YELLOW, COLOR_WITH_ALPHA(COLOR_WHITE, OPACITY_MEDIUM)
            }, NULL, (void *)modeData
        );
        if (i == 0) {
            windowModesOptions = initList((void *)button);
        } else {
            CDLLInsertLast(windowModesOptions, (void *)button);
        }
    }
    UINode *windowModeCycle = UIcreateOptionCycle(
        (SDL_Rect){.x = 0, .y = 0, .w = LOGICAL_WIDTH * 0.8, .h = LOGICAL_HEIGHT * listHeight * windowModesHeight},
        UIcreateLayout(
            UI_LAYOUT_HORIZONTAL, (UIPadding){
                .bottom = 0.0, .top = 0.0, .left = windowModesPaddingH, .right = windowModesPaddingH
                }, (UIAlignment){.h = UI_ALIGNMENT_CENTER, .v = UI_ALIGNMENT_CENTER}, windowModesSpacing
        ), windowModeButton, windowModesOptions
    );
    // Add those as children to apply layout
    UIinsertNode(zEngine->uiManager, windowModeCycle, windowModeButton);
    UIinsertNode(zEngine->uiManager, windowModeCycle, (UINode *)windowModesOptions->data);

    UIinsertNode(zEngine->uiManager, listDiv, windowModeCycle);
    zEngine->uiManager->focusedNode = windowModeCycle;  // Initial focus on the first option cycle

    int resCount = 0;
    SDL_DisplayMode *modes = getAvailableDisplayModes(zEngine->display, &resCount);
    char **resolutions = calloc(resCount, sizeof(char *));
    if (!resolutions) {
        printf("Failed to allocate memory for resolutions strings\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < resCount; i++) {
        resolutions[i] = calloc(16, sizeof(char));
        if (!resolutions[i]) {
            printf("Failed to allocate memory for a resolution string\n");
            exit(EXIT_FAILURE);
        }
        snprintf(resolutions[i], 16, "%dx%d", modes[i].w, modes[i].h);
    }

    UINode *resButton = UIcreateButton(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#28"),
        strdup(optionCyclesLabels[1]), UI_STATE_NORMAL, (SDL_Color[]) {
            COLOR_WHITE, COLOR_YELLOW, COLOR_WITH_ALPHA(COLOR_WHITE, OPACITY_MEDIUM)
        }, optionCyclesActions[1], NULL  // Data is fetched from the current option in navigation
    );

    // Populate the resolutions list
    CDLLNode *resOptions = NULL;
    for (Uint8 i = 0; i < resCount; i++) {
        UINode *button = UIcreateButton(
            zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#28"),
            strdup(resolutions[i]), UI_STATE_NORMAL, (SDL_Color[]) {
                COLOR_WHITE, COLOR_YELLOW, COLOR_WITH_ALPHA(COLOR_WHITE, OPACITY_MEDIUM)
            }, NULL, (void *)(&modes[i])
        );
        if (i == 0) {
            resOptions = initList((void *)button);
        } else {
            CDLLInsertLast(resOptions, (void *)button);
        }
    }
    UINode *resCycle = UIcreateOptionCycle(
        (SDL_Rect){.x = 0, .y = 0, .w = LOGICAL_WIDTH * 0.8, .h = LOGICAL_HEIGHT * listHeight * resolutionsHeight},
        UIcreateLayout(
            UI_LAYOUT_HORIZONTAL, (UIPadding){
                .bottom = 0.0, .top = 0.0, .left = resolutionsPaddingH, .right = resolutionsPaddingH
                }, (UIAlignment){.h = UI_ALIGNMENT_CENTER, .v = UI_ALIGNMENT_CENTER}, resolutionsSpacing
        ), resButton, resOptions
    );
    // Add those as children to apply layout
    UIinsertNode(zEngine->uiManager, resCycle, resButton);
    UIinsertNode(zEngine->uiManager, resCycle, (UINode *)resOptions->data);

    UIinsertNode(zEngine->uiManager, listDiv, resCycle);

    UIapplyLayout(zEngine->uiManager->root);
}

void onExitVideoOptions(ZENg zEngine) {
    UIclear(zEngine->uiManager);
}

void changeRes(ZENg zEngine, void *data) {
    if (!data) {
        printf("No display mode data provided to changeRes\n");
        return;
    }
    SDL_DisplayMode *mode = (SDL_DisplayMode *)data;
    setDisplayMode(zEngine->display, mode);
}

void changeWindowMode(ZENg zEngine, void *data) {
    if (!data) {
        printf("No window mode data provided to changeWindowMode\n");
        return;
    }
    Uint8 currMode = zEngine->display->fullscreen;
    Uint8 newMode = *((Uint8 *)data);
    if (currMode != newMode) {
        toggleFullscreen(zEngine->display);
    }
}

Uint8 handleVideoOptionsEvents(SDL_Event *event, ZENg zEngine) {
    return handleMenuNavigation(event, zEngine);
}

void videoOptToOpt(ZENg zEngine, void *data) {
    popState(zEngine);  // -> Options menu
}