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
        200.0, *posComp, AXIS_NONE, 1
    );
    addComponent(zEngine->ecs, id, VELOCITY_COMPONENT, (void *)speedComp);

    CollisionComponent *colComp = createCollisionComponent(
        posComp->x, posComp->y, TILE_SIZE * 2, TILE_SIZE * 2,
        1, COL_ACTOR
    );
    addComponent(zEngine->ecs, id, COLLISION_COMPONENT, (void *)colComp);

    RenderComponent *renderComp = createRenderComponent(
        getTexture(zEngine->resources, "assets/textures/tank2.png"),
        posComp->x, posComp->y, TILE_SIZE * 2, TILE_SIZE * 2,
        1, 0
    );
    addComponent(zEngine->ecs, id, RENDER_COMPONENT, (void *)renderComp);

    Entity mainGunID = createEntity(zEngine->ecs, STATE_PLAYING);
    ProjectileComponent *mainProjComp = createProjectileComponent(45, 0, 0, 1);
    WeaponComponent *mainG = createWeaponComponent(
        strdup("Main Gun"), 1.0, &spawnBulletProjectile, 20, 20, 400.0,
        mainProjComp, 5.0, getTexture(zEngine->resources, "assets/textures/bullet.png"),
        getSound(zEngine->resources, "assets/sounds/shell1.mp3")
    );
    addComponent(zEngine->ecs, mainGunID, WEAPON_COMPONENT, (void *)mainG);

    Entity secGun1ID = createEntity(zEngine->ecs, STATE_PLAYING);
    ProjectileComponent *secProjComp1 = createProjectileComponent(15, 0, 0, 1);
    WeaponComponent *secG1 = createWeaponComponent(
        strdup("Coaxial Machine Gun 1"), 5.0, &spawnBulletProjectile, 10, 10, 300.0,
        secProjComp1, 3.0, getTexture(zEngine->resources, "assets/textures/bullet.png"),
        getSound(zEngine->resources, "assets/sounds/coaxmg1.mp3")
    );
    addComponent(zEngine->ecs, secGun1ID, WEAPON_COMPONENT, (void *)secG1);
    CDLLNode *weapList = initList((void *)secG1);

    Entity secGun2ID = createEntity(zEngine->ecs, STATE_PLAYING);
    ProjectileComponent *secProjComp2 = createProjectileComponent(10, 0, 0, 1);
    WeaponComponent *secG2 = createWeaponComponent(
        strdup("Coaxial Machine Gun 2"), 6.5, &spawnBulletProjectile, 10, 10, 300.0,
        secProjComp2, 3.0, getTexture(zEngine->resources, "assets/textures/bullet.png"),
        getSound(zEngine->resources, "assets/sounds/coaxmg2.mp3")
    );
    addComponent(zEngine->ecs, secGun2ID, WEAPON_COMPONENT, (void *)secG2);
    CDLLInsertLast(weapList, (void *)secG2);

    Entity hullID = createEntity(zEngine->ecs, STATE_PLAYING);
    Entity moduleID = createEntity(zEngine->ecs, STATE_PLAYING);
    LoadoutComponent *loadout = createLoadoutComponent(mainGunID, weapList, hullID, moduleID);
    addComponent(zEngine->ecs, id, LOADOUT_COMPONENT, (void *)loadout);

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
    clearLevel(zEngine);

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

void spawnBulletProjectile( ZENg zEngine, Entity shooter, int bulletW, int bulletH,
    double_t speed, ProjectileComponent *projComp, double_t lifeTime, SDL_Texture *texture, Mix_Chunk *sound ) {
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

    ProjectileComponent *projCompCopy = createProjectileComponent(
        projComp->dmg, projComp->piercing, projComp->exploding, projComp->friendly
    );  // Copy the gun's projectile component to prevent deleting the original at bullet collision
    addComponent(zEngine->ecs, bulletID, PROJECTILE_COMPONENT, (void *)projCompCopy);

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

    // Play firing sound
    Mix_PlayChannel(-1, sound, 0);
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
                Uint64 loadDenseIdx = zEngine->ecs->components[LOADOUT_COMPONENT].sparse[page][pageIdx];
                LoadoutComponent *playerLoadout = (LoadoutComponent *)zEngine->ecs->components[LOADOUT_COMPONENT].dense[loadDenseIdx];

                CDLLNode *currSecGun = playerLoadout->currSecondaryGun;
                WeaponComponent *secGun = (WeaponComponent *)currSecGun->data;
                if (!secGun) {
                    #ifdef DEBUG
                        printf("No secondary weapons to switch to!\n");
                    #endif
                    return 1;
                }

                playerLoadout->currSecondaryGun = currSecGun->prev;
                #ifdef DEBUG
                    WeaponComponent *newWeapon = (WeaponComponent *)(playerLoadout->currSecondaryGun->data);
                    printf("Switched weapon left: %s -> %s\n", secGun->name, newWeapon->name);
                #endif
                return 1;
            }

            case INPUT_SWITCH_RIGHT: {
                Uint64 loadDenseIdx = zEngine->ecs->components[LOADOUT_COMPONENT].sparse[page][pageIdx];
                LoadoutComponent *playerLoadout = (LoadoutComponent *)zEngine->ecs->components[LOADOUT_COMPONENT].dense[loadDenseIdx];

                CDLLNode *currSecGun = playerLoadout->currSecondaryGun;
                WeaponComponent *secGun = (WeaponComponent *)currSecGun->data;
                if (!secGun) {
                    #ifdef DEBUG
                        printf("No secondary weapons to switch to!\n");
                    #endif
                    return 1;
                }

                playerLoadout->currSecondaryGun = currSecGun->next;
                #ifdef DEBUG
                    WeaponComponent *newWeapon = (WeaponComponent *)(playerLoadout->currSecondaryGun->data);
                    printf("Switched weapon right: %s -> %s\n", secGun->name, newWeapon->name);
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

    // Shooting

    if (isActionPressed(zEngine->inputMng, INPUT_SHOOT)) {
        Uint64 loadoutDenseIdx = zEngine->ecs->components[LOADOUT_COMPONENT].sparse[page][pageIdx];
        LoadoutComponent *playerLoadout = (LoadoutComponent *)zEngine->ecs->components[LOADOUT_COMPONENT].dense[loadoutDenseIdx];
        Entity mainGunID = playerLoadout->primaryGun;
        Uint64 mainGunPage = mainGunID / PAGE_SIZE;
        Uint64 mainGunPageIdx = mainGunID % PAGE_SIZE;

        Uint64 mainGunDenseIdx = zEngine->ecs->components[WEAPON_COMPONENT].sparse[mainGunPage][mainGunPageIdx];
        WeaponComponent *mainGun = (WeaponComponent *)zEngine->ecs->components[WEAPON_COMPONENT].dense[mainGunDenseIdx];

        #ifdef DEBUG
            printf(
                "Trying to shoot... timeSinceUse: %.4f    timeRequired: %.4f\n",
                mainGun->timeSinceUse, (1.0 / mainGun->fireRate)
            );
        #endif

        if (mainGun->timeSinceUse > (1.0 / mainGun->fireRate)) {
            mainGun->spawnProj(
                zEngine, PLAYER_ID,
                mainGun->projW, mainGun->projH,
                mainGun->projSpeed, mainGun->projComp,
                mainGun->projLifeTime, mainGun->projTexture,
                mainGun->projSound
            );
            mainGun->timeSinceUse = 0;
        }
        #ifdef DEBUG
            else {
                printf("Weapon has cooldown, can't shoot\n");
            }
        #endif
    }

    if (isActionPressed(zEngine->inputMng, INPUT_SECONDARY)) {
        Uint64 loadoutDenseIdx = zEngine->ecs->components[LOADOUT_COMPONENT].sparse[page][pageIdx];
        LoadoutComponent *playerLoadout = (LoadoutComponent *)zEngine->ecs->components[LOADOUT_COMPONENT].dense[loadoutDenseIdx];
        WeaponComponent *currSecGun = playerLoadout->currSecondaryGun->data;

        #ifdef DEBUG
            printf(
                "Trying to shoot... timeSinceUse: %.4f    timeRequired: %.4f\n",
                currSecGun->timeSinceUse, (1.0 / currSecGun->fireRate)
            );
        #endif

        if (currSecGun->timeSinceUse > (1.0 / currSecGun->fireRate)) {
            currSecGun->spawnProj(
                zEngine, PLAYER_ID,
                currSecGun->projW, currSecGun->projH,
                currSecGun->projSpeed, currSecGun->projComp,
                currSecGun->projLifeTime, currSecGun->projTexture,
                currSecGun->projSound
            );
            currSecGun->timeSinceUse = 0;
        }
        #ifdef DEBUG
            else {
                printf("Weapon has cooldown, can't shoot\n");
            }
        #endif
    }
}