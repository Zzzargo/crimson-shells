#include "states/stateManager.h"

void onEnterSettingsMenu(ZENg zEngine) {
    zEngine->uiManager->root = UIparseFromFile(zEngine, "data/states/UIsettingsState.json");
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
