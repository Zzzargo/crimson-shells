#include "collisionManager.h"
#include "arena.h"
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

void actorVsWorldColHandler(ZENg zEngine, Entity actor, Tile *tile) {
#ifdef DEBUGPP
    printf("[WORLD COLLISION SYSTEM] Actor VS World collision: Entity %lu | Tile %d", actor, tile->type);
#endif
    if (!HAS_COMPONENT(zEngine->ecs, actor, COLLISION_COMPONENT))
        THROW_ERROR_AND_RETURN_VOID("Actor with invalid colComp in actorVsWorldColHandler");
    CollisionComponent *aColComp = NULL;
    GET_COMPONENT(zEngine->ecs, actor, COLLISION_COMPONENT, aColComp, CollisionComponent);

    if (!HAS_COMPONENT(zEngine->ecs, actor, VELOCITY_COMPONENT)) return;
    VelocityComponent *aVelComp = NULL;
    GET_COMPONENT(zEngine->ecs, actor, VELOCITY_COMPONENT, aVelComp, VelocityComponent);

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
    aVelComp->predictedPos.x = aColComp->hitbox->x;

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
    aVelComp->predictedPos.y = aColComp->hitbox->y;
#ifdef DEBUGPP
    printf("Clamped entity %lu on Y axis\n", actor);
#endif
}

// =====================================================================================================================

void projectileVsWorldColHandler(ZENg zEngine, Entity projectile, Tile *tile) {
    if (!HAS_COMPONENT(zEngine->ecs, projectile, PROJECTILE_COMPONENT))
        THROW_ERROR_AND_RETURN_VOID("Projectile entity without projectile component in projectileVsWorldCollision");
     ProjectileComponent *projComp = NULL;
     GET_COMPONENT(zEngine->ecs, projectile, PROJECTILE_COMPONENT, projComp, ProjectileComponent);
     if (
         (tile->type == TILE_BRICKS && projComp->dmg >= 15)
         || (tile->type == TILE_ROCK && projComp->dmg >= 30)
     ) {
         char *tileTypeToStr[] = {
             [TILE_EMPTY] = "TILE_EMPTY",
             [TILE_GRASS] = "TILE_GRASS",
             [TILE_WATER] = "TILE_WATER",
             [TILE_ROCK] = "TILE_ROCK",
             [TILE_BRICKS] = "TILE_BRICKS",
             [TILE_WOOD] = "TILE_WOOD",
             [TILE_SPAWN] = "TILE_SPAWN"
         };
         Uint32 tileY = tile->idx / ARENA_WIDTH;
         Uint32 tileX = tile->idx % ARENA_WIDTH;
         zEngine->map->tiles[tileY][tileX] = getTilePrefab(zEngine->prefabs, tileTypeToStr[TILE_EMPTY]);
     }
     deleteEntity(zEngine->ecs, projectile);
}

// =====================================================================================================================

void populateHandlersTables(CollisionManager cm) {
    registerEVsWHandler(cm, COL_ACTOR, &actorVsWorldColHandler);
    registerEVsWHandler(cm, COL_BULLET, &projectileVsWorldColHandler);
}

// =====================================================================================================================

void insertEntityToSG(CollisionManager cm, Entity e, CollisionComponent *colComp) {
    if (!cm) THROW_ERROR_AND_RETURN_VOID("Collision manager NULL in insertEntityToSG");
    if (!colComp) THROW_ERROR_AND_RETURN_VOID("Entity's colComp NULL in insertEntityToSG");
    SDL_Rect *hb = colComp->hitbox;
    if (!hb) THROW_ERROR_AND_RETURN_VOID("Entity's hitbox NULL in insertEntityToSG");

    // Compute the tile range that the entity covers
    Int32 startX = hb->x / TILE_SIZE;
    Int32 startY = hb->y / TILE_SIZE;
    Int32 endX = (hb->x + hb->w) / TILE_SIZE;
    Int32 endY = (hb->y + hb->h) / TILE_SIZE;

    // Clamp to grid boundaries
    if (endX < 0 || endY < 0 || startX >= ARENA_WIDTH || startY >= ARENA_HEIGHT) return;
    if (startX < 0) startX = 0;
    if (startY < 0) startY = 0;
    if (endX >= ARENA_WIDTH) endX = ARENA_WIDTH - 1;
    if (endY >= ARENA_HEIGHT) endY = ARENA_HEIGHT - 1;

    // Insert into all covered cells
    for (Int32 x = startX; x <= endX; x++) {
        for (Int32 y = startY; y <= endY; y++) {
            size_t idx = y * ARENA_WIDTH + x;
            GridCell *cell = &cm->spatialGrid[idx];

            if (cell->entityCount >= cell->capacity) {
                // Reallocate the entities array for this cell
                Entity *tmp = realloc(cell->entities, cell->capacity * 2);
                if (!tmp) THROW_ERROR_AND_EXIT("Failed to reallocate a grid cell's entity array at insertion");
                cell->capacity *= 2;
            }
            cell->entities[cell->entityCount++] = e;
        }
    }
}

// =====================================================================================================================

void removeEntityFromSG(CollisionManager cm, ECS ecs, Entity e, CollisionComponent *colComp) {
    for (size_t i = 0; i < colComp->numCells; i++) {
        size_t cellIdx = colComp->cellIdxs[i];
        size_t slot = colComp->entityArrIdx[i];

        GridCell *cell = &cm->spatialGrid[cellIdx];

        // Swap-remove
        Entity swapped = cell->entities[--cell->entityCount];
        cell->entities[slot] = swapped;

        if (slot < cell->entityCount && swapped != e) {
            CollisionComponent *swappedCol = NULL;
            GET_COMPONENT(ecs, swapped, COLLISION_COMPONENT, swappedCol, CollisionComponent);

            for (size_t j = 0; j < swappedCol->numCells; j++) {
                if (swappedCol->cellIdxs[j] == cellIdx) {
                    swappedCol->entityArrIdx[j] = slot;
                    break;
                }
            }
        }
    }
    colComp->numCells = 0;
}

// =====================================================================================================================

void updateGridMembership(
    CollisionManager cm, Entity e, PositionComponent *posComp, VelocityComponent *velComp, CollisionComponent *colComp
) {
    // Get the changed grid coverage and delete/insert from corresponding cells
}
