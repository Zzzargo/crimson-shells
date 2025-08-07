#ifndef ZENG_H
#define ZENG_H

#include "ecs.h"
#include "inputManager.h"
#include "resourceManager.h"
#include "displayManager.h"
#include "arena.h"

struct statemng;  // forward declaration
typedef struct statemng *StateManager;

// Engine structure definition
typedef struct engine {
    DisplayManager display;  // Pointer to the display manager
    ResourceManager resources;  // Pointer to the resource manager
    InputManager inputMng;  // Pointer to the input manager
    StateManager stateMng;  // Pointer to the state manager
    ECS uiEcs;  // Pointer to the UI ECS
    ECS gEcs;  // Pointer to the game ECS
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
 */
void initLevel(ZENg zEngine, const char *levelFilePath);

/**
 * Initialises the game engine
 * @return ZENg = struct engine*
 */
ZENg initGame();

/**
 * Ensures the entities are aligned to the grid
 * @param zEngine pointer to the engine
 */
void positionSystem(ZENg zEngine);

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
void entityCollisionSystem(ZENg zEngine);

/**
 * Checks whether a SDL_Rect (usually a hitbox) collides with the world (walls)
 * @param zEngine pointer to the engine
 * @param hitbox pointer to a SDL_Rect
 * @param result a pointer to a SDL_Rect representing the world part with which the entity collided, or NULL
 * @return 1 if a collision was detected, 0 otherwise
 * @note the result must be allocated before calling the function
 */
Uint8 checkWorldCollision(ZENg zEngine, SDL_Rect *hitbox, SDL_Rect *result);

/**
 * Passes the collision components to the collision handler
 * @param zEngine pointer to the engine
 */
void worldCollisionSystem(ZENg zEngine, double_t deltaTime);

/**
 * Updates entities based on their health
 * @param zEngine pointer to the engine
 */
void healthSystem(ZENg zEngine);

/**
 * Updates the rendered entities based on their position
 * @param ecs pointer to the ECS
 */
void transformSystem(ECS ecs);

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