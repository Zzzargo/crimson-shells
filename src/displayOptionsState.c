#include "include/stateManager.h"

void onEnterVideoOptions(ZENg zEngine) {
    // Percentages from the top of the screen
    double listStartPos = 0.3;
    double listItemsSpacing = 0.04;

    // Get the available display modes
    int modesCount = 0;
    SDL_DisplayMode *modes = getAvailableDisplayModes(zEngine->display, &modesCount);

    #ifdef DEBUG
        for (Uint8 i = 0; i < modesCount; i++) {
            printf("Available mode %d: %dx%d @ %dHz\n", i, modes[i].w, modes[i].h, modes[i].refresh_rate);
        }
    #endif

    // Create buttons with evenly spaced positions
    char *buttonLabels[modesCount + 1];  // +1 for the "Back" button
    for (Uint8 i = 0; i <= modesCount; i++) {
        buttonLabels[i] = calloc(64, sizeof(char));
        if (!buttonLabels[i]) {
            printf("Failed to allocate memory for display mode button label\n");
            exit(EXIT_FAILURE);
        }
        snprintf(buttonLabels[i], 64, "%dx%d", modes[i].w, modes[i].h);
    }
    snprintf(buttonLabels[modesCount], 64, "Back");

    Entity id;

    Uint8 orderIdx;
    for (orderIdx = 0; orderIdx <= modesCount; orderIdx++) {
        ButtonComponent *button = createButtonComponent(
            zEngine->display->renderer,
            getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#28"),
            strdup(buttonLabels[orderIdx]),
            orderIdx == 0 ? COLOR_YELLOW : COLOR_WHITE, // First button selected (color)
            orderIdx == modesCount ? &videoOptToOpt : &changeRes,
            &modes[orderIdx],  // the button data is a pointer to the current display mode
            orderIdx == 0 ? 1 : 0,  // First button selected (field flag)
            orderIdx
        );

        button->destRect->x = (LOGICAL_WIDTH - button->destRect->w) / 2;
        button->destRect->y = LOGICAL_HEIGHT * (listStartPos + orderIdx * listItemsSpacing);

        id = createEntity(zEngine->ecs, STATE_OPTIONS_VIDEO);
        addComponent(zEngine->ecs, id, BUTTON_COMPONENT, (void *)button);
    }
}

void onExitVideoOptions(ZENg zEngine) {
    sweepState(zEngine->ecs, STATE_OPTIONS_VIDEO);
}

void changeRes(ZENg zEngine, void *data) {
    if (!data) {
        printf("No display mode data provided to changeRes\n");
        return;
    }
    SDL_DisplayMode *mode = (SDL_DisplayMode *)data;
    setDisplayMode(zEngine->display, mode);
}

Uint8 handleVideoOptionsEvents(SDL_Event *event, ZENg zEngine) {
    return handleMenuNavigation(event, zEngine, "1920x1080", "Back");
}

void videoOptToOpt(ZENg zEngine, void *data) {
    popState(zEngine);  // -> Options menu
}