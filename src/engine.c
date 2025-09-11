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
            "Crimson Shells",
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
                } else if (strcmp(setting, "SECONDARY_SHOOT") == 0) {
                    zEngine->inputMng->bindings[INPUT_SECONDARY] = scancode;
                } else if (strcmp(setting, "SWITCH_LEFT") == 0) {
                    zEngine->inputMng->bindings[INPUT_SWITCH_LEFT] = scancode;
                } else if (strcmp(setting, "SWITCH_RIGHT") == 0) {
                    zEngine->inputMng->bindings[INPUT_SWITCH_RIGHT] = scancode;
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
    zEngine->display->fullscreen = fullscreen;
    zEngine->display->vsync = vsync;

    // Create the window with the read settings
    zEngine->display->window = SDL_CreateWindow(
        "Crimson Shells",
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

    #ifdef DEBUG
        printf("Display settings: %dx%d, fullscreen: %d, vsync: %d\n",
            zEngine->display->currentMode.w,
            zEngine->display->currentMode.h,
            zEngine->display->fullscreen,
            zEngine->display->vsync
        );
    #endif
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

    zEngine->map->tiles = calloc(ARENA_HEIGHT, sizeof(Tile*));
    if (!zEngine->map->tiles) {
        fprintf(stderr, "Failed to allocate memory for arena tiles rows\n");
        free(zEngine->map);
        exit(EXIT_FAILURE);
    }
    for (Uint32 i = 0; i < ARENA_HEIGHT; i++) {
        zEngine->map->tiles[i] = calloc(ARENA_WIDTH, sizeof(Tile));
        if (!zEngine->map->tiles[i]) {
            fprintf(stderr, "Failed to allocate memory for arena tiles columns\n");
            exit(EXIT_FAILURE);
        }
    }

    // Initialize the tileDefs
    Tile *defs = zEngine->map->tileDefs;
    defs[TILE_EMPTY].isWalkable = 1;
    defs[TILE_EMPTY].type = TILE_EMPTY;

    defs[TILE_BRICKS].isSolid = 1;
    defs[TILE_BRICKS].type = TILE_BRICKS;
    defs[TILE_BRICKS].texture = getTexture(zEngine->resources, "assets/textures/brick.jpg");

    defs[TILE_ROCK].isSolid = 1;
    defs[TILE_ROCK].type = TILE_ROCK;
    defs[TILE_ROCK].texture = getTexture(zEngine->resources, "assets/textures/rocks.jpg");


    FILE *fin = fopen(levelFilePath, "r");
    if (!fin) {
        fprintf(stderr, "Failed to open level file: %s\n", levelFilePath);
        exit(EXIT_FAILURE);
    }

    enum {
        NONE,
        SECTION_TILES,
        SECTION_ENTITIES
    } currSect = NONE;

    char line[256];
    Uint32 tileRow;

    while (fgets(line, sizeof(line), fin)) {
        line[strcspn(line, "\r\n")] = 0;  // Remove newline characters

        // Skip empty lines
        if (strlen(line) == 0) continue;
        // Skip comments
        if (line[0] == '#') continue;

        // Section headers
        if (strcmp(line, "[TILES]") == 0) {
            currSect = SECTION_TILES;
            tileRow = 0;  // Init the counter
            continue;
        } else if (strcmp(line, "[ENTITIES]") == 0) {
            currSect = SECTION_ENTITIES;
            continue;
        }

        switch (currSect) {
            case SECTION_TILES: {
                // Skip comments
                if (line[0] == '#') continue;

                if (tileRow >= ARENA_HEIGHT) {
                    fprintf(stderr, "Warning: More tile rows in level file than expected (%d). Ignoring extra rows.\n", ARENA_HEIGHT);
                    continue;
                }

                // Parse the tiles row
                Uint32 tileCol = 0;
                char *token = strtok(line, " ");
                while (token && tileCol < ARENA_WIDTH) {
                    TileType currTileType = (TileType)atoi(token);
                    if (currTileType < 0 || currTileType >= TILE_COUNT) {
                        fprintf(stderr, "Invalid tile type %d at row %d, column %d. Defaulting to TILE_EMPTY.\n", currTileType, tileRow, tileCol);
                        currTileType = TILE_EMPTY;
                    }
                    zEngine->map->tiles[tileRow][tileCol] = zEngine->map->tileDefs[currTileType];
                    zEngine->map->tiles[tileRow][tileCol].idx = tileRow * ARENA_WIDTH + tileCol;
                    token = strtok(NULL, " ");
                    tileCol++;
                }

                tileRow++;
                break;
            }
            case SECTION_ENTITIES: {
                // Skip comments
                if (line[0] == '#') continue;

                // Format: <X> <Y>=<ENTITY_TYPE>
                char *token = strtok(line, " ");
                if (!token) break;
                int x = atoi(token);

                token = strtok(NULL, "=");
                if (!token) break;
                int y = atoi(token);

                token = strtok(NULL, "\n");
                if (!token) break;
                EntityType entityType = (EntityType)atoi(token);
                #ifdef DEBUG
                    printf("Spawning entity type %d @ (%d, %d)\n", entityType, x, y);
                #endif

                switch (entityType) {
                    case ENTITY_PLAYER: {
                        instantiateTank(
                            zEngine, getTankPrefab(zEngine->prefabs, "player"),
                            (Vec2){.x = x * TILE_SIZE, .y = y * TILE_SIZE}
                        );
                        break;
                    }
                    case ENTITY_TANK_BASIC: {
                        instantiateTank(zEngine, getTankPrefab(zEngine->prefabs, "tankBasic"),
                        (Vec2){.x = x * TILE_SIZE, .y = y * TILE_SIZE});
                        break;
                    }
                }
                break;
            }
            case NONE: {
                break;
            }
            default: {
                // It shouldn't reach here
                fprintf(stderr, "Unknown section in level file: %s\n", line);
                break;
            }
        }
    }
    fclose(fin);
}

/**
 * =====================================================================================================================
 */

void clearLevel(ZENg zEngine) {
    if (!zEngine || !zEngine->map) {
        fprintf(stderr, "Invalid engine or map, cannot clear level\n");
        return;
    }

    if (zEngine->map->tiles) {
        for (Uint32 i = 0; i < ARENA_HEIGHT; i++) {
            free(zEngine->map->tiles[i]);
        }
        free(zEngine->map->tiles);
    }

    // Free the map structure
    free(zEngine->map);
    zEngine->map = NULL;
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
        {SYS_LIFETIME, &lifetimeSystem, 0},
        {SYS_VELOCITY, &velocitySystem, 0},
        {SYS_WORLD_COLLISIONS, &worldCollisionSystem, 0},
        {SYS_ENTITY_COLLISIONS, &entityCollisionSystem, 0},
        {SYS_POSITION, &positionSystem, 0},
        {SYS_HEALTH, &healthSystem, 1},
        {SYS_TRANSFORM, &transformSystem, 0},
        {SYS_RENDER, &renderSystem, 0},
        {SYS_BUTTONS, &buttonSystem, 1},
        {SYS_WEAPONS, &weaponSystem, 0}
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
                    "SYS_WEAPONS",
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
    if (!zEngine) {
        printf("Failed to allocate memory for the game engine\n");
        exit(EXIT_FAILURE);
    }
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // And open the audio device
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Couldn't initialize SDL_Mixer: %s\n", Mix_GetError());
        exit(EXIT_FAILURE);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    // Initalize the display and input managers by reading settings file if existent
    loadSettings(zEngine, "settings.ini");
    // Set logical screen size
    if (SDL_RenderSetLogicalSize(zEngine->display->renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT) < 0) {
        printf("Could not set logical size: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // After setting the display resolution define the tile size
    // The tiles are guaranteed to be square integers
    TILE_SIZE = LOGICAL_HEIGHT / ARENA_HEIGHT;

    // Initialize ECS
    initECS(&zEngine->ecs);

    // Initialize the resource manager and preload resources
    initResourceManager(&zEngine->resources);
    preloadResources(zEngine->resources, zEngine->display->renderer);

    // Initialize the prefabs manager
    initPrefabsManager(&zEngine->prefabs);
    loadPrefabs(zEngine->prefabs, "data");

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
    pushState(zEngine, mainMenuState);

    return zEngine;
}

/**
 * =====================================================================================================================
 */

void velocitySystem(ZENg zEngine, double_t deltaTime) {
    if (zEngine->ecs->depGraph->nodes[SYS_VELOCITY]->isDirty == 0) {
        if (zEngine->ecs->components[PROJECTILE_COMPONENT].denseSize > 0) {
            // If there are projectiles, let them behave
            zEngine->ecs->depGraph->nodes[SYS_VELOCITY]->isDirty = 1;
        } else {
            #ifdef DEBUG
                printf("[VELOCITY SYSTEM] Velocity system is not dirty\n");
            #endif
            return;
        }
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
            if (velComp->predictedPos.x + entityRect->w >= LOGICAL_WIDTH) {
                velComp->predictedPos.x = LOGICAL_WIDTH - entityRect->w;  // prevent going out of bounds
            }
            if (velComp->predictedPos.y + entityRect->h >= LOGICAL_HEIGHT) {
                velComp->predictedPos.y = LOGICAL_HEIGHT - entityRect->h;  // prevent going out of bounds
            }
        }
        // Hitboxes are updated in the collision system
    }
    propagateSystemDirtiness(zEngine->ecs->depGraph->nodes[SYS_VELOCITY]);
    zEngine->ecs->depGraph->nodes[SYS_VELOCITY]->isDirty = 0;
}

/**
 * =====================================================================================================================
 */

void positionSystem(ZENg zEngine, double_t deltaTime) {
    if (zEngine->ecs->depGraph->nodes[SYS_POSITION]->isDirty == 0) {
        #ifdef DEBUG
            printf("[POSITION SYSTEM] Position system is not dirty\n");
        #endif
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
        VelocityComponent *velComp = zEngine->ecs->components[VELOCITY_COMPONENT].dense[velDenseIdx];

        // Entities with velocity are guaranteed to have also direction
        Uint64 dirDenseIdx = zEngine->ecs->components[DIRECTION_COMPONENT].sparse[page][idx];
        DirectionComponent *dirComp = (DirectionComponent *)(zEngine->ecs->components[DIRECTION_COMPONENT].dense[dirDenseIdx]);

        Uint8 movingX = fabs(dirComp->x) > EPSILON;
        Uint8 movingY = fabs(dirComp->y) > EPSILON;
        
        Int32 currTileIdx = worldToTile(zEngine->map, *posComp);
        Uint32 currTileX = currTileIdx % ARENA_WIDTH;
        Uint32 currTileY = currTileIdx / ARENA_WIDTH;
        
        if (movingX && (velComp->prevAxis != AXIS_X)) {
            // Axis change -> snap the position to a tile
            posComp->y = round((posComp->y) / TILE_SIZE) * TILE_SIZE;
            velComp->prevAxis = AXIS_X;
        } else if (movingY && (velComp->prevAxis != AXIS_Y)) {
            // Axis change -> snap the position to a tile
            posComp->x = round((posComp->x) / TILE_SIZE) * TILE_SIZE;
            velComp->prevAxis = AXIS_Y;
        }
    }

    propagateSystemDirtiness(zEngine->ecs->depGraph->nodes[SYS_POSITION]);
    zEngine->ecs->depGraph->nodes[SYS_POSITION]->isDirty = 0;
}

/**
 * =====================================================================================================================
 */

void lifetimeSystem(ZENg zEngine, double_t deltaTime) {
    if (zEngine->ecs->depGraph->nodes[SYS_LIFETIME]->isDirty == 0) {
        #ifdef DEBUG
            printf("[LIFETIME SYSTEM] Lifetime system is not dirty. Something wrong happened\n");
        #endif
        return;
    }
    
    #ifdef DEBUG
        printf("[LIFETIME SYSTEM] Running lifetime system for %lu entities\n", zEngine->ecs->components[LIFETIME_COMPONENT].denseSize);
    #endif

    for (Uint64 i = 0; i < zEngine->ecs->components[LIFETIME_COMPONENT].denseSize; i++) {
        LifetimeComponent *lftComp = (LifetimeComponent *)(zEngine->ecs->components[LIFETIME_COMPONENT].dense[i]);
        lftComp->timeAlive += deltaTime;
        if (lftComp->timeAlive >= lftComp->lifeTime) {
            Entity dirtyOwner = zEngine->ecs->components[LIFETIME_COMPONENT].denseToEntity[i];
            deleteEntity(zEngine->ecs, dirtyOwner);
        }
    }
    propagateSystemDirtiness(zEngine->ecs->depGraph->nodes[SYS_LIFETIME]);
    // There is no point in unmarking the lifetime system clean
}

/**
 * =====================================================================================================================
 */

void handleEntitiesCollision(ZENg zEngine, CollisionComponent *AColComp, CollisionComponent *BColComp, Entity AOwner, Entity BOwner) {
    if (!AColComp->isSolid && !BColComp->isSolid) {
        // two bullets collide - skip collision check
        return;
    }

    if (AColComp->role == COL_ACTOR && BColComp->role == COL_ACTOR) {
        printf("QUIT THE SMOOCHING!1!\n");
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
        #ifdef DEBUG
            printf("[ENTITY COLLISION SYSTEM] Entity collision system is not dirty\n");
        #endif
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

        // if a bullet hits the arena edge - remove it
        if (AColComp->role == COL_BULLET && (AColComp->hitbox->x <= 0
            || AColComp->hitbox->y <= 0
            || AColComp->hitbox->x + AColComp->hitbox->w >= LOGICAL_WIDTH
            || AColComp->hitbox->y + AColComp->hitbox->h >= LOGICAL_HEIGHT)
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

    propagateSystemDirtiness(zEngine->ecs->depGraph->nodes[SYS_ENTITY_COLLISIONS]);
    zEngine->ecs->depGraph->nodes[SYS_ENTITY_COLLISIONS]->isDirty = 0;
}

/**
 * =====================================================================================================================
 */

Uint8 checkWorldCollision(ZENg zEngine, SDL_Rect *hitbox, Tile *collidedTile) {
    // For entities bigger than one tile. consider the base the top-left tile
    Uint32 tileIdx = worldToTile(zEngine->map, (PositionComponent){.x = hitbox->x, .y = hitbox->y});
    Uint32 tileX = tileIdx % ARENA_WIDTH;
    Uint32 tileY = tileIdx / ARENA_WIDTH;

    int entityWidth = hitbox->w / TILE_SIZE;
    int entityHeight = hitbox->h / TILE_SIZE;
    Int32 dxMax = entityWidth + 1;
    Int32 dyMax = entityHeight + 1;

    for (Int32 dy = -1; dy <= dyMax; dy++) {
        Uint32 neighY = (Int32)tileY + dy;
        for (Int32 dx = -1; dx <= dxMax; dx++) {
            Uint32 neighX = (Int32)tileX + dx;

            if ((neighX >= 0 && neighX < ARENA_WIDTH) && (neighY >= 0 && neighY < ARENA_HEIGHT)) {
                Tile *neighTile = &zEngine->map->tiles[neighY][neighX];
                SDL_Rect neighTileRect = {
                    .x = neighX * TILE_SIZE,
                    .y = neighY * TILE_SIZE,
                    .w = TILE_SIZE,
                    .h = TILE_SIZE
                };

                if (neighTile->isSolid && SDL_HasIntersection(hitbox, &neighTileRect)) {
                    *collidedTile = *neighTile;
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
        #ifdef DEBUG
            printf("[WORLD COLLISION SYSTEM] World collision system is not dirty\n");
        #endif
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

        Tile collidedTile;  // passed to the function checking world collisions
        
        // Resolve X-Axis collisions
        double_t moveX = eVelComp->currVelocity.x * deltaTime;
        eColComp->hitbox->x = eVelComp->predictedPos.x;

        if (checkWorldCollision(zEngine, eColComp->hitbox, &collidedTile)) {
            if (eColComp->role == COL_BULLET) {
                Uint64 projDenseIdx = zEngine->ecs->components[PROJECTILE_COMPONENT].sparse[page][index];
                ProjectileComponent *projComp =
                (ProjectileComponent *)(zEngine->ecs->components[PROJECTILE_COMPONENT].dense[projDenseIdx]);
                if (
                    (collidedTile.type == TILE_BRICKS && projComp->dmg >= 15)
                    || (collidedTile.type == TILE_ROCK && projComp->dmg >= 30)
                ) {
                    Uint32 tileY = collidedTile.idx / ARENA_WIDTH;
                    Uint32 tileX = collidedTile.idx % ARENA_WIDTH;
                    zEngine->map->tiles[tileY][tileX] = zEngine->map->tileDefs[TILE_EMPTY];
                }
                deleteEntity(zEngine->ecs, e);
                continue;
            }

            Vec2 tileCoords = tileToWorld(zEngine->map, collidedTile.idx);
            if (moveX > 0) {
                eColComp->hitbox->x = tileCoords.x - eColComp->hitbox->w;
            } else if (moveX < 0) {
                eColComp->hitbox->x = tileCoords.x + TILE_SIZE;
            }
            eVelComp->currVelocity.x = 0.0;
        }
        posComp->x = eColComp->hitbox->x;

        // Resolve Y-Axis collisions
        double_t moveY = eVelComp->currVelocity.y * deltaTime;
        eColComp->hitbox->y = eVelComp->predictedPos.y;

        if (checkWorldCollision(zEngine, eColComp->hitbox, &collidedTile)) {
            if (eColComp->role == COL_BULLET) {
                Uint64 projDenseIdx = zEngine->ecs->components[PROJECTILE_COMPONENT].sparse[page][index];
                ProjectileComponent *projComp =
                (ProjectileComponent *)(zEngine->ecs->components[PROJECTILE_COMPONENT].dense[projDenseIdx]);
                if (
                    (collidedTile.type == TILE_BRICKS && projComp->dmg >= 15)
                    || (collidedTile.type == TILE_ROCK && projComp->dmg >= 30)
                ) {
                    Uint32 tileY = collidedTile.idx / ARENA_WIDTH;
                    Uint32 tileX = collidedTile.idx % ARENA_WIDTH;
                    zEngine->map->tiles[tileY][tileX] = zEngine->map->tileDefs[TILE_EMPTY];
                }
                deleteEntity(zEngine->ecs, e);
                continue;
            }

            Vec2 tileCoords = tileToWorld(zEngine->map, collidedTile.idx);
            if (moveY > 0) {
                eColComp->hitbox->y = tileCoords.y - eColComp->hitbox->h;
            } else if (moveY < 0) {
                eColComp->hitbox->y = tileCoords.y + TILE_SIZE;
            }
            eVelComp->currVelocity.y = 0.0;
        }
        posComp->y = eColComp->hitbox->y;
    }

    propagateSystemDirtiness(zEngine->ecs->depGraph->nodes[SYS_WORLD_COLLISIONS]);
    zEngine->ecs->depGraph->nodes[SYS_WORLD_COLLISIONS]->isDirty = 0;
}


/**
 * =====================================================================================================================
 */

void healthSystem(ZENg zEngine, double_t deltaTime) {
    #ifdef DEBUG
        printf("[HEALTH SYSTEM] There are %lu dirty health components\n", zEngine->ecs->components[HEALTH_COMPONENT].dirtyCount);
    #endif

    if (zEngine->ecs->components[HEALTH_COMPONENT].dirtyCount != 0) {
        propagateSystemDirtiness(zEngine->ecs->depGraph->nodes[SYS_HEALTH]);
    }
    
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

void weaponSystem(ZENg zEngine, double_t deltaTime) {
    if (zEngine->ecs->depGraph->nodes[SYS_WEAPONS]->isDirty == 0) {
        #ifdef DEBUG
            printf("[WEAPON SYSTEM] Weapon system is not dirty\n");
        #endif
        return;
    }

    #ifdef DEBUG
        printf(
            "[WEAPON SYSTEM] Running weapon system for %lu entities\n",
            zEngine->ecs->components[WEAPON_COMPONENT].denseSize
        );
    #endif

    for (Uint64 i = 0; i < zEngine->ecs->components[WEAPON_COMPONENT].denseSize; i++) {
        WeaponComponent *currWeapon = (WeaponComponent *)(zEngine->ecs->components[WEAPON_COMPONENT].dense[i]);

        // Prevent overflow
        if (currWeapon->timeSinceUse > (1 / currWeapon->fireRate + EPSILON)) {
            continue;
        }
        #ifdef DEBUG
            printf("Updating timeSinceUse... %.4f -> ", currWeapon->timeSinceUse);
        #endif
        currWeapon->timeSinceUse += deltaTime;

        #ifdef DEBUG
            printf("%.4f\n", currWeapon->timeSinceUse);
        #endif
    }
    propagateSystemDirtiness(zEngine->ecs->depGraph->nodes[SYS_TRANSFORM]);
    // zEngine->ecs->depGraph->nodes[SYS_TRANSFORM]->isDirty = 0;
}

/**
 * =====================================================================================================================
 */

void transformSystem(ZENg zEngine, double_t deltaTime) {
    if (zEngine->ecs->depGraph->nodes[SYS_TRANSFORM]->isDirty == 0) {
        #ifdef DEBUG
            printf("[TRANSFORM SYSTEM] Transform system is not dirty\n");
        #endif
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

    propagateSystemDirtiness(zEngine->ecs->depGraph->nodes[SYS_TRANSFORM]);
    zEngine->ecs->depGraph->nodes[SYS_TRANSFORM]->isDirty = 0;
}

/**
 * =====================================================================================================================
 */
void renderSystem(ZENg zEngine, double_t deltaTime) {
    if (zEngine->ecs->depGraph->nodes[SYS_RENDER]->isDirty == 0) {
        #ifdef DEBUG
            printf("[RENDER SYSTEM] Render system is not dirty. Not good.\n");
        #endif
        return;  // Rendering should always be done every frame
    }

    #ifdef DEBUG
        Uint64 renderCount = zEngine->ecs->components[RENDER_COMPONENT].denseSize + zEngine->ecs->components[BUTTON_COMPONENT].denseSize;
        printf("[RENDER SYSTEM] Running render system for %lu entities\n", renderCount);
    #endif

    GameState *currState = getCurrState(zEngine->stateMng);
    if (currState->type == STATE_PLAYING || currState->type == STATE_PAUSED) {
        renderArena(zEngine);
    }

    #ifdef DEBUG
        if (currState->type == STATE_PLAYING || currState->type == STATE_PAUSED) renderDebugGrid(zEngine);
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
        if (currState->type == STATE_PLAYING || currState->type == STATE_PAUSED) renderDebugCollision(zEngine);
    #endif

    if (currState->type == STATE_PAUSED) {
        // Make the game appear as in background
        int screenW = zEngine->display->currentMode.w;
        int screenH = zEngine->display->currentMode.h;

        SDL_SetRenderDrawBlendMode(zEngine->display->renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(zEngine->display->renderer, 0, 0, 0, 128); // Half opaque
        SDL_RenderFillRect(zEngine->display->renderer, NULL);
        SDL_SetRenderDrawBlendMode(zEngine->display->renderer, SDL_BLENDMODE_NONE); // Reset if needed
    }

    for (Uint64 i = 0; i < zEngine->ecs->components[TEXT_COMPONENT].denseSize; i++) {
        TextComponent *text = (TextComponent *)(zEngine->ecs->components[TEXT_COMPONENT].dense[i]);

        if (text && text->texture && text->destRect) {
            SDL_RenderCopy(zEngine->display->renderer, text->texture, NULL, text->destRect);
        } else {
            Entity owner = (zEngine->ecs->components[TEXT_COMPONENT].denseToEntity[i]);
            printf("Warning: Entity %ld has an invalid text component\n", owner);
        }
    }

    for (Uint64 i = 0; i < zEngine->ecs->components[BUTTON_COMPONENT].denseSize; i++) {
        ButtonComponent *button = (ButtonComponent *)(zEngine->ecs->components[BUTTON_COMPONENT].dense[i]);

        if (button && button->texture && button->destRect) {
            SDL_RenderCopy(zEngine->display->renderer, button->texture, NULL, button->destRect);
        } else {
            Entity owner = (zEngine->ecs->components[BUTTON_COMPONENT].denseToEntity[i]);
            printf("Warning: Entity %ld has an invalid button component\n", owner);
        }
    }

    // Should propagate the dirtiness here, but the render system is pretty much always the last
    // Rendering should always be done every frame
    // zEngine->ecs->depGraph->nodes[SYS_RENDER]->isDirty = 0;
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

void renderArena(ZENg zEngine) {
    SDL_SetRenderDrawColor(zEngine->display->renderer, 20, 20, 20, 200);  // background color - grey
    SDL_RenderClear(zEngine->display->renderer);

    if (!zEngine->map || !zEngine->map->tiles) {
        printf("The arena was no initialized correctly.\n");
        exit(EXIT_FAILURE);
    }

    for (Uint64 y = 0; y < ARENA_HEIGHT; y++) {
        for (Uint64 x = 0; x < ARENA_WIDTH; x++) {
            SDL_Rect tileRect = {
                .x =  x * TILE_SIZE,
                .y = y * TILE_SIZE,
                .w = TILE_SIZE,
                .h = TILE_SIZE
            };
            if (zEngine->map->tiles[y][x].texture) {
                SDL_RenderCopy(
                    zEngine->display->renderer,
                    zEngine->map->tiles[y][x].texture,
                    NULL,
                    &tileRect
                );
            }
        }
    }
}

/**
 * =====================================================================================================================
 */

#ifdef DEBUG
void renderDebugGrid(ZENg zEngine) {
    SDL_SetRenderDrawColor(zEngine->display->renderer, 100, 100, 100, 50);
    
    // Draw vertical grid lines
    for (int x = 0; x <= ARENA_WIDTH; x++) {
        SDL_RenderDrawLine(
            zEngine->display->renderer,
            x * TILE_SIZE, 0,
            x * TILE_SIZE, LOGICAL_HEIGHT
        );
    }
    
    // Draw horizontal grid lines
    for (int y = 0; y <= ARENA_HEIGHT; y++) {
        SDL_RenderDrawLine(
            zEngine->display->renderer,
            0, y * TILE_SIZE,
            LOGICAL_WIDTH, y * TILE_SIZE
        );
    }
}

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
            if (zEngine->map->tiles[y][x].isSolid) {
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

void runSystems(ZENg zEngine, double_t deltaTime) {
    for (Uint64 i = 0; i < zEngine->ecs->depGraph->nodeCount; i++) {
        SystemNode *curr = zEngine->ecs->depGraph->sortedNodes[i];
        if (curr->isActive) {
            curr->update(zEngine, deltaTime);
        }
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
