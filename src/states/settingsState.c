#include "stateManager.h"

void onEnterSettingsMenu(ZENg zEngine) {
    // How much each of the height containers take
    float titleSize = 0.3;
    
    float listSize = 0.7;
    float listPaddingTop = 0.2;  // 20% of the list height
    float listSpacing = 0.08;  // 8% of the list height

    UINode *titleDiv = UIcreateContainer(
        (SDL_Rect){.x = 0, .y = 0, .w = LOGICAL_WIDTH, .h = (int)(LOGICAL_HEIGHT * titleSize)},
        UIcreateLayout(
            UI_LAYOUT_VERTICAL, (UIPadding){0.0, 0.0, 0.0, 0.0},
            (UIAlignment){.h = UI_ALIGNMENT_CENTER, .v = UI_ALIGNMENT_END}, 0.0
        )
    );
    UIinsertNode(zEngine->uiManager, zEngine->uiManager->root, titleDiv);

    UINode *titleLabel = UIcreateLabel(
        zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#48"),
        strdup("Settings"), COLOR_CRIMSON
    );
    UIinsertNode(zEngine->uiManager, titleDiv, titleLabel);


    UINode *listDiv = UIcreateContainer(
        (SDL_Rect) {
            .x = 0,
            .y = (int)(LOGICAL_HEIGHT * titleSize),
            .w = LOGICAL_WIDTH,
            .h = (int)(LOGICAL_HEIGHT * listSize)
        },
        UIcreateLayout(
            UI_LAYOUT_VERTICAL, (UIPadding){.top = listPaddingTop, .bottom = 0.0, .left = 0.0, .right = 0.0},
            (UIAlignment){.h = UI_ALIGNMENT_CENTER, .v = UI_ALIGNMENT_START}, listSpacing
        )
    );
    UIinsertNode(zEngine->uiManager, zEngine->uiManager->root, listDiv);

    char* buttonLabels[] = {
        "Game", "Audio", "Display", "Controls", "Back"
    };
    void (*buttonActions[])(ZENg, void *) = {
        &settingsToGameSettings, &settingsToAudioSettings, &settingsToVideoSettings, &settingsToControlsSettings, &settingsToMMenu
    };
    size_t buttonCount = sizeof(buttonLabels) / sizeof(buttonLabels[0]);

    for (Uint8 i = 0; i < buttonCount; i++) {
        UINode *button = UIcreateButton(
            zEngine->display->renderer, getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf#28"),
            strdup(buttonLabels[i]), i == 0 ? UI_STATE_FOCUSED : UI_STATE_NORMAL,
            (SDL_Color[]){ COLOR_WHITE, COLOR_YELLOW, COLOR_WITH_ALPHA(COLOR_WHITE, OPACITY_MEDIUM) },
            buttonActions[i], NULL
        );

        if (i == 0) {
            // First button gets the focus
            zEngine->uiManager->focusedNode = button;
        }
        UIinsertNode(zEngine->uiManager, listDiv, button);
    }

    UIapplyLayout(zEngine->uiManager->root);
}

/**
 * =====================================================================================================================
 */

void onExitSettingsMenu(ZENg zEngine) {
    UIclear(zEngine->uiManager);
}

/**
 * =====================================================================================================================
 */

Uint8 handleSettingsMenuEvents(SDL_Event *event, ZENg zEngine) {
    return handleMenuNavigation(event, zEngine);
}

/**
 * =====================================================================================================================
 */

void settingsToGameSettings(ZENg zEngine, void *data) {
    GameState *gameSet = calloc(1, sizeof(GameState));
    if (!gameSet) {
        printf("Failed to allocate memory for the game settings state");
        exit(EXIT_FAILURE);
    }
    gameSet->type = STATE_GAME_SETTINGS;
    gameSet->onEnter = &onEnterGameSettings;
    gameSet->onExit = &onExitGameSettings;
    gameSet->handleEvents = &handleGameSettingsEvents;

    pushState(zEngine, gameSet);
}

 /**
 * =====================================================================================================================
 */

void settingsToAudioSettings(ZENg zEngine, void *data) {
    GameState *audioSet = calloc(1, sizeof(GameState));
    if (!audioSet) {
        printf("Failed to allocate memory for the audio settings state");
        exit(EXIT_FAILURE);
    }
    audioSet->type = STATE_AUDIO_SETTINGS;
    audioSet->onEnter = &onEnterAudioSettings;
    audioSet->onExit = &onExitAudioSettings;
    audioSet->handleEvents = &handleAudioSettingsEvents;

    pushState(zEngine, audioSet);
}

/**
 * =====================================================================================================================
 */

void settingsToVideoSettings(ZENg zEngine, void *data) {
    GameState *videoSet = calloc(1, sizeof(GameState));
    if (!videoSet) {
        printf("Failed to allocate memory for the video settings state");
        exit(EXIT_FAILURE);
    }
    videoSet->type = STATE_VIDEO_SETTINGS;
    videoSet->onEnter = &onEnterVideoSettings;
    videoSet->onExit = &onExitVideoSettings;
    videoSet->handleEvents = &handleVideoSettingsEvents;

    pushState(zEngine, videoSet);
}

/**
 * =====================================================================================================================
 */

void settingsToControlsSettings(ZENg zEngine, void *data) {
    GameState *controlsSet = calloc(1, sizeof(GameState));
    if (!controlsSet) {
        printf("Failed to allocate memory for the controls settings state");
        exit(EXIT_FAILURE);
    }
    controlsSet->type = STATE_CONTROLS_SETTINGS;
    controlsSet->onEnter = &onEnterControlsSettings;
    controlsSet->onExit = &onExitControlsSettings;
    controlsSet->handleEvents = &handleControlsSettingsEvents;

    pushState(zEngine, controlsSet);
}

/**
 * =====================================================================================================================
 */

void settingsToMMenu(ZENg zEngine, void *data) {
    popState(zEngine);  // -> Main menu
}