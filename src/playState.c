#include "include/stateManager.h"

void onEnterPlayState(ZENg zEngine) {
    initLevel(zEngine, "arenatest");

    // Add the initial game entities to the ECS
    Entity id = createEntity(zEngine->ecs, STATE_PLAYING);
    PLAYER_ID = id;  // set the global player ID

    HealthComponent *healthComp = createHealthComponent(100, 100, 1);
    addComponent(zEngine->ecs, id, HEALTH_COMPONENT, (void *)healthComp);

    Uint32 playerStartTileX = ARENA_WIDTH / 2;
    Uint32 playerStartTileY = ARENA_HEIGHT - 4;  // bottom
    Int32 playerStartTile = playerStartTileY * ARENA_WIDTH + playerStartTileX;

    PositionComponent *posComp = createPositionComponent(tileToWorld(zEngine->map, playerStartTile));
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

    Weapon *weap = createWeapon(strdup("Machine Gun"), 10.0, &spawnBulletProjectile);
    CDLLNode *weapList = initList((void *)weap);
    Weapon *weap2 = createWeapon(strdup("Pistol"), 2.0, &spawnBulletProjectile);
    CDLLInsertLast(weapList, (void *)weap2);
    WeaponComponent *weapComp = createWeaponComponent(weapList);
    addComponent(zEngine->ecs, id, WEAPON_COMPONENT, (void *)weapComp);

    // test entity
    Int32 testEStartTileX = ARENA_WIDTH / 2;
    Int32 testEStartTileY = 5;
    Int32 testEStartTileIdx = testEStartTileY * ARENA_WIDTH + testEStartTileX;

    id = createEntity(zEngine->ecs, STATE_PLAYING);
    HealthComponent *ThealthComp = createHealthComponent(
        100, 100, 1
    );
    addComponent(zEngine->ecs, id, HEALTH_COMPONENT, (void *)ThealthComp);

    PositionComponent *TposComp = createPositionComponent(
        tileToWorld(zEngine->map, testEStartTileIdx)
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

    // Enable the systems required by the play state
    SystemNode **systems = zEngine->ecs->depGraph->nodes;
    systems[SYS_LIFETIME]->isActive = 1;
    systems[SYS_WEAPONS]->isActive = 1;
    systems[SYS_VELOCITY]->isActive = 1;
    systems[SYS_WORLD_COLLISIONS]->isActive = 1;
    systems[SYS_ENTITY_COLLISIONS]->isActive = 1;
    systems[SYS_POSITION]->isActive = 1;
    systems[SYS_HEALTH]->isActive = 1;
    systems[SYS_TRANSFORM]->isActive = 1;

    // Force a systems run
    systems[SYS_VELOCITY]->isDirty = 1;

    // Mix_Chunk *music = getSound(zEngine->resources, "assets/sounds/music.mp3");
    // Mix_PlayChannel(-1, music, -1);
}

void onExitPlayState(ZENg zEngine) {
    // Delete game entities
    sweepState(zEngine->ecs, STATE_PLAYING);

    // Destroy the arena memory
    if (zEngine->map) {
        free(zEngine->map);
    }

    // Disable the play state's systems
    SystemNode **systems = zEngine->ecs->depGraph->nodes;
    systems[SYS_LIFETIME]->isActive = 0;
    systems[SYS_WEAPONS]->isActive = 0;
    systems[SYS_VELOCITY]->isActive = 0;
    systems[SYS_WORLD_COLLISIONS]->isActive = 0;
    systems[SYS_ENTITY_COLLISIONS]->isActive = 0;
    systems[SYS_POSITION]->isActive = 0;
    systems[SYS_HEALTH]->isActive = 0;
    systems[SYS_TRANSFORM]->isActive = 0;
}

void spawnBulletProjectile(
    ZENg zEngine, Entity shooter, int bulletW, int bulletH,
    double_t speed, ProjectileComponent *projComp,
    double_t lifeTime, SDL_Texture *texture, Mix_Chunk *sound
) {
    Entity bulletID = createEntity(zEngine->ecs, STATE_PLAYING);

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
        (Vec2) {bulletDir->x * speed, bulletDir->y * speed},
        speed, *bulletPos, AXIS_NONE, 1
    );
    addComponent(zEngine->ecs, bulletID, VELOCITY_COMPONENT, (void *)bulletSpeed);

    addComponent(zEngine->ecs, bulletID, PROJECTILE_COMPONENT, (void *)projComp);

    LifetimeComponent *lifeComp = calloc(1, sizeof(LifetimeComponent));
    if (!lifeComp) {
        printf("Failed to allocate memory for bullet lifetime component\n");
        exit(EXIT_FAILURE);
    }
    *lifeComp = (LifetimeComponent) {
        .lifeTime = lifeTime,
        .timeAlive = 0
    };
    addComponent(zEngine->ecs, bulletID, LIFETIME_COMPONENT, (void *)lifeComp);

    CollisionComponent *bulletColl = createCollisionComponent(
        (int)bulletPos->x, (int)bulletPos->y, bulletW, bulletH,
        0, COL_BULLET
    );
    addComponent(zEngine->ecs, bulletID, COLLISION_COMPONENT, (void *)bulletColl);

    RenderComponent *bulletRender = createRenderComponent(
        texture, (int)bulletPos->x, (int)bulletPos->y,
        bulletW, bulletH, 1, 0
    );
    addComponent(zEngine->ecs, bulletID, RENDER_COMPONENT, (void *)bulletRender);

    Uint64 weapDenseIdx = zEngine->ecs->components[WEAPON_COMPONENT].sparse[shooterPage][shooterPageIdx];
    WeaponComponent *shooterWeap = (WeaponComponent *)zEngine->ecs->components[WEAPON_COMPONENT].dense[weapDenseIdx];
    Weapon *currWeapon = (Weapon *)(shooterWeap->currWeapon->data);
    if (strcmp(currWeapon->name, "Machine Gun") == 0) {
        Mix_Chunk *buttonSound = getSound(zEngine->resources, "assets/sounds/rifle.mp3");
        if (!buttonSound) {
            printf("Failed to load button sound: %s\n", Mix_GetError());
            exit(EXIT_FAILURE);
        }
        Mix_PlayChannel(-1, buttonSound, 0);
    } else if (strcmp(currWeapon->name, "Pistol") == 0) {
        Mix_Chunk *buttonSound = getSound(zEngine->resources, "assets/sounds/mg.mp3");
        if (!buttonSound) {
            printf("Failed to load button sound: %s\n", Mix_GetError());
            exit(EXIT_FAILURE);
        }
        Mix_PlayChannel(-1, buttonSound, 0);
    }
}

Uint8 handlePlayStateEvents(SDL_Event *e, ZENg zEngine) {
    if (e->type == SDL_KEYDOWN) {
        InputAction action = scancodeToAction(zEngine->inputMng, e->key.keysym.scancode);
        if (action == INPUT_UNKNOWN) {
            printf("Unknown input action for scancode %d\n", e->key.keysym.scancode);
            return 1;
        }
        Uint64 page = PLAYER_ID / PAGE_SIZE;
        Uint64 pageIdx = PLAYER_ID % PAGE_SIZE;

        switch (action) {
            case INPUT_BACK: {
                GameState *pauseState = calloc(1, sizeof(GameState));
                if (!pauseState) {
                    printf("Failed to allocate memory for pause state\n");
                    exit(EXIT_FAILURE);
                }
                pauseState->type = STATE_PAUSED;
                pauseState->handleEvents = &handlePauseStateEvents;
                pauseState->onEnter = &onEnterPauseState;
                pauseState->onExit = &onExitPauseState;
                pauseState->isOverlay = 1;
                pushState(zEngine, pauseState);
                return 1;
            }
            case INPUT_SWITCH_LEFT: {
                Uint64 weapDenseIdx = zEngine->ecs->components[WEAPON_COMPONENT].sparse[page][pageIdx];
                WeaponComponent *playerWeap = (WeaponComponent *)zEngine->ecs->components[WEAPON_COMPONENT].dense[weapDenseIdx];
                Weapon *currWeapon = (Weapon *)(playerWeap->currWeapon->data);

                playerWeap->currWeapon = playerWeap->currWeapon->prev;
                #ifdef DEBUG
                    Weapon *newWeapon = (Weapon *)(playerWeap->currWeapon->data);
                    printf("Switched weapon left: %s -> %s\n", currWeapon->name, newWeapon->name);
                #endif
                return 1;
            }

            case INPUT_SWITCH_RIGHT: {
                Uint64 weapDenseIdx = zEngine->ecs->components[WEAPON_COMPONENT].sparse[page][pageIdx];
                WeaponComponent *playerWeap = (WeaponComponent *)zEngine->ecs->components[WEAPON_COMPONENT].dense[weapDenseIdx];
                Weapon *currWeapon = (Weapon *)(playerWeap->currWeapon->data);

                playerWeap->currWeapon = playerWeap->currWeapon->next;
                #ifdef DEBUG
                    Weapon *newWeapon = (Weapon *)(playerWeap->currWeapon->data);
                    printf("Switched weapon right: %s -> %s\n", currWeapon->name, newWeapon->name);
                #endif
                return 1;
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

    Uint64 velDenseIdx = zEngine->ecs->components[VELOCITY_COMPONENT].sparse[page][pageIdx];
    VelocityComponent *playerSpeed = (VelocityComponent *)(zEngine->ecs->components[VELOCITY_COMPONENT].dense[velDenseIdx]);

    Uint64 posDenseIdx = zEngine->ecs->components[POSITION_COMPONENT].sparse[page][pageIdx];
    PositionComponent *playerPos = (PositionComponent *)(zEngine->ecs->components[POSITION_COMPONENT].dense[posDenseIdx]);

    Uint64 dirDenseIdx = zEngine->ecs->components[DIRECTION_COMPONENT].sparse[page][pageIdx];
    DirectionComponent *playerDir = (DirectionComponent *)(zEngine->ecs->components[DIRECTION_COMPONENT].dense[dirDenseIdx]);

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

    if (isActionPressed(zEngine->inputMng, INPUT_SHOOT)) {
        Uint64 weapDenseIdx = zEngine->ecs->components[WEAPON_COMPONENT].sparse[page][pageIdx];
        WeaponComponent *playerWeap = (WeaponComponent *)zEngine->ecs->components[WEAPON_COMPONENT].dense[weapDenseIdx];
        Weapon *currWeapon = (Weapon *)(playerWeap->currWeapon->data);

        #ifdef DEBUG
            printf(
                "Trying to shoot... timeSinceUse: %.4f    timeRequired: %.4f\n",
                currWeapon->timeSinceUse, (1.0 / currWeapon->fireRate)
            );
        #endif

        if (currWeapon->timeSinceUse > (1.0 / currWeapon->fireRate)) {
            if (strcmp(currWeapon->name, "Machine Gun") == 0) {
                ProjectileComponent *projComp = createProjectileComponent(10, 0, 0, 1);

                currWeapon->spawnProj(
                    zEngine, PLAYER_ID, TILE_SIZE / 3, TILE_SIZE / 3,
                    400.0, projComp, 4.0, getTexture(zEngine->resources, "assets/textures/bullet.png"),
                    getSound(zEngine->resources, "assets/sounds/rifle.mp3")
                );
            } else if (strcmp(currWeapon->name, "Pistol") == 0) {
                ProjectileComponent *projComp = createProjectileComponent(45, 0, 0, 1);

                currWeapon->spawnProj(
                    zEngine, PLAYER_ID, TILE_SIZE / 2, TILE_SIZE / 2,
                    600.0, projComp, 3.0, getTexture(zEngine->resources, "assets/textures/bullet.png"),
                    getSound(zEngine->resources, "assets/sounds/mg.mp3")
                );
            }
            currWeapon->timeSinceUse = 0;
        }
        #ifdef DEBUG
            else {
                printf("Weapon has cooldown, can't shoot\n");
            }
        #endif
        return;
    }
}