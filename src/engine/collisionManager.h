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
typedef void (*entityVsEntityHandler)(ECS ecs, Entity a, Entity b);
typedef void (*entityVsWorldHandler)(ECS ecs, Entity entity, Tile *tile);

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
void actorVsWorldColHandler(ECS ecs, Entity actor, Tile *tile);

/**
 * Populates the collision handlers tables for the collision manager
 * @param cm pointer to the collision manager
 */
void populateHandlersTables(CollisionManager cm);

/**
 * Frees the memory allocated for the collision manager
 * @param cm CollisionManager
 */
void freeCollisionManager(CollisionManager cm);

#endif // COLLISION_MANAGER_H
