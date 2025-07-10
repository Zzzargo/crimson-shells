#include "include/engine.h"

ZENg initGame() {
    ZENg zEngine = calloc(1, sizeof(struct engine));
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Initialize font manager
    initFonts(&zEngine->fonts);

    // Initialize ECS
    initGECS(&zEngine->gEcs);
    initUIECS(&zEngine->uiEcs);

    // Create a window
    (zEngine->window) = SDL_CreateWindow(
        "GOAT Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280, 720,  // a medium 16:9 window in the center of the monitor if windowed mode
        SDL_WINDOW_FULLSCREEN_DESKTOP
    );
    if (!zEngine->window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Create a renderer
    (zEngine->renderer) = SDL_CreateRenderer(zEngine->window, -1, SDL_RENDERER_ACCELERATED);
    if (!zEngine->renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(zEngine->window);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // start on the main menu
    zEngine->state = STATE_MAIN_MENU;
    return zEngine;
}

void destroyEngine(ZENg *zEngine) {
    freeECS((*zEngine)->gEcs);
    freeECS((*zEngine)->uiEcs);
    freeFonts(&((*zEngine)->fonts));

    SDL_DestroyRenderer((*zEngine)->renderer);
    SDL_DestroyWindow((*zEngine)->window);
    SDL_Quit();
    free(*zEngine);
}