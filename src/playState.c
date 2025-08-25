#include "include/stateManager.h"

void onEnterPlayState(ZENg zEngine) {
    // Add the initial game entities to the ECS
    Entity id = createEntity(zEngine->ecs);
    PLAYER_ID = id;  // set the global player ID

    HealthComponent *healthComp = createHealthComponent(100, 100, 1);
    addComponent(zEngine->ecs, id, HEALTH_COMPONENT, (void *)healthComp);

    Uint32 playerStartTileX = ARENA_WIDTH / 2;
    Uint32 playerStartTileY = ARENA_HEIGHT - 2;  // bottom row
    Int32 playerStartTile = playerStartTileY * ARENA_WIDTH + playerStartTileX;
    int wW, wH;
    SDL_GetWindowSize(zEngine->display->window, &wW, &wH);

    PositionComponent *posComp = createPositionComponent(tileToWorld(playerStartTile));
    addComponent(zEngine->ecs, id, POSITION_COMPONENT, (void *)posComp);

    DirectionComponent *dirComp = createDirectionComponent(DIR_UP);
    addComponent(zEngine->ecs, id, DIRECTION_COMPONENT, (void *)dirComp);

    VelocityComponent *speedComp = createVelocityComponent(
        (Vec2){0.0, 0.0},
        250.0, *posComp, AXIS_NONE, 1
    );
    addComponent(zEngine->ecs, id, VELOCITY_COMPONENT, (void *)speedComp);

    CollisionComponent *colComp = createCollisionComponent(
        posComp->x, posComp->y, TILE_SIZE * 2, TILE_SIZE * 2,
        1, COL_ACTOR
    );
    addComponent(zEngine->ecs, id, COLLISION_COMPONENT, (void *)colComp);

    RenderComponent *renderComp = createRenderComponent(
        getTexture(zEngine->resources, "assets/textures/tank.png"),
        posComp->x, posComp->y, TILE_SIZE * 2, TILE_SIZE * 2,
        1, 0
    );
    addComponent(zEngine->ecs, id, RENDER_COMPONENT, (void *)renderComp);

    // prepare the pause menu

    int screenH = zEngine->display->currentMode.h;
    int screenW = zEngine->display->currentMode.w;

    // Percentages from the top of the screen
    double titlePos = 0.3;
    double listStartPos = 0.45;
    double listItemsSpacing = 0.08;

    // Create buttons with evenly spaced positions
    char* buttonLabels[] = {
        "Resume", "Exit to main menu"
    };
    // this is amazing
    void (*buttonActions[])(ZENg) = {
        &pauseToPlay, &pauseToMMenu
    };

    for (Uint8 orderIdx = 0; orderIdx < (sizeof(buttonLabels) / sizeof(buttonLabels[0])); orderIdx++) {
        ButtonComponent *button = createButtonComponent (
            zEngine->display->renderer, 
            getFont(zEngine->resources, "assets/fonts/ByteBounce.ttf"),
            strdup(buttonLabels[orderIdx]), 
            orderIdx == 0 ? COLOR_YELLOW : COLOR_WHITE, // First button selected (color)
            buttonActions[orderIdx], 
            orderIdx == 0 ? 1 : 0,  // First button selected (field flag)
            orderIdx
        );
        
        button->destRect->x = (screenW - button->destRect->w) / 2;
        button->destRect->y = screenH * (listStartPos + orderIdx * listItemsSpacing);
        
        id = createEntity(zEngine->ecs);
        addComponent(zEngine->ecs, id, BUTTON_COMPONENT, (void *)button);
    }

    // test entity
    Int32 testEStartTileX = playerStartTileX + 5;
    Int32 testEStartTileY = playerStartTileY - 5;
    Int32 testEStartTileIdx = testEStartTileY * ARENA_WIDTH + testEStartTileX;

    id = createEntity(zEngine->ecs);
    HealthComponent *ThealthComp = createHealthComponent(
        100, 100, 1
    );
    addComponent(zEngine->ecs, id, HEALTH_COMPONENT, (void *)ThealthComp);

    PositionComponent *TposComp = createPositionComponent(
        tileToWorld(testEStartTileIdx)
    );
    addComponent(zEngine->ecs, id, POSITION_COMPONENT, (void *)TposComp);

    CollisionComponent *TcolComp = createCollisionComponent(
        TposComp->x, TposComp->y,
        TILE_SIZE * 2, TILE_SIZE * 2, 1, COL_ACTOR
    );
    addComponent(zEngine->ecs, id, COLLISION_COMPONENT, (void *)TcolComp);

    RenderComponent *TrenderComp = createRenderComponent(
        getTexture(zEngine->resources, "assets/textures/tank.png"),
        TposComp->x, TposComp->y, TILE_SIZE * 2, TILE_SIZE * 2,
        1, 0
    );
    addComponent(zEngine->ecs, id, RENDER_COMPONENT, (void *)TrenderComp);

    initLevel(zEngine, "null for now");
}

void onExitPlayState(ZENg zEngine) {
    // delete game entities
    while (zEngine->ecs->entityCount > 0) {
        deleteEntity(zEngine->ecs, zEngine->ecs->activeEntities[0]);
    }

    // and the pause menu UI entities
    while (zEngine->ecs->entityCount > 0) {
        deleteEntity(zEngine->ecs, zEngine->ecs->activeEntities[0]);
    }
}

void spawnBulletProjectile(ZENg zEngine, Entity shooter) {
    Entity bulletID = createEntity(zEngine->ecs);

    // Bullet size
    int bulletW = 10, bulletH = 10;

    // get the shooter components
    Uint64 shooterPage = shooter / PAGE_SIZE;
    Uint64 shooterPageIdx = shooter % PAGE_SIZE;
    
    Uint64 shooterDirDenseIdx = zEngine->ecs->components[DIRECTION_COMPONENT].sparse[shooterPage][shooterPageIdx];
    // Bullet inherits the shooter's direction
    DirectionComponent *bulletDir = createDirectionComponent(
        *(DirectionComponent *)(zEngine->ecs->components[DIRECTION_COMPONENT].dense[shooterDirDenseIdx])
    );
    addComponent(zEngine->ecs, bulletID, DIRECTION_COMPONENT, (void *)bulletDir);

    Uint64 shooterRdrDenseIdx = zEngine->ecs->components[RENDER_COMPONENT].sparse[shooterPage][shooterPageIdx];
    SDL_Rect *shooterRect = (*(RenderComponent *)(zEngine->ecs->components[RENDER_COMPONENT].dense[shooterRdrDenseIdx])).destRect;

    Uint64 shooterPosDenseIdx = zEngine->ecs->components[POSITION_COMPONENT].sparse[shooterPage][shooterPageIdx];
    PositionComponent *shooterPos = (PositionComponent *)(zEngine->ecs->components[POSITION_COMPONENT].dense[shooterPosDenseIdx]);

    double_t playerCenterX = shooterPos->x + shooterRect->w / 2.0;
    double_t playerCenterY = shooterPos->y + shooterRect->h / 2.0;

    double_t bulletOffsetX = bulletDir->x * (shooterRect->w / 2.0 + bulletW / 2.0);
    double_t bulletOffsetY = bulletDir->y * (shooterRect->h / 2.0 + bulletH / 2.0);

    PositionComponent *bulletPos = createPositionComponent(
        (Vec2) {
            playerCenterX + bulletOffsetX - bulletW / 2.0,
            playerCenterY + bulletOffsetY - bulletH / 2.0
        }
    );
    addComponent(zEngine->ecs, bulletID, POSITION_COMPONENT, (void *)bulletPos);

    VelocityComponent *bulletSpeed = createVelocityComponent(
        (Vec2) {bulletDir->x * 500, bulletDir->y * 500},
        500.0, *bulletPos, AXIS_NONE, 1
    );
    addComponent(zEngine->ecs, bulletID, VELOCITY_COMPONENT, (void *)bulletSpeed);

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
    addComponent(zEngine->ecs, bulletID, PROJECTILE_COMPONENT, (void *)projComp);

    LifetimeComponent *lifeComp = calloc(1, sizeof(LifetimeComponent));
    if (!lifeComp) {
        printf("Failed to allocate memory for bullet lifetime component\n");
        exit(EXIT_FAILURE);
    }
    *lifeComp = (LifetimeComponent) {
        .lifeTime = 5,  // 5s
        .timeAlive = 0
    };
    addComponent(zEngine->ecs, bulletID, LIFETIME_COMPONENT, (void *)lifeComp);

    CollisionComponent *bulletColl = createCollisionComponent(
        (int)bulletPos->x, (int)bulletPos->y, bulletW, bulletH,
        0, COL_BULLET
    );
    addComponent(zEngine->ecs, bulletID, COLLISION_COMPONENT, (void *)bulletColl);

    RenderComponent *bulletRender = createRenderComponent(
        getTexture(zEngine->resources, "assets/textures/bullet.png"),
        (int)bulletPos->x, (int)bulletPos->y, bulletW, bulletH,
        1, 0
    );
    addComponent(zEngine->ecs, bulletID, RENDER_COMPONENT, (void *)bulletRender);
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
    Uint64 denseIdx = zEngine->ecs->components[VELOCITY_COMPONENT].sparse[page][pageIdx];

    VelocityComponent *playerSpeed = (VelocityComponent *)(zEngine->ecs->components[VELOCITY_COMPONENT].dense[denseIdx]);
    PositionComponent *playerPos = (PositionComponent *)(zEngine->ecs->components[POSITION_COMPONENT].dense[denseIdx]);
    DirectionComponent *playerDir = (DirectionComponent *)(zEngine->ecs->components[DIRECTION_COMPONENT].dense[denseIdx]);

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

        // Tell the velocity system to wake up
        zEngine->ecs->depGraph->nodes[SYS_VELOCITY]->isDirty = 1;
    } else {
        // If no movement input, stop the player
        playerSpeed->currVelocity = (Vec2) { .x = 0.0, .y = 0.0 };
    }
}

void updatePlayStateLogic(ZENg zEngine, double_t deltaTime) {
    lifetimeSystem(zEngine, deltaTime);  // Deletes expired entities
    velocitySystem(zEngine, deltaTime);  // Updates predicted positions
    worldCollisionSystem(zEngine, deltaTime);  // Handles world collisions based on predicted positions
    entityCollisionSystem(zEngine, deltaTime);  // Handles entities collisions based on predicted positions and validates them
    positionSystem(zEngine, deltaTime);  // Snaps entities to the grid when needed
    healthSystem(zEngine, deltaTime);  // Deletes entities with <= 0 health
    transformSystem(zEngine, deltaTime);  // Updates the screen textures of entities based on their virtual positions
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

#ifdef DEBUG
void renderDebugGrid(ZENg zEngine) {
    SDL_SetRenderDrawColor(zEngine->display->renderer, 100, 100, 100, 50);
    
    // Draw vertical grid lines
    for (int x = 0; x <= ARENA_WIDTH; x++) {
        SDL_RenderDrawLine(
            zEngine->display->renderer,
            x * TILE_SIZE, 0,
            x * TILE_SIZE, zEngine->display->currentMode.h
        );
    }
    
    // Draw horizontal grid lines
    for (int y = 0; y <= ARENA_HEIGHT; y++) {
        SDL_RenderDrawLine(
            zEngine->display->renderer,
            0, y * TILE_SIZE,
            zEngine->display->currentMode.w, y * TILE_SIZE
        );
    }
}
#endif

void renderPlayState(ZENg zEngine) {
    renderArena(zEngine);
    #ifdef DEBUG
        renderDebugGrid(zEngine);
        printf("There are %lu entities with a dirty render component\n", zEngine->ecs->components[RENDER_COMPONENT].dirtyCount);
    #endif

    while (zEngine->ecs->components[RENDER_COMPONENT].dirtyCount > 0) {
        // Render only the dirty entities
        Entity dirtyOwner = (zEngine->ecs->components[RENDER_COMPONENT].dirtyEntities[0]);
        Uint64 page = dirtyOwner / PAGE_SIZE;
        Uint64 index = dirtyOwner % PAGE_SIZE;
        Uint64 rdrDenseIdx = zEngine->ecs->components[RENDER_COMPONENT].sparse[page][index];
        RenderComponent *render = (RenderComponent *)(zEngine->ecs->components[RENDER_COMPONENT].dense[rdrDenseIdx]);

        if (render && render->destRect) {
            double angle = 0.0;

            // Check if the entity has a direction component
            bitset hasDirection = 1 << DIRECTION_COMPONENT;
            if (zEngine->ecs->componentsFlags[dirtyOwner] & hasDirection) {
                Uint64 dirDenseIdx = zEngine->ecs->components[DIRECTION_COMPONENT].sparse[page][index];
                DirectionComponent *dirComp = (DirectionComponent *)(zEngine->ecs->components[DIRECTION_COMPONENT].dense[dirDenseIdx]);

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
        unmarkComponentDirty(zEngine->ecs, RENDER_COMPONENT);  // mark the render component as clean
    }
    
    #ifdef DEBUG
        renderDebugCollision(zEngine);
    #endif
}