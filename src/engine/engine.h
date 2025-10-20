#ifndef ZENG_H
#define ZENG_H

#include "ecs.h"
#include "inputManager.h"
#include "resourceManager.h"
#include "displayManager.h"
#include "arena.h"
#include "uiManager.h"
#include "collisionManager.h"

struct statemng;  // forward declaration
typedef struct statemng *StateManager;

// Engine structure definition
typedef struct engine {
    DisplayManager display;  // Pointer to the display manager
    UIManager uiManager;  // Pointer to the UI manager
    HashMap resources;  // Pointer to the resource manager
    HashMap prefabs;  // Pointer to the prefabs manager
    InputManager inputMng;  // Pointer to the input manager
    StateManager stateMng;  // Pointer to the state manager
    CollisionManager collisionMng;  // Pointer to the collision manager
    ECS ecs;  // Pointer to the game ECS
    Arena map;  // Pointer to the arena structure
} *ZENg;

#include "../states/stateManager.h"

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
 * Updates real positions and ensures the entities are aligned to the grid
 * @param zEngine pointer to the engine
 * @param deltaTime time since the last frame in seconds
 */
void positionSystem(ZENg zEngine, double_t deltaTime);

/**
 * Updates the entities' (predicted) positions based on their velocity
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
 * Checks and handles collisions between entities
 * @param zEngine pointer to the engine
 * @param entity entity for which the collisions are checked
 * @return the number of entities the current one has collided with (maybe will change to track them later)
 */
Uint8 checkAndHandleEntityCollisions(ZENg zEngine, Entity entity);

/**
 * Passes the collision components to the collision handler
 * @param zEngine pointer to the engine
 * @note works with the predicted position of entities, not the real one
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
 * Checks whether an entity collides with the world (walls mostly) and passes the colliders to a handling function
 * @param zEngine pointer to the engine
 * @param entity entity for which the collision is checker
 * @return number of tiles the entity has collided with
 */
Uint8 checkAndHandleWorldCollisions(ZENg zEngine, Entity entity);

/**
 * Passes the collision components to the collision handler
 * @param zEngine pointer to the engine
 * @note works with the predicted position of entities, not the real one
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
