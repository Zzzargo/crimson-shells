#include "include/stateManager.h"

void onEnterVideoOptions(ZENg zEngine) {
    // // Percentages from the top of the screen
    // double listStartPos = 0.3;
    // double listItemsSpacing = 0.07;
    // double titlePos = 0.15;
    // double footerPos = 0.85;

    // Entity id = createEntity(zEngine->ecs, STATE_OPTIONS_VIDEO);
    // Uint8 orderIdx = 0;

    // // Title
    // TextComponent *title = createTextComponent(
    //     zEngine->display->renderer,
    //     getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#48"),
    //     strdup("Display Options"), COLOR_CRIMSON, 1
    // );
    // title->destRect->x = (LOGICAL_WIDTH - title->destRect->w) / 2;
    // title->destRect->y = LOGICAL_HEIGHT * titlePos;
    // addComponent(zEngine->ecs, id, TEXT_COMPONENT, (void *)title);

    // // Get the available display modes
    // int modesCount = 0;
    // SDL_DisplayMode *modes = getAvailableDisplayModes(zEngine->display, &modesCount);

    // #ifdef DEBUG
    //     for (Uint8 i = 0; i < modesCount; i++) {
    //         printf("Available display mode %d: %dx%d @ %dHz\n", i, modes[i].w, modes[i].h, modes[i].refresh_rate);
    //     }
    // #endif

    // char *resButtonLabels[modesCount];
    // for (Uint8 i = 0; i < modesCount; i++) {
    //     resButtonLabels[i] = calloc(64, sizeof(char));
    //     if (!resButtonLabels[i]) {
    //         printf("Failed to allocate memory for display mode button label\n");
    //         exit(EXIT_FAILURE);
    //     }
    //     snprintf(resButtonLabels[i], 64, "%dx%d", modes[i].w, modes[i].h);
    // }

    // char *leftButtonLabels[] = {
    //     "Display mode", "Resolution"
    // };
    // void (*leftButtonActions[])(ZENg, void *) = {
    //     NULL, &changeRes
    // };
    // size_t leftButtonCount = sizeof(leftButtonLabels) / sizeof(leftButtonLabels[0]);

    // // Create the left column of buttons
    // for (; orderIdx < leftButtonCount; orderIdx++) {
    //     ButtonComponent *leftButton = createButtonComponent(
    //         zEngine->display->renderer,
    //         getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#28"),
    //         strdup(leftButtonLabels[orderIdx]),
    //         orderIdx == 0 ? COLOR_YELLOW : COLOR_WHITE, // First leftButton selected (color)
    //         &changeRes,
    //         &modes[orderIdx],  // the leftButton data is a pointer to the current display mode
    //         orderIdx == 0 ? 1 : 0,  // First leftButton selected (field flag)
    //         orderIdx
    //     );

    //     leftButton->destRect->x = (LOGICAL_WIDTH / 4) - (leftButton->destRect->w / 2);
    //     leftButton->destRect->y = LOGICAL_HEIGHT * (listStartPos + orderIdx * listItemsSpacing);

    //     id = createEntity(zEngine->ecs, STATE_OPTIONS_VIDEO);
    //     addComponent(zEngine->ecs, id, BUTTON_COMPONENT, (void *)leftButton);
    // }

    // // Back button
    // id = createEntity(zEngine->ecs, STATE_OPTIONS_VIDEO);
    // ButtonComponent *backButton = createButtonComponent(
    //     zEngine->display->renderer,
    //     getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#28"),
    //     strdup("Back"),
    //     COLOR_WHITE,
    //     &videoOptToOpt,
    //     NULL,  // no extra data needed
    //     0,  // not selected
    //     orderIdx  // last in order
    // );
    // backButton->destRect->x = (LOGICAL_WIDTH - backButton->destRect->w) / 2;
    // backButton->destRect->y = LOGICAL_HEIGHT * footerPos;
    // addComponent(zEngine->ecs, id, BUTTON_COMPONENT, (void *)backButton);
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
    return handleMenuNavigation(event, zEngine, "Display mode", "Back");
}

void videoOptToOpt(ZENg zEngine, void *data) {
    popState(zEngine);  // -> Options menu
}