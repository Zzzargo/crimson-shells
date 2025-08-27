#ifndef ECS_H
#define ECS_H

// ECS (Entity Component System) header file
// Little game - big ideas
// Using sparse sets with pagination for efficiency

#include "global.h"

// =================================================ENTITIES============================================================

typedef Uint64 Entity;  // In an ECS, an entity is just an ID
extern Entity PLAYER_ID;  // Global variable for the player entity ID
typedef Uint64 bitset;  // A bitset to indicate which components an entity has

// ===============================================COMPONENTS============================================================

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

    Entity *dirtyEntities;  // Array of entities that need to be updated
    Uint64 dirtyCount;  // Number of dirty entities
    Uint64 dirtyCapacity;  // Capacity of the dirty entities array
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
    Axis prevAxis;  // The last axis the entity moved on
    PositionComponent predictedPos;  // holds the predicted position based on the current velocity and direction
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

// ===============================================SYSTEMS===============================================================

typedef enum {
    SYS_LIFETIME,  // Coarse-grained
    SYS_VELOCITY,  // Coarse-grained
    SYS_WORLD_COLLISIONS,  // Coarse-grained
    SYS_ENTITY_COLLISIONS,  // Coarse-grained
    SYS_POSITION,  // Coarse-grained
    SYS_HEALTH,  // Fine-grained
    SYS_TRANSFORM,  // Fine-grained
    SYS_RENDER,  // Coarse-grained
    SYS_BUTTONS,  // Fine-grained
    SYS_COUNT  // Automatically counts
} SystemType;

typedef struct sysNode {
    SystemType type;  // System identifier
    void (*update)(ZENg, double_t);  // Function pointer to the system's update function

    struct sysNode **dependents;  // Array of systems depending on this one
    Uint64 dependentsCount;  // Number of dependents

    struct sysNode **dependencies;  // Array of systems this one depends on
    Uint64 dependenciesCount;  // Number of dependencies

    // Indicates if the system is fine-grained (updates only dirty components) or coarse-grained(updates all components)
    Uint8 isFineGrained;
    Uint8 isDirty;  // For coarse-grained systems, indicates if the system needs to be updated
    Uint8 isActive;  // Indicates if this system is currently active
} SystemNode;

typedef struct depGraph {
    SystemNode **nodes;  // Array of all systems in the graph
    Uint64 nodeCount;  // Number of systems in the graph
    SystemNode **sortedNodes;  // Array of systems sorted by dependencies
} DependencyGraph;

// =====================================================================================================================

#define INIT_CAPACITY 10  // Capacity with which the ECS is initialised

// ECS structure definition
typedef struct EeSiEs {
    Entity nextEntityID;  // Next available entity ID
    Entity *activeEntities;  // Array of active entities
    Uint64 entityCount;  // Number of entities currently in the ECS
    Uint64 capacity;  // Current capacity of the ECS
    Entity *freeEntities;  // Array of free entities, used for recycling IDs
    Uint64 freeEntityCount;  // Number of free entities available for reuse
    Uint64 freeEntityCapacity;  // Capacity of the free entities array

    bitset *componentsFlags;  // An array of bitsets, one for each entity
    Component *components;  // Array of components, each component is a sparse set

    DependencyGraph *depGraph;  // Dependency graph for systems
} *ECS;

/**
 * Initialises the game ECS
 * @param Ecs pointer to the ECS = struct ecs**
 */
void initECS(ECS *Ecs);

/**
 * Frees the memory allocated for and inside an ECS
 * @param ecs an ECS struct = struct ecs*
 */
void freeECS(ECS ecs);

/**
 * Creates a new entity in an ECS
 * @param ecs an ECS struct = struct ecs*
 * @return Entity(ID)
 * @note an entity in an ECS is just the ID (a number)
*/
Entity createEntity(ECS ecs);

/**
 * Deletes an entity along with all its associated components
 * @param ecs an ECS struct = struct ecs*
 * @param id the entity ID
 */
void deleteEntity(ECS ecs, Entity id);

/**
 * Marks an entity's component as dirty, meaning it needs to be updated
 * @param ecs an ECS struct = struct ecs*
 * @param id ID of the owner entity
 * @param compType component type = enum variable
 */
void markComponentDirty(ECS ecs, Entity id, ComponentType compType);

/**
 * Propagates the dirty state through the dependency graph
 * @param node the system node to start propagation from
 * @note the propagation is done at system level, fine-grained dirtiness propagation is done only where needed
 */
void propagateSystemDirtiness(SystemNode *node);

/**
 * Deletes a component from the dirty array
 * @param ecs an ECS struct = struct ecs*
 * @param compType component type = enum variable
 * @note the first entity in the dirty array is removed, as the array acts like a queue
 */
void unmarkComponentDirty(ECS ecs, ComponentType compType);

/**
 * Creates a new system node
 * @param type the type of the system
 * @param update the function pointer to the system's update function
 * @param isFineGrained indicates if the system is fine-grained (1) or coarse-grained (0)
 * @return a pointer to the newly created SystemNode
 */
SystemNode* createSystemNode(SystemType type, void (*update)(ZENg, double_t), Uint8 isFineGrained);

/**
 * Adds a dependency between two system nodes
 * @param dependency the system node that will be updated first
 * @param dependent the system node that depends on the other one
 * @note Update order is dependency -> dependent
 */
void addSystemDependency(SystemNode *dependency, SystemNode *dependent);

/**
 * Creates a new dependency graph for systems
 * @return a pointer to the newly created DependencyGraph
 * @note the plain systems are added to the graph inside this function
 */
DependencyGraph* initDependencyGraph();

/**
 * Helper that maps a component type to its corresponding system type
 * @param compType component type = enum variable
 * @return the corresponding system type = enum variable
 */
SystemType componentToSystem(ComponentType compType);

/**
 * Adds a system node to the dependency graph
 * @param graph the dependency graph
 * @param node the system node to be added
 * @note the graph needs to be sorted after adding all systems
 */
void insertSystem(DependencyGraph *graph, SystemNode *node);

/**
 * Performs a topological sort by Kahn's algorithm on the dependency graph
 * @param graph pointer to the dependency graph
 * @note the function allocates and populates the sortedNodes array in the graph
 */
void kahnTopSort(DependencyGraph *graph);

/**
 * Runs the active systems for the current frame
 * @param zEngine pointer to the engine
 * @param deltaTime time since the last frame in seconds
 * @note a topological sort must be done on the dependency graph prior to calling this function
 */
void runSystems(ZENg zEngine, double_t deltaTime);

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
 * @param selected indicates if the component is selected (for UI components)
 * @return a pointer to a RenderComponent
 */
RenderComponent* createRenderComponent(SDL_Texture *texture, int x, int y, int w, int h, Uint8 active, Uint8 selected);

/**
 * Adds a component to an entity in an ECS
 * @param ecs an ECS struct = struct ecs*
 * @param id ID of the owner entity
 * @param compType component type = enum variable
 * @param component address of a component to be added to the entity
*/
void addComponent(ECS ecs, Entity id, ComponentType compType, void *component);

#endif // ECS_H