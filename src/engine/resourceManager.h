#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "../global/global.h"

/**
 * Retrieves a texture resource from the Resource Manager
 * @param resMng the Resource Manager HashMap = struct map*
 * @param key the resource's path
 * @return the texture resource if found, NULL otherwise
*/
SDL_Texture* getTexture(HashMap resMng, const char *key);

/**
 * Retrieves a font resource from the Resource Manager
 * @param resMng the Resource Manager HashMap = struct map*
 * @param key the resource's path
 * @return the font resource if found, NULL otherwise
*/ 
TTF_Font* getFont(HashMap resMng, const char *key);

/**
 * Retrieves a sound resource from the Resource Manager\
 * @param resMng the Resource Manager HashMap = struct map*
 * @param key the resource's path
 * @return the sound resource if found, NULL otherwise
 */
Mix_Chunk* getSound(HashMap resMng, const char *key);

/**
 * Retrieves a resource from the Resource Manager if it's there, otherwise loads it
 * @param resMng the Resource Manager HashMap = struct map*
 * @param renderer the SDL_Renderer, needed for loading textures (can be NULL for non-texture resources)
 * @param key the resource's path
 * @param type the type of resource to load
 * @return an entry value if found or loaded successfully, a MapEntryVal with .ptr = NULL otherwise
 */
MapEntryVal getOrLoadResource(HashMap resMng, SDL_Renderer *renderer, const char *key, MapEntryType type);

/**
 * Preloads the frequently used resources into the Resource Manager
 * @param resMng the Resource Manager HashMap = struct map*
 * @param renderer the SDL_Renderer, needed for loading textures
 * @note be careful about which resources to preload, as they have a lifetime of the entire game's duration
 */
void preloadResources(HashMap resMng, SDL_Renderer *renderer);

/**
 * Frees all resources used by the Resource Manager
 * @param resMng pointer to the Resource Manager HashMap = struct map**
 */
void freeResourceManager(HashMap *resMng);

#endif // RESOURCE_MANAGER_H