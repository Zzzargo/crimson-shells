#include "collisionManager.h"
#include "ecs.h"
#include "engine.h"

CollisionManager initCollisionManager() {
    CollisionManager cm = calloc(1, sizeof(struct colmng));
    if (!cm) THROW_ERROR_AND_EXIT("Failed allocating memory for the Collision Manager");

    // Initialize the spatial grid
    size_t totalCells = ARENA_WIDTH * ARENA_HEIGHT;
    cm->spatialGrid = calloc(totalCells, sizeof(GridCell));
    if (!cm->spatialGrid) THROW_ERROR_AND_EXIT("Failed allocating memory for the spatial grid");

    // Initialize each cell's entity array for 4 entities
    for (size_t i = 0; i < totalCells; i++) {
        GridCell *cell = &cm->spatialGrid[i];

        cell->entities = calloc(4, sizeof(Entity));
        if (!cell->entities) THROW_ERROR_AND_EXIT("Failed allocating memory for grid cell entities");
        cell->capacity = 4;
        cell->entityCount = 0;
    }

    populateHandlersTables(cm);
    return cm;
}

// =====================================================================================================================

void freeCollisionManager(CollisionManager cm) {
    if (!cm) return;

    // Free each cell's entity array
    size_t totalCells = ARENA_WIDTH * ARENA_HEIGHT;
    for (size_t i = 0; i < totalCells; i++) {
        GridCell *cell = &cm->spatialGrid[i];
        if (cell->entities) free(cell->entities);
    }

    // Free the spatial grid
    if (cm->spatialGrid) free(cm->spatialGrid);

    // Free the collision manager itself
    free(cm);
}

// =====================================================================================================================

void registerEVsEHandler(CollisionManager cm, CollisionRole roleA, CollisionRole roleB, entityVsEntityHandler handler) {
    if (roleA >= COL_ROLE_COUNT || roleB >= COL_ROLE_COUNT) return;
    cm->eVsEHandlers[roleA][roleB] = handler;
}

// =====================================================================================================================

void registerEVsWHandler(CollisionManager cm, CollisionRole role, entityVsWorldHandler handler) {
    if (role >= COL_ROLE_COUNT) return;
    cm->eVsWHandlers[role] = handler;
}

// =====================================================================================================================

void normalizeRoles(Entity *A, Entity *B, CollisionComponent **colCompA, CollisionComponent **colCompB) {
    if ((*colCompA)->role > (*colCompB)->role) {
        Entity *tmp = A;
        *A = *B;
        *B = *tmp;

        CollisionComponent *tmpComp = *colCompA;
        *colCompA = *colCompB;
        *colCompB = tmpComp;
    }
}

// =====================================================================================================================

void actorVsWorldColHandler(ECS ecs, Entity actor, Tile *tile) {
    if (!HAS_COMPONENT(ecs, actor, COLLISION_COMPONENT))
        THROW_ERROR_AND_RETURN_VOID("Actor with invalid colComp in actorVsWorldColHandler");
    CollisionComponent *aColComp = NULL;
    GET_COMPONENT(ecs, actor, COLLISION_COMPONENT, aColComp, CollisionComponent);

    if (!HAS_COMPONENT(ecs, actor, VELOCITY_COMPONENT)) return;
    VelocityComponent *aVelComp = NULL;
    GET_COMPONENT(ecs, actor, VELOCITY_COMPONENT, aVelComp, VelocityComponent);

    // Entities with a velocity component must have a position component
    PositionComponent *aPosComp = NULL;
    GET_COMPONENT(ecs, actor, POSITION_COMPONENT, aPosComp, PositionComponent);

    // Resolve X-Axis wall collisions
    double_t moveX = aVelComp->currVelocity.x;
    aColComp->hitbox->x = aVelComp->predictedPos.x;
    Vec2 tileCoords = tileToWorld(tile->idx);
    if (moveX > 0) {
        aColComp->hitbox->x = tileCoords.x - aColComp->hitbox->w;
    } else if (moveX < 0) {
        aColComp->hitbox->x = tileCoords.x + TILE_SIZE;
    }
    aVelComp->currVelocity.x = 0.0;
    aPosComp->x = aColComp->hitbox->x;

#ifdef DEBUGPP
    printf("Clamped entity %lu on X axis\n", actor);
#endif

    // Resolve Y-Axis wall collisions
    double_t moveY = aVelComp->currVelocity.y;
    aColComp->hitbox->y = aVelComp->predictedPos.y;

    tileCoords = tileToWorld(tile->idx);
    if (moveY > 0) {
        aColComp->hitbox->y = tileCoords.y - aColComp->hitbox->h;
    } else if (moveY < 0) {
        aColComp->hitbox->y = tileCoords.y + TILE_SIZE;
    }
    aVelComp->currVelocity.y = 0.0;
    aPosComp->y = aColComp->hitbox->y;
#ifdef DEBUGPP
    printf("Clamped entity %lu on Y axis\n", actor);
#endif
}

// =====================================================================================================================

void populateHandlersTables(CollisionManager cm) {
    registerEVsWHandler(cm, COL_ACTOR, &actorVsWorldColHandler);
}
