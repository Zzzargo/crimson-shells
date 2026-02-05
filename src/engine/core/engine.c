#include "engine.h"
#include "engine/arena.h"
#include "engine/collisionManager.h"
#include "engine/core/ecs.h"
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>

void loadSettings(ZENg zEngine, const char *filePath) {
    // look for the file
    FILE *fin = fopen(filePath, "r");

    zEngine->inputMng = calloc(1, sizeof(struct inputmng));
    if (!zEngine->inputMng) THROW_ERROR_AND_EXIT("Failed allocating memory for the input manager");

    zEngine->display = calloc(1, sizeof(struct displaymng));
    if (!zEngine->display) THROW_ERROR_AND_EXIT("Failed allocating memory for the display manager");

    if (!fin) {
        // file doesn't exist
        fprintf(stderr, "No config file found in %s. Using defaults\n", filePath);
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
        if (!zEngine->display->window) THROW_ERROR_AND_DO(
            "Window creation failed: ", fprintf(stderr, "%s\n", SDL_GetError()); exit(EXIT_FAILURE);
        );
        zEngine->display->renderer = SDL_CreateRenderer(
            zEngine->display->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
        );
        if (!zEngine->display->renderer) THROW_ERROR_AND_DO(
            "Renderer creation failed: ", fprintf(stderr, "%s\n", SDL_GetError());
            SDL_DestroyWindow(zEngine->display->window); exit(EXIT_FAILURE);
        );
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

    if (!zEngine->display->window) THROW_ERROR_AND_DO(
        "Window creation failed: ", fprintf(stderr, "%s\n", SDL_GetError()); exit(EXIT_FAILURE);
    );

    // Create renderer
    Int32 rendererFlags = SDL_RENDERER_ACCELERATED;
    if (vsync) rendererFlags |= SDL_RENDERER_PRESENTVSYNC;

    zEngine->display->renderer = SDL_CreateRenderer(
        zEngine->display->window, -1, rendererFlags
    );
    if (!zEngine->display->renderer) THROW_ERROR_AND_DO(
        "Renderer creation failed: ", fprintf(stderr, "%s\n", SDL_GetError());
        SDL_DestroyWindow(zEngine->display->window); exit(EXIT_FAILURE);
    );

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
    if (!zEngine || !levelFilePath) THROW_ERROR_AND_RETURN_VOID("zEngine or levelFilePath is NULL in initLevel");

    zEngine->map = calloc(1, sizeof(struct arena));
    if (!zEngine->map) THROW_ERROR_AND_EXIT("Failed to allocate memory for Arena");

    zEngine->map->tiles = calloc(ARENA_HEIGHT, sizeof(Tile*));
    if (!zEngine->map->tiles) THROW_ERROR_AND_EXIT("Failed to allocate memory for arena tiles rows");
    for (Uint32 i = 0; i < ARENA_HEIGHT; i++) {
        zEngine->map->tiles[i] = calloc(ARENA_WIDTH, sizeof(Tile));
        if (!zEngine->map->tiles[i]) THROW_ERROR_AND_EXIT("Failed to allocate memory for arena tiles columns");
    }

    // Don't forget about the spatial grid
    zEngine->collisionMng = initCollisionManager();

    FILE *f = fopen(levelFilePath, "rb");
    if (!f) THROW_ERROR_AND_DO("Failed to open level file: ", fprintf(stderr, "'%s'\n", levelFilePath); return;);

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = malloc(size + 1);
    fread(data, 1, size, f);
    data[size] = '\0';
    fclose(f);

    cJSON *root = cJSON_Parse(data);
    free(data);

    if (!root) THROW_ERROR_AND_RETURN_VOID("Failed to parse level JSON");

    // The arena files have an object with the arrays "tiles" and "entities"
    cJSON *tilesArray = cJSON_GetObjectItem(root, "tiles");
    if (!cJSON_IsArray(tilesArray)) THROW_ERROR_AND_RETURN_VOID("Invalid or missing 'tiles' array in level file");

    char *tileTypeToStr[] = {
        [TILE_EMPTY] = "TILE_EMPTY",
        [TILE_GRASS] = "TILE_GRASS",
        [TILE_WATER] = "TILE_WATER",
        [TILE_ROCK] = "TILE_ROCK",
        [TILE_BRICKS] = "TILE_BRICKS",
        [TILE_WOOD] = "TILE_WOOD",
        [TILE_SPAWN] = "TILE_SPAWN"
    };

    Uint32 row = 0;
    cJSON *tileRow = NULL;

    cJSON_ArrayForEach(tileRow, tilesArray) {
        if (!cJSON_IsArray(tileRow)) THROW_ERROR_AND_DO(
            "Invalid tile row at index ", fprintf(stderr, "%d\n", row); continue;
        );
        if (row >= ARENA_HEIGHT) THROW_ERROR_AND_DO(
            "Warning: More tile rows in level file than expected",
            fprintf(stderr, " (%d). Ignoring extra rows\n", ARENA_HEIGHT); break;
        );
        Uint32 col = 0;
        cJSON *tile = NULL;
        cJSON_ArrayForEach(tile, tileRow) {
            if (!cJSON_IsNumber(tile)) THROW_ERROR_AND_DO(
                "Invalid tile type at row ", fprintf(stderr, "%d, column %d\n", row, col); continue;
            );
            if (col >= ARENA_WIDTH) THROW_ERROR_AND_DO(
                "Warning: More tile columns in row ",
                fprintf(stderr, "%d than expected (%d). Ignoring extra columns\n", row, ARENA_WIDTH); break;
            );
            TileType currTileType = (TileType)tile->valueint;
            if (currTileType < 0 || currTileType >= TILE_COUNT) THROW_ERROR_AND_DO(
                "Invalid tile type value at row ",
                fprintf(stderr, "%d, column %d: %d. Defaulting to TILE_EMPTY\n", row, col, currTileType);
                currTileType = TILE_EMPTY;
            );
            zEngine->map->tiles[row][col] = getTilePrefab(zEngine->prefabs, tileTypeToStr[currTileType]);
            zEngine->map->tiles[row][col].idx = row * ARENA_WIDTH + col;
            col++;
        }
        if (col < ARENA_WIDTH) THROW_ERROR_AND_DO(
            "Warning: Fewer tile columns (",
            fprintf(stderr, "%d) in row %d than expected (%d). The rest are TILE_EMPTY.\n",
            col, row, ARENA_WIDTH);
        );
        row++;
    }
    if (row < ARENA_HEIGHT) THROW_ERROR_AND_DO(
        "Warning: Fewer tile rows (",
        fprintf(stderr, "%d) in level file than expected (%d). The rest are TILE_EMPTY.\n",
        row, ARENA_HEIGHT);
    );

    cJSON *entitiesArray = cJSON_GetObjectItem(root, "entities");
    if (cJSON_IsArray(entitiesArray)) {
        cJSON *entityJson;
        cJSON_ArrayForEach(entityJson, entitiesArray) {
            cJSON *entityTypeJson = cJSON_GetObjectItem(entityJson, "entityType");
            cJSON *xJson = cJSON_GetObjectItem(entityJson, "x");
            cJSON *yJson = cJSON_GetObjectItem(entityJson, "y");

            if (!cJSON_IsString(entityTypeJson) || !cJSON_IsNumber(xJson) || !cJSON_IsNumber(yJson)) THROW_ERROR_AND_DO(
                "Invalid entity in entities array. Skipping.\n", continue;
            );
            char entityTypeStr[64];
            strncpy(entityTypeStr, entityTypeJson->valuestring, sizeof(entityTypeStr));
            entityTypeStr[sizeof(entityTypeStr) - 1] = '\0';

            int x = xJson->valueint;
            int y = yJson->valueint;

            Entity tank = instantiateTank(
                zEngine, getTankPrefab(zEngine->prefabs, entityTypeStr), (Vec2){.x = x * TILE_SIZE, .y = y * TILE_SIZE}
            );
            if (!HAS_COMPONENT(zEngine->ecs, tank, COLLISION_COMPONENT)) THROW_ERROR_AND_CONTINUE(
                    "Failed to add collision component to spawned tank in initLevel()");
            CollisionComponent *tankColComp = NULL;
            GET_COMPONENT(zEngine->ecs, tank, COLLISION_COMPONENT, tankColComp, CollisionComponent);
            registerEntityToSG(zEngine->collisionMng, tank, tankColComp);

            #ifdef DEBUG
                printf("Instantiated tank of type %s at (%d, %d)\n", entityTypeStr, y, x);
            #endif
        }
    }

    cJSON_Delete(root);
}

/**
 * =====================================================================================================================
 */

void clearLevel(ZENg zEngine) {
    if (!zEngine || !zEngine->map) THROW_ERROR_AND_RETURN_VOID("zEngine or zEngine->map is NULL in clearLevel");

    if (zEngine->map->tiles) {
        for (Uint32 i = 0; i < ARENA_HEIGHT; i++) {
            free(zEngine->map->tiles[i]);
        }
        free(zEngine->map->tiles);
    }

    // Free the map structure
    free(zEngine->map);
    zEngine->map = NULL;

    // Free the collision manager
    if (zEngine->collisionMng) freeCollisionManager(zEngine->collisionMng);
    zEngine->collisionMng = NULL;
}

/**
 * =====================================================================================================================
 */

DependencyGraph* initDependencyGraph() {
    DependencyGraph *graph = calloc(1, sizeof(DependencyGraph));
    if (!graph) THROW_ERROR_AND_EXIT("Failed to allocate memory for dependency graph");

    graph->nodes = calloc(SYS_COUNT, sizeof(SystemNode*));
    if (!graph->nodes) THROW_ERROR_AND_EXIT("Failed to allocate memory for system nodes array");

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
        {SYS_UI, &uiSystem, 1},
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
        {SYS_WORLD_COLLISIONS, SYS_ENTITY_COLLISIONS},
        {SYS_WORLD_COLLISIONS, SYS_POSITION},
        {SYS_WORLD_COLLISIONS, SYS_HEALTH},
        {SYS_ENTITY_COLLISIONS, SYS_POSITION},
        {SYS_ENTITY_COLLISIONS, SYS_HEALTH},
        {SYS_POSITION, SYS_TRANSFORM},
        {SYS_TRANSFORM, SYS_RENDER},
        {SYS_UI, SYS_RENDER}
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
                    "SYS_UI"
                };
                printf("Added system dependency: %s -> %s\n", sysNames[dependency->type], sysNames[dependent->type]);
            #endif
        } else THROW_ERROR_AND_DO(
            "Invalid system relationship in dependencies array: ",
            fprintf(stderr, "(%d -> %d)\n", dependencies[i].dependency, dependencies[i].dependent);
        );
    }

    return graph;
}

/**
 * =====================================================================================================================
 */

ZENg initGame() {
    ZENg zEngine = calloc(1, sizeof(struct engine));
    if (!zEngine) THROW_ERROR_AND_EXIT("Failed to allocate memory for the game engine");
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) THROW_ERROR_AND_DO(
        "SDL_Init Error: ", fprintf(stderr, "%s\n", SDL_GetError()); exit(EXIT_FAILURE);
    );

    // Prepare the audio device
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) THROW_ERROR_AND_DO(
        "SDL_Mixer could not initialize: ",
        fprintf(stderr, "%s\n", Mix_GetError()); exit(EXIT_FAILURE);
    );

    // Init fonts
    if (TTF_Init() == -1) THROW_ERROR_AND_DO(
        "SDL_TTF could not initialize: ",
        fprintf(stderr, "%s\n", TTF_GetError()); exit(EXIT_FAILURE);
    );

    // Init SDL_Image
    if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) == 0) THROW_ERROR_AND_DO(
        "SDL_Image could not initialize: ",
        fprintf(stderr, "%s\n", IMG_GetError()); exit(EXIT_FAILURE);
    );

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    
    // Initalize the display and input managers by reading settings file if existent
    loadSettings(zEngine, "settings.ini");
    // Set logical screen size
    if (SDL_RenderSetLogicalSize(zEngine->display->renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT) < 0)
        THROW_ERROR_AND_DO("SDL_RenderSetLogicalSize failed: ", fprintf(stderr, "%s\n", SDL_GetError()););
                
    // After setting the display resolution define the tile size
    // The tiles are guaranteed to be square integers
    TILE_SIZE = LOGICAL_HEIGHT / ARENA_HEIGHT;

    // Initialize ECS
    initECS(&zEngine->ecs);

    // Initialize the resource manager and preload resources
    zEngine->resources = MapInit(257, MAP_RESOURCES);
    preloadResources(zEngine->resources, zEngine->display->renderer);

    // Initialize the prefabs manager
    zEngine->prefabs = MapInit(127, MAP_PREFABS);
    loadPrefabs(zEngine, "data/prefabs.json");

    zEngine->uiManager = initUIManager();

    // Start on the main menu
    initStateManager(&zEngine->stateMng);
    GameState *mainMenuState = calloc(1, sizeof(GameState));
    if (!mainMenuState) THROW_ERROR_AND_EXIT("Failed to allocate memory for main menu state");

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
            #ifdef DEBUGSYSTEMS
                printf("[VELOCITY SYSTEM] Velocity system is not dirty\n");
            #endif
            return;
        }
    }
    
    #ifdef DEBUGSYSTEMS
        printf(
            "[VELOCITY SYSTEM] Running velocity system for %lu entities\n",
            zEngine->ecs->components[VELOCITY_COMPONENT].denseSize
        );
    #endif

    Component velComps = zEngine->ecs->components[VELOCITY_COMPONENT];
    // for each entity with a VELOCITY_COMPONENT, update its position based on the current velocity
    for (Uint64 i = 0; i < velComps.denseSize; i++) {
        VelocityComponent *velComp = (VelocityComponent *)(velComps.dense[i]);
        Entity entitty = velComps.denseToEntity[i];
        
        if (!HAS_COMPONENT(zEngine->ecs, entitty, POSITION_COMPONENT))
            THROW_ERROR_AND_RETURN_VOID("Entity with VELOCITY_COMPONENT has no POSITION_COMPONENT");
        PositionComponent *posComp = NULL;
        GET_COMPONENT(zEngine->ecs, entitty, POSITION_COMPONENT, posComp, PositionComponent);

        // Update the predicted position based on the velocity
        velComp->predictedPos.x = posComp->x + velComp->currVelocity.x * deltaTime;
        velComp->predictedPos.y = posComp->y + velComp->currVelocity.y * deltaTime;

        // Clamp the position to the window bounds
        if (velComp->predictedPos.x < 0) velComp->predictedPos.x = 0;
        if (velComp->predictedPos.y < 0) velComp->predictedPos.y = 0;
        RenderComponent *rendComp = NULL;
        GET_COMPONENT(zEngine->ecs, entitty, RENDER_COMPONENT, rendComp, RenderComponent);
        SDL_Rect *entityRect = rendComp->destRect;
        if (entityRect) {
            if (velComp->predictedPos.x + entityRect->w >= LOGICAL_WIDTH) {
                velComp->predictedPos.x = LOGICAL_WIDTH - entityRect->w;
            }
            if (velComp->predictedPos.y + entityRect->h >= LOGICAL_HEIGHT) {
                velComp->predictedPos.y = LOGICAL_HEIGHT - entityRect->h;
            }
        }

        if (HAS_COMPONENT(zEngine->ecs, entitty, COLLISION_COMPONENT)) {
            CollisionComponent *colComp = NULL;
            GET_COMPONENT(zEngine->ecs, entitty, COLLISION_COMPONENT, colComp, CollisionComponent);
            colComp->hitbox->x = velComp->predictedPos.x;
            colComp->hitbox->y = velComp->predictedPos.y;
        }
    }
    propagateSystemDirtiness(zEngine->ecs->depGraph->nodes[SYS_VELOCITY]);
    zEngine->ecs->depGraph->nodes[SYS_VELOCITY]->isDirty = 0;
}

/**
 * =====================================================================================================================
 */

void positionSystem(ZENg zEngine, double_t deltaTime) {
    if (zEngine->ecs->depGraph->nodes[SYS_POSITION]->isDirty == 0) {
        #ifdef DEBUGSYSTEMS
            printf("[POSITION SYSTEM] Position system is not dirty\n");
        #endif
        return;
    }
    Component posComps = zEngine->ecs->components[POSITION_COMPONENT];
    #ifdef DEBUGSYSTEMS
        printf(
            "[POSITION SYSTEM] Running position system for %lu entities\n",
            posComps.denseSize
        );
    #endif

    for (Uint64 i = 0; i  < posComps.denseSize; i++) {
        PositionComponent *posComp = (PositionComponent *)(posComps.dense[i]);
        Entity owner = posComps.denseToEntity[i];

        // Common part - to update the real positions
        if (!HAS_COMPONENT(zEngine->ecs, owner, VELOCITY_COMPONENT)) continue;
        VelocityComponent *velComp = NULL;
        GET_COMPONENT(zEngine->ecs, owner, VELOCITY_COMPONENT, velComp, VelocityComponent);

        posComp->x = velComp->predictedPos.x;
        posComp->y = velComp->predictedPos.y;

        // Snapping part

        // Only actors can have health (at least for now) and only actors obey the snap rule
        if (!HAS_COMPONENT(zEngine->ecs, owner, HEALTH_COMPONENT)) continue;

        DirectionComponent *dirComp = NULL;
        GET_COMPONENT(zEngine->ecs, owner, DIRECTION_COMPONENT, dirComp, DirectionComponent);

        Uint8 movingX = fabs(dirComp->x) > EPSILON;
        Uint8 movingY = fabs(dirComp->y) > EPSILON;
        Uint32 currTileIdx = worldToTile(*posComp);
        Uint32 currTileX = currTileIdx % ARENA_WIDTH;
        Uint32 currTileY = currTileIdx / ARENA_WIDTH;
        
        if (movingX && (velComp->prevAxis != AXIS_X)) {
            // Axis change -> snap the position to a tile
            posComp->y = round((posComp->y) / TILE_SIZE) * TILE_SIZE;
            if (HAS_COMPONENT(zEngine->ecs, owner, COLLISION_COMPONENT)) {
                CollisionComponent *colComp = NULL;
                GET_COMPONENT(zEngine->ecs, owner, COLLISION_COMPONENT, colComp, CollisionComponent);
                colComp->hitbox->y = posComp->y;
            }
            velComp->prevAxis = AXIS_X;
        } else if (movingY && (velComp->prevAxis != AXIS_Y)) {
            // Axis change -> snap the position to a tile
            posComp->x = round((posComp->x) / TILE_SIZE) * TILE_SIZE;
            if (HAS_COMPONENT(zEngine->ecs, owner, COLLISION_COMPONENT)) {
                CollisionComponent *colComp = NULL;
                GET_COMPONENT(zEngine->ecs, owner, COLLISION_COMPONENT, colComp, CollisionComponent);
                colComp->hitbox->x = posComp->x;
            }
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
        #ifdef DEBUGSYSTEMS
            printf("[LIFETIME SYSTEM] Lifetime system is not dirty. Something wrong happened\n");
        #endif
        return;
    }
    Component *comps = zEngine->ecs->components;
    #ifdef DEBUGSYSTEMS
        printf(
            "[LIFETIME SYSTEM] Running lifetime system for %lu entities\n",
            comps[LIFETIME_COMPONENT].denseSize
        );
    #endif

    for (Uint64 i = 0; i < comps[LIFETIME_COMPONENT].denseSize; i++) {
        LifetimeComponent *lftComp = (LifetimeComponent *)(comps[LIFETIME_COMPONENT].dense[i]);
        lftComp->timeAlive += deltaTime;
        if (lftComp->timeAlive >= lftComp->lifeTime) {
            Entity dirtyOwner = comps[LIFETIME_COMPONENT].denseToEntity[i];
            deleteEntity(zEngine->ecs, dirtyOwner);
        }
    }
    propagateSystemDirtiness(zEngine->ecs->depGraph->nodes[SYS_LIFETIME]);
    // There is no point in unmarking the lifetime system clean
}

/**
 * =====================================================================================================================
 */

Uint8 checkAndHandleEntityCollisions(ZENg zEngine, Entity entity) {
    CollisionComponent *colComp = NULL;
    GET_COMPONENT(zEngine->ecs, entity, COLLISION_COMPONENT, colComp, CollisionComponent);
    SDL_Rect *hitbox = colComp->hitbox;

    Uint16 minX = colComp->coverageStart % ARENA_WIDTH;
    Uint16 minY = colComp->coverageStart / ARENA_WIDTH;
    Uint16 maxX = colComp->coverageEnd % ARENA_WIDTH;
    Uint16 maxY = colComp->coverageEnd / ARENA_WIDTH;

#ifdef DEBUGCOLLISIONS
    printf("[ENTITY COLLISION SYSTEM] Entity %lu spans over (y:%d-%d, x:%d-%d)\n", entity, minY, maxY, minX, maxX);
#endif

    Uint8 numCollided = 0;

    for (Int32 y = minY; y <= maxY; y++) {
        for (Int32 x = minX; x <= maxX; x++) {
            size_t neighIdx = COL_GRID_INDEX(zEngine->collisionMng, x, y);
            GridCell *neighCell = &zEngine->collisionMng->spatialGrid[neighIdx];
            SDL_Rect neighCellRect = {
                .x = x * TILE_SIZE,
                .y = y * TILE_SIZE,
                .w = TILE_SIZE,
                .h = TILE_SIZE
            };

            if (SDL_HasIntersection(hitbox, &neighCellRect)) {
                // Search for colliding entities in the collided tile
                for (int i = 0; i < neighCell->entityCount; i++) {
                    Entity susColEntity = neighCell->entities[i];
                    if (susColEntity == entity) continue;

                    if (!HAS_COMPONENT(zEngine->ecs, susColEntity, COLLISION_COMPONENT)) continue;
                    CollisionComponent *susEColComp = NULL;
                    GET_COMPONENT(zEngine->ecs, susColEntity, COLLISION_COMPONENT, susEColComp, CollisionComponent);
                    if (SDL_HasIntersection(hitbox, susEColComp->hitbox)) {
                        // Call the appropriate handler function
#ifdef DEBUGCOLLISIONS
                        printf("[ENTITY COLLISION SYSTEM] Attempting to call a collision handler for entities");
                        printf(" %lu(role %d) vs %lu(role %d)...\n", entity, colComp->role, susColEntity, susEColComp->role);
#endif
                        if (zEngine->collisionMng->eVsEHandlers[colComp->role][susEColComp->role]) {
                            normalizeRoles(&entity, &susColEntity, &colComp, &susEColComp);
                            zEngine->collisionMng->eVsEHandlers[colComp->role][susEColComp->role](
                                zEngine, entity, susColEntity
                            );
                        }
                        numCollided++;

                        // Prevent further iterations if the entity was deleted as an outcome of the collision handling
                        if (!HAS_COMPONENT(zEngine->ecs, entity, ACTIVE_TAG_COMPONENT)) return numCollided;
                    }
                }
            }
        }
    }
    return numCollided;
}

/**
 * =====================================================================================================================
 */

void entityCollisionSystem(ZENg zEngine, double_t deltaTime) {
    if (zEngine->ecs->depGraph->nodes[SYS_ENTITY_COLLISIONS]->isDirty == 0) {
        #ifdef DEBUGSYSTEMS
            printf("[ENTITY COLLISION SYSTEM] Entity collision system is not dirty\n");
        #endif
        return;
    }
    Component *colComps = &zEngine->ecs->components[COLLISION_COMPONENT];

    #ifdef DEBUGSYSTEMS
        printf(
            "[ENTITY COLLISION SYSTEM] Running entity collision system for %lu entities\n",
            colComps->denseSize
        );
    #endif

    for (Uint64 i = 0; i < colComps->denseSize; i++) {
        CollisionComponent *colComp = (CollisionComponent *)(colComps->dense[i]);
        Entity owner = colComps->denseToEntity[i];

        // If a bullet hits the arena edge - remove it
        if (colComp->role == COL_BULLET && (colComp->hitbox->x <= 0
            || colComp->hitbox->y <= 0
            || colComp->hitbox->x + colComp->hitbox->w >= LOGICAL_WIDTH
            || colComp->hitbox->y + colComp->hitbox->h >= LOGICAL_HEIGHT)
        ) {
            deleteEntity(zEngine->ecs, owner);
            continue;  // skip to the next entity
        }

        // Check and handle collisions with other entities in the vicinity (in the spatial grid)
        Uint8 numCollided = checkAndHandleEntityCollisions(zEngine, owner);
    }

    propagateSystemDirtiness(zEngine->ecs->depGraph->nodes[SYS_ENTITY_COLLISIONS]);
    zEngine->ecs->depGraph->nodes[SYS_ENTITY_COLLISIONS]->isDirty = 0;
}

/**
 * =====================================================================================================================
 */

Uint8 checkAndHandleWorldCollisions(ZENg zEngine, Entity entity) {
    CollisionComponent *colComp = NULL;
    GET_COMPONENT(zEngine->ecs, entity, COLLISION_COMPONENT, colComp, CollisionComponent);
    SDL_Rect *hitbox = colComp->hitbox;

    // For entities bigger than one tile. consider the base the top-left tile
    Uint32 tileIdx = worldToTile((PositionComponent){.x = hitbox->x, .y = hitbox->y});
    Uint32 tileX = tileIdx % ARENA_WIDTH;
    Uint32 tileY = tileIdx / ARENA_WIDTH;

    int entityWidth = hitbox->w / TILE_SIZE;
    int entityHeight = hitbox->h / TILE_SIZE;
    Int32 dxMax = entityWidth + 1;
    Int32 dyMax = entityHeight + 1;

    Uint8 numCollided = 0;

    for (Int32 dy = -1; dy <= dyMax; dy++) {
        Uint32 neighY = (Int32)tileY + dy;
        for (Int32 dx = -1; dx <= dxMax; dx++) {
            Uint32 neighX = (Int32)tileX + dx;

            if (neighX < 0 || neighX >= ARENA_WIDTH || neighY < 0 || neighY >= ARENA_HEIGHT) continue;
            Tile *neighTile = &zEngine->map->tiles[neighY][neighX];
            if (neighTile->type == TILE_EMPTY) continue;
            SDL_Rect neighTileRect = {
                .x = neighX * TILE_SIZE,
                .y = neighY * TILE_SIZE,
                .w = TILE_SIZE,
                .h = TILE_SIZE
            };

            if (SDL_HasIntersection(hitbox, &neighTileRect)) {
#ifdef DEBUGCOLLISIONS
                printf("[WORLD COLLISION SYSTEM] Attempting to call handler function for entity");
                printf(" %lu vs tile type %d\n", entity, neighTile->type);
#endif
                if (zEngine->collisionMng->eVsWHandlers[colComp->role])
                    zEngine->collisionMng->eVsWHandlers[colComp->role](zEngine, entity, neighTile);
                numCollided++;

                // Prevent further iterations if the entity was deleted as an outcome of collision handling
                if (!HAS_COMPONENT(zEngine->ecs, entity, ACTIVE_TAG_COMPONENT)) return numCollided;
            }
        }
    }
    return numCollided;
}

/**
 * =====================================================================================================================
 */

void worldCollisionSystem(ZENg zEngine, double_t deltaTime) {
    if (zEngine->ecs->depGraph->nodes[SYS_WORLD_COLLISIONS]->isDirty == 0) {
        #ifdef DEBUGSYSTEMS
            printf("[WORLD COLLISION SYSTEM] World collision system is not dirty\n");
        #endif
        return;
    }

    Component *colComps = &zEngine->ecs->components[COLLISION_COMPONENT];

    #ifdef DEBUGSYSTEMS
        printf(
            "[WORLD COLLISION SYSTEM] Running world collision system for %lu entities\n",
            colComps->denseSize
        );
    #endif

    for (Uint64 i = 0; i < colComps->denseSize; i++) {
        CollisionComponent *colComp = colComps->dense[i];
        Entity e = colComps->denseToEntity[i];

        if (!HAS_COMPONENT(zEngine->ecs, e, VELOCITY_COMPONENT)) continue;  // Skip entities without velocity component
        VelocityComponent *velComp = NULL;
        GET_COMPONENT(zEngine->ecs, e, VELOCITY_COMPONENT, velComp, VelocityComponent);

        Uint8 numCollided = checkAndHandleWorldCollisions(zEngine, e);

        // Between world collisions and entity collisions make sure the entities' spatial grid memberships are valid
        if (!HAS_COMPONENT(zEngine->ecs, e, ACTIVE_TAG_COMPONENT)) continue;
        updateGridMembership(zEngine->collisionMng, e, velComp, colComp);
    }

    propagateSystemDirtiness(zEngine->ecs->depGraph->nodes[SYS_WORLD_COLLISIONS]);
    zEngine->ecs->depGraph->nodes[SYS_WORLD_COLLISIONS]->isDirty = 0;
}


/**
 * =====================================================================================================================
 */

void healthSystem(ZENg zEngine, double_t deltaTime) {
    Component *comps = zEngine->ecs->components;
    #ifdef DEBUGSYSTEMS
        printf(
            "[HEALTH SYSTEM] There are %lu dirty health components\n",
            comps[HEALTH_COMPONENT].dirtyCount
        );
    #endif

    if (comps[HEALTH_COMPONENT].dirtyCount != 0) {
        propagateSystemDirtiness(zEngine->ecs->depGraph->nodes[SYS_HEALTH]);
    }
    
    while (comps[HEALTH_COMPONENT].dirtyCount > 0) {
        Entity ownerID = comps[HEALTH_COMPONENT].dirtyEntities[0];
        HealthComponent *helfComp = NULL;
        GET_COMPONENT(zEngine->ecs, ownerID, HEALTH_COMPONENT, helfComp, HealthComponent);

        if (helfComp->currentHealth <= 0) deleteEntity(zEngine->ecs, ownerID);
        unmarkComponentDirty(zEngine->ecs, HEALTH_COMPONENT);
    }
}

/**
 * =====================================================================================================================
 */

void weaponSystem(ZENg zEngine, double_t deltaTime) {
    if (zEngine->ecs->depGraph->nodes[SYS_WEAPONS]->isDirty == 0) {
        #ifdef DEBUGSYSTEMS
            printf("[WEAPON SYSTEM] Weapon system is not dirty\n");
        #endif
        return;
    }
    Component weapComps = zEngine->ecs->components[WEAPON_COMPONENT];
    #ifdef DEBUGSYSTEMS
        printf(
            "[WEAPON SYSTEM] Running weapon system for %lu entities\n",
            weapComps.denseSize
        );
    #endif

    for (Uint64 i = 0; i < weapComps.denseSize; i++) {
        WeaponComponent *currWeapon = (WeaponComponent *)(weapComps.dense[i]);

        // Prevent overflow
        if (currWeapon->timeSinceUse > (1 / currWeapon->fireRate + EPSILON)) {
            continue;
        }
        currWeapon->timeSinceUse += deltaTime;
    }
    propagateSystemDirtiness(zEngine->ecs->depGraph->nodes[SYS_TRANSFORM]);
    // zEngine->ecs->depGraph->nodes[SYS_TRANSFORM]->isDirty = 0;
}

/**
 * =====================================================================================================================
 */

void transformSystem(ZENg zEngine, double_t deltaTime) {
    if (zEngine->ecs->depGraph->nodes[SYS_TRANSFORM]->isDirty == 0) {
        #ifdef DEBUGSYSTEMS
            printf("[TRANSFORM SYSTEM] Transform system is not dirty\n");
        #endif
        return;
    }
    Component posComps = zEngine->ecs->components[POSITION_COMPONENT];
    #ifdef DEBUGSYSTEMS
        printf(
            "[TRANSFORM SYSTEM] Running transform system for %lu entities\n",
            posComps.denseSize
        );
    #endif

    // iterate through all entities with POSITION_COMPONENT and update their rendered textures
    for (Uint64 i = 0; i < posComps.denseSize; i++) {
        PositionComponent *posComp = (PositionComponent *)(posComps.dense[i]);
        Entity entitty = posComps.denseToEntity[i];

        if ((zEngine->ecs->componentsFlags[entitty] & (1 << RENDER_COMPONENT)) == 0) continue;

        RenderComponent *renderComp = NULL;
        GET_COMPONENT(zEngine->ecs, entitty, RENDER_COMPONENT, renderComp, RenderComponent);

        renderComp->destRect->x = (int)posComp->x;
        renderComp->destRect->y = (int)posComp->y;
        
        markComponentDirty(zEngine->ecs, entitty, RENDER_COMPONENT);
    }

    propagateSystemDirtiness(zEngine->ecs->depGraph->nodes[SYS_TRANSFORM]);
    zEngine->ecs->depGraph->nodes[SYS_TRANSFORM]->isDirty = 0;
}

/**
 * =====================================================================================================================
 */

#ifdef DEBUG
void UIdebugRenderNode(SDL_Renderer *rdr, UIManager uiManager, UINode *node) {
    if (!node || !node->isVisible || !node->rect) return;

    // Base debug color by type
    SDL_Color color;
    switch (node->type) {
        case UI_BUTTON:       color = COLOR_WITH_ALPHA(COLOR_RED, OPACITY_VERYLOW); break;
        case UI_CONTAINER:    color = COLOR_WITH_ALPHA(COLOR_GREEN, OPACITY_VERYLOW); break;
        case UI_LABEL:        color = COLOR_WITH_ALPHA(COLOR_BLUE, OPACITY_VERYLOW); break;
        case UI_OPTION_CYCLE: color = COLOR_WITH_ALPHA(COLOR_YELLOW, OPACITY_VERYLOW); break;
        default:              color = COLOR_WITH_ALPHA(COLOR_GRAY, OPACITY_VERYLOW); break;
    }

    // Fill with transparency
    SDL_SetRenderDrawBlendMode(rdr, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(rdr, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(rdr, node->rect);

    // Outline in solid for clarity
    SDL_SetRenderDrawColor(rdr, color.r, color.g, color.b, OPACITY_FULL);
    SDL_RenderDrawRect(rdr, node->rect);

    // Highlight focused node
    if (node == uiManager->focusedNode) {
        SDL_SetRenderDrawColor(rdr, 255, 255, 0, 255); // Yellow
        SDL_RenderDrawRect(rdr, node->rect);
    }

    // Recurse
    for (size_t i = 0; i < node->childrenCount; i++) {
        UIdebugRenderNode(rdr, uiManager, node->children[i]);
    }
}
#endif

/**
 * =====================================================================================================================
 */

void renderSystem(ZENg zEngine, double_t deltaTime) {
    if (zEngine->ecs->depGraph->nodes[SYS_RENDER]->isDirty == 0) {
        #ifdef DEBUGSYSTEMS
            printf("[RENDER SYSTEM] Render system is not dirty. Not good.\n");
        #endif
    }
    Component rdrComps = zEngine->ecs->components[RENDER_COMPONENT];
    #ifdef DEBUGSYSTEMS
        Uint64 renderCount = rdrComps.denseSize;
        printf("[RENDER SYSTEM] Running render system for %lu entities\n", renderCount);
    #endif

    GameState *currState = getCurrState(zEngine->stateMng);
    if (currState->type == STATE_PLAYING || currState->type == STATE_PAUSED) {
        renderArena(zEngine);
    }

    #ifdef DEBUG
        if (currState->type == STATE_PLAYING || currState->type == STATE_PAUSED) renderDebugGrid(zEngine);
    #endif

    for (Uint64 i = 0; i < rdrComps.denseSize; i++) {
        RenderComponent *render = (RenderComponent *)(rdrComps.dense[i]);
        Entity owner = rdrComps.denseToEntity[i];

        if (!render || !render->destRect || !render->active) continue;
        
        double angle = 0.0;
        if (HAS_COMPONENT(zEngine->ecs, owner, DIRECTION_COMPONENT)) {
            DirectionComponent *dirComp = NULL;
            GET_COMPONENT(zEngine->ecs, owner, DIRECTION_COMPONENT, dirComp, DirectionComponent);
            angle = vec2_to_angle(*dirComp);
        }

        SDL_RenderCopyEx(
            zEngine->display->renderer, render->texture, NULL, render->destRect, angle, NULL, SDL_FLIP_NONE
        );
    }
    
    #ifdef DEBUGCOLLISIONS
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

    #ifdef DEBUGUI
        // Render UI nodes with debug outlines
        if (zEngine->uiManager->root) {
            UIdebugRenderNode(zEngine->display->renderer, zEngine->uiManager, zEngine->uiManager->root);
        }
    #endif
    UIrender(zEngine->uiManager, zEngine->display->renderer);  // Voila

    // Should propagate the dirtiness here, but the render system is pretty much always the last
    // Rendering should always be done every frame
    // zEngine->ecs->depGraph->nodes[SYS_RENDER]->isDirty = 0;
}

/**
 * =====================================================================================================================
 */

void uiSystem(ZENg zEngine, double_t deltaTime) {
    #ifdef DEBUGSYSTEMS
        printf("[UI SYSTEM] There are %lu dirty UI components\n", zEngine->uiManager->dirtyCount);
    #endif
    
    while (zEngine->uiManager->dirtyCount > 0) {
        UINode *dirtyNode = zEngine->uiManager->dirtyNodes[0];
        switch (dirtyNode->type) {
            case UI_BUTTON: {
                // Recreate the button texture
                UIButton *btn = (UIButton *)(dirtyNode->widget);
                SDL_DestroyTexture(btn->texture);

                SDL_Surface *btnSurface = TTF_RenderText_Solid(
                    btn->font,
                    btn->text,
                    btn->currColor
                );
                if (!btnSurface) THROW_ERROR_AND_DO(
                    "Failed to create button surface: ",
                    fprintf(stderr, "%s\n", SDL_GetError()); exit(EXIT_FAILURE);
                );
                btn->texture = SDL_CreateTextureFromSurface(zEngine->display->renderer, btnSurface);
                SDL_FreeSurface(btnSurface);
                if (!btn->texture) THROW_ERROR_AND_DO(
                    "Failed to create button texture: ",
                    fprintf(stderr, "%s\n", SDL_GetError()); exit(EXIT_FAILURE);
                );
                break;
            }
        }
        UIunmarkNodeDirty(zEngine->uiManager);
    }
}

/**
 * =====================================================================================================================
 */

void renderArena(ZENg zEngine) {
    SDL_SetRenderDrawColor(zEngine->display->renderer, 20, 20, 20, 200);  // background color - grey
    SDL_RenderClear(zEngine->display->renderer);

    if (!zEngine->map || !zEngine->map->tiles) THROW_ERROR_AND_EXIT(
        "Error: Cannot render arena - map or tiles are NULL\n"
    );

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
#endif

#ifdef DEBUGCOLLISIONS
void renderDebugCollision(ZENg zEngine) {
    // Draw entity hitboxes in red and grid coverage in yellow
    Component colComps = zEngine->ecs->components[COLLISION_COMPONENT];
    SDL_SetRenderDrawBlendMode(zEngine->display->renderer, SDL_BLENDMODE_BLEND);

    for (Uint64 i = 0; i < colComps.denseSize; i++) {
        CollisionComponent *colComp = (CollisionComponent *)(colComps.dense[i]);
        if (!colComp || !colComp->hitbox) THROW_ERROR_AND_CONTINUE("Invalid colComp in renderDebugCollision\n");

        // Red
        SDL_SetRenderDrawColor(zEngine->display->renderer, 255, 0, 0, 255);
        SDL_RenderDrawRect(zEngine->display->renderer, colComp->hitbox);

        // Get the grid coverage and render it transparent yellow
        Uint16 covS = colComp->coverageStart;
        Uint16 covE = colComp->coverageEnd;
        Uint8 covSX = covS % ARENA_WIDTH;
        Uint8 covSY = covS / ARENA_WIDTH;
        Uint8 covEX = covE % ARENA_WIDTH;
        Uint8 covEY = covE / ARENA_WIDTH;

        SDL_Rect *hb = colComp->hitbox;

        // An arbitrary value to make the coverage more obvious by making it larger
        Uint32 oflw = 0;
        Uint32 coverageX = covSX * TILE_SIZE - oflw;
        Uint32 coverageW = (covEX - covSX) * TILE_SIZE + oflw * 2;
        Uint32 coverageY = covSY * TILE_SIZE - oflw;
        Uint32 coverageH = (covEY - covSY) * TILE_SIZE + oflw * 2;

        SDL_Rect rect = (SDL_Rect) {
            .x = coverageX,
            .y = coverageY,
            .w = coverageW,
            .h = coverageH
        };
        // Yellow
        SDL_SetRenderDrawColor(zEngine->display->renderer, 255, 255, 0, 100);
        SDL_RenderFillRect(zEngine->display->renderer, &rect);
    }
    SDL_SetRenderDrawBlendMode(zEngine->display->renderer, SDL_BLENDMODE_NONE);
    
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
    freePrefabsManager(&(*zEngine)->prefabs);

    freeECS((*zEngine)->ecs);

    // Free the UI tree
    UIclose((*zEngine)->uiManager);

    free((*zEngine)->stateMng->states[0]);  // free the main menu state
    free((*zEngine)->stateMng);

    SDL_DestroyRenderer((*zEngine)->display->renderer);
    SDL_DestroyWindow((*zEngine)->display->window);
    free((*zEngine)->display);
    SDL_Quit();
    free(*zEngine);
}
