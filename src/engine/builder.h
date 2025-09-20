#ifndef BUILDER_H
#define BUILDER_H

#include "../engine/arena.h"

typedef enum {
    BUILDER_TANKS,
    BUILDER_WEAPONS,
    BUILDER_TILES,
    BUILDER_COUNT  // automatically counts
} BuilderType;

/**
 * This enum is used to differentiate between entity types only at creation, for example in arena initialization
 * It should never be added as a component to an entity for the sake of ECS purity
*/
typedef enum {
    ENTITY_PLAYER,  // Your tank
    ENTITY_TANK_BASIC,
    ENTITY_TANK_LIGHT,
    ENTITY_TANK_HEAVY,
    ENTITY_TYPE_COUNT  // Automatically counts
} EntityType;

typedef struct {
    char *name;
    char *projTexturePath;  // path to the projectile texture
    char *projHitSoundPath;  // path to the projectile hit sound
    double_t fireRate;  // shots per second
    double_t projSpeed;  // projectile speed
    double_t projLifeTime;  // projectile lifetime in seconds
    int projW;  // projectile width
    int projH;  // projectile height
    int dmg;  // damage dealt by the projectile
    Uint8 isPiercing;  // does the projectile pierce through targets
    Uint8 isExplosive;  // does the projectile explode on impact
} WeaponPrefab;

typedef struct {
    char *name;
    EntityType entityType;
    Int32 maxHealth;
    double_t maxSpeed;
    int w;  // Width of the tank sprite (in tiles)
    int h;  // Height of the tank sprite (in tiles)
    Uint8 isSolid;  // Is the tank solid (for collisions)
    char *texturePath;  // Path to the tank texture
} TankPrefab;

#ifndef PREFAB_HASHMAP_SIZE
#define PREFAB_HASHMAP_SIZE 256
#endif

typedef struct buildEntry {
    char *key;  // An entry's key is the prefab name
    void *data;  // Pointer to the (parsed) prefab data
    struct buildEntry *next;  // This hashmap uses chaining for collision resolution
    BuilderType type;  // Type of the entity the prefab represents
} BuilderEntry;

// Prefabs hashmap
typedef struct prefabsmng {
    BuilderEntry *hashmap[PREFAB_HASHMAP_SIZE];
} *PrefabsManager;

/**
 * Allocates memory for a new PrefabsManager
 * @param prefabmng pointer to the PrefabsManager = struct prefabsmng**
 */
void initPrefabsManager(PrefabsManager *prefabmng);

/**
 * Hashing function for this hashmap
 * Uses djb2
 * @param key the string to hash
 * @return the index in the hashmap
 */
static inline Uint32 hashFunc(const char *key);

/**
 * Finds a prefab in the hashmap
 * @param prefabMng the PrefabsManager = struct prefabsmng*
 * @param key the prefab's key
 * @return the BuilderEntry if found, NULL otherwise
 */
BuilderEntry* getPrefab(PrefabsManager prefabMng, const char *key);

/**
 * Gets a weapon prefab from the PrefabsManager
 * @param prefabMng the PrefabsManager = struct prefabsmng*
 * @param key the prefab's key
 * @return pointer to the WeaponPrefab if found, NULL otherwise
 */
WeaponPrefab* getWeaponPrefab(PrefabsManager prefabMng, const char *key);

/**
 * Gets a tile prefab from the PrefabsManager
 * @param prefabMng the PrefabsManager = struct prefabsmng*
 * @param key the prefab's key
 * @return a tile definition if found, NULL otherwise
 */
Tile getTilePrefab(PrefabsManager prefabMng, const char *key);

/**
 * Gets a tank prefab from the PrefabsManager
 * @param prefabMng the PrefabsManager = struct prefabsmng*
 * @param key the prefab's key
 * @return pointer to the TankPrefab if found, NULL otherwise
 */
TankPrefab* getTankPrefab(PrefabsManager prefabMng, const char *key);

/**
 * Adds a prefab entry to the hashmap
 * @param prefabMng the PrefabsManager = struct prefabsmng*
 * @param key the prefab's key
 * @param data pointer to the prefab data
 * @param type the type of the prefab (BuilderType enum)
 */
void addPrefab(PrefabsManager prefabMng, const char *key, void *data, BuilderType type);

/**
 * Removes a prefab entry from the hashmap
 * @param prefabMng the PrefabsManager = struct prefabsmng*
 * @param key the prefab's key
 */
void removePrefab(PrefabsManager prefabMng, const char *key);

/**
 * Preloads prefabs from a file into the PrefabsManager
 * @param zEngine pointer to the engine = struct engine*
 * @param filePath path to the prefab file
 * @note The prefab file should have lines in the format TYPE:KEY:DATA
 */
void loadPrefabs(ZENg zEngine, const char *filePath);

/**
 * Frees all the memory used by the PrefabsManager
 * @param prefabmng pointer to the PrefabsManager = struct prefabsmng**
 */
void freePrefabsManager(PrefabsManager *prefabmng);

// =====================================================================================================================

/**
 * Creates a weapon prefab
 * @param name the name of the weapon
 * @param fireRate shots per second
 * @param projW projectile width
 * @param projH projectile height
 * @param projSpeed projectile speed
 * @param dmg damage dealt by the projectile
 * @param isPiercing does the projectile pierce through targets
 * @param isExplosive does the projectile explode on impact
 * @param projLifeTime projectile lifetime in seconds
 * @param projTexturePath path to the projectile texture
 * @param projHitSoundPath path to the projectile hit sound
 * @return pointer to the created WeaponPrefab
 * @note the strings should be allocated before calling this function
 */
WeaponPrefab* createWeaponPrefab(
    const char *name,
    double_t fireRate,
    int projW,
    int projH,
    double_t projSpeed,
    int dmg,
    Uint8 isPiercing,
    Uint8 isExplosive,
    double_t projLifeTime,
    const char *projTexturePath,
    const char *projHitSoundPath
);

/**
 * Creates a tank prefab
 * @param name the name of the tank
 * @param entityType the type of the tank entity (EntityType enum)
 * @param maxHealth the maximum health of the tank
 * @param maxSpeed the maximum speed of the tank
 * @param w width of the tank sprite (in tiles)
 * @param h height of the tank sprite (in tiles)
 * @param isSolid is the tank solid (for collisions)
 * @param texturePath path to the tank texture
 * @return pointer to the created TankPrefab
 * @note the strings should be allocated before calling this function
 */
TankPrefab* createTankPrefab(
    const char *name,
    EntityType entityType,
    Int32 maxHealth,
    double_t maxSpeed,
    int w,
    int h,
    Uint8 isSolid,
    const char *texturePath
);

#endif // BUILDER_H