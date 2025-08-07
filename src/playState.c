#include "include/stateManager.h"

void onEnterPlayState(ZENg zEngine) {
    // Add the initial game entities to the ECS
    Entity id = createEntity(zEngine->gEcs);
    PLAYER_ID = id;  // set the global player ID

    HealthComponent *healthComp = createHealthComponent(100, 100, 1);
    addComponent(zEngine->gEcs, id, HEALTH_COMPONENT, (void *)healthComp);


    Uint32 playerStartTileX = 28;
    Uint32 playerStartTileY = ARENA_HEIGHT - 2;  // bottom row
    Int32 playerStartTile = playerStartTileY * ARENA_WIDTH + playerStartTileX;
    int wW, wH;
    SDL_GetWindowSize(zEngine->display->window, &wW, &wH);

    PositionComponent *posComp = createPositionComponent(tileToWorld(playerStartTile));
    addComponent(zEngine->gEcs, id, POSITION_COMPONENT, (void *)posComp);

    DirectionComponent *dirComp = createDirectionComponent(DIR_UP);
    addComponent(zEngine->gEcs, id, DIRECTION_COMPONENT, (void *)dirComp);

    VelocityComponent *speedComp = createVelocityComponent(
        (Vec2){0.0, 0.0},
        300.0, *posComp, 1
    );
    addComponent(zEngine->gEcs, id, VELOCITY_COMPONENT, (void *)speedComp);

    CollisionComponent *colComp = createCollisionComponent(
        posComp->x, posComp->y, TILE_SIZE * 2, TILE_SIZE * 2,
        1, COL_ACTOR
    );
    addComponent(zEngine->gEcs, id, COLLISION_COMPONENT, (void *)colComp);

    RenderComponent *renderComp = createRenderComponent(
        getTexture(zEngine->resources, "assets/textures/tank.png"),
        posComp->x, posComp->y, TILE_SIZE * 2, TILE_SIZE * 2,
        1, 0
    );
    addComponent(zEngine->gEcs, id, RENDER_COMPONENT, (void *)renderComp);

    // prepare the pause menu

    // continue option
    ButtonComponent *cont = createButtonComponent(
        zEngine->display->renderer,
        getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf"),
        "Continue",
        COLOR_YELLOW,
        &pauseToPlay,
        1, 0
    );

    *cont->destRect = (SDL_Rect){
        .x = (wW - cont->destRect->w) / 2,
        .y = (wH - cont->destRect->h) * 4 / 9,
        .w = cont->destRect->w,
        .h = cont->destRect->h
    };
    id = createEntity(zEngine->uiEcs);  // get a new entity's ID
    addComponent(zEngine->uiEcs, id, BUTTON_COMPONENT, (void *)cont);

    // exit to main menu option
    ButtonComponent *exitToMMenu = calloc(1, sizeof(ButtonComponent));
    if (!exitToMMenu) {
        printf("Failed to allocate memory for exit to main menu text component\n");
        exit(EXIT_FAILURE);
    }
    
    exitToMMenu->selected = 0;
    exitToMMenu->orderIdx = 1;
    exitToMMenu->font = getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf");
    exitToMMenu->text = strdup("Exit to main menu");
    exitToMMenu->color = COLOR_WHITE;
    exitToMMenu->onClick = &pauseToMMenu;

    SDL_Surface *exitSurf = TTF_RenderText_Solid(exitToMMenu->font, exitToMMenu->text, exitToMMenu->color);
    if (!exitSurf) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    exitToMMenu->texture = SDL_CreateTextureFromSurface(zEngine->display->renderer, exitSurf);
    if (!exitToMMenu->texture){
        printf("Failed to create text texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    exitToMMenu->destRect = calloc(1, sizeof(SDL_Rect));
    if (!exitToMMenu->destRect) {
        printf("Failed to allocate memory for exit to main menu rectangle\n");
        exit(EXIT_FAILURE);
    }
    *exitToMMenu->destRect = (SDL_Rect){
        .x = (wW - exitSurf->w) / 2,
        .y = (wH - exitSurf->h) * 4 / 7,
        .w = exitSurf->w,
        exitSurf->h
    };
    SDL_FreeSurface(exitSurf);

    id = createEntity(zEngine->uiEcs);  // get a new entity's ID
    addComponent(zEngine->uiEcs, id, BUTTON_COMPONENT, (void *)exitToMMenu);

    SDL_SetRenderTarget(zEngine->display->renderer, NULL);  // Reset the render target

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
        zEngine->display->renderer,
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

    SDL_SetRenderTarget(zEngine->display->renderer, TrenderComp->texture);  // draw only to the dot texture
    SDL_SetRenderDrawColor(zEngine->display->renderer, 255, 255, 255, 255);  // White color for the dot
    SDL_RenderFillRect(zEngine->display->renderer, NULL);  // Fill the rectangle with white color
    SDL_SetRenderTarget(zEngine->display->renderer, NULL);  // Reset the render target

    initLevel(zEngine, "null for now");
}

void onExitPlayState(ZENg zEngine) {
    // delete game entities
    while (zEngine->gEcs->entityCount > 0) {
        deleteEntity(zEngine->gEcs, zEngine->gEcs->activeEntities[0]);
    }

    // and the pause menu UI entities
    while (zEngine->uiEcs->entityCount > 0) {
        deleteEntity(zEngine->uiEcs, zEngine->uiEcs->activeEntities[0]);
    }
}

void spawnBulletProjectile(ZENg zEngine, Entity shooter) {
    Entity bulletID = createEntity(zEngine->gEcs);

    // Bullet size
    int bulletW = 10, bulletH = 10;

    // get the shooter components
    Uint64 shooterPage = shooter / PAGE_SIZE;
    Uint64 shooterPageIdx = shooter % PAGE_SIZE;
    Uint64 shooterDenseIdx = zEngine->gEcs->components[RENDER_COMPONENT].sparse[shooterPage][shooterPageIdx];

    // Bullet inherits the shooter's direction
    DirectionComponent *bulletDir = createDirectionComponent(
        *(DirectionComponent *)(zEngine->gEcs->components[DIRECTION_COMPONENT].dense[shooterDenseIdx])
    );
    addComponent(zEngine->gEcs, bulletID, DIRECTION_COMPONENT, (void *)bulletDir);

    SDL_Rect *shooterRect = (*(RenderComponent *)(zEngine->gEcs->components[RENDER_COMPONENT].dense[shooterDenseIdx])).destRect;
    PositionComponent *shooterPos = (PositionComponent *)(zEngine->gEcs->components[POSITION_COMPONENT].dense[shooterDenseIdx]);

    double playerCenterX = shooterPos->x + shooterRect->w / 2.0;
    double playerCenterY = shooterPos->y + shooterRect->h / 2.0;

    double bulletOffsetX = bulletDir->x * (shooterRect->w / 2.0 + bulletW / 2.0);
    double bulletOffsetY = bulletDir->y * (shooterRect->h / 2.0 + bulletH / 2.0);

    PositionComponent *bulletPos = calloc(1, sizeof(PositionComponent));
    if (!bulletPos) {
        printf("Failed to allocate memory for bullet position component\n");
        exit(EXIT_FAILURE);
    }
    *bulletPos = (PositionComponent) {
        // bullet starts at the edge of the player rect corresponding to the player's direction
        playerCenterX + bulletOffsetX - bulletW / 2.0,
        playerCenterY + bulletOffsetY - bulletH / 2.0
    };

    addComponent(zEngine->gEcs, bulletID, POSITION_COMPONENT, (void *)bulletPos);

    VelocityComponent *bulletSpeed = createVelocityComponent(
        (Vec2) {bulletDir->x * 500, bulletDir->y * 500},
        500.0, *bulletPos, 1
    );
    addComponent(zEngine->gEcs, bulletID, VELOCITY_COMPONENT, (void *)bulletSpeed);

    ProjectileComponent *projComp = calloc(1, sizeof(ProjectileComponent));
    if (!projComp) {
        printf("Failed to allocate memory for bullet projectile component\n");
        exit(EXIT_FAILURE);
    }
    *projComp = (ProjectileComponent) {
        .dmg = 15,
        .piercing = 0,
        .exploding = 0,
        .friendly = (shooter == PLAYER_ID) ? 1 : 0
    };
    addComponent(zEngine->gEcs, bulletID, PROJECTILE_COMPONENT, (void *)projComp);

    LifetimeComponent *lifeComp = calloc(1, sizeof(LifetimeComponent));
    if (!lifeComp) {
        printf("Failed to allocate memory for bullet lifetime component\n");
        exit(EXIT_FAILURE);
    }
    *lifeComp = (LifetimeComponent) {
        .lifeTime = 5,  // 5s
        .timeAlive = 0
    };
    addComponent(zEngine->gEcs, bulletID, LIFETIME_COMPONENT, (void *)lifeComp);

    CollisionComponent *bulletColl = calloc(1, sizeof(CollisionComponent));
    if (!bulletColl) {
        printf("Failed to allocate memory for bullet collision component\n");
        exit(EXIT_FAILURE);
    }
    bulletColl->hitbox = calloc(1, sizeof(SDL_Rect));
    if (!bulletColl->hitbox) {
        printf("Failed to allocate memory for bullet collision hitbox\n");
        exit(EXIT_FAILURE);
    }
    bulletColl->hitbox->x = (int)bulletPos->x;
    bulletColl->hitbox->y = (int)bulletPos->y;
    bulletColl->hitbox->w = 10;  // bullet size
    bulletColl->hitbox->h = 10;
    bulletColl->isSolid = 0;  // bullets can pass through each other
    bulletColl->role = COL_BULLET;
    addComponent(zEngine->gEcs, bulletID, COLLISION_COMPONENT, (void *)bulletColl);

    RenderComponent *bulletRender = calloc(1, sizeof(RenderComponent));
    if (!bulletRender) {
        printf("Failed to allocate memory for bullet render component\n");
        exit(EXIT_FAILURE);
    }

    bulletRender->active = 1;
    bulletRender->selected = 0;
    bulletRender->destRect = calloc(1, sizeof(SDL_Rect));
    if (!bulletRender->destRect) {
        printf("Failed to allocate memory for bullet rectangle\n");
        exit(EXIT_FAILURE);
    }
    *bulletRender->destRect = (SDL_Rect){
        .x = (int)bulletPos->x,
        .y = (int)bulletPos->y,
        .w = 10,  // bullet size
        .h = 10
    };
    bulletRender->texture = getTexture(zEngine->resources, "assets/textures/bullet.png");
    if (!bulletRender->texture) {
        printf("Failed to create bullet texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    addComponent(zEngine->gEcs, bulletID, RENDER_COMPONENT, (void *)bulletRender);
}

Uint8 handlePlayStateEvents(SDL_Event *e, ZENg zEngine) {
    if (e->type == SDL_KEYDOWN) {
        InputAction action = scancodeToAction(zEngine->inputMng, e->key.keysym.scancode);
        if (action == INPUT_UNKNOWN) {
            printf("Unknown input action for scancode %d\n", e->key.keysym.scancode);
            return 1;
        }
        switch (action) {
            case INPUT_BACK: {
                GameState *pauseState = calloc(1, sizeof(GameState));
                if (!pauseState) {
                    printf("Failed to allocate memory for pause state\n");
                    exit(EXIT_FAILURE);
                }
                pauseState->type = STATE_PAUSED;
                pauseState->handleEvents = &handlePauseStateEvents;
                pauseState->render = NULL;  // rendering is done only when needed
                pauseState->isOverlay = 1;
                pushState(zEngine, pauseState);
                renderPauseState(zEngine);  // render the pause state once
                return 1;
            }
            case INPUT_SHOOT: {
                // spawn a bullet with the owner PLAYER
                spawnBulletProjectile(zEngine, PLAYER_ID);
            }
        }
    }
    return 1;
}

void handlePlayStateInput(ZENg zEngine) {
    // Keyboard state
    zEngine->inputMng->keyboardState = SDL_GetKeyboardState(NULL);

    Uint64 page = PLAYER_ID / PAGE_SIZE;
    Uint64 pageIdx = PLAYER_ID % PAGE_SIZE;
    Uint64 denseIdx = zEngine->gEcs->components[VELOCITY_COMPONENT].sparse[page][pageIdx];

    VelocityComponent *playerSpeed = (VelocityComponent *)(zEngine->gEcs->components[VELOCITY_COMPONENT].dense[denseIdx]);
    PositionComponent *playerPos = (PositionComponent *)(zEngine->gEcs->components[POSITION_COMPONENT].dense[denseIdx]);
    DirectionComponent *playerDir = (DirectionComponent *)(zEngine->gEcs->components[DIRECTION_COMPONENT].dense[denseIdx]);

    Uint8 moving = 0;  // flag to check if the player is moving
    if (isActionPressed(zEngine->inputMng, INPUT_MOVE_UP)) {
        *playerDir = DIR_UP;
        moving = 1;
    }
    if (isActionPressed(zEngine->inputMng, INPUT_MOVE_DOWN)) {
        *playerDir = DIR_DOWN;
        moving = 1;
    }
    if (isActionPressed(zEngine->inputMng, INPUT_MOVE_LEFT)) {
        *playerDir = DIR_LEFT;
        moving = 1;
    }
    if (isActionPressed(zEngine->inputMng, INPUT_MOVE_RIGHT)) {
        *playerDir = DIR_RIGHT;
        moving = 1;
    }

    if (moving) {
        // set the velocity vector
        playerSpeed->currVelocity.x = playerSpeed->maxVelocity * playerDir->x;
        playerSpeed->currVelocity.y = playerSpeed->maxVelocity * playerDir->y;
    } else {
        // If no movement input, stop the player
        playerSpeed->currVelocity = (Vec2) { .x = 0.0, .y = 0.0 };
    }
}

void updatePlayStateLogic(ZENg zEngine, double_t deltaTime) {
    lifetimeSystem(zEngine, deltaTime);  // Deletes expired entities
    velocitySystem(zEngine, deltaTime);  // Updates predicted positions
    worldCollisionSystem(zEngine, deltaTime);  // Handles world collisions based on predicted positions
    entityCollisionSystem(zEngine);  // Handles entities collisions based on predicted positions and validates them
    healthSystem(zEngine);  // Deletes entities with <= health
    transformSystem(zEngine->gEcs);  // Updates the screen textures of entities based on their virtual positions
}

void renderArena(ZENg zEngine) {
    SDL_SetRenderDrawColor(zEngine->display->renderer, 20, 20, 20, 200);  // background color - grey
    SDL_RenderClear(zEngine->display->renderer);

    for (Uint64 y = 0; y < ARENA_HEIGHT; y++) {
        for (Uint64 x = 0; x < ARENA_WIDTH; x++) {
            SDL_Rect tileRect = {
                .x = x * TILE_SIZE,
                .y = y * TILE_SIZE,
                .w = TILE_SIZE,
                .h = TILE_SIZE
            };
            if (zEngine->map->tiles[x][y].texture) {
                SDL_RenderCopy(
                    zEngine->display->renderer,
                    zEngine->map->tiles[x][y].texture,
                    NULL,
                    &tileRect
                );
            }
        }
    }
}

void renderPlayState(ZENg zEngine) {
    renderArena(zEngine);

    // Render game entities
    for (Uint64 i = 0; i < zEngine->gEcs->components[RENDER_COMPONENT].denseSize; i++) {
        RenderComponent *render = (RenderComponent *)(zEngine->gEcs->components[RENDER_COMPONENT].dense[i]);
        if (render) {
            double angle = 0.0;
            Entity owner = zEngine->gEcs->components[RENDER_COMPONENT].denseToEntity[i];
            Uint64 ownerPage = owner / PAGE_SIZE;
            Uint64 ownerPageIdx = owner % PAGE_SIZE;
            Uint64 ownerDenseIdx = zEngine->gEcs->components[DIRECTION_COMPONENT].sparse[ownerPage][ownerPageIdx];

            // Check if the entity has a direction component
            bitset hasDirection = 1 << DIRECTION_COMPONENT;
            if (zEngine->gEcs->componentsFlags[owner] & hasDirection) {
                DirectionComponent *dirComp = (DirectionComponent *)(zEngine->gEcs->components[DIRECTION_COMPONENT].dense[ownerDenseIdx]);

                if (VEC2_EQUAL(*dirComp, DIR_UP)) {
                    angle = 0.0;
                } else if (VEC2_EQUAL(*dirComp, DIR_DOWN)) {
                    angle = 180.0;
                } else if (VEC2_EQUAL(*dirComp, DIR_LEFT)) {
                    angle = 270.0;
                } else if (VEC2_EQUAL(*dirComp, DIR_RIGHT)) {
                    angle = 90.0;
                }
            }

            SDL_RenderCopyEx(zEngine->display->renderer, render->texture, NULL, render->destRect, angle, NULL, SDL_FLIP_NONE);
        }
    }
    renderDebugCollision(zEngine);
}