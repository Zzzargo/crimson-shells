#include "include/mainLoop.h"

Uint8 handleEvents(SDL_Event *e, ZENg zEngine) {
    switch (zEngine->state) {
        case STATE_MAIN_MENU: {
            handleMainMenuEvents(e, zEngine);
            return (zEngine->state == STATE_EXIT ? 0 : 1);
        }
        case STATE_PLAYING: {
            handlePlayStateEvents(e, zEngine);
            return (zEngine->state == STATE_EXIT ? 0 : 1);
        }
        case STATE_PAUSED: {
            handlePauseStateEvents(e, zEngine);
            return (zEngine->state == STATE_EXIT ? 0 : 1);
        }
        case STATE_GAME_OVER: {
            // todo
            return (zEngine->state == STATE_EXIT ? 0 : 1);
        }
        case STATE_EXIT: {
            return 0;  // game stops
        }
    }
}

void handleInput(ZENg zEngine) {
    switch (zEngine->state) {
        case STATE_PLAYING: {
            handlePlayStateInput(zEngine);
            break;
        }
    }
}

void updateGameLogic(ZENg zEngine, double_t deltaTime) {
    switch (zEngine->state) {
        // Main menu logic is handled via events

        case STATE_PLAYING: {
            // continous shii
            lifetimeSystem(zEngine, deltaTime);
            velocitySystem(zEngine, deltaTime);
            collisionSystem(zEngine);
            healthSystem(zEngine);
            transformSystem(zEngine->gEcs);  // syncs position updates with the render components
            break;
        }
        case STATE_PAUSED: {
            // later
            break;
        }
        case STATE_GAME_OVER: {
            // later
            break;
        }
    }  
}

void renderFrame(ZENg zEngine) {
    switch (zEngine->state) {
        case STATE_MAIN_MENU: {
            renderMainMenu(zEngine);
            break;
        }
        case STATE_PLAYING: {
            renderPlayState(zEngine);
            break;
        }
        case STATE_PAUSED: {
            renderPlayState(zEngine);

            // transparent overlay
            SDL_SetRenderDrawBlendMode(zEngine->renderer, SDL_BLENDMODE_BLEND);  // Enable blending for transparency
            SDL_SetRenderDrawColor(zEngine->renderer, 0, 0, 0, 150);  // semi-transparent black
            SDL_RenderFillRect(zEngine->renderer, NULL);  // Fill the entire screen with the semi-transparent color
            
            renderPauseState(zEngine);

            SDL_SetRenderDrawBlendMode(zEngine->renderer, SDL_BLENDMODE_NONE);  // Disable blending
            break;
        }
        case STATE_GAME_OVER: {
            // todo
            break;
        }
    }
    SDL_RenderPresent(zEngine->renderer);  // render the current frame
}

void onEnterMainMenu(ZENg zEngine) {
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
    title->orderIdx = 0;  // first item
    title->font = getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf");
    title->text = strdup("GOAT Game");
    title->color = COLOR_WHITE_TRANSPARENT;  // semi-transparent white

    SDL_Surface *titleSurface = TTF_RenderText_Solid(title->font, title->text, COLOR_WHITE_TRANSPARENT);
    if (!titleSurface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    title->texture = SDL_CreateTextureFromSurface(zEngine->renderer, titleSurface);
    if (!title->texture) {
        printf("Failed to create text texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    title->destRect = calloc(1, sizeof(SDL_Rect));
    if (!title->destRect) {
        printf("Failed to allocate memory for title rectangle\n");
        exit(EXIT_FAILURE);
    }
    int wW, wH;
    SDL_GetWindowSize(zEngine->window, &wW, &wH);  // Get the window size
    // center the title horizontally and position it at the top of the options
    *title->destRect = (SDL_Rect) {
        .x = (wW - titleSurface->w) / 2,  // center horizontally
        .y = (wH - titleSurface->h) / 4,  // position at the top of the options
        .w = titleSurface->w,
        .h = titleSurface->h
    };

    SDL_FreeSurface(titleSurface);  // we don't need the surface anymore

    Entity id = createEntity(zEngine->uiEcs);
    addComponent(zEngine->uiEcs, id, TEXT_COMPONENT, (void *)title);

    // "Play" option
    TextComponent *play = calloc(1, sizeof(TextComponent));
    if (!play) {
        printf("Failed to allocate memory for play text component\n");
        exit(EXIT_FAILURE);
    }
    
    play->state = STATE_MAIN_MENU;
    play->active = 1;
    play->selected = 1;  // "Play" is selected by default
    play->orderIdx = 1;
    play->font = getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf");
    play->text = strdup("Play");
    play->color = COLOR_YELLOW;  // highlighted color

    SDL_Surface *playSurface = TTF_RenderText_Solid(play->font, play->text, play->color);
    if (!playSurface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    play->texture = SDL_CreateTextureFromSurface(zEngine->renderer, playSurface);
    if (!play->texture) {
        printf("Failed to create text texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    play->destRect = calloc(1, sizeof(SDL_Rect));
    if (!play->destRect) {
        printf("Failed to allocate memory for play rectangle\n");
        exit(EXIT_FAILURE);
    }
    *play->destRect = (SDL_Rect){
        .x = (wW - playSurface->w) / 2,
        .y = (wH - playSurface->h) / 2,
        .w = playSurface->w,
        playSurface->h
    };
    SDL_FreeSurface(playSurface);

    id = createEntity(zEngine->uiEcs);  // get a new entity's ID
    addComponent(zEngine->uiEcs, id, TEXT_COMPONENT, (void *)play);

    // "Exit" option
    TextComponent *exitOpt = calloc(1, sizeof(TextComponent));
    if (!exitOpt) {
        printf("Failed to allocate memory for exit text component\n");
        exit(EXIT_FAILURE);
    }
    exitOpt->state = STATE_MAIN_MENU;
    exitOpt->active = 1;
    exitOpt->selected = 0;  // "Exit" is not selected by default
    exitOpt->orderIdx = 2;
    exitOpt->font = getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf");
    exitOpt->text = strdup("Exit");
    exitOpt->color = COLOR_WHITE;  // normal color

    SDL_Surface *exitSurface = TTF_RenderText_Solid(exitOpt->font, exitOpt->text, exitOpt->color);
    if (!exitSurface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    exitOpt->texture = SDL_CreateTextureFromSurface(zEngine->renderer, exitSurface);
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
        .x = (wW - exitSurface->w) / 2,  // center horizontally
        .y = (wH - exitSurface->h) * 4 / 7,
        .w = exitSurface->w,
        .h = exitSurface->h
    };
    SDL_FreeSurface(exitSurface);
    id = createEntity(zEngine->uiEcs);  // get a new entity's ID
    addComponent(zEngine->uiEcs, id, TEXT_COMPONENT, (void *)exitOpt);

    // Instructions
    TextComponent *instructions = calloc(1, sizeof(TextComponent));
    if (!instructions) {
        printf("Failed to allocate memory for instructions text component\n");
        exit(EXIT_FAILURE);
    }
    instructions->state = STATE_MAIN_MENU;
    instructions->active = 1;
    instructions->selected = 0;  // not selectable
    instructions->orderIdx = 3;
    instructions->font = getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf");
    instructions->text = strdup("Use W/S or Arrow Keys to navigate, Enter/Space to select");
    instructions->color = COLOR_WHITE_TRANSPARENT;

    SDL_Surface *instrSurface = TTF_RenderText_Solid(instructions->font, instructions->text, instructions->color);
    if (!instrSurface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    instructions->texture = SDL_CreateTextureFromSurface(zEngine->renderer, instrSurface);
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
        .x = (wW - instrSurface->w) / 2,  // center horizontally
        .y = (wH - instrSurface->h) * 5 / 7,  // position below the options
        .w = instrSurface->w,
        .h = instrSurface->h
    };
    SDL_FreeSurface(instrSurface);

    id = createEntity(zEngine->uiEcs);  // get a new entity's ID
    addComponent(zEngine->uiEcs, id, TEXT_COMPONENT, (void *)instructions);
}

void onExitMainMenu(ZENg zEngine) {
    // delete all main menu entities
    Uint64 i = 0;
    while (zEngine->uiEcs->entityCount != 0) {
        deleteEntity(zEngine->uiEcs, i);
        i++;
    }
}

void onEnterPlayState(ZENg zEngine) {
    // Add the initial game entities to the ECS
    Entity id = createEntity(zEngine->gEcs);
    PLAYER_ID = id;  // set the global player ID

    HealthComponent *healthComp = calloc(1, sizeof(HealthComponent));
    if (!healthComp) {
        printf("Failed to allocate memory for health component\n");
        exit(EXIT_FAILURE);
    }
    *healthComp = (HealthComponent) {100, 100, 1};  // current, max, active
    addComponent(zEngine->gEcs, id, HEALTH_COMPONENT, (void *)healthComp);

    PositionComponent *posComp = calloc(1, sizeof(PositionComponent));
    if (!posComp) {
        printf("Failed to allocate memory for position component\n");
        exit(EXIT_FAILURE);
    }
    int wW, wH;
    SDL_GetWindowSize(zEngine->window, &wW, &wH);
    *posComp = (PositionComponent) {
        wW / 2.0,
        wH / 2.0
    };
    addComponent(zEngine->gEcs, id, POSITION_COMPONENT, (void *)posComp);

    DirectionComponent *dirComp = calloc(1, sizeof(DirectionComponent));
    if (!dirComp) {
        printf("Failed to allocate memory for direction component\n");
        exit(EXIT_FAILURE);
    }
    *dirComp = (Vec2) DIR_UP;  // initial direction - up
    addComponent(zEngine->gEcs, id, DIRECTION_COMPONENT, (void *)dirComp);

    VelocityComponent *speedComp = calloc(1, sizeof(VelocityComponent));
    if (!speedComp) {
        printf("Failed to allocate memory for velocity component\n");
        exit(EXIT_FAILURE);
    }
    *speedComp = (VelocityComponent) {
        (Vec2) {0.0, 0.0 },
        10.0,
        1
    };

    addComponent(zEngine->gEcs, id, VELOCITY_COMPONENT, (void *)speedComp);

    CollisionComponent *colComp = calloc(1, sizeof(CollisionComponent));
    if (!colComp) {
        printf("Failed to allocate memory for collision component\n");
        exit(EXIT_FAILURE);
    }
    colComp->hitbox = calloc(1, sizeof(SDL_Rect));
    if (!colComp->hitbox) {
        printf("Failed to allocate memory for collision hitbox\n");
        exit(EXIT_FAILURE);
    }
    colComp->hitbox->x = posComp->x;
    colComp->hitbox->y = posComp->y;
    colComp->hitbox->w = wH / 20;  // size of the hitbox
    colComp->hitbox->h = wH / 20;
    colComp->isSolid = 1;  // player is sure solid
    colComp->role = COL_ACTOR;  // player is an actor in the game
    addComponent(zEngine->gEcs, id, COLLISION_COMPONENT, (void *)colComp);

    RenderComponent *renderComp = calloc(1, sizeof(RenderComponent));
    if (!renderComp) {
        printf("Failed to allocate memory for render component\n");
        exit(EXIT_FAILURE);
    }
    renderComp->active = 1;
    renderComp->selected = 0;

    renderComp->destRect = calloc(1, sizeof(SDL_Rect));
    if (!renderComp->destRect) {
        printf("Failed to allocate memory for dot rectangle\n");
        exit(EXIT_FAILURE);
    }
    // Initial position and size of the dot
    *renderComp->destRect = (SDL_Rect){
        .x = posComp->x,  // Centered horizontally
        .y = posComp->y,  // Centered vertically
        .w = wH / 20,
        .h = wH / 20
    };

    renderComp->texture = SDL_CreateTexture(
        zEngine->renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        renderComp->destRect->w,
        renderComp->destRect->h
    );
    if (!renderComp->texture) {
        printf("Failed to create dot texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    addComponent(zEngine->gEcs, id, RENDER_COMPONENT, (void *)renderComp);

    SDL_SetRenderTarget(zEngine->renderer, renderComp->texture);  // draw only to the dot texture
    SDL_SetRenderDrawColor(zEngine->renderer, 255, 255, 255, 255);  // White color for the dot
    SDL_RenderFillRect(zEngine->renderer, NULL);  // Fill the rectangle with white color

    // prepare the pause menu

    // continue option
    TextComponent *cont = calloc(1, sizeof(TextComponent));
    if (!cont) {
        printf("Failed to allocate memory for continue text component\n");
        exit(EXIT_FAILURE);
    }
    
    cont->state = STATE_PAUSED;
    cont->active = 1;
    cont->selected = 1;  // "Continue" is selected by default
    cont->orderIdx = 0;
    cont->font = getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf");
    cont->text = strdup("Continue");
    cont->color = COLOR_YELLOW;  // highlighted color

    SDL_Surface *contSurface = TTF_RenderText_Solid(cont->font, cont->text, cont->color);
    if (!contSurface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    cont->texture = SDL_CreateTextureFromSurface(zEngine->renderer, contSurface);
    if (!cont->texture){
        printf("Failed to create text texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    cont->destRect = calloc(1, sizeof(SDL_Rect));
    if (!cont->destRect) {
        printf("Failed to allocate memory for continue rectangle\n");
        exit(EXIT_FAILURE);
    }
    *cont->destRect = (SDL_Rect){
        .x = (wW - contSurface->w) / 2,
        .y = (wH - contSurface->h) * 4 / 9,
        .w = contSurface->w,
        contSurface->h
    };
    SDL_FreeSurface(contSurface);

    id = createEntity(zEngine->uiEcs);  // get a new entity's ID
    addComponent(zEngine->uiEcs, id, TEXT_COMPONENT, (void *)cont);

    // exit to main menu option
    TextComponent *exitMMenu = calloc(1, sizeof(TextComponent));
    if (!exitMMenu) {
        printf("Failed to allocate memory for exit to main menu text component\n");
        exit(EXIT_FAILURE);
    }
    
    exitMMenu->state = STATE_PAUSED;
    exitMMenu->active = 1;
    exitMMenu->selected = 0;
    exitMMenu->orderIdx = 1;
    exitMMenu->font = getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf");
    exitMMenu->text = strdup("Exit to main menu");
    exitMMenu->color = COLOR_WHITE;

    SDL_Surface *exitSurf = TTF_RenderText_Solid(exitMMenu->font, exitMMenu->text, exitMMenu->color);
    if (!exitSurf) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    exitMMenu->texture = SDL_CreateTextureFromSurface(zEngine->renderer, exitSurf);
    if (!exitMMenu->texture){
        printf("Failed to create text texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    exitMMenu->destRect = calloc(1, sizeof(SDL_Rect));
    if (!exitMMenu->destRect) {
        printf("Failed to allocate memory for exit to main menu rectangle\n");
        exit(EXIT_FAILURE);
    }
    *exitMMenu->destRect = (SDL_Rect){
        .x = (wW - exitSurf->w) / 2,
        .y = (wH - exitSurf->h) * 4 / 7,
        .w = exitSurf->w,
        exitSurf->h
    };
    SDL_FreeSurface(exitSurf);

    id = createEntity(zEngine->uiEcs);  // get a new entity's ID
    addComponent(zEngine->uiEcs, id, TEXT_COMPONENT, (void *)exitMMenu);

    SDL_SetRenderTarget(zEngine->renderer, NULL);  // Reset the render target




    // test entity
    id = createEntity(zEngine->gEcs);
    HealthComponent *ThealthComp = calloc(1, sizeof(HealthComponent));
    if (!ThealthComp) {
        printf("Failed to allocate memory for health component\n");
        exit(EXIT_FAILURE);
    }
    *ThealthComp = (HealthComponent) {100, 100, 1};  // current, max, active
    addComponent(zEngine->gEcs, id, HEALTH_COMPONENT, (void *)ThealthComp);

    PositionComponent *TposComp = calloc(1, sizeof(PositionComponent));
    if (!TposComp) {
        printf("Failed to allocate memory for position component\n");
        exit(EXIT_FAILURE);
    }
    *TposComp = (PositionComponent) {
        wW / 2.0 + 500,
        wH / 2.0
    };
    addComponent(zEngine->gEcs, id, POSITION_COMPONENT, (void *)TposComp);

    CollisionComponent *TcolComp = calloc(1, sizeof(CollisionComponent));
    if (!TcolComp) {
        printf("Failed to allocate memory for collision component\n");
        exit(EXIT_FAILURE);
    }
    TcolComp->hitbox = calloc(1, sizeof(SDL_Rect));
    if (!TcolComp->hitbox) {
        printf("Failed to allocate memory for collision hitbox\n");
        exit(EXIT_FAILURE);
    }
    TcolComp->hitbox->x = TposComp->x;
    TcolComp->hitbox->y = TposComp->y;
    TcolComp->hitbox->w = wH / 20;  // size of the hitbox
    TcolComp->hitbox->h = wH / 20;
    TcolComp->isSolid = 1;  // player is sure solid
    TcolComp->role = COL_ACTOR;
    addComponent(zEngine->gEcs, id, COLLISION_COMPONENT, (void *)TcolComp);

    RenderComponent *TrenderComp = calloc(1, sizeof(RenderComponent));
    if (!TrenderComp) {
        printf("Failed to allocate memory for render component\n");
        exit(EXIT_FAILURE);
    }
    TrenderComp->active = 1;
    TrenderComp->selected = 0;

    TrenderComp->destRect = calloc(1, sizeof(SDL_Rect));
    if (!TrenderComp->destRect) {
        printf("Failed to allocate memory for dot rectangle\n");
        exit(EXIT_FAILURE);
    }
    // Initial position and size of the dot
    *TrenderComp->destRect = (SDL_Rect) {
        .x = TposComp->x,  // Centered horizontally
        .y = TposComp->y,  // Centered vertically
        .w = wH / 20,
        .h = wH / 20
    };

    TrenderComp->texture = SDL_CreateTexture(
        zEngine->renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        TrenderComp->destRect->w,
        TrenderComp->destRect->h
    );
    if (!TrenderComp->texture) {
        printf("Failed to create dot texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    addComponent(zEngine->gEcs, id, RENDER_COMPONENT, (void *)TrenderComp);

    SDL_SetRenderTarget(zEngine->renderer, TrenderComp->texture);  // draw only to the dot texture
    SDL_SetRenderDrawColor(zEngine->renderer, 255, 255, 255, 255);  // White color for the dot
    SDL_RenderFillRect(zEngine->renderer, NULL);  // Fill the rectangle with white color
    SDL_SetRenderTarget(zEngine->renderer, NULL);  // Reset the render target
}

void onExitPlayState(ZENg zEngine) {
    // delete game entities
    while (zEngine->gEcs->entityCount > 0) {
        // deleteEntity internally rearranges the indexes
        deleteEntity(zEngine->gEcs, 0);
    }

    // and the pause menu UI entities
    Uint64 i = 0;
    while (i < zEngine->uiEcs->nextEntityID) {
        bitset target = 1 << TEXT_COMPONENT;
        bitset currFlags = zEngine->uiEcs->componentsFlags[i];

        if ((target & currFlags) == target) {
            Uint64 page = i / PAGE_SIZE;
            Uint64 idx = i % PAGE_SIZE;
            Uint64 denseIdx = zEngine->uiEcs->components[TEXT_COMPONENT].sparse[page][idx];

            TextComponent *curr = (TextComponent *)(zEngine->uiEcs->components[TEXT_COMPONENT].dense[denseIdx]);
            if (curr && curr->state  == STATE_PAUSED) {
                deleteEntity(zEngine->uiEcs, i);
                continue;
            }
        }
        i++;
    }
}