#include "include/engine.h"

ZENg initGame() {
    ZENg zEngine = calloc(1, sizeof(struct engine));
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Initialize the input manager
    zEngine->inputMng = calloc(1, sizeof(struct inputmng));
    if (!zEngine->inputMng) {
        printf("Failed to allocate memory for input manager\n");
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    loadKeyBindings(zEngine->inputMng, "keys.cfg");
    zEngine->inputMng->keyboardState = SDL_GetKeyboardState(NULL);  // get the current keyboard state

    // Initialize font manager
    initFonts(&zEngine->fonts);

    // Initialize ECS
    initGECS(&zEngine->gEcs);
    initUIECS(&zEngine->uiEcs);

    // Create a window
    (zEngine->window) = SDL_CreateWindow(
        "GOAT Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280, 720,  // a medium 16:9 window in the center of the monitor if windowed mode
        SDL_WINDOW_FULLSCREEN_DESKTOP
    );
    if (!zEngine->window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Create a renderer
    (zEngine->renderer) = SDL_CreateRenderer(zEngine->window, -1, SDL_RENDERER_ACCELERATED);
    if (!zEngine->renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(zEngine->window);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // start on the main menu
    zEngine->state = STATE_MAIN_MENU;
    return zEngine;
}

void velocitySystem(ZENg zEngine, double_t deltaTime) {
    // for each entity with a VELOCITY_COMPONENT, update its position based on the current velocity
    for (Uint64 i = 0; i < zEngine->gEcs->components[VELOCITY_COMPONENT].denseSize; i++) {
        VelocityComponent *velComp = (VelocityComponent *)(zEngine->gEcs->components[VELOCITY_COMPONENT].dense[i]);
        Entity entitty = zEngine->gEcs->components[VELOCITY_COMPONENT].denseToEntity[i];  // get the owner entity

        if ((zEngine->gEcs->componentsFlags[entitty] & (1 << POSITION_COMPONENT)) == (1 << POSITION_COMPONENT)) {
            // if the entity has also the position component
            Uint64 page = entitty / PAGE_SIZE;  // determine the page for the entity
            Uint64 index = entitty % PAGE_SIZE;  // determine the index within the page
            Uint64 denseIndex = zEngine->gEcs->components[POSITION_COMPONENT].sparse[page][index];
            if (denseIndex >= zEngine->gEcs->components[POSITION_COMPONENT].denseSize) {
                printf("Warning: Entity %ld has a position component with invalid dense index %lu\n", entitty, denseIndex);
                continue;
            }
            PositionComponent *posComp = (PositionComponent *)(zEngine->gEcs->components[POSITION_COMPONENT].dense[denseIndex]);

            // Update the position based on the velocity
            posComp->x += velComp->currVelocity.x * deltaTime;
            posComp->y += velComp->currVelocity.y * deltaTime;

            // and the hitboxes
            CollisionComponent *colComp = (CollisionComponent *)(zEngine->gEcs->components[COLLISION_COMPONENT].dense[denseIndex]);
            if (colComp && colComp->hitbox) {
                colComp->hitbox->x = posComp->x;
                colComp->hitbox->y = posComp->y;
            }

            // clamp the position to the window bounds
            if (posComp->x < 0) posComp->x = 0;  // prevent going out of bounds
            if (posComp->y < 0) posComp->y = 0;  // prevent going out of bounds

            int wW, wH;
            SDL_GetWindowSize(zEngine->window, &wW, &wH);
            Uint64 denseRenderIndex = zEngine->gEcs->components[RENDER_COMPONENT].sparse[page][index];
            SDL_Rect *entityRect = ((RenderComponent *)(zEngine->gEcs->components[RENDER_COMPONENT].dense[denseRenderIndex]))->destRect;
            if (entityRect) {
                if (posComp->x + entityRect->w > wW) {
                    posComp->x = wW - entityRect->w;  // prevent going out of bounds
                }
                if (posComp->y + entityRect->h > wH) {
                    posComp->y = wH - entityRect->h;  // prevent going out of bounds
                }
            }
        }
    }
}

void lifetimeSystem(ZENg zEngine, double_t deltaTime) {
    for (Uint64 i = 0; i < zEngine->gEcs->components[LIFETIME_COMPONENT].denseSize; i++) {
        LifetimeComponent *lifeComp = (LifetimeComponent *)(zEngine->gEcs->components[LIFETIME_COMPONENT].dense[i]);
        lifeComp->timeAlive += deltaTime;
        if (lifeComp->timeAlive >= lifeComp->lifeTime) {
            Entity owner = zEngine->gEcs->components[LIFETIME_COMPONENT].denseToEntity[i];
            deleteEntity(zEngine->gEcs, owner);
        }
    }   
}

void handleCollision(ZENg zEngine, CollisionComponent *AColComp, CollisionComponent *BColComp, Entity AOwner, Entity BOwner) {
    if (!AColComp->isSolid && !BColComp->isSolid) {
        // two bullets collide - skip collision check
        return;
    }

    Uint64 APage = AOwner / PAGE_SIZE;
    Uint64 AIndex = AOwner % PAGE_SIZE;
    Uint64 ADenseIndex = zEngine->gEcs->components[PROJECTILE_COMPONENT].sparse[APage][AIndex];
    ProjectileComponent *AProjComp = (ProjectileComponent *)(zEngine->gEcs->components[PROJECTILE_COMPONENT].dense[ADenseIndex]);
    
    Uint64 BPage = BOwner / PAGE_SIZE;
    Uint64 BIndex = BOwner % PAGE_SIZE;
    Uint64 BDenseIndex = zEngine->gEcs->components[PROJECTILE_COMPONENT].sparse[BPage][BIndex];
    ProjectileComponent *BProjComp = (ProjectileComponent *)(zEngine->gEcs->components[PROJECTILE_COMPONENT].dense[BDenseIndex]);

    // if a bullet hits a solid entity, delete the bullet and do damage
    if (AColComp->role == COL_BULLET && BColComp->isSolid && BOwner != PLAYER_ID && AProjComp) {
        Uint64 HPage = BOwner / PAGE_SIZE;
        Uint64 HIdx = BOwner % PAGE_SIZE;
        Uint64 HDenseIdx = zEngine->gEcs->components[HEALTH_COMPONENT].sparse[HPage][HIdx];
        HealthComponent *actorHealth = (HealthComponent *)(zEngine->gEcs->components[HEALTH_COMPONENT].dense[HDenseIdx]);
        if (actorHealth && actorHealth->active) {
            actorHealth->currentHealth -= AProjComp->dmg;
            printf("Bullet (%lu) takes %u damage from entity %lu\n", AOwner, AProjComp->dmg, BOwner);
        }
        deleteEntity(zEngine->gEcs, AOwner);  // delete the bullet
    } else if (BColComp->role == COL_BULLET && AColComp->isSolid && AOwner != PLAYER_ID && BProjComp) {
        Uint64 HPage = AOwner / PAGE_SIZE;
        Uint64 HIdx = AOwner % PAGE_SIZE;
        Uint64 HDenseIdx = zEngine->gEcs->components[HEALTH_COMPONENT].sparse[HPage][HIdx];
        
        HealthComponent *actorHealth = (HealthComponent *)(zEngine->gEcs->components[HEALTH_COMPONENT].dense[HDenseIdx]);
        if (actorHealth && actorHealth->active) {
            actorHealth->currentHealth -= BProjComp->dmg;
            printf("Bullet (%lu) takes %u damage from entity %lu\n", BOwner, BProjComp->dmg, AOwner);
        }
        deleteEntity(zEngine->gEcs, BOwner);  // delete the bullet
    }
}

void collisionSystem(ZENg zEngine) {
    for (Uint64 i = 0; i < zEngine->gEcs->components[COLLISION_COMPONENT].denseSize; i++) {
        CollisionComponent *AColComp = (CollisionComponent *)(zEngine->gEcs->components[COLLISION_COMPONENT].dense[i]);
        Entity AOwner = zEngine->gEcs->components[COLLISION_COMPONENT].denseToEntity[i];

        if (!AColComp || !AColComp->hitbox) {
            printf("Warning: Entity %ld has an invalid collision component\n", AOwner);
            continue;
        }

        int wW, wH;
        SDL_GetWindowSize(zEngine->window, &wW, &wH);
        // if a bullet hits the screen edge - remove it
        if (AColComp->role == COL_BULLET && (AColComp->hitbox->x < 0
            || AColComp->hitbox->y < 0
            || AColComp->hitbox->x + AColComp->hitbox->w > wW
            || AColComp->hitbox->y + AColComp->hitbox->h > wH)
        ) {
            deleteEntity(zEngine->gEcs, AOwner);
            continue;  // skip to the next entity
        }

        // Check collisions with other entities
        for (Uint64 j = i + 1; j < zEngine->gEcs->components[COLLISION_COMPONENT].denseSize; j++) {
            CollisionComponent *BColComp = (CollisionComponent *)(zEngine->gEcs->components[COLLISION_COMPONENT].dense[j]);
            Entity BOwner = zEngine->gEcs->components[COLLISION_COMPONENT].denseToEntity[j];

            if (!BColComp || !BColComp->hitbox) {
                printf("Warning: Entity %ld has an invalid collision component\n", BOwner);
                continue;
            }

            // Check for intersections
            if (SDL_HasIntersection(AColComp->hitbox, BColComp->hitbox)) {
                handleCollision(zEngine, AColComp, BColComp, AOwner, BOwner);
            }
        }
    }
}

void healthSystem(ZENg zEngine) {
    for (Uint64 i = 0; i < zEngine->gEcs->components[HEALTH_COMPONENT].denseSize; i++) {
        HealthComponent *healthComp = (HealthComponent *)(zEngine->gEcs->components[HEALTH_COMPONENT].dense[i]);
        Entity ownerID = zEngine->gEcs->components[HEALTH_COMPONENT].denseToEntity[i];

        if (ownerID == PLAYER_ID) {
            printf("PLAYER has %u/%u health\n", healthComp->currentHealth, healthComp->maxHealth);
        } else {
            printf("Entity %ld has %u/%u health\n", ownerID, healthComp->currentHealth, healthComp->maxHealth);
        }
        if (healthComp->currentHealth <= 0) {
            deleteEntity(zEngine->gEcs, ownerID);
        }
    }
}

void transformSystem(ECS ecs) {
    // iterate through all entities with POSITION_COMPONENT and update their rendered textures
    for (Uint64 i = 0; i < ecs->components[POSITION_COMPONENT].denseSize; i++) {
        PositionComponent *posComp = (PositionComponent *)(ecs->components[POSITION_COMPONENT].dense[i]);
        Entity entitty = ecs->components[POSITION_COMPONENT].denseToEntity[i];  // get the owner entity

        if ((ecs->componentsFlags[entitty] & (1 << RENDER_COMPONENT)) == (1 << RENDER_COMPONENT)) {
            // if the entity has also the render component
            Uint64 page = entitty / PAGE_SIZE;  // determine the page for the entity
            Uint64 index = entitty % PAGE_SIZE;  // determine the index within the page
            Uint64 denseIndex = ecs->components[RENDER_COMPONENT].sparse[page][index];
            if (denseIndex >= ecs->components[RENDER_COMPONENT].denseSize) {
                printf("Warning: Entity %ld has a render component with invalid dense index %lu\n", entitty, denseIndex);
                continue;
            }
            // Update the render component's destination rectangle based on the position component
            RenderComponent *renderComp = (RenderComponent *)(ecs->components[RENDER_COMPONENT].dense[denseIndex]);

            if (renderComp) {  // sanity check cause I've been pretty insane lately
                renderComp->destRect->x = (int)posComp->x;
                renderComp->destRect->y = (int)posComp->y;
            }
        }
    }
}

void destroyEngine(ZENg *zEngine) {
    // save the current input bindings to the config file
    saveKeyBindings((*zEngine)->inputMng, "keys.cfg");
    free((*zEngine)->inputMng);

    freeECS((*zEngine)->gEcs);
    freeECS((*zEngine)->uiEcs);
    freeFonts(&((*zEngine)->fonts));

    SDL_DestroyRenderer((*zEngine)->renderer);
    SDL_DestroyWindow((*zEngine)->window);
    SDL_Quit();
    free(*zEngine);
}