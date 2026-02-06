#include "componentFactory.h"

DirectionComponent* createDirectionComponent(DirectionComponent dir) {
    DirectionComponent *comp = calloc(1, sizeof(DirectionComponent));
    if (!comp) {
        printf("Failed to allocate memory for direction component\n");
        exit(EXIT_FAILURE);
    }
    *comp = dir;
    return comp;
}

/**
 * =====================================================================================================================
 */

PositionComponent* createPositionComponent(PositionComponent pos) {
    PositionComponent *comp = calloc(1, sizeof(PositionComponent));
    if (!comp) {
        printf("Failed to allocate memory for position component\n");
        exit(EXIT_FAILURE);
    }
    *comp = pos;
    return comp;
}


/**
 * =====================================================================================================================
 */

VelocityComponent* createVelocityComponent(Vec2 velocity, double_t maxVelocity, PositionComponent predictedPos, Axis lastAxis, Uint8 active) {
    VelocityComponent *comp = calloc(1, sizeof(VelocityComponent));
    if (!comp) {
        printf("Failed to allocate memory for velocity component\n");
        exit(EXIT_FAILURE);
    }
    comp->currVelocity = velocity;
    comp->maxVelocity = maxVelocity;
    comp->predictedPos = predictedPos;
    comp->prevAxis = lastAxis;
    comp->active = active;
    return comp;
}

/**
 * =====================================================================================================================
 */

HealthComponent* createHealthComponent(Int32 maxHealth, Int32 currentHealth, Uint8 active) {
    HealthComponent *comp = calloc(1, sizeof(HealthComponent));
    if (!comp) {
        printf("Failed to allocate memory for health component\n");
        exit(EXIT_FAILURE);
    }
    comp->maxHealth = maxHealth;
    comp->currentHealth = currentHealth;
    comp->active = active;
    return comp;
}

/**
 * =====================================================================================================================
 */

CollisionComponent* createCollisionComponent(int x, int y, int w, int h, Uint8 isSolid, CollisionRole role) {
    CollisionComponent *comp = calloc(1, sizeof(CollisionComponent));
    if (!comp) {
        printf("Failed to allocate memory for collision component\n");
        exit(EXIT_FAILURE);
    }
    comp->hitbox = calloc(1, sizeof(SDL_Rect));
    if (!comp->hitbox) {
        printf("Failed to allocate memory for collision hitbox\n");
        exit(EXIT_FAILURE);
    }
    comp->hitbox->x = x;
    comp->hitbox->y = y;
    comp->hitbox->w = w;
    comp->hitbox->h = h;
    comp->isSolid = isSolid;
    comp->role = role;
    return comp;
}

/**
 * =====================================================================================================================
 */

RenderComponent* createRenderComponent(SDL_Texture *texture, int x, int y, int w, int h, Uint8 active) {
    RenderComponent *comp = calloc(1, sizeof(RenderComponent));
    if (!comp) {
        printf("Failed to allocate memory for render component\n");
        exit(EXIT_FAILURE);
    }
    comp->texture = texture;
    comp->active = active;

    comp->destRect = calloc(1, sizeof(SDL_Rect));
    if (!comp->destRect) {
        printf("Failed to allocate memory for render destination rectangle\n");
        exit(EXIT_FAILURE);
    }

    *comp->destRect = (SDL_Rect) {
        .x = x,
        .y = y,
        .w = w,
        .h = h
    };

    return comp;
}

/**
 * =====================================================================================================================
 */

LoadoutComponent *createLoadoutComponent(Entity primaryGun, CDLLNode *currSecondaryGun, Entity hull, Entity module) {
    LoadoutComponent *comp = calloc(1, sizeof(LoadoutComponent));
    if (!comp) {
        printf("Failed to allocate memory for loadout component\n");
        exit(EXIT_FAILURE);
    }
    comp->primaryGun = primaryGun;
    comp->currSecondaryGun = currSecondaryGun;
    comp->hull = hull;
    comp->module = module;
    return comp;
}

/**
 * =====================================================================================================================
 */

ProjectileComponent *createProjectileComponent(Int32 dmg, Uint8 piercing, Uint8 exploding, Uint8 friendly) {
    ProjectileComponent *comp = calloc(1, sizeof(ProjectileComponent));
    if (!comp) {
        printf("Failed to allocate memory for projectile component\n");
        exit(EXIT_FAILURE);
    }
    comp->dmg = dmg;
    comp->piercing = piercing;
    comp->exploding = exploding;
    comp->friendly = friendly;
    return comp;
}
