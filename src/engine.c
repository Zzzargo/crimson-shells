#include "include/engine.h"

void loadSettings(ZENg zEngine, const char *filePath) {
    // look for the file
    FILE *fin = fopen(filePath, "r");

    zEngine->inputMng = calloc(1, sizeof(struct inputmng));
    if (!zEngine->inputMng) {
        printf("Failed allocating memory for the input manager");
        exit(EXIT_FAILURE);
    }

    zEngine->display = calloc(1, sizeof(struct displaymng));
    if (!zEngine->display) {
        printf("Failed allocating memory for the display manager");
        exit(EXIT_FAILURE);
    }

    if (!fin) {
        // file doesn't exist
        printf("No config file found in %s. Using defaults\n", filePath);
        setDefaultBindings(zEngine->inputMng);
        setDefaultDisplaySettings(zEngine->display);
        
        // the function above doesn't create the window and the renderer
        zEngine->display->window = SDL_CreateWindow(
            "Adele's Adventure",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            zEngine->display->currentMode.w,
            zEngine->display->currentMode.h,
            zEngine->display->wdwFlags
        );
        if (!zEngine->display->window) {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            exit(EXIT_FAILURE);
        }
        zEngine->display->renderer = SDL_CreateRenderer(
            zEngine->display->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
        );
        if (!zEngine->display->renderer) {
            printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
            SDL_DestroyWindow(zEngine->display->window);
            exit(EXIT_FAILURE);
        }
        return;
    }

    // if the file exists, read the settings

    enum {
        NONE,
        SECTION_DISPLAY,
        SECTION_BINDINGS
    } currSect = NONE;

    /* In case display settings are not fully specified, here's a failsafe*/
    Int32 width = 1280, height = 720;
    Int32 fullscreen = 0; Int32 vsync = 0;

    char line[128];

    while (fgets(line, sizeof(line), fin)) {
        line[strcspn(line, "\r\n")] = 0;

        // Skip empty lines
        if (strlen(line) == 0) continue;

        // Section headers
        if (strcmp(line, "[DISPLAY]") == 0) {
            currSect = SECTION_DISPLAY;
            continue;
        } else if (strcmp(line, "[INPUT]") == 0) {
            currSect = SECTION_BINDINGS;
            continue;
        }

        // Skip comments
        if (line[0] == '#') continue;
        
        char setting[64], value[64];
        if (sscanf(line, "%[^=]=%s", setting, value) != 2) continue;  // format is [SETTING]=[VALUE]

        switch (currSect) {
            case SECTION_DISPLAY: {
                if (strcmp(setting, "WIDTH") == 0) {
                    width = atoi(value);
                } else if (strcmp(setting, "HEIGHT") == 0) {
                    height = atoi(value);
                } else if (strcmp(setting, "FULLSCREEN") == 0) {
                    fullscreen = atoi(value);
                } else if (strcmp(setting, "VSYNC") == 0) {
                    vsync = atoi(value);
                } else {
                    printf("Unknown DISPLAY setting: %s\n", setting);
                }
                break;
            }
            case SECTION_BINDINGS: {
                SDL_Scancode scancode = SDL_GetScancodeFromName(value);
                if (scancode == SDL_SCANCODE_UNKNOWN) {
                    printf("Unknown value '%s' for action '%s'\n", value, setting);
                    continue;
                }
                if (strcmp(setting, "MOVE_UP") == 0) {
                    zEngine->inputMng->bindings[INPUT_MOVE_UP] = scancode;
                } else if (strcmp(setting, "MOVE_DOWN") == 0) {
                    zEngine->inputMng->bindings[INPUT_MOVE_DOWN] = scancode;
                } else if (strcmp(setting, "MOVE_LEFT") == 0) {
                    zEngine->inputMng->bindings[INPUT_MOVE_LEFT] = scancode;
                } else if (strcmp(setting, "MOVE_RIGHT") == 0) {
                    zEngine->inputMng->bindings[INPUT_MOVE_RIGHT] = scancode;
                } else if (strcmp(setting, "SELECT") == 0) {
                    zEngine->inputMng->bindings[INPUT_SELECT] = scancode;
                } else if (strcmp(setting, "BACK") == 0) {
                    zEngine->inputMng->bindings[INPUT_BACK] = scancode;
                } else if (strcmp(setting, "INTERACT") == 0) {
                    zEngine->inputMng->bindings[INPUT_INTERACT] = scancode;
                } else if (strcmp(setting, "SHOOT") == 0) {
                    zEngine->inputMng->bindings[INPUT_SHOOT] = scancode;
                } else if (strcmp(setting, "SPECIAL") == 0) {
                    zEngine->inputMng->bindings[INPUT_SPECIAL] = scancode;
                } else {
                    printf("Unknown action '%s'\n", setting);
                }
                break;
            }
        }
    }
    fclose(fin);

    zEngine->display->currentMode.w = width;
    zEngine->display->currentMode.h = height;
    zEngine->display->wdwFlags = fullscreen ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_SHOWN;

    // Create the window with the read settings
    zEngine->display->window = SDL_CreateWindow(
        "Adele's Adventure",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        zEngine->display->currentMode.w,
        zEngine->display->currentMode.h,
        zEngine->display->wdwFlags
    );

    if (!zEngine->display->window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Create renderer
    Int32 rendererFlags = SDL_RENDERER_ACCELERATED;
    if (vsync) rendererFlags |= SDL_RENDERER_PRESENTVSYNC;

    zEngine->display->renderer = SDL_CreateRenderer(
        zEngine->display->window, -1, rendererFlags
    );
    if (!zEngine->display->renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(zEngine->display->window);
        exit(EXIT_FAILURE);
    }

    printf("Settings loaded from %s\n", filePath);
}

/**
 * =====================================================================================================================
 */

void initLevel(ZENg zEngine, const char *levelFilePath) {
    if (!zEngine || !levelFilePath) {
        fprintf(stderr, "Invalid engine or level file path\n");
        return;
    }

    zEngine->map = calloc(1, sizeof(struct map));
    if (!zEngine->map) {
        fprintf(stderr, "Failed to allocate memory for the arena map\n");
        exit(EXIT_FAILURE);
    }

    for (Uint32 y = 0; y < ARENA_HEIGHT; y++) {
        for (Uint32 x = 0; x < ARENA_WIDTH; x++) {
            if ((y < 3 || y > 25) && (x % 3 == 0)) {
                zEngine->map->tiles[x][y] = (Tile) {
                    .type = TILE_BRICKS,
                    .texture = getTexture(zEngine->resources, "assets/textures/brick.jpg"),
                    .speedMod = 1.0,
                    .idx = y * ARENA_WIDTH + x,
                    .isWalkable = 0,
                    .isSolid = 1,
                    .damage = 0
                };
            } else {
                zEngine->map->tiles[x][y] = (Tile){
                    .type = TILE_EMPTY,
                    .texture = NULL,
                    .speedMod = 1.0,
                    .idx = y * ARENA_WIDTH + x,
                    .isWalkable = 1,
                    .isSolid = 0,
                    .damage = 0
                };
            }
        }
    }
}

/**
 * =====================================================================================================================
 */

DependencyGraph* initDependencyGraph() {
    DependencyGraph *graph = calloc(1, sizeof(DependencyGraph));
    if (!graph) {
        fprintf(stderr, "Failed to allocate memory for dependency graph\n");
        exit(EXIT_FAILURE);
    }
    graph->nodes = calloc(SYS_COUNT, sizeof(SystemNode*));
    if (!graph->nodes) {
        fprintf(stderr, "Failed to allocate memory for dependency graph nodes\n");
        free(graph);
        exit(EXIT_FAILURE);
    }

    typedef struct {
        SystemType type;
        void (*update)(ZENg, double_t);
        Uint8 isFineGrained;
    } SysPair;

    const SysPair sysPairs[] = {
        {SYS_LIFETIME, &lifetimeSystem, 1},
        {SYS_VELOCITY, &velocitySystem, 0},
        {SYS_WORLD_COLLISIONS, &worldCollisionSystem, 0},
        {SYS_ENTITY_COLLISIONS, &entityCollisionSystem, 0},
        {SYS_POSITION, &positionSystem, 0},
        {SYS_HEALTH, &healthSystem, 1},
        {SYS_TRANSFORM, &transformSystem, 0},
        {SYS_RENDER, &renderSystem, 0},
        {SYS_BUTTONS, &buttonSystem, 1}
    };

    for (Uint64 i = 0; i < SYS_COUNT; i++) {
        insertSystem(graph, createSystemNode(sysPairs[i].type, sysPairs[i].update, sysPairs[i].isFineGrained));
    }

    typedef struct {
        SystemType dependency;
        SystemType dependent;
    } DependencyPair;

    const DependencyPair dependencies[] = {
        {SYS_VELOCITY, SYS_WORLD_COLLISIONS},
        {SYS_VELOCITY, SYS_ENTITY_COLLISIONS},
        {SYS_WORLD_COLLISIONS, SYS_POSITION},
        {SYS_WORLD_COLLISIONS, SYS_HEALTH},
        {SYS_ENTITY_COLLISIONS, SYS_POSITION},
        {SYS_ENTITY_COLLISIONS, SYS_HEALTH},
        {SYS_POSITION, SYS_TRANSFORM},
        {SYS_TRANSFORM, SYS_RENDER},
        {SYS_BUTTONS, SYS_RENDER}
    };
    const size_t depCount = sizeof(dependencies) / sizeof(DependencyPair);

    for (size_t i = 0; i < depCount; i++) {
        SystemNode *dependency = graph->nodes[dependencies[i].dependency];
        SystemNode *dependent = graph->nodes[dependencies[i].dependent];
        if (dependency && dependent) {
            addSystemDependency(dependency, dependent);

            #ifdef DEBUG
                const char* sysNames[] = {
                    "SYS_LIFETIME",
                    "SYS_VELOCITY",
                    "SYS_WORLD_COLLISIONS",
                    "SYS_ENTITY_COLLISIONS",
                    "SYS_POSITION",
                    "SYS_HEALTH",
                    "SYS_TRANSFORM",
                    "SYS_RENDER",
                    "SYS_BUTTONS"
                };
                printf("Added system dependency: %s -> %s\n", sysNames[dependency->type], sysNames[dependent->type]);
            #endif
        } else {
            fprintf(stderr, "Invalid dependency pair: %d -> %d\n", dependencies[i].dependency, dependencies[i].dependent);
        }
    }

    return graph;
}

/**
 * =====================================================================================================================
 */

ZENg initGame() {
    ZENg zEngine = calloc(1, sizeof(struct engine));
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Initalize the display and input managers by reading settings file if existent
    loadSettings(zEngine, "settings.ini");

    // After setting the display resolution define the tile size
    // The tiles are guaranteed to be square
    TILE_SIZE = zEngine->display->currentMode.w / ARENA_WIDTH;

    // Initialize ECS
    initECS(&zEngine->ecs);

    // Initialize the resource manager and preload resources
    initResourceManager(&zEngine->resources);
    preloadResources(zEngine->resources, zEngine->display->renderer);

    // start on the main menu
    initStateManager(&zEngine->stateMng);
    GameState *mainMenuState = calloc(1, sizeof(GameState));
    if (!mainMenuState) {
        printf("Failed to allocate memory for main menu state\n");
        exit(EXIT_FAILURE);
    }

    mainMenuState->type = STATE_MAIN_MENU;
    mainMenuState->onEnter = &onEnterMainMenu;
    mainMenuState->onExit = &onExitMainMenu;
    mainMenuState->handleEvents = &handleMainMenuEvents;
    mainMenuState->update = &buttonSystem;
    pushState(zEngine, mainMenuState);

    return zEngine;
}

/**
 * =====================================================================================================================
 */

void velocitySystem(ZENg zEngine, double_t deltaTime) {
    if (zEngine->ecs->depGraph->nodes[SYS_VELOCITY]->isDirty == 0) {
        return;
    }
    
    #ifdef DEBUG
        printf("[VELOCITY SYSTEM] Running velocity system for %lu entities\n", zEngine->ecs->components[VELOCITY_COMPONENT].denseSize);
    #endif

    // for each entity with a VELOCITY_COMPONENT, update its position based on the current velocity
    for (Uint64 i = 0; i < zEngine->ecs->components[VELOCITY_COMPONENT].denseSize; i++) {
        VelocityComponent *velComp = (VelocityComponent *)(zEngine->ecs->components[VELOCITY_COMPONENT].dense[i]);
        Entity entitty = zEngine->ecs->components[VELOCITY_COMPONENT].denseToEntity[i];  // get the owner entity
        Uint64 page = entitty / PAGE_SIZE;
        Uint64 idx = entitty % PAGE_SIZE;

        if ((zEngine->ecs->componentsFlags[entitty] & (1 << POSITION_COMPONENT)) == 0) {
            continue;  // Skip entities without position
        }

        Uint64 posDenseIdx = zEngine->ecs->components[POSITION_COMPONENT].sparse[page][idx];
        PositionComponent *posComp = (PositionComponent *)zEngine->ecs->components[POSITION_COMPONENT].dense[posDenseIdx];
        if (!posComp) {
            continue;
        }

        // Update the predicted position based on the velocity
        velComp->predictedPos.x = posComp->x + velComp->currVelocity.x * deltaTime;
        velComp->predictedPos.y = posComp->y + velComp->currVelocity.y * deltaTime;

        // Clamp the position to the window bounds
        if (velComp->predictedPos.x < 0) velComp->predictedPos.x = 0;  // prevent going out of bounds
        if (velComp->predictedPos.y < 0) velComp->predictedPos.y = 0;  // prevent going out of bounds

        Uint64 denseRenderIndex = zEngine->ecs->components[RENDER_COMPONENT].sparse[page][idx];
        SDL_Rect *entityRect = ((RenderComponent *)(zEngine->ecs->components[RENDER_COMPONENT].dense[denseRenderIndex]))->destRect;
        if (entityRect) {
            int screenW = zEngine->display->currentMode.w;
            int screenH = zEngine->display->currentMode.h;

            if (velComp->predictedPos.x + entityRect->w > screenW) {
                velComp->predictedPos.x = screenW - entityRect->w;  // prevent going out of bounds
            }
            if (velComp->predictedPos.y + entityRect->h > screenH) {
                velComp->predictedPos.y = screenH - entityRect->h;  // prevent going out of bounds
            }
        }
        // Hitboxes are updates in the collision system
    }
    zEngine->ecs->depGraph->nodes[SYS_VELOCITY]->isDirty = 0;
}

/**
 * =====================================================================================================================
 */

void positionSystem(ZENg zEngine, double_t deltaTime) {
    if (zEngine->ecs->depGraph->nodes[SYS_POSITION]->isDirty == 0) {
        return;
    }
    #ifdef DEBUG
        printf("[POSITION SYSTEM] Running position system for %lu entities\n", zEngine->ecs->components[POSITION_COMPONENT].denseSize);
    #endif

    for (Uint64 i = 0; i  < zEngine->ecs->components[POSITION_COMPONENT].denseSize; i++) {
        PositionComponent *posComp = (PositionComponent *)(zEngine->ecs->components[POSITION_COMPONENT].dense[i]);
        Entity owner = zEngine->ecs->components[POSITION_COMPONENT].denseToEntity[i];
        Uint64 page = owner / PAGE_SIZE;
        Uint64 idx = owner % PAGE_SIZE;

        if ((zEngine->ecs->componentsFlags[owner] & (1 << HEALTH_COMPONENT)) == 0) {
            // Only actors can have health (at least for now)
            // And only actors obey the snap rule
            continue;
        }
        if ((zEngine->ecs->componentsFlags[owner] & (1 << VELOCITY_COMPONENT)) == 0) {
            // Only entities with a velocity component can be snapped
            continue;
        }
        Uint64 velDenseIdx = zEngine->ecs->components[VELOCITY_COMPONENT].sparse[page][idx];
        VelocityComponent *velComp = (VelocityComponent *)(zEngine->ecs->components[VELOCITY_COMPONENT].dense[velDenseIdx]);

        Uint8 movingX = fabs(velComp->currVelocity.x) > EPSILON;
        Uint8 movingY = fabs(velComp->currVelocity.y) > EPSILON;
        
        Int32 currTileIdx = worldToTile(*posComp);
        Uint32 currTileX = currTileIdx % ARENA_WIDTH;
        Uint32 currTileY = currTileIdx / ARENA_WIDTH;
        
        if (movingX && (velComp->prevAxis != AXIS_X)) {
            // Axis change -> snap the position to a tile
            posComp->y = round(posComp->y / TILE_SIZE) * TILE_SIZE;
            velComp->prevAxis = AXIS_X;
        } else if (movingY && (velComp->prevAxis != AXIS_Y)) {
            // Axis change -> snap the position to a tile
            posComp->x = round(posComp->x / TILE_SIZE) * TILE_SIZE;
            velComp->prevAxis = AXIS_Y;
        } else {
            // Last case can happen when the collision system changed the velocity
            if ((zEngine->ecs->componentsFlags[owner] & (1 << DIRECTION_COMPONENT)) != 0) {
                // If the current direction is on a different axis than the prevAxis - snap
                Uint64 dirDenseIdx = zEngine->ecs->components[DIRECTION_COMPONENT].sparse[page][idx];
                DirectionComponent *dirComp = (DirectionComponent *)(zEngine->ecs->components[DIRECTION_COMPONENT].dense[dirDenseIdx]);

                // TODO YOOOO THIS CAN ACTUALLY BE DONE ONLY WITH THE DIRECTION
            }
        }
    }
}

/**
 * =====================================================================================================================
 */

void lifetimeSystem(ZENg zEngine, double_t deltaTime) {
    for (Uint64 i = 0; i < zEngine->ecs->components[LIFETIME_COMPONENT].denseSize; i++) {
        LifetimeComponent *lifeComp = (LifetimeComponent *)(zEngine->ecs->components[LIFETIME_COMPONENT].dense[i]);
        lifeComp->timeAlive += deltaTime;
        if (lifeComp->timeAlive >= lifeComp->lifeTime) {
            Entity owner = zEngine->ecs->components[LIFETIME_COMPONENT].denseToEntity[i];
            deleteEntity(zEngine->ecs, owner);
        }
    }   
}

/**
 * =====================================================================================================================
 */

void handleEntitiesCollision(ZENg zEngine, CollisionComponent *AColComp, CollisionComponent *BColComp, Entity AOwner, Entity BOwner) {
    if (!AColComp->isSolid && !BColComp->isSolid) {
        // two bullets collide - skip collision check
        return;
    }

    Uint64 APage = AOwner / PAGE_SIZE;
    Uint64 AIndex = AOwner % PAGE_SIZE;
    Uint64 ADenseIndex = zEngine->ecs->components[PROJECTILE_COMPONENT].sparse[APage][AIndex];
    ProjectileComponent *AProjComp = (ProjectileComponent *)(zEngine->ecs->components[PROJECTILE_COMPONENT].dense[ADenseIndex]);
    
    Uint64 BPage = BOwner / PAGE_SIZE;
    Uint64 BIndex = BOwner % PAGE_SIZE;
    Uint64 BDenseIndex = zEngine->ecs->components[PROJECTILE_COMPONENT].sparse[BPage][BIndex];
    ProjectileComponent *BProjComp = (ProjectileComponent *)(zEngine->ecs->components[PROJECTILE_COMPONENT].dense[BDenseIndex]);

    // if a bullet hits a solid entity, delete the bullet and do damage
    if (AColComp->role == COL_BULLET && BColComp->isSolid && BOwner != PLAYER_ID && AProjComp) {
        Uint64 HPage = BOwner / PAGE_SIZE;
        Uint64 HIdx = BOwner % PAGE_SIZE;
        Uint64 HDenseIdx = zEngine->ecs->components[HEALTH_COMPONENT].sparse[HPage][HIdx];
        HealthComponent *actorHealth = (HealthComponent *)(zEngine->ecs->components[HEALTH_COMPONENT].dense[HDenseIdx]);
        if (actorHealth && actorHealth->active) {
            actorHealth->currentHealth -= AProjComp->dmg;
            markComponentDirty(zEngine->ecs, BOwner, HEALTH_COMPONENT);

            #ifdef DEBUG
                printf("Bullet (%lu) takes %u health from entity %lu\n", AOwner, AProjComp->dmg, BOwner);
            #endif
        }
        deleteEntity(zEngine->ecs, AOwner);  // delete the bullet
    } else if (BColComp->role == COL_BULLET && AColComp->isSolid && AOwner != PLAYER_ID && BProjComp) {
        Uint64 HPage = AOwner / PAGE_SIZE;
        Uint64 HIdx = AOwner % PAGE_SIZE;
        Uint64 HDenseIdx = zEngine->ecs->components[HEALTH_COMPONENT].sparse[HPage][HIdx];
        
        HealthComponent *actorHealth = (HealthComponent *)(zEngine->ecs->components[HEALTH_COMPONENT].dense[HDenseIdx]);
        if (actorHealth && actorHealth->active) {
            actorHealth->currentHealth -= BProjComp->dmg;
            markComponentDirty(zEngine->ecs, AOwner, HEALTH_COMPONENT);
            #ifdef DEBUG
                printf("Bullet (%lu) takes %u health from entity %lu\n", BOwner, BProjComp->dmg, AOwner);
            #endif
        }
        deleteEntity(zEngine->ecs, BOwner);  // delete the bullet
    }
}

/**
 * =====================================================================================================================
 */

Uint8 checkEntityCollision(ZENg zEngine, SDL_Rect *hitbox, SDL_Rect *result) {

    // For entities bigger than one tile. consider the base the top-left tile
    Uint32 tileX = (Uint32)(hitbox->x / TILE_SIZE);
    Uint32 tileY = (Uint32)(hitbox->y / TILE_SIZE);
    int eWidth = hitbox->w / TILE_SIZE;
    int eHeight = hitbox->h / TILE_SIZE;
    Int32 dxMax = eWidth + 1;
    Int32 dyMax = eHeight + 1;

    for (Int32 dx = -1; dx < dxMax; dx++) {
        for (Int32 dy = -1; dy < dyMax; dy++) {
            Uint32 neighX = (Int32)tileX + dx;
            Uint32 neighY = (Int32)tileY + dy;

            if (neighX > 0 && neighX < ARENA_WIDTH && neighY > 0 && neighY < ARENA_HEIGHT) {
                Tile *neighTile = &zEngine->map->tiles[neighX][neighY];
                SDL_Rect neighTileRect = {
                    .x = neighX * TILE_SIZE,
                    .y = neighY * TILE_SIZE,
                    .w = TILE_SIZE,
                    .h = TILE_SIZE
                };

                if (neighTile->isSolid && SDL_HasIntersection(hitbox, &neighTileRect)) {
                    *result = neighTileRect;
                    return 1;  // Collision detected
                }
            }
        }
    }
    return 0;  // No collision
}

/**
 * =====================================================================================================================
 */

void entityCollisionSystem(ZENg zEngine, double_t deltaTime) {
    if (zEngine->ecs->depGraph->nodes[SYS_ENTITY_COLLISIONS]->isDirty == 0) {
        return;
    }

    #ifdef DEBUG
        printf(
            "[ENTITY COLLISION SYSTEM] Running entity collision system for %lu entities\n",
            zEngine->ecs->components[COLLISION_COMPONENT].denseSize
        );
    #endif

    for (Uint64 i = 0; i < zEngine->ecs->components[COLLISION_COMPONENT].denseSize; i++) {
        CollisionComponent *AColComp = (CollisionComponent *)(zEngine->ecs->components[COLLISION_COMPONENT].dense[i]);
        Entity AOwner = zEngine->ecs->components[COLLISION_COMPONENT].denseToEntity[i];

        if (!AColComp || !AColComp->hitbox) {
            printf("Warning: Entity %ld has an invalid collision component\n", AOwner);
            continue;
        }

        int wW, wH;
        SDL_GetWindowSize(zEngine->display->window, &wW, &wH);
        // if a bullet hits the screen edge - remove it
        if (AColComp->role == COL_BULLET && (AColComp->hitbox->x <= 0
            || AColComp->hitbox->y <= 0
            || AColComp->hitbox->x + AColComp->hitbox->w >= wW
            || AColComp->hitbox->y + AColComp->hitbox->h >= wH)
        ) {
            deleteEntity(zEngine->ecs, AOwner);
            continue;  // skip to the next entity
        }

        // TODO: REDUCE O(N^2) COMPLEXITY BY CHECKING ONLY NEIGHBORING ENTITIES
        // quadtrees or spatial partitioning


        // Check collisions with other entities in the vicinity
        for (Uint64 j = i + 1; j < zEngine->ecs->components[COLLISION_COMPONENT].denseSize; j++) {
            CollisionComponent *BColComp = (CollisionComponent *)(zEngine->ecs->components[COLLISION_COMPONENT].dense[j]);
            Entity BOwner = zEngine->ecs->components[COLLISION_COMPONENT].denseToEntity[j];

            if (!BColComp || !BColComp->hitbox) {
                printf("Warning: Entity %ld has an invalid collision component\n", BOwner);
                continue;
            }

            // Check for intersections
            if (SDL_HasIntersection(AColComp->hitbox, BColComp->hitbox)) {
                #ifdef DEBUG
                    printf("Collision detected between entities %ld and %ld\n", AOwner, BOwner);
                #endif

                handleEntitiesCollision(zEngine, AColComp, BColComp, AOwner, BOwner);
            }
        }
    }
}

/**
 * =====================================================================================================================
 */

#ifdef DEBUG
// Add this function to help debug collision
void renderDebugCollision(ZENg zEngine) {
    // Draw entity hitboxes in red
    SDL_SetRenderDrawColor(zEngine->display->renderer, 255, 0, 0, 255);
    for (Uint64 i = 0; i < zEngine->ecs->components[COLLISION_COMPONENT].denseSize; i++) {
        CollisionComponent *colComp = (CollisionComponent *)(zEngine->ecs->components[COLLISION_COMPONENT].dense[i]);
        if (colComp && colComp->hitbox) {
            SDL_RenderDrawRect(zEngine->display->renderer, colComp->hitbox);
        }
    }
    
    // Draw solid tile boundaries in green
    SDL_SetRenderDrawColor(zEngine->display->renderer, 0, 255, 0, 255);
    for (Uint32 y = 0; y < ARENA_HEIGHT; y++) {
        for (Uint32 x = 0; x < ARENA_WIDTH; x++) {
            if (zEngine->map->tiles[x][y].isSolid) {
                SDL_Rect tileRect = {
                    .x = x * TILE_SIZE,
                    .y = y * TILE_SIZE,
                    .w = TILE_SIZE,
                    .h = TILE_SIZE
                };
                SDL_RenderDrawRect(zEngine->display->renderer, &tileRect);
            }
        }
    }
}
#endif

/**
 * =====================================================================================================================
 */

Uint8 checkWorldCollision(ZENg zEngine, SDL_Rect *hitbox, SDL_Rect *result) {

    // For entities bigger than one tile. consider the base the top-left tile
    Uint32 tileX = (Uint32)(hitbox->x / TILE_SIZE);
    Uint32 tileY = (Uint32)(hitbox->y / TILE_SIZE);
    int eWidth = hitbox->w / TILE_SIZE;
    int eHeight = hitbox->h / TILE_SIZE;
    Int32 dxMax = eWidth + 1;
    Int32 dyMax = eHeight + 1;

    for (Int32 dx = -1; dx <= dxMax; dx++) {
        for (Int32 dy = -1; dy <= dyMax; dy++) {
            Uint32 neighX = (Int32)tileX + dx;
            Uint32 neighY = (Int32)tileY + dy;

            if (neighX > 0 && neighX < ARENA_WIDTH && neighY > 0 && neighY < ARENA_HEIGHT) {
                Tile *neighTile = &zEngine->map->tiles[neighX][neighY];
                SDL_Rect neighTileRect = {
                    .x = neighX * TILE_SIZE,
                    .y = neighY * TILE_SIZE,
                    .w = TILE_SIZE,
                    .h = TILE_SIZE
                };

                if (neighTile->isSolid && SDL_HasIntersection(hitbox, &neighTileRect)) {
                    *result = neighTileRect;
                    return 1;  // Collision detected
                }
            }
        }
    }
    return 0;  // No collision
}

/**
 * =====================================================================================================================
 */

void worldCollisionSystem(ZENg zEngine, double_t deltaTime) {
    if (zEngine->ecs->depGraph->nodes[SYS_WORLD_COLLISIONS]->isDirty == 0) {
        return;
    }

    #ifdef DEBUG
        printf(
            "[WORLD COLLISION SYSTEM] Running world collision system for %lu entities\n",
            zEngine->ecs->components[COLLISION_COMPONENT].denseSize
        );
    #endif

    for (Uint64 i = 0; i < zEngine->ecs->components[COLLISION_COMPONENT].denseSize; i++) {
        Entity e = zEngine->ecs->components[COLLISION_COMPONENT].denseToEntity[i];
        Uint64 page = e / PAGE_SIZE;  // determine the page for the entity
        Uint64 index = e % PAGE_SIZE;  // determine the index within the page

        CollisionComponent *eColComp = (CollisionComponent *)(zEngine->ecs->components[COLLISION_COMPONENT].dense[i]);
        if (!eColComp || !eColComp->hitbox) {
            printf("Warning: Entity %ld has an invalid collision component\n", e);
            continue;
        }
        
        if ((zEngine->ecs->componentsFlags[e] & (1 << VELOCITY_COMPONENT)) == 0) {
            continue;  // Skip entities without velocity component
        }

        Uint64 velIdx = zEngine->ecs->components[VELOCITY_COMPONENT].sparse[page][index];
        VelocityComponent *eVelComp = (VelocityComponent *)(zEngine->ecs->components[VELOCITY_COMPONENT].dense[velIdx]);
        if (!eVelComp) {
            printf("Warning: Entity %ld has an invalid velocity component\n", e);
            continue;
        }

        if ((zEngine->ecs->componentsFlags[e] & (1 << POSITION_COMPONENT)) == 0) {
            continue;  // Skip entities without position component
        }
        Uint64 posIdx = zEngine->ecs->components[POSITION_COMPONENT].sparse[page][index];
        PositionComponent *posComp = (PositionComponent *)(zEngine->ecs->components[POSITION_COMPONENT].dense[posIdx]);
        if (!posComp) {
            printf("Warning: Entity %ld has an invalid position component\n", e);
            continue;
        }

        SDL_Rect collidedTile;  // passed to the function checking world collisions
        
        // Resolve X-Axis collisions
        double_t moveX = eVelComp->currVelocity.x * deltaTime;
        eColComp->hitbox->x = eVelComp->predictedPos.x;

        if (checkWorldCollision(zEngine, eColComp->hitbox, &collidedTile)) {
            
            if (moveX > 0) {
                eColComp->hitbox->x = collidedTile.x - eColComp->hitbox->w;
            } else if (moveX < 0) {
                eColComp->hitbox->x = collidedTile.x + collidedTile.w;
            }
            eVelComp->currVelocity.x = 0.0;
        }
        posComp->x = eColComp->hitbox->x;

        // Resolve Y-Axis collisions
        double_t moveY = eVelComp->currVelocity.y * deltaTime;
        eColComp->hitbox->y = eVelComp->predictedPos.y;

        if (checkWorldCollision(zEngine, eColComp->hitbox, &collidedTile)) {

            if (moveY > 0) {
                eColComp->hitbox->y = collidedTile.y - eColComp->hitbox->h;
            } else if (moveY < 0) {
                eColComp->hitbox->y = collidedTile.y + collidedTile.h;
            }
            eVelComp->currVelocity.y = 0.0;
        }
        posComp->y = eColComp->hitbox->y;
    }
}


/**
 * =====================================================================================================================
 */

void healthSystem(ZENg zEngine, double_t deltaTime) {
    #ifdef DEBUG
        printf("[HEALTH SYSTEM] There are %lu dirty health components\n", zEngine->ecs->components[HEALTH_COMPONENT].dirtyCount);
    #endif
    
    while (zEngine->ecs->components[HEALTH_COMPONENT].dirtyCount > 0) {
        Entity ownerID = zEngine->ecs->components[HEALTH_COMPONENT].dirtyEntities[0];
        Uint64 page = ownerID / PAGE_SIZE;
        Uint64 index = ownerID % PAGE_SIZE;

        Uint64 helfDenseIdx = zEngine->ecs->components[HEALTH_COMPONENT].sparse[page][index];
        HealthComponent *helfComp = (HealthComponent *)(zEngine->ecs->components[HEALTH_COMPONENT].dense[helfDenseIdx]);

        if (helfComp->currentHealth <= 0) {
            deleteEntity(zEngine->ecs, ownerID);
        }
        unmarkComponentDirty(zEngine->ecs, HEALTH_COMPONENT);
    }
}

/**
 * =====================================================================================================================
 */

void transformSystem(ZENg zEngine, double_t deltaTime) {
    if (zEngine->ecs->depGraph->nodes[SYS_TRANSFORM]->isDirty == 0) {
        return;
    }

    #ifdef DEBUG
        printf(
            "[TRANSFORM SYSTEM] Running transform system for %lu entities\n",
            zEngine->ecs->components[POSITION_COMPONENT].denseSize
        );
    #endif

    // iterate through all entities with POSITION_COMPONENT and update their rendered textures
    for (Uint64 i = 0; i < zEngine->ecs->components[POSITION_COMPONENT].denseSize; i++) {
        PositionComponent *posComp = (PositionComponent *)(zEngine->ecs->components[POSITION_COMPONENT].dense[i]);
        Entity entitty = zEngine->ecs->components[POSITION_COMPONENT].denseToEntity[i];  // get the owner entity

        if ((zEngine->ecs->componentsFlags[entitty] & (1 << RENDER_COMPONENT)) == (1 << RENDER_COMPONENT)) {
            // if the entity has also the render component
            Uint64 page = entitty / PAGE_SIZE;  // determine the page for the entity
            Uint64 index = entitty % PAGE_SIZE;  // determine the index within the page

            Uint64 denseIndex = zEngine->ecs->components[RENDER_COMPONENT].sparse[page][index];
            if (denseIndex >= zEngine->ecs->components[RENDER_COMPONENT].denseSize) {
                printf("Warning: Entity %ld has a render component with invalid dense index %lu\n", entitty, denseIndex);
                continue;
            }
            // Update the render component's destination rectangle based on the position component
            RenderComponent *renderComp = (RenderComponent *)(zEngine->ecs->components[RENDER_COMPONENT].dense[denseIndex]);

            if (renderComp) {  // sanity check cause I've been pretty insane lately
                renderComp->destRect->x = (int)posComp->x;
                renderComp->destRect->y = (int)posComp->y;
                
                markComponentDirty(zEngine->ecs, entitty, RENDER_COMPONENT);
            }
        }
    }
}

/**
 * =====================================================================================================================
 */
void renderSystem(ZENg zEngine, double_t deltaTime) {
    if (zEngine->ecs->depGraph->nodes[SYS_RENDER]->isDirty == 0) {
        return;  // No need to render if nothing changed
    }

    #ifdef DEBUG
        Uint64 renderCount = zEngine->ecs->components[RENDER_COMPONENT].denseSize + zEngine->ecs->components[BUTTON_COMPONENT].denseSize;
        printf("[RENDER SYSTEM] Running render system for %lu entities\n", renderCount);
    #endif
    
    // Clear the screen
    SDL_SetRenderDrawColor(zEngine->display->renderer, 0, 0, 0, 255);  // Pitch black
    SDL_RenderClear(zEngine->display->renderer);

    if (getCurrState(zEngine->stateMng)->type == STATE_PLAYING) renderArena(zEngine);

    #ifdef DEBUG
        if (getCurrState(zEngine->stateMng)->type == STATE_PLAYING) renderDebugGrid(zEngine);
    #endif

    for (Uint64 i = 0; i < zEngine->ecs->components[RENDER_COMPONENT].denseSize; i++) {
        RenderComponent *render = (RenderComponent *)(zEngine->ecs->components[RENDER_COMPONENT].dense[i]);
        Entity owner = (zEngine->ecs->components[RENDER_COMPONENT].denseToEntity[i]);
        Uint64 page = owner / PAGE_SIZE;
        Uint64 index = owner % PAGE_SIZE;

        if (render && render->destRect) {
            double angle = 0.0;

            // Check if the entity has a direction component
            bitset hasDirection = 1 << DIRECTION_COMPONENT;
            if (zEngine->ecs->componentsFlags[owner] & hasDirection) {
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
    }
    
    #ifdef DEBUG
        if (getCurrState(zEngine->stateMng)->type == STATE_PLAYING) renderDebugCollision(zEngine);
    #endif

    for (Uint64 i = 0; i < zEngine->ecs->components[BUTTON_COMPONENT].denseSize; i++) {
        ButtonComponent *button = (ButtonComponent *)(zEngine->ecs->components[BUTTON_COMPONENT].dense[i]);

        if (button && button->texture && button->destRect) {
            SDL_RenderCopy(zEngine->display->renderer, button->texture, NULL, button->destRect);
        } else {
            Entity owner = (zEngine->ecs->components[BUTTON_COMPONENT].denseToEntity[i]);
            printf("Warning: Entity %ld has an invalid button component\n", owner);
        }
    }

    zEngine->ecs->depGraph->nodes[SYS_RENDER]->isDirty = 0;
}

/**
 * =====================================================================================================================
 */

void buttonSystem(ZENg zEngine, double_t deltaTime) {
    // Rerender the UI based on the entities' current components' states

    #ifdef DEBUG
        printf("[BUTTON SYSTEM] There are %lu dirty button components\n", zEngine->ecs->components[BUTTON_COMPONENT].dirtyCount);
    #endif

    while (zEngine->ecs->components[BUTTON_COMPONENT].dirtyCount > 0) {
        Entity dirtyOwner  = zEngine->ecs->components[BUTTON_COMPONENT].dirtyEntities[0];
        Uint64 page = dirtyOwner / PAGE_SIZE;
        Uint64 index = dirtyOwner % PAGE_SIZE;
        Uint64 buttDenseIdx = zEngine->ecs->components[BUTTON_COMPONENT].sparse[page][index];

        ButtonComponent *curr = (ButtonComponent *)(zEngine->ecs->components[BUTTON_COMPONENT].dense[buttDenseIdx]);
        if (!curr) {
            printf("Warning: Entity %ld has an invalid button component\n", dirtyOwner);
            continue;
        }

        // Update the texture
        SDL_DestroyTexture(curr->texture);
        SDL_Surface *surface = TTF_RenderText_Solid(
            curr->font,
            curr->text,
            curr->selected ? COLOR_YELLOW : COLOR_WHITE
        );
        curr->texture = SDL_CreateTextureFromSurface(zEngine->display->renderer, surface);
        SDL_FreeSurface(surface);

        unmarkComponentDirty(zEngine->ecs, BUTTON_COMPONENT);
    }
}

/**
 * =====================================================================================================================
 */

void saveSettings(ZENg zEngine, const char *filePath) {
    saveKeyBindings(zEngine->inputMng, filePath);
    saveDisplaySettings(zEngine->display, filePath);
    printf("Settings saved to %s\n", filePath);
}

/**
 * =====================================================================================================================
 */

void destroyEngine(ZENg *zEngine) {
    saveSettings((*zEngine), "settings.ini");
    free((*zEngine)->inputMng);

    freeResourceManager(&(*zEngine)->resources);

    freeECS((*zEngine)->ecs);

    free((*zEngine)->stateMng->states[0]);  // free the main menu state
    free((*zEngine)->stateMng);

    SDL_DestroyRenderer((*zEngine)->display->renderer);
    SDL_DestroyWindow((*zEngine)->display->window);
    free((*zEngine)->display);
    SDL_Quit();
    free(*zEngine);
}
