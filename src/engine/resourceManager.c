#include "resourceManager.h"

SDL_Texture* getTexture(HashMap resMng, const char *key) {
    if (!resMng || !key) THROW_ERROR_AND_RETURN("Resource manager or key is NULL", NULL);
    if (resMng->type != MAP_RESOURCES) THROW_ERROR_AND_RETURN("Resource manager is of wrong type", NULL);
    MapEntry *entry = MapGetEntry(resMng, key);
    if (entry && entry->type == ENTRY_TEXTURE) {
        return (SDL_Texture *)entry->data.ptr;
    }
    THROW_ERROR_AND_DO("Texture with key ", fprintf(stderr, "'%s' not found\n", key); return NULL;);
}

/**
 * =====================================================================================================================
 */

TTF_Font* getFont(HashMap resMng, const char *key) {
    if (!resMng || !key) THROW_ERROR_AND_RETURN("Resource manager or key is NULL", NULL);
    if (resMng->type != MAP_RESOURCES) THROW_ERROR_AND_RETURN("Resource manager is of wrong type", NULL);
    MapEntry *entry = MapGetEntry(resMng, key);
    if (entry && entry->type == ENTRY_FONT) {
        return (TTF_Font *)entry->data.ptr;
    }
    THROW_ERROR_AND_DO("Font with key ", fprintf(stderr, "'%s' not found\n", key); return NULL;);
}

/**
 * =====================================================================================================================
 */

Mix_Chunk *getSound(HashMap resMng, const char *key) {
    if (!resMng || !key) THROW_ERROR_AND_RETURN("Resource manager or key is NULL", NULL);
    if (resMng->type != MAP_RESOURCES) THROW_ERROR_AND_RETURN("Resource manager is of wrong type", NULL);
    MapEntry *entry = MapGetEntry(resMng, key);
    if (entry && entry->type == ENTRY_SOUND) {
        return (Mix_Chunk *)entry->data.ptr;
    }
    THROW_ERROR_AND_DO("Sound with key ", fprintf(stderr, "'%s' not found\n", key); return NULL;);
}

/**
 * =====================================================================================================================
 */

MapEntryVal getOrLoadResource(HashMap resMng, SDL_Renderer *renderer, const char *key, MapEntryType type) {
    if (!resMng || !key) THROW_ERROR_AND_RETURN("Resource manager or key is NULL", (MapEntryVal){.ptr = NULL});
    if (resMng->type != MAP_RESOURCES)
        THROW_ERROR_AND_RETURN("Resource manager is of wrong type", (MapEntryVal){.ptr = NULL});
    MapEntry *entry = MapGetEntry(resMng, key);
    if (entry) {
        // Already loaded, just return it
        return entry->data;
    }

    THROW_ERROR_AND_DO("Resource with key ", fprintf(stderr, "'%s' not found. Loading...\n", key); );
    MapEntryVal resource = {.ptr = NULL};
    switch (type) {
        case ENTRY_TEXTURE: {
            resource.ptr = IMG_LoadTexture(renderer, key);
            if (!resource.ptr) THROW_ERROR_AND_DO(
                "Failed to load texture ", fprintf(stderr, "'%s': %s\n", key, IMG_GetError());
                return (MapEntryVal){.ptr = NULL};
            );
            SDL_SetTextureBlendMode((SDL_Texture *)resource.ptr, SDL_BLENDMODE_BLEND);  // activate blending
            break;
        }

        case ENTRY_FONT: {
            int pSize = 20;  // Default
            char *sizeStr = strchr(key, '#'); // look for '#' separator
            size_t pathLen = 0;
            if (sizeStr) {
                pathLen = sizeStr - key; // Yes this is valid pointer arithmetic
                pSize = atoi(sizeStr + 1); // Convert the size part to an integer
            } else THROW_ERROR_AND_DO(
                "Font path ", fprintf(stderr, "'%s' is missing size specifier. Using default size %d\n", key, pSize);
                return (MapEntryVal){.ptr = NULL};
            );

            char pathBuff[pathLen + 1];
            strncpy(pathBuff, key, pathLen);
            pathBuff[pathLen] = '\0'; // Null-terminate the string
            resource.ptr = TTF_OpenFont(pathBuff, pSize);
            if (!resource.ptr) THROW_ERROR_AND_DO(
                "Failed to load font ", fprintf(stderr, "'%s': %s\n", pathBuff, TTF_GetError());
                return (MapEntryVal){.ptr = NULL};
            );
            break;
        }
        case ENTRY_SOUND: {
            resource.ptr = Mix_LoadWAV(key);
            if (!resource.ptr) THROW_ERROR_AND_DO(
                "Failed to load sound ", fprintf(stderr, "'%s': %s\n", key, Mix_GetError());
                return (MapEntryVal){.ptr = NULL};
            );
            break;
        }
        default: {
            THROW_ERROR_AND_DO(
                "Unsupported resource type ", fprintf(stderr, "%d for key '%s'\n", type, key);
                return (MapEntryVal){.ptr = NULL};
            );
        }
    }
    if (!resource.ptr) THROW_ERROR_AND_DO(
        "Resource loading failed for key ", fprintf(stderr, "'%s'\n", key);
        return (MapEntryVal){.ptr = NULL};
    );
    
    // Add the newly loaded resource to the resource manager
    MapAddEntry(resMng, key, resource, type);
    return resource;
}

/**
 * =====================================================================================================================
 */

void preloadResources(HashMap resMng, SDL_Renderer *renderer) {
    // The main font with some sizes
    getOrLoadResource(resMng, renderer, "assets/fonts/ByteBounce.ttf#28", ENTRY_FONT);
    getOrLoadResource(resMng, renderer, "assets/fonts/ByteBounce.ttf#32", ENTRY_FONT);
    getOrLoadResource(resMng, renderer, "assets/fonts/ByteBounce.ttf#48", ENTRY_FONT);

    // Textures
    getOrLoadResource(resMng, renderer, "assets/textures/tank.png", ENTRY_TEXTURE);
    getOrLoadResource(resMng, renderer, "assets/textures/tank2.png", ENTRY_TEXTURE);
    getOrLoadResource(resMng, renderer, "assets/textures/bullet.png", ENTRY_TEXTURE);
    getOrLoadResource(resMng, renderer, "assets/textures/brick.jpg", ENTRY_TEXTURE);
    getOrLoadResource(resMng, renderer, "assets/textures/rocks.jpg", ENTRY_TEXTURE);
    getOrLoadResource(resMng, renderer, "assets/textures/testgun.png", ENTRY_TEXTURE);
    getOrLoadResource(resMng, renderer, "assets/textures/testgun2.png", ENTRY_TEXTURE);

    // Sounds
    getOrLoadResource(resMng, renderer, "assets/sounds/button-press.mp3", ENTRY_SOUND);
    getOrLoadResource(resMng, renderer, "assets/sounds/mg.mp3", ENTRY_SOUND);
    getOrLoadResource(resMng, renderer, "assets/sounds/rifle.mp3", ENTRY_SOUND);
    getOrLoadResource(resMng, renderer, "assets/sounds/shell1.mp3", ENTRY_SOUND);
    getOrLoadResource(resMng, renderer, "assets/sounds/shell2.mp3", ENTRY_SOUND);
    getOrLoadResource(resMng, renderer, "assets/sounds/coaxmg1.mp3", ENTRY_SOUND);
    getOrLoadResource(resMng, renderer, "assets/sounds/coaxmg2.mp3", ENTRY_SOUND);
    getOrLoadResource(resMng, renderer, "assets/sounds/coaxmg3.mp3", ENTRY_SOUND);

    // UI
    getOrLoadResource(resMng, renderer, "assets/ui/arrow.png", ENTRY_TEXTURE);
    getOrLoadResource(resMng, renderer, "assets/ui/metalwall.png", ENTRY_TEXTURE);
}

/**
 * =====================================================================================================================
 */

void freeResourceManager(HashMap *resMng) {
    if (!resMng || !*resMng) return;
    if ((*resMng)->type != MAP_RESOURCES) THROW_ERROR_AND_RETURN_VOID("Resource manager is of wrong type. Can't free");

    size_t size = (*resMng)->size;
    for (size_t i = 0; i < size; i++) {
        MapEntry *entry = (*resMng)->entries[i];
        while (entry) {
            MapEntry *next = entry->next;
            free(entry->key);
            switch (entry->type) {
                case ENTRY_TEXTURE: {
                    SDL_DestroyTexture((SDL_Texture *)entry->data.ptr);
                    break;
                }
                case ENTRY_FONT: {
                    TTF_CloseFont((TTF_Font *)entry->data.ptr);
                    break;
                }
                case ENTRY_SOUND: {
                    Mix_FreeChunk((Mix_Chunk *)entry->data.ptr);
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