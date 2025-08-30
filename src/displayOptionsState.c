#include "include/stateManager.h"

void onEnterVideoOptions(ZENg zEngine) {
    int screenW = zEngine->display->currentMode.w;
    int screenH = zEngine->display->currentMode.h;

    // Percentages from the top of the screen
    double listStartPos = 0.3;
    double listItemsSpacing = 0.04;

    // Get the available display modes
    int modesCount = 0;
    SDL_DisplayMode *modes = getAvailableDisplayModes(zEngine->display, &modesCount);
    for (Uint8 i = 0; i < modesCount; i++) {
        printf("Available mode %d: %dx%d @ %dHz\n", i, modes[i].w, modes[i].h, modes[i].refresh_rate);
    }

    // Create buttons with evenly spaced positions
    char *buttonLabels[modesCount];
    for (Uint8 i = 0; i < modesCount; i++) {
        buttonLabels[i] = calloc(64, sizeof(char));
        if (!buttonLabels[i]) {
            printf("Failed to allocate memory for display mode button label\n");
            exit(EXIT_FAILURE);
        }
        snprintf(buttonLabels[i], 64, "%dx%d", modes[i].w, modes[i].h);
    }
    Entity id;

    Uint8 orderIdx;
    for (orderIdx = 0; orderIdx < (sizeof(buttonLabels) / sizeof(buttonLabels[0])); orderIdx++) {
        ButtonComponent *button = createButtonComponent(
            zEngine->display->renderer,
            getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf"),
            strdup(buttonLabels[orderIdx]),
            orderIdx == 0 ? COLOR_YELLOW : COLOR_WHITE, // First button selected (color)
            NULL,
            orderIdx == 0 ? 1 : 0,  // First button selected (field flag)
            orderIdx
        );
        
        button->destRect->x = (screenW - button->destRect->w) / 2;
        button->destRect->y = screenH * (listStartPos + orderIdx * listItemsSpacing);
        
        id = createEntity(zEngine->ecs, STATE_OPTIONS_VIDEO);
        addComponent(zEngine->ecs, id, BUTTON_COMPONENT, (void *)button);
    }

    // Back button
    ButtonComponent *backButton = createButtonComponent(
        zEngine->display->renderer,
        getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf"),
        strdup("Back"), COLOR_WHITE, &videoOptToOpt, 0, orderIdx++
    );
    backButton->destRect->x = (screenW - backButton->destRect->w) / 2;
    backButton->destRect->y = screenH * (listStartPos + orderIdx * listItemsSpacing);
    id = createEntity(zEngine->ecs, STATE_OPTIONS_VIDEO);
    addComponent(zEngine->ecs, id, BUTTON_COMPONENT, (void *)backButton);
}

void onExitVideoOptions(ZENg zEngine) {
    onExitMainMenu(zEngine);  // delete all video options entities
}

Uint8 handleVideoOptionsEvents(SDL_Event *event, ZENg zEngine) {
    return handleMenuNavigation(event, zEngine, "1920x1080", "Back");
}

void videoOptToOpt(ZENg zEngine) {
    popState(zEngine);  // -> Options menu
}