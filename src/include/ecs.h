#ifndef ECS_H
#define ECS_H

// ECS (Entity Component System) header file
// Little game - big ideas
// Using sparse sets with pagination for efficiency

#include "global.h"

typedef Uint64 Entity;  // In an ECS, an entity is just an ID
extern Entity PLAYER_ID;  // Global variable for the player entity ID
typedef Uint64 bitset;  // A bitset to indicate which components an entity has

// Available component types enum
typedef enum {
    HEALTH_COMPONENT,
    POSITION_COMPONENT,
    VELOCITY_COMPONENT,
    DIRECTION_COMPONENT,
    PROJECTILE_COMPONENT,
    LIFETIME_COMPONENT,
    COLLISION_COMPONENT,
    TEXT_COMPONENT,  // From here more UI-based components
    BUTTON_COMPONENT,
    RENDER_COMPONENT,
    COMPONENT_COUNT  // Automatically counts
} ComponentType;

#define PAGE_SIZE 64  // Size of a page of a component's sparse array

// General definition of a component with its sparse and dense arrays
typedef struct {
    Uint64 **sparse;  // Sparse set - array of arrays  sparse[page][offset] = denseIndex
    void **dense;  // Dense set - dense[index] = (void *)component
    Entity *denseToEntity;  // Maps component in dense array to its owner entity's ID
    Uint64 denseSize;  // Current size of the dense array
    Uint64 pageCount;  // Number of pages allocated for this component
    ComponentType type;  // The type of the component, needed for further info on each component
} Component;

typedef struct {
    Int32 maxHealth; // Maximum health of the entity
    Int32 currentHealth; // Current health of the entity
    Uint8 active;  // Indicates if the component is active
} HealthComponent;

typedef Vec2 PositionComponent;  // Position in 2D space

typedef struct {
    Vec2 currVelocity;  // Current velocity
    double_t maxVelocity;  // Maximum speed of the entity
    Uint8 active;
} VelocityComponent;

// Moving directions - pay attention to the direction of the Y axis in SDL

#define DIR_LEFT (Vec2){-1.0, 0.0}
#define DIR_RIGHT (Vec2){1.0, 0.0}
#define DIR_UP (Vec2){0.0, -1.0}
#define DIR_DOWN (Vec2){0.0, 1.0}

typedef Vec2 DirectionComponent;

typedef struct {
    Int32 dmg;  // How much damage does the projectile do
    Uint8 piercing;
    Uint8 exploding;
    Uint8 friendly;  // Indicates whether the projectile can damage the player
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
    SDL_Rect *hitbox;  // The square where the entity can touch others
    CollisionRole role;  // The role of the entity in the collision
    Uint8 isSolid;  // Indicates if entities can pass through
} CollisionComponent;

typedef struct {
    SDL_Texture *texture;  // Texture to render
    SDL_Rect *destRect;  // Where to render the texture
    Uint8 active;
    Uint8 selected;  // For UI components
} RenderComponent;

typedef struct {
    SDL_Color color;  // Color of the text
    TTF_Font *font;  // Font to use for rendering text
    char *text;
    SDL_Texture *texture;  // Texture for the text
    SDL_Rect *destRect;  // Where to render the text
    Uint8 active;  // Indicates if the text component should be rendered
} TextComponent;

typedef struct engine *ZENg;  // forward declaration of the engine
typedef struct {
    SDL_Color color;  // Color of the button text
    void (*onClick)(ZENg);  // What the button does
    TTF_Font *font;  // Font used by the button text
    char *text;
    SDL_Texture *texture;
    SDL_Rect *destRect;  // Where to render the button
    Uint8 selected;  // For UI components
    Uint8 orderIdx;  // Used to maintain order in UI entities
} ButtonComponent;

#define INIT_CAPACITY 10  // Capacity with which the ECS is initialised

// ECS structure definition
typedef struct EeSiEs {
    char *name;  // Used to distinguish the ECS
    Entity nextEntityID;  // Next available entity ID
    Entity *activeEntities;  // Array of active entities
    Uint64 entityCount;  // Number of entities currently in the ECS
    Uint64 capacity;  // Current capacity of the ECS
    bitset *componentsFlags;  // An array of bitsets, one for each entity
    Component *components;  // Array of components, each component is a sparse set

    Entity *freeEntities;  // Array of free entities, used for recycling IDs
    Uint64 freeEntityCount;  // Number of free entities available for reuse
    Uint64 freeEntityCapacity;  // Capacity of the free entities array
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
 * Creates a text component for menus
 * @param rdr
 * @param font
 * @param text
 * @param color
 * @param active
 * @note the returned text component is created @ 0x0, so further positioning is needed
 */
TextComponent* createTextComponent(SDL_Renderer *rdr, TTF_Font *font, char *text, SDL_Color color, Uint8 active);

/**
 * Creates a button component for menus
 * @param rdr
 * @param font
 * @param text
 * @param color
 * @param onClick
 * @param selected
 * @param orderIdx
 * @note the returned button component is created @ 0x0, so further positioning is needed
 */
ButtonComponent* createButtonComponent(
    SDL_Renderer *rdr, TTF_Font *font, char *text, SDL_Color color,
    void (*onClick)(ZENg), Uint8 selected, Uint8 orderIdx
);

/**
 * 
 */
DirectionComponent* createDirectionComponent(DirectionComponent dir);

/**
 * 
 */

VelocityComponent* createVelocityComponent(Vec2 velocity, double_t maxVelocity, Uint8 active);

/**
 * 
 */
HealthComponent* createHealthComponent(Int32 maxHealth, Int32 currentHealth, Uint8 active);

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