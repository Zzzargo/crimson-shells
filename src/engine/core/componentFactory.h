#ifndef COMPONENT_FACTORY_H
#define COMPONENT_FACTORY_H
#include "ecs.h"

/**
 * Creates a direction component
 * @param dir a direction vector
 * @return a pointer to a DirectionComponent
 */
DirectionComponent* createDirectionComponent(DirectionComponent dir);

/**
 * Creates a position component
 * @param pos a position vector
 * @return a pointer to a PositionComponent
 */
PositionComponent* createPositionComponent(PositionComponent pos);

/**
 * Creates a velocity component
 * @param velocity the current velocity vector
 * @param maxVelocity the maximum speed of the entity
 * @param predictedPos the predicted position based on the current velocity and direction
 * @param lastAxis the last axis the entity moved on
 * @param active indicates if the component is active
 * @return a pointer to a VelocityComponent
 */
VelocityComponent* createVelocityComponent(
    Vec2 velocity, double_t maxVelocity, PositionComponent predictedPos, Axis lastAxis, Uint8 active
);

/**
 * Creates a health component
 * @param maxHealth the maximum health of the entity
 * @param currentHealth the current health of the entity
 * @param active indicates if the component is active
 * @return a pointer to a HealthComponent
 */
HealthComponent* createHealthComponent(Int32 maxHealth, Int32 currentHealth, Uint8 active);

/**
 * Creates a collision component
 * @param x the x coordinate of the hitbox
 * @param y the y coordinate of the hitbox
 * @param w the width of the hitbox
 * @param h the height of the hitbox
 * @param isSolid indicates if the entity can be passed through
 * @param role the role of the entity in the collision
 * @return a pointer to a CollisionComponent
 */
CollisionComponent* createCollisionComponent(int x, int y, int w, int h, Uint8 isSolid, CollisionRole role);

/**
 * Creates a render component
 * @param texture the texture to render
 * @param x the x coordinate of the destination rectangle
 * @param y the y coordinate of the destination rectangle
 * @param w the width of the destination rectangle
 * @param h the height of the destination rectangle
 * @param active indicates if the component is active
 * @return a pointer to a RenderComponent
 */
RenderComponent* createRenderComponent(SDL_Texture *texture, int x, int y, int w, int h, Uint8 active);

/**
 * Creates a loadout component
 * @param primaryGun entity ID of the primary weapon
 * @param currSecondaryGun pointer to a circular doubly linked list node, holding the current weapon
 * @param hull entity ID of the hull
 * @param module entity ID of the equipped module
 * @return a pointer to a WeaponComponent
 */
LoadoutComponent *createLoadoutComponent(Entity primaryGun, CDLLNode *currSecondaryGun, Entity hull, Entity module);

/**
 * Creates a projectile component
 * @param dmg damage the projectile will deal on hit
 * @param piercing whether the projectile can pierce through entities (1) or not (0)
 * @param exploding whether the projectile explodes on impact (1) or not (0)
 * @param friendly if the projectile does damage to the player
 * @return a pointer to a ProjectileComponent
 */
ProjectileComponent *createProjectileComponent(Int32 dmg, Uint8 piercing, Uint8 exploding, Uint8 friendly);

#endif // COMPONENT_FACTORY_H