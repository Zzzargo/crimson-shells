#include "include/mainLoop.h"

void initGame(SDL_Window **wdw, SDL_Renderer **rdr, ECS *gEcs, ECS *uiEcs, FontManager *fonts) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Initialize font manager
    initFonts(fonts);

    // Initialize ECS
    initGECS(gEcs);
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

void onEnterMainMenu(ECS uiEcs, SDL_Renderer *rdr, FontManager fonts) {
    // add the entities(text) with render components to the UI ECS

    // Create the title text component
    TextComponent *title = calloc(1, sizeof(TextComponent));
    if (!title) {
        printf("Failed to allocate memory for title text component\n");
        exit(EXIT_FAILURE);
    }

    title->state = STATE_MAIN_MENU;
    title->active = 1;
    title->selected = 0;  // not selectable
    title->font = fonts->titleFont;
    title->text = strdup("GOAT Game");
    title->color = COLOR_WHITE_TRANSPARENT;  // semi-transparent white

    SDL_Surface *titleSurface = TTF_RenderText_Solid(title->font, title->text, COLOR_WHITE_TRANSPARENT);
    if (!titleSurface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    title->texture = SDL_CreateTextureFromSurface(rdr, titleSurface);
    if (!title->texture) {
        printf("Failed to create text texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    title->destRect = calloc(1, sizeof(SDL_Rect));
    if (!title->destRect) {
        printf("Failed to allocate memory for title rectangle\n");
        exit(EXIT_FAILURE);
    }

    *title->destRect = (SDL_Rect){
        .x = 800 - titleSurface->w/2,
        .y = 200,
        .w = titleSurface->w,
        .h = titleSurface->h
    };

    SDL_FreeSurface(titleSurface);  // we don't need the surface anymore

    Entity id = createEntity(uiEcs);
    addComponent(uiEcs, id, TEXT_COMPONENT, (void *)title);

    // "Play" option
    TextComponent *play = calloc(1, sizeof(TextComponent));
    if (!play) {
        printf("Failed to allocate memory for play text component\n");
        exit(EXIT_FAILURE);
    }
    
    play->state = STATE_MAIN_MENU;
    play->active = 1;
    play->selected = 1;  // "Play" is selected by default
    play->font = fonts->menuFont;
    play->text = strdup("Play");
    play->color = COLOR_YELLOW;  // highlighted color

    SDL_Surface *playSurface = TTF_RenderText_Solid(play->font, play->text, play->color);
    if (!playSurface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    play->texture = SDL_CreateTextureFromSurface(rdr, playSurface);
    if (!play->texture){
        printf("Failed to create text texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    play->destRect = calloc(1, sizeof(SDL_Rect));
    if (!play->destRect) {
        printf("Failed to allocate memory for play rectangle\n");
        exit(EXIT_FAILURE);
    }
    *play->destRect = (SDL_Rect){
        .x = 800 - playSurface->w/2,
        .y = 400,
        .w = playSurface->w,
        playSurface->h
    };
    SDL_FreeSurface(playSurface);

    id = createEntity(uiEcs);  // get a new entity's ID
    addComponent(uiEcs, id, TEXT_COMPONENT, (void *)play);

    // "Exit" option
    TextComponent *exitOpt = calloc(1, sizeof(TextComponent));
    if (!exitOpt) {
        printf("Failed to allocate memory for exit text component\n");
        exit(EXIT_FAILURE);
    }
    exitOpt->state = STATE_MAIN_MENU;
    exitOpt->active = 1;
    exitOpt->selected = 0;  // "Exit" is not selected by default
    exitOpt->font = fonts->menuFont;
    exitOpt->text = strdup("Exit");
    exitOpt->color = COLOR_WHITE;  // normal color

    SDL_Surface *exitSurface = TTF_RenderText_Solid(exitOpt->font, exitOpt->text, exitOpt->color);
    if (!exitSurface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    exitOpt->texture = SDL_CreateTextureFromSurface(rdr, exitSurface);
    if (!exitOpt->texture){
        printf("Failed to create text texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    exitOpt->destRect = calloc(1, sizeof(SDL_Rect));
    if (!exitOpt->destRect) {
        printf("Failed to allocate memory for exit rectangle\n");
        exit(EXIT_FAILURE);
    }
    *exitOpt->destRect = (SDL_Rect){
        .x = 800 - exitSurface->w/2,
        .y = 450,
        .w = exitSurface->w,
        .h = exitSurface->h
    };
    SDL_FreeSurface(exitSurface);
    id = createEntity(uiEcs);  // get a new entity's ID
    addComponent(uiEcs, id, TEXT_COMPONENT, (void *)exitOpt);

    // Instructions
    TextComponent *instructions = calloc(1, sizeof(TextComponent));
    if (!instructions) {
        printf("Failed to allocate memory for instructions text component\n");
        exit(EXIT_FAILURE);
    }
    instructions->state = STATE_MAIN_MENU;
    instructions->active = 1;
    instructions->selected = 0;  // not selectable
    instructions->font = fonts->menuFont;
    instructions->text = strdup("Use W/S or Arrow Keys to navigate, Enter/Space to select");
    instructions->color = COLOR_WHITE;  // normal color

    SDL_Surface *instrSurface = TTF_RenderText_Solid(instructions->font, instructions->text, instructions->color);
    if (!instrSurface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    instructions->texture = SDL_CreateTextureFromSurface(rdr, instrSurface);
    if (!instructions->texture) {
        printf("Failed to create text texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    instructions->destRect = calloc(1, sizeof(SDL_Rect));
    if (!instructions->destRect) {
        printf("Failed to allocate memory for instructions rectangle\n");
        exit(EXIT_FAILURE);
    }
    *instructions->destRect = (SDL_Rect){
        .x = 800 - instrSurface->w/2,
        .y = 600,
        .w = instrSurface->w,
        .h = instrSurface->h
    };
    SDL_FreeSurface(instrSurface);

    id = createEntity(uiEcs);  // get a new entity's ID
    addComponent(uiEcs, id, TEXT_COMPONENT, (void *)instructions);
}

// void onExitMainMenu(UIECS uiEcs, SDL_Renderer *rdr) {
//     // Clear the main menu UI components from the ECS
//     for (int64_t i = (int64_t)uiEcs->entityCount - 1; i >= 0; i--) {
//         TextComponent *curr = &uiEcs->textComponents[i];
//         if (curr->active) {
//             deleteUiTextEntity(uiEcs, i);
//         }
//     }
//     SDL_SetRenderDrawColor(rdr, 0, 0, 0, 255);  // Clear the renderer with black
//     SDL_RenderClear(rdr);
//     SDL_RenderPresent(rdr);  // Present the cleared renderer
// }

// void onEnterPlayState(GameECS ecs, SDL_Renderer *rdr) {
//     // Add the initial game entities to the ECS
//     HealthComponent health = {1, 100, 100};  // Active, max health, current health
//     SpeedComponent speed = {1, 0.0, 10.0};

//     SDL_Rect *dotRect = malloc(sizeof(SDL_Rect));
//     if (!dotRect) {
//         printf("Failed to allocate memory for dot rectangle\n");
//         exit(EXIT_FAILURE);
//     }
//     // Initial position and size of the dot
//     dotRect->x = 800 - 16;  // Centered horizontally
//     dotRect->y = 450 - 16;  // Centered vertically
//     dotRect->w = 32;  // Width of the dot
//     dotRect->h = 32;  // Height of the dot

//     SDL_Texture *dotTexture = SDL_CreateTexture(
//         rdr,
//         SDL_PIXELFORMAT_RGBA8888,
//         SDL_TEXTUREACCESS_TARGET,
//         dotRect->w,
//         dotRect->h
//     );
//     if (!dotTexture) {
//         printf("Failed to create dot texture: %s\n", SDL_GetError());
//         exit(EXIT_FAILURE);
//     }
//     SDL_SetRenderTarget(rdr, dotTexture);  // draw only to the dot texture
//     SDL_SetRenderDrawColor(rdr, 255, 255, 255, 255);  // White color for the dot
//     SDL_RenderFillRect(rdr, NULL);  // Fill the rectangle with white color
//     SDL_SetRenderTarget(rdr, NULL);  // Reset the render target

//     RenderComponent render = {1, 0, dotTexture, dotRect};  // Active, not selected, texture, destination rectangle

//     // put the created entity into the ECS
//     spawnGameEntity(ecs, health, speed, render);
// }