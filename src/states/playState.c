#include "stateManager.h"

void onEnterPlayState(ZENg zEngine) {
    initLevel(zEngine, "data/arenatest");

    // Add some weapons to the player while testing the arena parser
    Entity mainGunID = createEntity(zEngine->ecs, STATE_PLAYING);
    WeaponPrefab *mainGunPrefab = getWeaponPrefab(zEngine->prefabs, "Bigfella");
    WeaponComponent *mainG = instantiateWeapon(zEngine, mainGunPrefab, PLAYER_ID);
    addComponent(zEngine->ecs, mainGunID, WEAPON_COMPONENT, (void *)mainG);

    Entity secGun1ID = createEntity(zEngine->ecs, STATE_PLAYING);
    WeaponPrefab *secGun1Prefab = getWeaponPrefab(zEngine->prefabs, "PKT");
    WeaponComponent *secGun1 = instantiateWeapon(zEngine, secGun1Prefab, PLAYER_ID);
    addComponent(zEngine->ecs, secGun1ID, WEAPON_COMPONENT, (void *)secGun1);
    CDLLNode *weapList = initList((void *)secGun1);

    Entity secGun2ID = createEntity(zEngine->ecs, STATE_PLAYING);
    WeaponPrefab *secGun2Prefab = getWeaponPrefab(zEngine->prefabs, "M240C");
    WeaponComponent *secGun2 = instantiateWeapon(zEngine, secGun2Prefab, PLAYER_ID);
    addComponent(zEngine->ecs, secGun2ID, WEAPON_COMPONENT, (void *)secGun2);
    CDLLInsertLast(weapList, (void *)secGun2);

    Entity hullID = createEntity(zEngine->ecs, STATE_PLAYING);
    Entity moduleID = createEntity(zEngine->ecs, STATE_PLAYING);
    LoadoutComponent *loadout = createLoadoutComponent(mainGunID, weapList, hullID, moduleID);
    addComponent(zEngine->ecs, PLAYER_ID, LOADOUT_COMPONENT, (void *)loadout);

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
}

/**
 * =====================================================================================================================
 */

void onExitPlayState(ZENg zEngine) {
    // Delete game entities
    sweepState(zEngine->ecs, STATE_PLAYING);
    UIclear(zEngine->uiManager);

    // Destroy the arena
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

/**
 * =====================================================================================================================
 */

WeaponComponent* instantiateWeapon(ZENg zEngine, WeaponPrefab *prefab, Entity owner) {
    WeaponComponent *weap = calloc(1, sizeof(WeaponComponent));
    if (!weap) {
        printf("Failed to allocate memory for weapon\n");
        exit(EXIT_FAILURE);
    }
    weap->name = strdup(prefab->name);
    weap->fireRate = prefab->fireRate;
    weap->timeSinceUse = 0.0;
    weap->spawnProj = &spawnBulletProjectile;
    weap->projW = prefab->projW;
    weap->projH = prefab->projH;
    weap->projSpeed = prefab->projSpeed;
    weap->projLifeTime = prefab->projLifeTime;
    weap->projTexture = getTexture(zEngine->resources, prefab->projTexturePath);
    weap->projSound = getSound(zEngine->resources, prefab->projHitSoundPath);
    weap->projComp = createProjectileComponent(
        prefab->dmg, prefab->isPiercing, prefab->isExplosive, owner == PLAYER_ID ? 1 : 0
    );
    return weap;
}

/**
 * =====================================================================================================================
 */

Entity instantiateTank(ZENg zEngine, TankPrefab *prefab, Vec2 position) {
    Entity id = createEntity(zEngine->ecs, STATE_PLAYING);
    if (prefab->entityType == ENTITY_PLAYER) {
        PLAYER_ID = id;  // set the global player ID
    }

    HealthComponent *healthComp = createHealthComponent(prefab->maxHealth, prefab->maxHealth, 1);
    addComponent(zEngine->ecs, id, HEALTH_COMPONENT, (void *)healthComp);

    Uint32 playerStartTileX = ARENA_WIDTH / 2;
    Uint32 playerStartTileY = ARENA_HEIGHT - 4;  // bottom
    Int32 playerStartTile = playerStartTileY * ARENA_WIDTH + playerStartTileX;

    PositionComponent *posComp = createPositionComponent(position);
    addComponent(zEngine->ecs, id, POSITION_COMPONENT, (void *)posComp);

    DirectionComponent *dirComp = createDirectionComponent(DIR_UP);  // Default direction
    addComponent(zEngine->ecs, id, DIRECTION_COMPONENT, (void *)dirComp);

    VelocityComponent *speedComp = createVelocityComponent(
        (Vec2){0.0, 0.0},
        prefab->maxSpeed, *posComp, AXIS_NONE, 1
    );
    addComponent(zEngine->ecs, id, VELOCITY_COMPONENT, (void *)speedComp);

    CollisionComponent *colComp = createCollisionComponent(
        posComp->x, posComp->y, prefab->w * TILE_SIZE, prefab->h * TILE_SIZE,
        1, COL_ACTOR
    );
    addComponent(zEngine->ecs, id, COLLISION_COMPONENT, (void *)colComp);

    RenderComponent *renderComp = createRenderComponent(
        getTexture(zEngine->resources, prefab->texturePath),
        posComp->x, posComp->y, colComp->hitbox->w, colComp->hitbox->h,
        1, 0
    );
    addComponent(zEngine->ecs, id, RENDER_COMPONENT, (void *)renderComp);
}

/**
 * =====================================================================================================================
 */

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

/**
 * =====================================================================================================================
 */

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

/**
 * =====================================================================================================================
 */

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