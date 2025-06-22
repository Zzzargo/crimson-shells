#include "include/mainLoop.h"

void initGame(SDL_Window **wdw, SDL_Renderer **rdr, GameECS *ecs, UIECS *uiEcs, FontManager *fonts) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Initialize fonts and font manager
    if (TTF_Init() < 0) {
        printf("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    (*fonts) = malloc(sizeof(struct fontmng));
    if (!(*fonts)) {
        printf("Failed to allocate memory for FontManager\n");
        TTF_Quit();
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    // Load fonts
    (*fonts)->titleFont = TTF_OpenFont("assets/fonts/ByteBounce.ttf", 48);
    (*fonts)->menuFont = TTF_OpenFont("assets/fonts/ByteBounce.ttf", 28);
    (*fonts)->gameFont = TTF_OpenFont("assets/fonts/ByteBounce.ttf", 20);

    if (!(*fonts)->titleFont || !(*fonts)->menuFont || !(*fonts)->gameFont) {
        printf("Failed to load font! TTF_Error: %s\n", TTF_GetError());
        return;
    }

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
    addUiTextEntity(uiEcs, fonts->titleFont, titleText, titleTexture, &titleRect);
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
    addUiTextEntity(uiEcs, fonts->menuFont, playText, playTexture, &playRect);
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
    addUiTextEntity(uiEcs, fonts->menuFont, exitText, exitTexture, &exitRect);
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
    addUiTextEntity(uiEcs, fonts->menuFont, instructionsText, instrTexture, &instrRect);
    SDL_FreeSurface(instrSurface);
}

void onExitMainMenu(UIECS uiEcs, SDL_Renderer *rdr) {
    // Clear the main menu UI components from the ECS
    for (Uint64 i = 0; i < uiEcs->entityCount; i++) {
        TextComponent *curr = &uiEcs->textComponents[i];
        if (curr->active) {
            deleteUiTextEntity(uiEcs, i);
        }
    }
    SDL_SetRenderDrawColor(rdr, 0, 0, 0, 255);  // Clear the renderer with black
    SDL_RenderClear(rdr);
    SDL_RenderPresent(rdr);  // Present the cleared renderer
}

void updateMenuUI(UIECS uiEcs, SDL_Renderer *rdr) {
    // Rerender the UI based on the entities' current state

    SDL_SetRenderDrawColor(rdr, 100, 50, 0, 200);  // background color - brownish
    SDL_RenderClear(rdr);  // clear the renderer

    for (Uint64 i = 0; i < uiEcs->entityCount; i++) {
        TextComponent *curr = &uiEcs->textComponents[i];
        if (curr->active) {
            // Update the texture or other properties if active
            SDL_DestroyTexture(curr->texture);
            SDL_Surface *surface = TTF_RenderText_Solid(
                curr->font,
                curr->text,
                curr->selected ? COLOR_YELLOW : COLOR_WHITE
            );
            curr->texture = SDL_CreateTextureFromSurface(rdr, surface);
            SDL_RenderCopy(rdr, curr->texture, NULL, curr->destRect);
            SDL_FreeSurface(surface);
        }
    }
}

void handleMainMenuEvents(GameState *currState, SDL_Event *event, UIECS uiEcs, GameECS gEcs, SDL_Renderer *rdr) {
    // Handle input
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_UP:
            case SDLK_w: {
                for (Uint64 i = 0; i < uiEcs->entityCount; i++) {
                    TextComponent *curr = &uiEcs->textComponents[i];
                    if (curr->selected) {
                        curr->selected = 0;  // Deselect current
                        if (strcmp(curr->text, "Play") == 0) {
                            // "Play" was selected, wrap around to "Exit"
                            for (Uint64 j = i; j < uiEcs->entityCount; j++) {
                                if (strcmp(uiEcs->textComponents[j].text, "Exit") == 0) {
                                    uiEcs->textComponents[j].selected = 1;  // select "Exit"
                                    break;
                                }
                            }
                        } else {
                            // select the previous item
                            uiEcs->textComponents[i - 1].selected = 1;
                        }
                        updateMenuUI(uiEcs, rdr);
                        break;
                    }
                }
                break;
            }
            case SDLK_DOWN:
            case SDLK_s: {
                for (Uint64 i = 0; i < uiEcs->entityCount; i++) {
                    TextComponent *curr = &uiEcs->textComponents[i];
                    if (curr->selected) {
                        curr->selected = 0;  // Deselect current
                        if (strcmp(curr->text, "Exit") == 0) {
                            // "Exit" was selected, wrap around to "Play"
                            for (Uint64 j = 0; j < uiEcs->entityCount; j++) {
                                if (strcmp(uiEcs->textComponents[j].text, "Play") == 0) {
                                    uiEcs->textComponents[j].selected = 1;  // select "Play"
                                    break;
                                }
                            }
                        } else {
                            // select the next item
                            uiEcs->textComponents[i + 1].selected = 1;
                        }
                        updateMenuUI(uiEcs, rdr);
                        break;
                    }
                }
                break;
            }
            case SDLK_RETURN:
            case SDLK_SPACE: {
                for (Uint64 i = 0; i < uiEcs->entityCount; i++) {
                    TextComponent *curr = &uiEcs->textComponents[i];
                    if (curr->selected) {
                        if (strcmp(curr->text, "Play") == 0) {
                            *currState = STATE_PLAYING;  // Start the game
                            onExitMainMenu(uiEcs, rdr);  // Clear the main menu UI
                            onEnterPlayState(gEcs, rdr);
                        } else if (strcmp(curr->text, "Exit") == 0) {
                            *currState = STATE_EXIT;  // Exit the game
                        }
                    }
                }
                break;
            }
        }
    }
}

void renderMainMenu(SDL_Renderer *rdr, UIECS uiEcs) {
    // Clear the screen
    SDL_SetRenderDrawColor(rdr, 100, 50, 0, 200);  // background color - brownish
    SDL_RenderClear(rdr);

    for (Uint64 i = 0; i < uiEcs->entityCount; i++) {
        TextComponent *curr = &uiEcs->textComponents[i];
        if (curr->active) {
            SDL_RenderCopy(rdr, curr->texture, NULL, curr->destRect);
        }
    }

    SDL_RenderPresent(rdr);  // render current frame
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

void renderPlayState(SDL_Renderer *rdr, GameState *currState, GameECS ecs, UIECS uiEcs, FontManager fonts) {
    // Clear the screen
    SDL_SetRenderDrawColor(rdr, 100, 50, 0, 200);  // background color - brownish
    SDL_RenderClear(rdr);

    // Keyboard state
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_ESCAPE]) {
        *currState = STATE_MAIN_MENU;  // Go back to main menu
        onEnterMainMenu(uiEcs, rdr, fonts);
    }
    if (keys[SDL_SCANCODE_W]) {
        SpeedComponent *dotSpeed = &ecs->speedComponents[0];
        if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_D]) {
            // Diagonal movement Speed / 1.414
            dotSpeed->velocity = dotSpeed->maxSpeed / 1.414;  // 1 / sqrt(2) - normalization
        } else {
            dotSpeed->velocity = dotSpeed->maxSpeed;  // Normal speed
        }
        ecs->renderComponents[0].destRect->y -= dotSpeed->velocity;  // Move up
    }
    if (keys[SDL_SCANCODE_S]) {
        SpeedComponent *dotSpeed = &ecs->speedComponents[0];
        if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_D]) {
            // Diagonal movement Speed / 1.414
            dotSpeed->velocity = dotSpeed->maxSpeed / 1.414;
        } else {
            dotSpeed->velocity = dotSpeed->maxSpeed;
        }
        ecs->renderComponents[0].destRect->y += dotSpeed->velocity;  // Move down
    }
    if (keys[SDL_SCANCODE_A]) {
        SpeedComponent *dotSpeed = &ecs->speedComponents[0];
        if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_W]) {
            dotSpeed->velocity = dotSpeed->maxSpeed / 1.414;
        } else {
            dotSpeed->velocity = dotSpeed->maxSpeed;
        }
        ecs->renderComponents[0].destRect->x -= dotSpeed->velocity;  // Move left
    }
    if (keys[SDL_SCANCODE_D]) {
        SpeedComponent *dotSpeed = &ecs->speedComponents[0];
        if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_W]) {
            dotSpeed->velocity = dotSpeed->maxSpeed / 1.414;
        } else {
            dotSpeed->velocity = dotSpeed->maxSpeed;
        }
        ecs->renderComponents[0].destRect->x += dotSpeed->velocity;  // Move right
    }

    // Render game entities
    // printf("Rendering %lu entities\n", ecs->entityCount);
    for (Uint64 i = 0; i < ecs->entityCount; i++) {
        RenderComponent *render = &ecs->renderComponents[i];
        if (render->active) {
            SDL_RenderCopy(rdr, render->texture, NULL, render->destRect);
        }
    }

    SDL_RenderPresent(rdr);  // render the current frame
}