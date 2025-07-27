#ifndef ECS_H
#define ECS_H

// ECS (Entity Component System) header file
// Little game - big ideas
// Using sparse sets with pagination for efficiency

#include "global.h"

typedef Uint64 Entity;  // in an ECS, an entity is just an ID
extern Entity PLAYER_ID;  // global variable for the player entity ID
typedef Uint64 bitset;  // a bitset to indicate which components an entity has

typedef enum {
    HEALTH_COMPONENT,
    POSITION_COMPONENT,
    VELOCITY_COMPONENT,
    DIRECTION_COMPONENT,
    PROJECTILE_COMPONENT,
    LIFETIME_COMPONENT,
    COLLISION_COMPONENT,
    TEXT_COMPONENT,  // from here more UI-based components
    RENDER_COMPONENT,
    COMPONENT_COUNT  // automatically counts
} ComponentType;

#define PAGE_SIZE 64  // size of a page of a component's sparse array
typedef struct {
    Uint64 **sparse;  // sparse set - array of arrays  sparse[page][offset] = denseIndex
    void **dense;  // dense set - dense[index] = (void *)component
    Entity *denseToEntity;  // maps component in dense array to its owner entity's ID
    Uint64 denseSize;  // current size of the dense array
    Uint64 pageCount;  // number of pages allocated for this component
    ComponentType type;  // the type of the component, needed for further info on each component
} Component;  // base component type

typedef struct {
    Int32 maxHealth; // maximum health of the entity
    Int32 currentHealth; // current health of the entity
    Uint8 active;  // indicates if the component is active
} HealthComponent;

typedef Vec2 PositionComponent;  // position in 2D space

typedef struct {
    Vec2 currVelocity;  // current velocity
    double_t maxVelocity;  // maximum speed of the entity
    Uint8 active;
} VelocityComponent;

// moving directions
#define DIR_LEFT (Vec2){-1.0, 0.0};
#define DIR_RIGHT (Vec2){1.0, 0.0};
#define DIR_UP (Vec2){0.0, -1.0};
#define DIR_DOWN (Vec2){0.0, 1.0};

typedef Vec2 DirectionComponent;

typedef struct {
    Int32 dmg;  // how much damage does the projectile do
    Uint8 piercing;
    Uint8 exploding;
    Uint8 friendly;  // indicates whether the projectile can damage the player
} ProjectileComponent;

typedef struct {
    double_t lifeTime;
    double_t timeAlive;
} LifetimeComponent;

typedef enum {
    COL_ACTOR,
    COL_BULLET,
    COL_WALL,
    COL_ITEM,
} CollisionRole;

typedef struct {
    SDL_Rect *hitbox;  // the square where the entity can touch others
    CollisionRole role;  // the role of the entity in the collision
    Uint8 isSolid;  // indicates if entities can pass through
} CollisionComponent;

typedef struct {
    SDL_Texture *texture;  // texture to render
    SDL_Rect *destRect;  // where to render the texture
    Uint8 active;
    Uint8 selected;  // for UI components
} RenderComponent;

typedef struct {
    Uint8 active;  // indicates if the text component is active
    Uint8 selected;  // for UI components
    Uint8 orderIdx;  // used to maintain order in ui entities
    TTF_Font *font;  // font to use for rendering text
    char *text;
    SDL_Color color;  // color of the text
    SDL_Texture *texture;  // texture for the text
    SDL_Rect *destRect;  // where to render the text
} TextComponent;

#define INIT_CAPACITY 10  // capacity with which the ECS is initialised
typedef struct EeSiEs {
    char *name;  // used to distinguish the ECS
    Entity nextEntityID;  // next available entity ID
    Entity *activeEntities;  // array of active entities
    Uint64 entityCount;  // number of entities currently in the ECS
    Uint64 capacity;  // current capacity of the ECS
    bitset *componentsFlags;  // an array of bitsets, one for each entity
    Component *components;  // array of components, each component is a sparse set

    Entity *freeEntities;  // array of free entities, used for recycling IDs
    Uint64 freeEntityCount;  // number of free entities available for reuse
    Uint64 freeEntityCapacity;  // capacity of the free entities array
} *ECS;

/**
 * Initialises the game ECS - more dynamic entities
 * @param gEcs pointer to an ECS = struct ecs**
 */
void initGECS(ECS *gEcs);

/**
 * Initialises the UI ECS - mostly static entities
 * @param uiEcs pointer to an ECS = struct ecs**
 */
void initUIECS(ECS *uiEcs);

/**
 * Creates a new entity in an ECS
 * @param ecs an ECS struct = struct ecs*
 * @return Entity(ID)
 * @note an entity in an ECS is just the ID (a number)
*/
Entity createEntity(ECS ecs);

/**
 * Adds a component to an entity in an ECS
 * @param ecs an ECS struct = struct ecs*
 * @param id ID of the owner entity
 * @param compType component type = enum variable
 * @param component address of a component to be added to the entity
*/
void addComponent(ECS ecs, Entity id, ComponentType compType, void *component);

/**
 * Deletes an entity along with all its associated components
 * @param ecs an ECS struct = struct ecs*
 * @param id the entity ID
 */
void deleteEntity(ECS ecs, Entity id);

/**
 * Frees the memory allocated for and inside an ECS
 * @param ecs an ECS struct = struct ecs*
 */
void freeECS(ECS ecs);

#endif // ECS_H