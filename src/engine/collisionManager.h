#ifndef COLLISION_MANAGER_H
#define COLLISION_MANAGER_H

#include "ecs.h"

// Collisions are a bit special so treat them in their own context

typedef struct {
    Entity *entities;  // Array of entity IDs in this cell
    size_t entityCount;  // Number of entities in this cell
    size_t capacity;  // Capacity of the entities array
} GridCell;

// Handler function types
typedef void (*entityVsEntityHandler)(ZENg zEngine, Entity a, Entity b);
typedef void (*entityVsWorldHandler)(ZENg zEngine, Entity entity, Tile *tile);

typedef struct colmng {
    GridCell *spatialGrid;  // Flattened 2D array representing the spatial grid
    entityVsEntityHandler eVsEHandlers[COL_ROLE_COUNT][COL_ROLE_COUNT];
    entityVsWorldHandler eVsWHandlers[COL_ROLE_COUNT];
} *CollisionManager;

// Macro to get the index of a cell in the spatial grid
#define COL_GRID_INDEX(cm, x, y) \
    ((y) * ARENA_WIDTH + (x))

/**
 * Initializes the collision manager
 * @return CollisionManager
 * @note uses the defined constants for grid and cell size
 */
CollisionManager initCollisionManager();

/**
 * Registers an Entity vs Entity handler to the collision manager's handlers table
 * @param colMng pointer to the collision manager
 * @param roleA enum type, role of the first entity
 * @param roleB enum type, role of the second entity
 * @param handler function pointer, the handler in question
 */
void registerEVsEHandler(CollisionManager cm, CollisionRole roleA, CollisionRole roleB, entityVsEntityHandler handler);

/**
 * Registers an Entity vs World collision handler to the collision manager's handlers table  
 * @param colMng pointer to the collision manager
 * @param role enum type, role of the entity
 * @param handler function pointer, the handler in question
 */
void registerEVsWHandler(CollisionManager cm, CollisionRole role, entityVsWorldHandler handler);

/**
 * Normalizes two entities and their collision components so that entity A has a smaller role
 * @param A entity A
 * @param B entity B
 * @param colCompA A's collision component
 * @param colCompB B's collision component
 * @note entities are pointers whereas collision components are double pointers
 */
void normalizeRoles(Entity *A, Entity *B, CollisionComponent **colCompA, CollisionComponent **colCompB);

/*
 * Entity vs World collision handler
 * Handles the collision between an actor and a tile
 */
void actorVsWorldColHandler(ZENg zEngine, Entity actor, Tile *tile);

/**
 * Projectile vs World collision handler
 * Handles the collision between a projectile and a tile
 */
void projectileVsWorldColHandler(ZENg zEngine, Entity projectile, Tile *tile);

/**
 * Populates the collision handlers tables for the collision manager
 * @param cm pointer to the collision manager
 */
void populateHandlersTables(CollisionManager cm);

/**
 * Inserts an entity to the grid cells its hitbox spans on
 * @param cm the collision manager
 * @param e entity to be inserted
 * @param colComp the entity's collision component
 */
void insertEntityToSG(CollisionManager cm, Entity e, CollisionComponent *colComp);

/*
 * Removes an entity from the grid cells it no longer stays on
 * @param cm the collision manager
 * @param ecs the ECS struct
 * @param e the entity to be removed
 * @param colComp the entity's collision component
 */
void removeEntityFromSG(CollisionManager cm, ECS ecs, Entity e, CollisionComponent *colComp);

/**
 * Updates the grid membership for an entity
 * @param cm the collision manager
 * @param posComp the entity's position component to get previous frame position
 * @param velComp the entity's velocity component to get current frame position (after world collisions)
 * @param colComp the entity's collision component
 */
void updateGridMembership(
    CollisionManager cm, Entity e, PositionComponent *posComp, VelocityComponent *velComp, CollisionComponent *colComp
);

/**
 * Frees the memory allocated for the collision manager
 * @param cm CollisionManager
 */
void freeCollisionManager(CollisionManager cm);

#endif // COLLISION_MANAGER_H
