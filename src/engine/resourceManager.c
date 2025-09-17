#include "resourceManager.h"

void initResourceManager(ResourceManager *resMng) {
    // allocate memory for the ResourceManager
    *resMng = calloc(1, sizeof(struct resmng));
    if (!*resMng) {
        fprintf(stderr, "Failed to allocate memory for ResourceManager\n");
        exit(EXIT_FAILURE);
    }
}

/**
 * =====================================================================================================================
 */

static inline Uint32 hashFunc(const char *key) {
    Uint32 hash = 5381;
    int c;

    while (c = *key++)
        hash = ((hash << 5) + hash) + (Uint32)c; // hash * 33 + c

    return hash % HASHMAP_SIZE;
}

/**
 * =====================================================================================================================
 */

static inline ResourceEntry* getResource(ResourceManager resMng, const char *key) {
    Uint32 idx = hashFunc(key);

    ResourceEntry *entry = resMng->hashmap[idx];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry;  // found the resource
        }
        entry = entry->next;  // go to the next entry in the chain if there was a collision
    }
    return NULL;  // resource not found
}

/**
 * =====================================================================================================================
 */

void addResource(ResourceManager resMng, const char *key, void *resource, ResourceType type) {
    Uint32 idx = hashFunc(key);  // index at which to add the resource
    ResourceEntry *new = calloc(1, sizeof(ResourceEntry));
    if (!new) {
        fprintf(stderr, "Failed to allocate memory for new resource entry\n");
        exit(EXIT_FAILURE);
    }

    new->key = strdup(key);  // safely copy the key
    if (!new->key) {
        fprintf(stderr, "Failed to allocate memory for resource key\n");
        free(new);
        exit(EXIT_FAILURE);
    }

    new->resource = resource;  // set the resource pointer
    new->type = type;  // set the resource type
    new->next = resMng->hashmap[idx];  // the new entry is added at the beginning of the chain
    resMng->hashmap[idx] = new;  // update the hashmap to point to the new entry
}

/**
 * =====================================================================================================================
 */

void removeResource(ResourceManager resMng, const char *key) {
    Uint32 idx = hashFunc(key);

    ResourceEntry *entry = resMng->hashmap[idx];
    ResourceEntry *prev = NULL;

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            // found the resource to remove
            if (prev) {
                prev->next = entry->next;  // link the previous entry to the one after the current
            } else {
                // if this is the first entry in the chain
                resMng->hashmap[idx] = entry->next;  // update the head of the chain
            }
            free(entry->key);  // free the key string
            free(entry);  // free the entry itself
            return;
        }
        prev = entry;
        entry = entry->next;  // go to the next entry in the chain
    }
    // if we reach here, the resource was not found
    fprintf(stderr, "Resource with key '%s' not found for removal\n", key);
}

/**
 * =====================================================================================================================
 */

SDL_Texture* getTexture(ResourceManager resMng, const char *key) {
    ResourceEntry *entry = getResource(resMng, key);
    if (entry && entry->type == RESOURCE_TEXTURE) {
        return (SDL_Texture *)entry->resource;  // cast the resource to SDL_Texture
    }
    fprintf(stderr, "Texture with key '%s' not found\n", key);
    return NULL;  // texture not found or wrong type
}

/**
 * =====================================================================================================================
 */

TTF_Font* getFont(ResourceManager resMng, const char *key) {
    ResourceEntry *entry = getResource(resMng, key);
    if (entry && entry->type == RESOURCE_FONT) {
        return (TTF_Font *)entry->resource;  // cast the resource to TTF_Font
    }
    fprintf(stderr, "Font with key '%s' not found\n", key);
    return NULL;  // font not found or wrong type
}

/**
 * =====================================================================================================================
 */

Mix_Chunk *getSound(ResourceManager resMng, const char *key) {
    ResourceEntry *entry = getResource(resMng, key);
    if (entry && entry->type == RESOURCE_SOUND) {
        return (Mix_Chunk *)entry->resource;  // cast the resource to Mix_Chunk
    }
    fprintf(stderr, "Sound with key '%s' not found\n", key);
    return NULL;  // sound not found or wrong type
}

/**
 * =====================================================================================================================
 */

 Mix_Music *getMusic(ResourceManager resMng, const char *key) {
    ResourceEntry *entry = getResource(resMng, key);
    if (entry && entry->type == RESOURCE_MUSIC) {
        return (Mix_Music *)entry->resource;  // cast the resource to Mix_Music
    }
    fprintf(stderr, "Music with key '%s' not found\n", key);
    return NULL;  // Music not found or wrong type
}

/**
 * =====================================================================================================================
 */

void *getOrLoadResource(ResourceManager resMng, SDL_Renderer *renderer, const char *key, ResourceType type) {
    ResourceEntry *entry = getResource(resMng, key);
    if (entry) {
        // Already loaded, just return it
        return entry->resource;
    }

    void *resource = NULL;
    switch (type) {
        case RESOURCE_TEXTURE: {
            resource = IMG_LoadTexture(renderer, key);
            if (!resource) {
                fprintf(stderr, "Failed to load texture %s: %s\n", key, IMG_GetError());
            }
            SDL_SetTextureBlendMode((SDL_Texture *)resource, SDL_BLENDMODE_BLEND);  // activate transparent background
            break;
        }

        case RESOURCE_FONT: {
            int pSize = 0;
            char *sizeStr = strchr(key, '#'); // look for '#' separator
            size_t pathLen = 0;
            if (sizeStr) {
                pathLen = sizeStr - key; // Yes this is valid pointer arithmetic
                pSize = atoi(sizeStr + 1); // Convert the size part to an integer
            } else {
                fprintf(stderr, "Font size not specified in key %s. Use format 'path#size'.\n", key);
                return NULL;
            }

            char pathBuff[pathLen + 1];
            strncpy(pathBuff, key, pathLen);
            pathBuff[pathLen] = '\0'; // Null-terminate the string
            resource = TTF_OpenFont(pathBuff, pSize);
            if (!resource) {
                fprintf(stderr, "Failed to load font %s: %s\n", key, TTF_GetError());
            }
            break;
        }
        case RESOURCE_SOUND: {
            resource = Mix_LoadWAV(key);
            if (!resource) {
                fprintf(stderr, "Failed to load sound %s: %s\n", key, Mix_GetError());
            }
            break;
        }
        case RESOURCE_MUSIC: {
            resource = Mix_LoadMUS(key);
            if (!resource) {
                fprintf(stderr, "Failed to load music file %s: %s\n", key, Mix_GetError());
            }
            break;
        }
    }

    if (resource) {
        // add the newly loaded resource to the resource manager
        addResource(resMng, key, resource, type);
    }

    return resource;
}

/**
 * =====================================================================================================================
 */

SDL_Texture* getOrLoadTexture(ResourceManager resMng, SDL_Renderer *renderer, const char *key) {
    return (SDL_Texture *)getOrLoadResource(resMng, renderer, key, RESOURCE_TEXTURE);
}

/**
 * =====================================================================================================================
 */

Mix_Chunk *getOrLoadSound(ResourceManager resMng, const char *key) {
    return (Mix_Chunk *)getOrLoadResource(resMng, NULL, key, RESOURCE_SOUND);
}

/**
 * =====================================================================================================================
 */

void preloadResources(ResourceManager resMng, SDL_Renderer *renderer) {
    if (TTF_Init() == -1) {
        fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        fprintf(stderr, "IMG_Init: %s\n", IMG_GetError());
        exit(EXIT_FAILURE);
    }
    getOrLoadResource(resMng, renderer, "assets/fonts/ByteBounce.ttf#28", RESOURCE_FONT);
    getOrLoadResource(resMng, renderer, "assets/fonts/ByteBounce.ttf#48", RESOURCE_FONT);
    getOrLoadResource(resMng, renderer, "assets/textures/tank.png", RESOURCE_TEXTURE);
    getOrLoadResource(resMng, renderer, "assets/textures/tank2.png", RESOURCE_TEXTURE);
    getOrLoadResource(resMng, renderer, "assets/textures/bullet.png", RESOURCE_TEXTURE);
    getOrLoadResource(resMng, renderer, "assets/textures/brick.jpg", RESOURCE_TEXTURE);
    getOrLoadResource(resMng, renderer, "assets/textures/rocks.jpg", RESOURCE_TEXTURE);
    getOrLoadResource(resMng, renderer, "assets/sounds/button-press.mp3", RESOURCE_SOUND);
    getOrLoadResource(resMng, renderer, "assets/sounds/mg.mp3", RESOURCE_SOUND);
    getOrLoadResource(resMng, renderer, "assets/sounds/rifle.mp3", RESOURCE_SOUND);
    getOrLoadResource(resMng, renderer, "assets/sounds/shell1.mp3", RESOURCE_SOUND);
    getOrLoadResource(resMng, renderer, "assets/sounds/shell2.mp3", RESOURCE_SOUND);
    getOrLoadResource(resMng, renderer, "assets/sounds/coaxmg1.mp3", RESOURCE_SOUND);
    getOrLoadResource(resMng, renderer, "assets/sounds/coaxmg2.mp3", RESOURCE_SOUND);
    getOrLoadResource(resMng, renderer, "assets/sounds/coaxmg3.mp3", RESOURCE_SOUND);
}

/**
 * =====================================================================================================================
 */

void freeResourceManager(ResourceManager *resMng) {
    if (!resMng || !*resMng) return;

    for (int i = 0; i < HASHMAP_SIZE; i++) {
        ResourceEntry *entry = (*resMng)->hashmap[i];
        while (entry) {
            ResourceEntry *next = entry->next;
            free(entry->key);
            switch (entry->type) {
                case RESOURCE_TEXTURE: {
                    SDL_DestroyTexture((SDL_Texture *)entry->resource);
                    break;
                }
                case RESOURCE_FONT: {
                    TTF_CloseFont((TTF_Font *)entry->resource);
                    break;
                }
                case RESOURCE_SOUND: {
                    Mix_FreeChunk((Mix_Chunk *)entry->resource);
                    break;
                }
            }
            free(entry);
            entry = next;
        }
    }
    free(*resMng);
    *resMng = NULL;
}