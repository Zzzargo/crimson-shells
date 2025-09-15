#ifndef ZENG_H
#define ZENG_H

#include "ecs.h"
#include "inputManager.h"
#include "resourceManager.h"
#include "builder.h"
#include "displayManager.h"
#include "arena.h"
#include "uiManager.h"

struct statemng;  // forward declaration
typedef struct statemng *StateManager;

// Engine structure definition
typedef struct engine {
    DisplayManager display;  // Pointer to the display manager
    UIManager uiManager;  // Pointer to the UI manager
    ResourceManager resources;  // Pointer to the resource manager
    PrefabsManager prefabs;  // Pointer to the prefabs manager
    InputManager inputMng;  // Pointer to the input manager
    StateManager stateMng;  // Pointer to the state manager
    ECS ecs;  // Pointer to the game ECS
    Arena map;  // Pointer to the arena structure
} *ZENg;

#include "stateManager.h"  // stateManager needs the engine definition

/**
 * Loads the settings from a file
 * @param zEngine pointer to the engine
 * @param filepath path to the settings file
 * @note The function loads the settings into the engine's display manager and input manager
 */
void loadSettings(ZENg zEngine, const char *filePath);

/**
 * Initializes a level arena from a file
 * @param zEngine pointer to the engine
 * @param levelFilePath path to the level file
 */
void initLevel(ZENg zEngine, const char *levelFilePath);

/**
 * Destroys the current level
 * @param zEngine pointer to the engine
 */
void clearLevel(ZENg zEngine);

/**
 * Initialises the game engine
 * @return ZENg = struct engine*
 */
ZENg initGame();

/**
 * Ensures the entities are aligned to the grid
 * @param zEngine pointer to the engine
 */
void positionSystem(ZENg zEngine, double_t deltaTime);

/**
 * Updates the entities' positions based on their velocity
 * @param zEngine pointer to the engine
 * @param deltaTime time since the last frame in seconds
 */
void velocitySystem(ZENg zEngine, double_t deltaTime);

/**
 * Destroys entities which lifetime has expired
 * @param zEngine pointer to the engine
 * @param deltaTime time since the last frame in seconds
 */
void lifetimeSystem(ZENg zEngine, double_t deltaTime);

/**
 * Handles collisions between entities
 * @param zEngine pointer to the engine
 * @param AColComp pointer to the first collision component
 * @param BColComp pointer to the second collision component
 * @param AOwner the owner entity of the first collision component
 * @param BOwner the owner entity of the second collision component
 */
void handleEntitiesCollision(ZENg zEngine, CollisionComponent *AColComp, CollisionComponent *BColComp, Entity AOwner, Entity BOwner);

/**
 * Passes the collision components to the collision handler
 * @param zEngine pointer to the engine
 */
void entityCollisionSystem(ZENg zEngine, double_t deltaTime);

#ifdef DEBUG
/**
 * Renders lines on margins of entities' hitboxes to visualize the collisions
 * @param zEngine pointer to the engine
 */
void renderDebugCollision(ZENg zEngine);
#endif

/**
 * Checks whether a SDL_Rect (usually a hitbox) collides with the world (walls)
 * @param zEngine pointer to the engine
 * @param hitbox pointer to a SDL_Rect
 * @param collidedTile a pointer to a tile with which the entity collided
 * @return 1 if a collision was detected, 0 otherwise
 */
Uint8 checkWorldCollision(ZENg zEngine, SDL_Rect *hitbox, Tile *collidedTile);

/**
 * Passes the collision components to the collision handler
 * @param zEngine pointer to the engine
 */
void worldCollisionSystem(ZENg zEngine, double_t deltaTime);

/**
 * Updates entities based on their health
 * @param zEngine pointer to the engine
 * @param deltaTime time since the last frame in seconds
 */
void healthSystem(ZENg zEngine, double_t deltaTime);

/**
 * @param zEngine pointer to the engine
 * @param deltaTime time since the last frame in seconds
 */
void weaponSystem(ZENg zEngine, double_t deltaTime);

/**
 * Updates the rendered entities based on their position
 * @param zEngine pointer to the engine
 * @param deltaTime time since the last frame in seconds
 */
void transformSystem(ZENg zEngine, double_t deltaTime);

#ifdef DEBUG
/**
 * Renders the UI node and its children with debug outlines
 * @param rdr pointer to the SDL_Renderer
 * @param uiManager pointer to the UI manager
 * @param node pointer to the UI node to render
*/
void UIdebugRenderNode(SDL_Renderer *rdr, UIManager uiManager, UINode *node);
#endif

/**
 * Renders the current frame
 * @param zEngine pointer to the engine
 * @param deltaTime time since the last frame in seconds
 */
void renderSystem(ZENg zEngine, double_t deltaTime);

/**
 * Renders the game arena
 * @param zEngine pointer to the engine
 */
void renderArena(ZENg zEngine);

#ifdef DEBUG
/**
 * Renders lines on the tiles' margins to visualize the grid
 * @param zEngine pointer to the engine
 */
void renderDebugGrid(ZENg zEngine);
#endif

/**
 * Updates UI elements
 * @param zEngine pointer to the engine
 * @param deltaTime time since the last frame in seconds
 */
void uiSystem(ZENg zEngine, double_t deltaTime);

/**
 * Saves the current settings to a file
 * @param zEngine pointer to the engine
 * @param filePath path to the settings file
 */
void saveSettings(ZENg zEngine, const char *filePath);

/**
 * Frees the memory used by the engine
 * @param zEngine pointer to the engine
 * @return void
 */
void destroyEngine(ZENg *zEngine);

#endif // ZENG_H