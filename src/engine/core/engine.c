#include "engine.h"
#include "engine/arena.h"
#include "engine/collisionManager.h"
#include "engine/core/ecs.h"
#include <SDL2/SDL_blendmode.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include "global/debug.h"

void loadSettings(ZENg zEngine, const char *filePath) {
    // look for the file
    FILE *fin = fopen(filePath, "r");

    zEngine->inputMng = calloc(1, sizeof(struct inputmng));
    ASSERT(zEngine->inputMng, "zEngine->inputMng = %p\n", zEngine->inputMng);

    zEngine->display = calloc(1, sizeof(struct displayMng));
    ASSERT(zEngine->display, "zEngine->display = %p\n", zEngine->display);

    if (!fin) {
        // file doesn't exist
        LOG(INFO, "No config file found in %s. Using defaults\n", filePath);
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
        ASSERT(zEngine->display->window != NULL, "%s\n", SDL_GetError());
        zEngine->display->renderer = SDL_CreateRenderer(
            zEngine->display->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
        );
        ASSERT(zEngine->display->renderer != NULL, "%s\n", SDL_GetError());
    }

    // if the file exists, read the settings

    enum SettingsSection {
        NONE,
        SECTION_DISPLAY,
        SECTION_BINDINGS
    } currSect = NONE;

    // In case display settings are not fully specified, here's a failsafe with maximized compatibility
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
                    width = (int)strtol(value, NULL, 10);
                } else if (strcmp(setting, "HEIGHT") == 0) {
                    height = (int)strtol(value, NULL, 10);
                } else if (strcmp(setting, "FULLSCREEN") == 0) {
                    fullscreen = (int)strtol(value, NULL, 10);
                } else if (strcmp(setting, "VSYNC") == 0) {
                    vsync = (int)strtol(value, NULL, 10);
                } else {
                    LOG(WARNING, "Unknown DISPLAY setting: %s\n", setting);
                }
                break;
            }
            case SECTION_BINDINGS: {
                SDL_Scancode scancode = SDL_GetScancodeFromName(value);
                if (scancode == SDL_SCANCODE_UNKNOWN) {
                    LOG(WARNING, "Unknown value '%s' for action '%s'\n", value, setting);
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
                    LOG(WARNING, "Unknown action '%s'\n", setting);
                }
                break;
            }
            default:
                break;
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

    ASSERT(zEngine->display->window != NULL, "%s\n", SDL_GetError());

    // Create renderer
    Int32 rendererFlags = SDL_RENDERER_ACCELERATED;
    if (vsync) rendererFlags |= SDL_RENDERER_PRESENTVSYNC;

    zEngine->display->renderer = SDL_CreateRenderer(
        zEngine->display->window, -1, rendererFlags
    );
    ASSERT(zEngine->display->renderer != NULL, "%s\n", SDL_GetError());

    LOG(INFO, "Settings loaded from %s\n", filePath);
    LOG(DEBUG, "Display settings: %dx%d, fullscreen: %d, vsync: %hhu\n",
            zEngine->display->currentMode.w,
            zEngine->display->currentMode.h,
            zEngine->display->fullscreen,
            zEngine->display->vsync
    );
}

/**
 * =====================================================================================================================
 */

void initLevel(ZENg zEngine, const char *levelFilePath) {
    ASSERT(zEngine && levelFilePath, "zEngine = %p, levelFilePath = %p\n", zEngine, levelFilePath);

    zEngine->map = calloc(1, sizeof(struct arena));
    ASSERT(zEngine->map != NULL, "Failed to allocate memory for Arena");

    zEngine->map->tiles = calloc(ARENA_HEIGHT, sizeof(Tile*));
    ASSERT(zEngine->map->tiles != NULL, "Failed to allocate memory for arena tiles rows");
    for (Uint32 i = 0; i < ARENA_HEIGHT; i++) {
        zEngine->map->tiles[i] = calloc(ARENA_WIDTH, sizeof(Tile));
        ASSERT(zEngine->map->tiles[i], "Failed to allocate memory for arena tiles columns");
    }

    zEngine->collisionMng = initCollisionManager();

    FILE *f = fopen(levelFilePath, "rb");
    ASSERT(f != NULL, "Failed to open level file: %s\n", levelFilePath);

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = malloc(size + 1);
    ASSERT(data != NULL, "Failed to allocate memory for level file data\n");
    fread(data, 1, size, f);
    data[size] = '\0';
    fclose(f);

    cJSON *root = cJSON_Parse(data);
    ASSERT(root != NULL, "Failed to parse game level JSON file");
    free(data);

    // The arena files have an object with the arrays "tiles" and "entities"
    cJSON *tilesArray = cJSON_GetObjectItem(root, "tiles");
    if (!cJSON_IsArray(tilesArray)) {
        LOG(WARNING, "Invalid or missing 'tiles' array in level file");
        return;
    }

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
    cJSON *tileRowJSON = NULL;

    cJSON_ArrayForEach(tileRowJSON, tilesArray) {
        if (!cJSON_IsArray(tileRowJSON)) {
            LOG(WARNING, "Invalid tile row at index %d\n", row);
            continue;
        }

        if (row >= ARENA_HEIGHT) {
            LOG(WARNING, "More tile rows in level file than expected (%d). Ignoring extra rows\n", ARENA_HEIGHT);
            break;
        }

        Uint32 col = 0;
        cJSON *tileJSON = NULL;
        cJSON_ArrayForEach(tileJSON, tileRowJSON) {
            if (!cJSON_IsNumber(tileJSON)) {
                LOG(WARNING, "Invalid tile type at row %d, column %d\n", row, col);
                continue;
            }

            if (col >= ARENA_WIDTH) {
                LOG(WARNING,
                    "More tile columns in row %d than expected (%d). Ignoring extra columns\n", row, ARENA_WIDTH);
                break;
            }

            TileType currTileType = (TileType)tileJSON->valueint;
            if (currTileType < 0 || currTileType >= TILE_COUNT) {
                LOG(WARNING, "Invalid tile type value at row %d, column %d: %d. Defaulting to TILE_EMPTY\n",
                    row, col, currTileType);
                currTileType = TILE_EMPTY;
            }

            zEngine->map->tiles[row][col] = getTilePrefab(zEngine->prefabs, tileTypeToStr[currTileType]);
            zEngine->map->tiles[row][col].idx = row * ARENA_WIDTH + col;
            col++;
        }
        if (col < ARENA_WIDTH) {
            LOG(WARNING, "Fewer tile columns (%d) in row %d than expected (%d). The rest will be set TILE_EMPTY.\n",
            col, row, ARENA_WIDTH);
        }
        row++;
    }

    if (row < ARENA_HEIGHT) {
        LOG(WARNING, "Fewer tile rows (%d) in level file than expected (%d). The remaining tiles are set TILE_EMPTY.\n",
        row, ARENA_HEIGHT);
    }

    cJSON *entitiesArray = cJSON_GetObjectItem(root, "entities");
    if (cJSON_IsArray(entitiesArray)) {
        cJSON *entityJson;
        cJSON_ArrayForEach(entityJson, entitiesArray) {
            cJSON *entityTypeJson = cJSON_GetObjectItem(entityJson, "entityType");
            cJSON *xJson = cJSON_GetObjectItem(entityJson, "x");
            cJSON *yJson = cJSON_GetObjectItem(entityJson, "y");

            if (!cJSON_IsString(entityTypeJson) || !cJSON_IsNumber(xJson) || !cJSON_IsNumber(yJson)) {
                LOG(WARNING, "Invalid entity entry in the JSON entities array. Skipping.\n");
                continue;
            }

            char entityTypeStr[64];
            strncpy(entityTypeStr, entityTypeJson->valuestring, sizeof(entityTypeStr));
            entityTypeStr[sizeof(entityTypeStr) - 1] = '\0';

            int x = xJson->valueint;
            int y = yJson->valueint;

            Entity tank = instantiateTank(
                zEngine, getTankPrefab(zEngine->prefabs, entityTypeStr), (Vec2){.x = x * TILE_SIZE, .y = y * TILE_SIZE}
            );
            if (!HAS_COMPONENT(zEngine->ecs, tank, COLLISION_COMPONENT)) {
                LOG(WARNING, "Failed to add collision component to spawned tank\n");
                continue;
            }

            CollisionComponent *tankColComp = NULL;
            GET_COMPONENT(zEngine->ecs, tank, COLLISION_COMPONENT, tankColComp, CollisionComponent);
            registerEntityToSG(zEngine->collisionMng, tank, tankColComp);

            LOG(DEBUG, "Instantiated tank of type %s at (%d, %d)\n", entityTypeStr, y, x);
        }
    }
    cJSON_Delete(root);
}

/**
 * =====================================================================================================================
 */

void clearLevel(ZENg zEngine) {
    if (!zEngine || !zEngine->map) {
        LOG(ERROR, "zEngine or zEngine->map is NULL");
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

    // Free the collision manager
    if (zEngine->collisionMng) freeCollisionManager(zEngine->collisionMng);
    zEngine->collisionMng = NULL;
}

/**
 * =====================================================================================================================
 */

DependencyGraph* initDependencyGraph() {
    DependencyGraph *graph = calloc(1, sizeof(DependencyGraph));
    ASSERT(graph != NULL, "Failed to allocate memory for dependency graph");

    graph->nodes = calloc(SYS_COUNT, sizeof(SystemNode*));
    ASSERT(graph->nodes != NULL, "Failed to allocate memory for system nodes array");

    typedef struct SysPair{
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

            if (LOG_LEVEL <= DEBUG) {
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
                LOG(DEBUG,
                    "Added system dependency: %s -> %s\n", sysNames[dependency->type], sysNames[dependent->type]);
            }
        } else
            LOG(ERROR, "Invalid system relationship in dependencies array: (%d -> %d)\n",
                dependencies[i].dependency, dependencies[i].dependent);
    }
    return graph;
}

/**
 * =====================================================================================================================
 */

ZENg initGame() {
    ZENg zEngine = calloc(1, sizeof(struct engine));
    ASSERT(zEngine != NULL, "Failed to allocate memory for engine");

    // Initialize SDL
    int successVal = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    ASSERT(successVal == 0, "SDL_Init Error: %s\n", SDL_GetError());

    // Init the audio device
    successVal = Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    ASSERT(successVal >= 0, "SDL_Mixer could not initialize: %s\n", Mix_GetError());

    // Init fonts
    successVal = TTF_Init();
    ASSERT(successVal != -1, "SDL_TTF could not initialize: %s\n", TTF_GetError());

    // Init SDL_Image
    successVal = IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    ASSERT(successVal != 0, "SDL_Image could not initialize: %s\n", IMG_GetError());
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    
    // Initalize the display and input managers by reading settings file if existent
    loadSettings(zEngine, "settings.ini");
    // Set logical screen size
    successVal = SDL_RenderSetLogicalSize(zEngine->display->renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT);
    ASSERT(successVal >= 0, "Failed to set renderer logical size: %s\n", SDL_GetError());
                
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
    ASSERT(mainMenuState != NULL, "Failed to allocate memory for the main menu state");
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
			SYS_DEBUG(DEBUG_SYSTEMS, "[VELOCITY SYSTEM] Velocity system is not dirty\n");
            return;
        }
    }
    
	SYS_DEBUG(DEBUG_SYSTEMS, "[VELOCITY SYSTEM] Running velocity system for %lu entities\n",
		   zEngine->ecs->components[VELOCITY_COMPONENT].denseSize);

    ComponentTypeSet velComps = zEngine->ecs->components[VELOCITY_COMPONENT];
    // for each entity with a VELOCITY_COMPONENT, update its position based on the current velocity
    for (Uint64 i = 0; i < velComps.denseSize; i++) {
        VelocityComponent *velComp = (VelocityComponent *)(velComps.dense[i]);
        Entity entitty = velComps.denseToEntity[i];
        
        if (!HAS_COMPONENT(zEngine->ecs, entitty, POSITION_COMPONENT))
            LOG(WARNING, "Entity with VELOCITY_COMPONENT has no POSITION_COMPONENT");
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
		SYS_DEBUG(DEBUG_SYSTEMS, "[POSITION SYSTEM] Position system is not dirty\n");
        return;
    }
    ComponentTypeSet posComps = zEngine->ecs->components[POSITION_COMPONENT];
	SYS_DEBUG(DEBUG_SYSTEMS, "[POSITION SYSTEM] Running position system for %lu entities\n", posComps.denseSize);

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
		SYS_DEBUG(DEBUG_SYSTEMS, "[LIFETIME SYSTEM] Lifetime system is not dirty. Something wrong happened\n");
        return;
    }
    ComponentTypeSet *comps = zEngine->ecs->components;
	SYS_DEBUG(DEBUG_SYSTEMS, "[LIFETIME SYSTEM] Running lifetime system for %lu entities\n",
		   comps[LIFETIME_COMPONENT].denseSize);

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

    SYS_DEBUG(DEBUG_COLLISIONS, "[ENTITY COLLISION SYSTEM] Entity %lu spans over (y:%d-%d, x:%d-%d)\n",
			  entity, minY, maxY, minX, maxX);

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

                        SYS_DEBUG(DEBUG_COLLISIONS,
								  "[ENTITY COLLISION SYSTEM] Attempting to call a collision handler for entities");
                        SYS_DEBUG(DEBUG_COLLISIONS, " %lu(role %d) vs %lu(role %d)...\n",
								  entity, colComp->role, susColEntity, susEColComp->role);

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
		SYS_DEBUG(DEBUG_SYSTEMS, "[ENTITY COLLISION SYSTEM] Entity collision system is not dirty\n");
        return;
    }
    ComponentTypeSet *colComps = &zEngine->ecs->components[COLLISION_COMPONENT];

	SYS_DEBUG(DEBUG_SYSTEMS, "[ENTITY COLLISION SYSTEM] Running entity collision system for %lu entities\n",
		   colComps->denseSize);

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

                SYS_DEBUG(DEBUG_COLLISIONS, "[WORLD COLLISION SYSTEM] Attempting to call handler function for entity");
                SYS_DEBUG(DEBUG_COLLISIONS, " %lu vs tile type %d\n", entity, neighTile->type);

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
		SYS_DEBUG(DEBUG_SYSTEMS, "[WORLD COLLISION SYSTEM] World collision system is not dirty\n");
        return;
    }

    ComponentTypeSet *colComps = &zEngine->ecs->components[COLLISION_COMPONENT];

	SYS_DEBUG(DEBUG_SYSTEMS, "[WORLD COLLISION SYSTEM] Running world collision system for %lu entities\n",
		colComps->denseSize);

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
    ComponentTypeSet *comps = zEngine->ecs->components;
	SYS_DEBUG(DEBUG_SYSTEMS, "[HEALTH SYSTEM] There are %lu dirty health components\n",
		   comps[HEALTH_COMPONENT].dirtyCount);

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
		SYS_DEBUG(DEBUG_SYSTEMS, "[WEAPON SYSTEM] Weapon system is not dirty\n");
        return;
    }
    ComponentTypeSet weapComps = zEngine->ecs->components[WEAPON_COMPONENT];
	SYS_DEBUG(DEBUG_SYSTEMS, "[WEAPON SYSTEM] Running weapon system for %lu entities\n", weapComps.denseSize);

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
		SYS_DEBUG(DEBUG_SYSTEMS, "[TRANSFORM SYSTEM] Transform system is not dirty\n");
        return;
    }
    ComponentTypeSet posComps = zEngine->ecs->components[POSITION_COMPONENT];
	SYS_DEBUG(DEBUG_SYSTEMS, "[TRANSFORM SYSTEM] Running transform system for %lu entities\n", posComps.denseSize);

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

/**
 * =====================================================================================================================
 */

void renderSystem(ZENg zEngine, double_t deltaTime) {
    if (zEngine->ecs->depGraph->nodes[SYS_RENDER]->isDirty == 0) {
		SYS_DEBUG(DEBUG_SYSTEMS, "[RENDER SYSTEM] Render system is not dirty. Not good.\n");
    }
	ComponentTypeSet rdrComps = zEngine->ecs->components[RENDER_COMPONENT];
	Uint64 renderCount = rdrComps.denseSize;
	SYS_DEBUG(DEBUG_SYSTEMS, "[RENDER SYSTEM] Running render system for %lu entities\n", renderCount);

    GameState *currState = getCurrState(zEngine->stateMng);
    if (currState->type == STATE_PLAYING || currState->type == STATE_PAUSED) {
        renderArena(zEngine);
    }

	if ((DEBUG_MASK & DEBUG_COLLISIONS) && (currState->type == STATE_PLAYING || currState->type == STATE_PAUSED))
		renderDebugGrid(zEngine);

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
    
	if (DEBUG_MASK & DEBUG_COLLISIONS && (currState->type == STATE_PLAYING || currState->type == STATE_PAUSED))
		renderDebugCollision(zEngine);

    if (currState->type == STATE_PAUSED) {
        // Make the game appear as in background
        int screenW = zEngine->display->currentMode.w;
        int screenH = zEngine->display->currentMode.h;

        SDL_SetRenderDrawBlendMode(zEngine->display->renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(zEngine->display->renderer, 0, 0, 0, 128); // Half opaque
        SDL_RenderFillRect(zEngine->display->renderer, NULL);
        SDL_SetRenderDrawBlendMode(zEngine->display->renderer, SDL_BLENDMODE_NONE); // Reset if needed
    }

        // Render UI nodes with debug outlines
        if (DEBUG_MASK & DEBUG_UI && zEngine->uiManager->root)
            UIdebugRenderNode(zEngine->display->renderer, zEngine->uiManager, zEngine->uiManager->root);

    UIrender(zEngine->uiManager, zEngine->display->renderer);  // Voila

    // Should propagate the dirtiness here, but the render system is pretty much always the last
    // Rendering should always be done every frame
    // zEngine->ecs->depGraph->nodes[SYS_RENDER]->isDirty = 0;
}

/**
 * =====================================================================================================================
 */

void uiSystem(ZENg zEngine, double_t deltaTime) {
	SYS_DEBUG(DEBUG_SYSTEMS, "[UI SYSTEM] There are %lu dirty UI components\n", zEngine->uiManager->dirtyCount);
    
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
                ASSERT(btnSurface != NULL, "Failed to create button surface: %s\n", SDL_GetError());
                btn->texture = SDL_CreateTextureFromSurface(zEngine->display->renderer, btnSurface);
                SDL_FreeSurface(btnSurface);
                ASSERT(btn->texture != NULL, "Failed to create texture: %s\n", SDL_GetError());
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

    if (!zEngine->map || !zEngine->map->tiles) LOG(ERROR, "Error: Cannot render arena - map or tiles are NULL\n");

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
    // Draw entity hitboxes in red and grid coverage in yellow
    ComponentTypeSet colComps = zEngine->ecs->components[COLLISION_COMPONENT];
    SDL_SetRenderDrawBlendMode(zEngine->display->renderer, SDL_BLENDMODE_BLEND);

    for (Uint64 i = 0; i < colComps.denseSize; i++) {
        CollisionComponent *colComp = (CollisionComponent *)(colComps.dense[i]);
        if (!colComp || !colComp->hitbox) {
			LOG(ERROR, "Invalid colComp\n");
			continue;
		}

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
    LOG(DEBUG, "Settings saved to %s\n", filePath);
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
