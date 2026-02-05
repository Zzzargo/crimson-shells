#ifndef HASHMAP_H
#define HASHMAP_H

// Generic Hash Map implementation. Maps are distinguished by map type, so do entries

#include <stdlib.h>

typedef enum {
    MAP_RESOURCES,
    MAP_PREFABS,
    MAP_PARSER,
    MAP_STATE_DATA
} MapType;

typedef enum {
    ENTRY_TEXTURE,
    ENTRY_SOUND,
    ENTRY_FONT,
    ENTRY_TANK_PREFAB,
    ENTRY_WEAPON_PREFAB,
    ENTRY_TILE_PREFAB,
    ENTRY_BTN_FUNC,
    ENTRY_COLOR,
    ENTRY_PROVIDER_FUNC,
    ENTRY_PROVIDER_RESULT
} MapEntryType;

// This boi right there can hold the Earth if he wants to
typedef union {
    void *ptr;  // For bigger data
    Uint64 u64;
    Sint64 i64;
    double_t d;
    float f;
    Uint32 u32;
    Sint32 i32;
    Uint16 u16;
    Sint16 i16;
    Uint8 boolean;
    Sint8 i8;
    SDL_Color color;  // For ease of use
} MapEntryVal;

typedef struct MapEntry {
    char *key;
    MapEntryVal data;
    struct MapEntry *next;  // This hashmap uses chaining for collision resolution
    MapEntryType type;  // type of the entry
} MapEntry;

typedef struct map {
    MapEntry **entries;  // Array of entry pointers
    size_t size;
    MapType type;
} *HashMap;

/**
 * Allocates memory for a new HashMap
 * @param size the size of the hashmap
 * @param type the type of the map
 * @return the created HashMap
 * @note the size should not be modified after creation
 */
HashMap MapInit(size_t size, MapType type);

/**
 * Hashes a string (a path usually) to an index in the hashmap
 * @param key the string to hash
 * @param mapSize the size of the hashmap
 * @return the index in the hashmap
 * @note uses djb2
*/
size_t hashFunc(const char *key, size_t mapSize);

/**
 * Fetch an entry in the hashmap
 * @param map the HashMap = struct map*
 * @param key the entry's key
 * @return the MapEntry if found, NULL otherwise
*/
MapEntry* MapGetEntry(HashMap map, const char *key);

/**
 * Adds an entry to the hashmap
 * @param map the HashMap = struct map*
 * @param key the entry's key
 * @param data the entry's data
 * @param type the entry's type
*/
void MapAddEntry(HashMap map, const char *key, MapEntryVal data, MapEntryType type);

/**
 * Removes an entry from the hashmap
 * @param map the HashMap = struct map*
 * @param key the entry's key
*/
void MapRemoveEntry(HashMap map, const char *key);

/**
 * Frees all the memory used by the HashMap
 * @param map the HashMap = struct map*
 * @note this is the generic free function, it only frees the key and the entry pointers
*/
void MapFree(HashMap map);

#endif  // HASHMAP_H