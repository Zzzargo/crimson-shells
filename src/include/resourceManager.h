#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "global.h"

typedef enum {
    RESOURCE_TEXTURE,
    RESOURCE_SOUND,
    RESOURCE_MUSIC,
    RESOURCE_FONT,
    RESOURCE_COUNT  // automatically counts the number of resource types
} ResourceType;

#define HASHMAP_SIZE 256  // size of the resource hash map

typedef struct resEntry {
    char *key;  // an entry's key is the resource's path
    void *resource;  // pointer to the resource
    struct resEntry *next;  // this hashmap uses chaining for collision resolution
    ResourceType type;  // type of the resource
} ResourceEntry;

typedef struct resmng {
    ResourceEntry *hashmap[HASHMAP_SIZE];  // resource hashmap
} *ResourceManager;

// allocates memory for a new ResourceManager
void initResourceManager(ResourceManager *resMng);

// hashes a string (resource path) to an index in the hashmap
// uses djb2
static inline Uint32 hashFunc(const char *key);

// finds a resource in the hashmap
static inline ResourceEntry* getResource(ResourceManager resMng, const char *key);

// adds a resource to the hashmap
void addResource(ResourceManager resMng, const char *key, void *resource, ResourceType type);

// removes a resource from the hashmap
void removeResource(ResourceManager resMng, const char *key);

// frees all the memory used by the ResourceManager
void freeResourceManager(ResourceManager *resMng);

// retrieves a texture resource from the ResourceManager
SDL_Texture* getTexture(ResourceManager resMng, const char *key);

// retrieves a font resource from the ResourceManager
TTF_Font* getFont(ResourceManager resMng, const char *key);

// retrieves a sound resource from the ResourceManager
Mix_Chunk *getSound(ResourceManager resMng, const char *key);

/**
 * 
 */
Mix_Music *getMusic(ResourceManager resMng, const char *key);

// retrieves a resource from the ResourceManager if it's there, otherwise loads it
void* getOrLoadResource(ResourceManager resMng, SDL_Renderer *renderer, const char *key, ResourceType type);

// retrieves a texture resource from the ResourceManager or loads it if not found
SDL_Texture* getOrLoadTexture(ResourceManager resMng, SDL_Renderer *renderer, const char *key);

// retrieves a sound resource from the ResourceManager or loads it if not found
Mix_Chunk *getOrLoadSound(ResourceManager resMng, const char *key);

// preloads the frequently used resources
void preloadResources(ResourceManager resMng, SDL_Renderer *renderer);

#endif // RESOURCE_MANAGER_H