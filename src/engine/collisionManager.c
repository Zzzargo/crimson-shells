#include "collisionManager.h"
#include "engine/arena.h"
#include "engine/core/ecs.h"
#include "engine/core/engine.h"
#include "global/debug.h"

CollisionManager initCollisionManager() {
    CollisionManager cm = calloc(1, sizeof(struct colmng));
    ASSERT(cm != NULL, "Failed allocating memory for the Collision Manager");

    // Initialize the spatial grid
    size_t totalCells = ARENA_WIDTH * ARENA_HEIGHT;
    cm->spatialGrid = calloc(totalCells, sizeof(GridCell));
    ASSERT(cm->spatialGrid != NULL, "Failed allocating memory for the spatial grid");

    // Initialize each cell's entity array for 2 entities
    for (size_t i = 0; i < totalCells; i++) {
        GridCell *cell = &cm->spatialGrid[i];

        cell->entities = calloc(2, sizeof(Entity));
        ASSERT(cell->entities != NULL, "Failed allocating memory for grid cell entities");
        cell->capacity = 2;
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

    // This matrix should be symmetric
    cm->eVsEHandlers[roleA][roleB] = handler;
    cm->eVsEHandlers[roleB][roleA] = handler;

#if defined(DEBUGCOLLISION) && defined(DEBUGPP)
    printf("Registered Entity VS Entity collision handler for roles %d | %d : %p\n", roleA, roleB, handler);
#endif
}

// =====================================================================================================================

void registerEVsWHandler(CollisionManager cm, CollisionRole role, entityVsWorldHandler handler) {
    if (role >= COL_ROLE_COUNT) return;
    cm->eVsWHandlers[role] = handler;

#if defined(DEBUGCOLLISION) && defined(DEBUGPP)
    printf("Registered Entity VS World collision handler for role %d : %p\n", role, handler);
#endif
}

// =====================================================================================================================

void normalizeRoles(Entity *A, Entity *B, CollisionComponent **colCompA, CollisionComponent **colCompB) {
    if ((*colCompA)->role > (*colCompB)->role) {
        Entity tmp = *A;
        *A = *B;
        *B = tmp;

        CollisionComponent *tmpComp = *colCompA;
        *colCompA = *colCompB;
        *colCompB = tmpComp;
    }
}

// =====================================================================================================================

void actorVsWorldColHandler(ZENg zEngine, Entity actor, Tile *tile) {
#ifdef DEBUGCOLLISIONS
    printf("[WORLD COLLISION SYSTEM] Actor VS World collision: Entity %lu | Tile type %d\n", actor, tile->type);
#endif
    if (tile->isWalkable) return;  // Move freely

    if (!HAS_COMPONENT(zEngine->ecs, actor, COLLISION_COMPONENT))
        LOG(WARNING, "Actor with invalid colComp in %s\n", __func__);
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

#ifdef DEBUGCOLLISIONS
    printf("[WORLD COLLISION SYSTEM] Clamped entity %lu on X axis\n", actor);
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
#if defined(DEBUGCOLLISIONS) && defined(DEBUGPP)
    printf("[WORLD COLLIISON SYSTEM] Clamped entity %lu on Y axis\n", actor);
#endif
}

// =====================================================================================================================

void projectileVsWorldColHandler(ZENg zEngine, Entity projectile, Tile *tile) {
#ifdef DEBUGCOLLISIONS
    printf("[WORLD COLLISION SYSTEM] Projectile VS World collision: Entity %lu | Tile %d\n", projectile, tile->type);
#endif
    if (!tile->isSolid) return;  // Bullet passes through this wall
    if (!HAS_COMPONENT(zEngine->ecs, projectile, PROJECTILE_COMPONENT))
        LOG(WARNING, "Projectile entity without projectile component in %s\n", __func__);
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

void projectileVsActorColHandler(ZENg zEngine, Entity actor, Entity projectile) {
#ifdef DEBUGCOLLISIONS
    printf("[ENTITY COLLISION SYSTEM] Projectile(%lu) VS Actor(%lu) collision\n", actor, projectile);
#endif
    if (!HAS_COMPONENT(zEngine->ecs, projectile, PROJECTILE_COMPONENT))
        LOG(WARNING,"Projectile without projectile component in %s\n", __func__);
    if (!HAS_COMPONENT(zEngine->ecs, actor, HEALTH_COMPONENT))
        LOG(WARNING, "Actor without health component in %s\n", __func__);
    ProjectileComponent *projComp = NULL;
    GET_COMPONENT(zEngine->ecs, projectile, PROJECTILE_COMPONENT, projComp, ProjectileComponent);
    HealthComponent *healthComp = NULL;
    GET_COMPONENT(zEngine->ecs, actor, HEALTH_COMPONENT, healthComp, HealthComponent);

    if (projComp->friendly && actor == PLAYER_ID) return;
    healthComp->currentHealth -= projComp->dmg;
    markComponentDirty(zEngine->ecs, actor, HEALTH_COMPONENT);

    deleteEntity(zEngine->ecs, projectile);
}

// =====================================================================================================================

void populateHandlersTables(CollisionManager cm) {
    registerEVsWHandler(cm, COL_ACTOR, &actorVsWorldColHandler);
    registerEVsWHandler(cm, COL_BULLET, &projectileVsWorldColHandler);
    registerEVsEHandler(cm, COL_BULLET, COL_ACTOR, &projectileVsActorColHandler);
}

// =====================================================================================================================

void registerEntityToSG(CollisionManager cm, Entity e, CollisionComponent *colComp) {
    ASSERT(cm && colComp && colComp->hitbox, "cm = %p, colComp = %p, hb = %p", cm, colComp, colComp->hitbox);
    SDL_Rect *hb = colComp->hitbox;

    // Compute the tile range that the entity covers. At instantiation the hitbox corresponds to the position component
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

    colComp->coverageStart = (Uint16)COL_GRID_INDEX(cm, startX, startY);
    colComp->coverageEnd = (Uint16)COL_GRID_INDEX(cm, endX, endY);

#ifdef DEBUGCOLLISIONS
    fprintf(stderr, "[ENTITY COLLISION SYSTEM] Registering entity %lu to spatial grid: y:%d-%d, x:%d-%d\n",
            e, startY, endY, startX, endX);
#endif

    // Insert into all covered cells
    for (Int32 x = startX; x <= endX; x++) {
        for (Int32 y = startY; y <= endY; y++) {
            size_t cellIdx = COL_GRID_INDEX(cm, x, y);
            GridCell *cell = &cm->spatialGrid[cellIdx];
            insertEntityToSGCell(e, cell);
        }
    }
}

// =====================================================================================================================

void insertEntityToSGCell(Entity e, GridCell *cell) {
    // Check if another would fit
    if (cell->entityCount >= cell->capacity) {
        Entity *tmp = realloc(cell->entities, sizeof(Entity) * cell->capacity * 2);
        ASSERT(tmp != NULL, "Memory reallocation failed for the entities array of a spatial grid cell");
        cell->entities = tmp;
        cell->capacity *= 2;
    }

    cell->entities[cell->entityCount++] = e;
}

// =====================================================================================================================

void removeEntityFromSGCell(Entity e, GridCell *cell) {
    // Swap delete
    for (size_t entityIdx = 0; entityIdx < cell->entityCount; entityIdx++) {
        if (cell->entities[entityIdx] != e) continue;

        // Below is the case where the entity to remove is found
        size_t last = --cell->entityCount;

        if (last != entityIdx) {
            Entity swapped = cell->entities[last];
            cell->entities[entityIdx] = swapped;
        }
        return;
    }
}

// =====================================================================================================================

void updateGridMembership(CollisionManager cm, Entity e, VelocityComponent *velComp, CollisionComponent *colComp) {
    // Get the changed grid coverage and delete/insert from corresponding cells
    
    SDL_Rect *hb = colComp->hitbox;
    Vec2 currPos = velComp->predictedPos;
    
    Uint16 prevMinX = colComp->coverageStart % ARENA_WIDTH;
    Uint16 prevMinY = colComp->coverageStart / ARENA_WIDTH;
    Uint16 prevMaxX = colComp->coverageEnd % ARENA_WIDTH;
    Uint16 prevMaxY = colComp->coverageEnd / ARENA_WIDTH;

    Int32 currMinX = currPos.x / TILE_SIZE;
    Int32 currMinY = currPos.y / TILE_SIZE;
    Int32 currMaxX = (currPos.x + hb->w + TILE_SIZE - 1) / TILE_SIZE;
    Int32 currMaxY = (currPos.y + hb->h + TILE_SIZE - 1) / TILE_SIZE;

    // Clamp to grid boundaries
    if (currMaxX < 0 || currMaxY < 0 || currMinX >= ARENA_WIDTH || currMinY >= ARENA_HEIGHT) return;
    if (currMinX < 0) currMinX = 0;
    if (currMinY < 0) currMinY = 0;
    if (currMaxX >= ARENA_WIDTH) currMaxX = ARENA_WIDTH - 1;
    if (currMaxY >= ARENA_HEIGHT) currMaxY = ARENA_HEIGHT - 1;

    // Update current coverage
    colComp->coverageStart = COL_GRID_INDEX(cm, currMinX, currMinY);
    colComp->coverageEnd = COL_GRID_INDEX(cm, currMaxX, currMaxY);

    // Insert the entity in the new cells
    for (Int32 y = currMinY; y <= currMaxY; y++) {
        for (Int32 x = currMinX; x <= currMaxX; x++) {
            // If this cell wasn’t part of the previous coverage - insert
            if (x < prevMinX || x > prevMaxX || y < prevMinY || y > prevMaxY) {
                size_t cellIdx = COL_GRID_INDEX(cm, x, y);
                GridCell *cell = &cm->spatialGrid[cellIdx];

#ifdef DEBUGCOLLISIONS
                printf("[ENTITY COLLISION SYSTEM] Inserting entity %lu to cell (y=%d, x=%d)\n", e, y, x);
#endif

                insertEntityToSGCell(e, cell);
            }
        }
    }

    // Delete the entity from the old cells
    for (Int32 y = prevMinY; y <= prevMaxY; y++) {
        for (Int32 x = prevMinX; x <= prevMaxX; x++) {
            // If this cell won’t be part of the current coverage - remove
            if (x < currMinX || x > currMaxX || y < currMinY || y > currMaxY) {
                size_t cellIdx = COL_GRID_INDEX(cm, x, y);
                GridCell *cell = &cm->spatialGrid[cellIdx];
#ifdef DEBUGCOLLISIONS
                printf("[ENTITY COLLISION SYSTEM] Removing entity %lu from cell (y=%d, x=%d)\n", e, y, x);
#endif
                removeEntityFromSGCell(e, cell);
            }
        }
    }
}
