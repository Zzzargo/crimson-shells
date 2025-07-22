#ifndef ZENG_H
#define ZENG_H

#include "ecs.h"
#include "inputManager.h"
#include "resourceManager.h"
#include "displayManager.h"

typedef struct engine {
    DisplayManager display;
    ResourceManager resources;
    InputManager inputMng;
    ECS uiEcs;
    ECS gEcs;
    GameState state;
} *ZENg;

/**
 * Loads the settings from a file
 * @param zEngine pointer to the engine
 * @param filepath path to the settings file
 * @note The function loads the settings into the engine's display manager and input manager
 */
void loadSettings(ZENg zEngine, const char *filePath);

/**
 * Initialises the game engine
 * @return ZENg = struct engine*
 */
ZENg initGame();

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
void handleCollision(ZENg zEngine, CollisionComponent *AColComp, CollisionComponent *BColComp, Entity AOwner, Entity BOwner);

/**
 * Passes the collision components to the collision handler
 * @param zEngine pointer to the engine
 */
void collisionSystem(ZENg zEngine);

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
 * Frees the memory used by the engine
 * @param zEngine pointer to the engine
 * @return void
 */
void destroyEngine(ZENg *zEngine);

#endif // ZENG_H