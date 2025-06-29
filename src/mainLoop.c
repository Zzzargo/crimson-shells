#include "include/mainLoop.h"

void initGame(SDL_Window **wdw, SDL_Renderer **rdr, GameECS *ecs, UIECS *uiEcs, FontManager *fonts) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Initialize font manager
    initFonts(fonts);

    // Initialize ECS
    initGECS(ecs);
    initUIECS(uiEcs);

    // Create a window
    (*wdw) = SDL_CreateWindow(
        "GOAT Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1600, 900,
        SDL_WINDOW_SHOWN
    );
    if (!wdw) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Create a renderer
    (*rdr) = SDL_CreateRenderer(*wdw, -1, SDL_RENDERER_ACCELERATED);
    if (!rdr) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(*wdw);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
}

void onEnterMainMenu(UIECS uiEcs, SDL_Renderer *rdr, FontManager fonts) {
    // add the entities(text) with render components to the UI ECS

    // Title
    char *titleText = "GOAT Game";
    SDL_Surface *titleSurface = TTF_RenderText_Solid(fonts->titleFont, titleText, COLOR_WHITE_TRANSPARENT);    
    if (!titleSurface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_Texture *titleTexture = SDL_CreateTextureFromSurface(rdr, titleSurface);
    if (!titleTexture) {
        printf("Failed to create text texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_Rect titleRect = {800 - titleSurface->w/2, 200, titleSurface->w, titleSurface->h};
    addUiTextEntity(uiEcs, fonts->titleFont, titleText, COLOR_WHITE_TRANSPARENT, titleTexture, &titleRect);
    SDL_FreeSurface(titleSurface);

    // "Play" option
    SDL_Color playColor = COLOR_YELLOW;
    char *playText = "Play";
    SDL_Surface *playSurface = TTF_RenderText_Solid(fonts->menuFont, playText, playColor);
    if (!playSurface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_Texture *playTexture = SDL_CreateTextureFromSurface(rdr, playSurface);
    if (!playTexture){
        printf("Failed to create text texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_Rect playRect = {800 - playSurface->w/2, 400, playSurface->w, playSurface->h};
    addUiTextEntity(uiEcs, fonts->menuFont, playText, playColor, playTexture, &playRect);
    uiEcs->textComponents[uiEcs->entityCount - 1].selected = 1;  // "Play" is selected by default
    SDL_FreeSurface(playSurface);

    // "Exit" option
    SDL_Color exitColor = COLOR_WHITE;
    char *exitText = "Exit";
    SDL_Surface *exitSurface = TTF_RenderText_Solid(fonts->menuFont, exitText, exitColor);
    if (!exitSurface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_Texture *exitTexture = SDL_CreateTextureFromSurface(rdr, exitSurface);
    if (!exitTexture){
        printf("Failed to create text texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_Rect exitRect = {800 - exitSurface->w/2, 450, exitSurface->w, exitSurface->h};
    addUiTextEntity(uiEcs, fonts->menuFont, exitText, exitColor, exitTexture, &exitRect);
    SDL_FreeSurface(exitSurface);
    
    // Instructions
    char *instructionsText = "Use W/S or Arrow Keys to navigate, Enter/Space to select";
    SDL_Surface *instrSurface = TTF_RenderText_Solid(fonts->menuFont, instructionsText, COLOR_WHITE);
    if (!instrSurface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_Texture *instrTexture = SDL_CreateTextureFromSurface(rdr, instrSurface);
    if (!instrTexture) {
        printf("Failed to create text texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_Rect instrRect = {800 - instrSurface->w/2, 600, instrSurface->w, instrSurface->h};
    addUiTextEntity(uiEcs, fonts->menuFont, instructionsText, COLOR_WHITE, instrTexture, &instrRect);
    SDL_FreeSurface(instrSurface);
}

void onExitMainMenu(UIECS uiEcs, SDL_Renderer *rdr) {
    // Clear the main menu UI components from the ECS
    for (int64_t i = (int64_t)uiEcs->entityCount - 1; i >= 0; i--) {
        TextComponent *curr = &uiEcs->textComponents[i];
        if (curr->active) {
            deleteUiTextEntity(uiEcs, i);
        }
    }
    SDL_SetRenderDrawColor(rdr, 0, 0, 0, 255);  // Clear the renderer with black
    SDL_RenderClear(rdr);
    SDL_RenderPresent(rdr);  // Present the cleared renderer
}

void onEnterPlayState(GameECS ecs, SDL_Renderer *rdr) {
    // Add the initial game entities to the ECS
    HealthComponent health = {1, 100, 100};  // Active, max health, current health
    SpeedComponent speed = {1, 0.0, 10.0};

    SDL_Rect *dotRect = malloc(sizeof(SDL_Rect));
    if (!dotRect) {
        printf("Failed to allocate memory for dot rectangle\n");
        exit(EXIT_FAILURE);
    }
    // Initial position and size of the dot
    dotRect->x = 800 - 16;  // Centered horizontally
    dotRect->y = 450 - 16;  // Centered vertically
    dotRect->w = 32;  // Width of the dot
    dotRect->h = 32;  // Height of the dot

    SDL_Texture *dotTexture = SDL_CreateTexture(
        rdr,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        dotRect->w,
        dotRect->h
    );
    if (!dotTexture) {
        printf("Failed to create dot texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_SetRenderTarget(rdr, dotTexture);  // draw only to the dot texture
    SDL_SetRenderDrawColor(rdr, 255, 255, 255, 255);  // White color for the dot
    SDL_RenderFillRect(rdr, NULL);  // Fill the rectangle with white color
    SDL_SetRenderTarget(rdr, NULL);  // Reset the render target

    RenderComponent render = {1, 0, dotTexture, dotRect};  // Active, not selected, texture, destination rectangle

    // put the created entity into the ECS
    spawnGameEntity(ecs, health, speed, render);
}