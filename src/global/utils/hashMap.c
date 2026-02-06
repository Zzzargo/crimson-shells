#include "global/debug.h"
#include "global/global.h"

HashMap MapInit(size_t size, MapType type) {
    HashMap map = malloc(sizeof(struct map));
    ASSERT(map != NULL, "Failed to allocate memory for HashMap");

    map->entries = calloc(size, sizeof(MapEntry*));
    ASSERT(map->entries != NULL, "Failed to allocate memory for HashMap entries");

    map->size = size;
    map->type = type;
    return map;
}

size_t hashFunc(const char *key, size_t mapSize) {
    size_t hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash % mapSize;
}

/**
 * =====================================================================================================================
*/

MapEntry* MapGetEntry(HashMap map, const char *key) {
    ASSERT(map && key, "map = %p, key = %p\n", map, key);

    size_t index = hashFunc(key, map->size);
    MapEntry *entry = map->entries[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry;
        }
        entry = entry->next;
    }

    LOG(ERROR, "Entry with key '%s' not found in map of type %d\n", key, map->type);
    return NULL;
}

/**
 * =====================================================================================================================
*/

void MapAddEntry(HashMap map, const char *key, MapEntryVal data, MapEntryType type) {
    ASSERT(map && key, "map = %p, key = %p\n", map, key);

    size_t idx = hashFunc(key, map->size);
    MapEntry *new = calloc(1, sizeof(MapEntry));
    ASSERT(new != NULL, "Failed to allocate memory for new map entry");

    new->key = strdup(key);  // Safely copy the key
    ASSERT(new->key != NULL, "Failed to allocate memory for new map entry key");

    new->data = data;
    new->type = type;
    new->next = map->entries[idx];  // The new entry is added at the beginning of the chain
    map->entries[idx] = new;
}

/**
 * =====================================================================================================================
*/

void MapRemoveEntry(HashMap map, const char *key) {
    size_t idx = hashFunc(key, map->size);

    MapEntry *entry = map->entries[idx];
    MapEntry *prev = NULL;

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            // Found the entry to remove
            if (prev) {
                prev->next = entry->next;
            } else {
                // If this is the first entry in the chain
                map->entries[idx] = entry->next;  // Update the head
            }
            free(entry->key);  // Free the key string
            free(entry);  // Free the entry itself
            return;
        }
        prev = entry;
        entry = entry->next;  // Go to the next entry in the chain
    }

    LOG(ERROR, "Entry with key '%s' not found for removal in map of type %d\n", key, map->type);
}

/**
 * =====================================================================================================================
*/

void MapFree(HashMap map) {
    ASSERT(map != NULL, "Map is NULL. Can't free");

    for (size_t i = 0; i < map->size; i++) {
        MapEntry *entry = map->entries[i];
        while (entry) {
            MapEntry *next = entry->next;
            free(entry->key);  // Free the key string
            free(entry);  // Free the entry itself
            entry = next;  // Move to the next entry in the chain
        }
    }

    free(map->entries);
    free(map);
}
