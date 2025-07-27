#include "include/ecs.h"
#include <stdlib.h>
#include <stdio.h>

void initGECS(ECS *gEcs) {
    (*gEcs) = malloc(sizeof(struct EeSiEs));
    if (!(*gEcs)) {
        fprintf(stderr, "Failed to allocate memory for ECS\n");
        exit(EXIT_FAILURE);
    }

    (*gEcs)->name = strdup("Game ECS");

    (*gEcs)->nextEntityID = 0;
    (*gEcs)->entityCount = 0;
    (*gEcs)->capacity = INIT_CAPACITY;
    (*gEcs)->activeEntities = calloc(INIT_CAPACITY, sizeof(Entity));
    if (!(*gEcs)->activeEntities) {
        fprintf(stderr, "Failed to allocate memory for ECS active entities\n");
        free(*gEcs);
        exit(EXIT_FAILURE);
    }

    (*gEcs)->freeEntities = calloc(INIT_CAPACITY, sizeof(Entity));
    if (!(*gEcs)->freeEntities) {
        fprintf(stderr, "Failed to allocate memory for ECS free entities\n");
        free(*gEcs);
        exit(EXIT_FAILURE);
    }
    (*gEcs)->freeEntityCount = 0;
    (*gEcs)->freeEntityCapacity = INIT_CAPACITY;

    // initialize the component arrays
    (*gEcs)->componentsFlags = calloc((*gEcs)->capacity, sizeof(bitset));
    if (!(*gEcs)->componentsFlags) {
        fprintf(stderr, "Failed to allocate memory for ECS components flags\n");
        free(*gEcs);
        exit(EXIT_FAILURE);
    }
    (*gEcs)->components = calloc(COMPONENT_COUNT, sizeof(Component));
    if (!(*gEcs)->components) {
        fprintf(stderr, "Failed to allocate memory for ECS components\n");
        free((*gEcs)->componentsFlags);
        free(*gEcs);
        exit(EXIT_FAILURE);
    }

    for (Uint64 i = 0; i < COMPONENT_COUNT; i++) {
        // initialise these with NULL, they will be allocated later at addition
        (*gEcs)->components[i].dense = NULL;
        (*gEcs)->components[i].denseSize = 0;
        (*gEcs)->components[i].sparse = NULL;
        (*gEcs)->components[i].pageCount = 0;

        (*gEcs)->components[i].type = i;
    }
}

/**
 * =====================================================================================================================
 */

void initUIECS(ECS *uiEcs) {
    (*uiEcs) = malloc(sizeof(struct EeSiEs));
    if (!(*uiEcs)) {
        fprintf(stderr, "Failed to allocate memory for UI ECS\n");
        exit(EXIT_FAILURE);
    }

    (*uiEcs)->name = strdup("UI ECS");
    (*uiEcs)->nextEntityID = 0;
    (*uiEcs)->entityCount = 0;
    (*uiEcs)->capacity = INIT_CAPACITY;
    (*uiEcs)->activeEntities = calloc(INIT_CAPACITY, sizeof(Entity));
    if (!(*uiEcs)->activeEntities) {
        fprintf(stderr, "Failed to allocate memory for UI ECS active entities\n");
        free(*uiEcs);
        exit(EXIT_FAILURE);
    }

    (*uiEcs)->freeEntities = calloc(INIT_CAPACITY, sizeof(Entity));
    if (!(*uiEcs)->freeEntities) {
        fprintf(stderr, "Failed to allocate memory for UI ECS free entities\n");
        free(*uiEcs);
        exit(EXIT_FAILURE);
    }
    (*uiEcs)->freeEntityCount = 0;
    (*uiEcs)->freeEntityCapacity = INIT_CAPACITY;

    (*uiEcs)->componentsFlags = calloc((*uiEcs)->capacity, sizeof(bitset));
    if (!(*uiEcs)->componentsFlags) {
        fprintf(stderr, "Failed to allocate memory for UI ECS components flags\n");
        free(*uiEcs);
        exit(EXIT_FAILURE);
    }
    (*uiEcs)->components = calloc(COMPONENT_COUNT, sizeof(Component));
    if (!(*uiEcs)->components) {
        fprintf(stderr, "Failed to allocate memory for UI ECS components\n");
        free((*uiEcs)->componentsFlags);
        free(*uiEcs);
        exit(EXIT_FAILURE);
    }

    for (Uint64 i = TEXT_COMPONENT; i < COMPONENT_COUNT; i++) {
        // initialise these with NULL, they will be allocated later at addition
        (*uiEcs)->components[i].dense = NULL;
        (*uiEcs)->components[i].denseSize = 0;
        (*uiEcs)->components[i].sparse = NULL;
        (*uiEcs)->components[i].pageCount = 0;
        (*uiEcs)->components[i].type = i;
    }
}

/**
 * =====================================================================================================================
 */

Entity createEntity(ECS ecs) {
    Entity entitty;
    if (ecs->freeEntityCount > 0) {
        // reuse an ID from the free entities array
        entitty = ecs->freeEntities[--ecs->freeEntityCount];
    } else {
        // no free entities, create a new one
        if (ecs->entityCount >= ecs->capacity) {
            // resize the ECS if needed
            Uint64 oldCapacity = ecs->capacity;
            ecs->capacity *= 2;
            printf("Resizing ECS %s from %lu to %lu\n", ecs->name, oldCapacity, ecs->capacity);

            Entity *tmpActive = realloc(ecs->activeEntities, ecs->capacity * sizeof(Entity));
            if (!tmpActive) {
                fprintf(stderr, "Failed to reallocate memory for ECS active entities\n");
                free(ecs->activeEntities);
                free(ecs->componentsFlags);
                free(ecs->components);
                free(ecs);
                exit(EXIT_FAILURE);
            }
            ecs->activeEntities = tmpActive;

            bitset *tmpFlags = realloc(ecs->componentsFlags, ecs->capacity * sizeof(bitset));
            if (!tmpFlags) {
                fprintf(stderr, "Failed to reallocate memory for ECS components flags\n");
                free(ecs->componentsFlags);
                free(ecs->components);
                free(ecs);
                exit(EXIT_FAILURE);
            }
            ecs->componentsFlags = tmpFlags;

            // initalize the new flags to 0
            for (Uint64 i = oldCapacity; i < ecs->capacity; i++) {
                ecs->componentsFlags[i] = 00000000;
            }

            // resize the components arrays - only dense part
            for (Uint64 i = 0; i < COMPONENT_COUNT; i++) {
                if (ecs->components[i].dense) {
                    void **tmpDense = realloc(ecs->components[i].dense, ecs->capacity * sizeof(void *));
                    Entity *tmpDenseToEntity = realloc(ecs->components[i].denseToEntity, ecs->capacity * sizeof(Entity));
                    if (!tmpDense || !tmpDenseToEntity) {
                        fprintf(stderr, "Failed to reallocate memory for component %lu dense arrays\n", i);
                        exit(EXIT_FAILURE);
                    }
                    ecs->components[i].dense = tmpDense;
                    ecs->components[i].denseToEntity = tmpDenseToEntity;
                }
            }
        }
        entitty = ecs->nextEntityID++;
    }

    ecs->componentsFlags[entitty] = 0;  // the new entity has no components
    ecs->activeEntities[ecs->entityCount++] = entitty;  // add it to the active entities array
    printf("Created entity with ID %ld in %s\n", entitty, ecs->name);
    return entitty;
}

/**
 * =====================================================================================================================
 */

void addComponent(ECS ecs, Entity id, ComponentType compType, void *component) {
    Uint64 page = id / PAGE_SIZE;  // determine the page for the entity
    Uint64 index = id % PAGE_SIZE;  // determine the index within the page

    if (compType >= COMPONENT_COUNT) {
        fprintf(stderr, "Invalid component type %d\n", compType);
        return;
    }

    // Check if the page exists, if not, allocate it
    if (ecs->components[compType].sparse == NULL || ecs->components[compType].pageCount <= page) {
        printf("Allocating memory for a page of the component %d's array\n", compType);
        // Allocate memory for a new page
        ecs->components[compType].sparse = realloc(ecs->components[compType].sparse, (page + 1) * sizeof(Uint64 *));
        if (!ecs->components[compType].sparse) {
            fprintf(stderr, "Failed to reallocate memory for ECS sparse set\n");
            exit(EXIT_FAILURE);
        }
        // Allocate PAGE_SIZE memory for each page
        for (Uint64 i = ecs->components[compType].pageCount; i <= page; i++) {
            ecs->components[compType].sparse[i] = calloc(PAGE_SIZE, sizeof(Uint64));
            if (!ecs->components[compType].sparse[i]) {
                fprintf(stderr, "Failed to allocate memory for ECS sparse set page %lu\n", i);
                exit(EXIT_FAILURE);
            }
        }
        ecs->components[compType].pageCount = page + 1;
    }
    
    // Check if the dense array exists, if not, allocate it
    if (ecs->components[compType].dense == NULL) {
        printf("Allocating memory for the dense array of the component %d\n", compType);
        // initially, it will have INIT_CAPACITY memory allocated
        ecs->components[compType].dense = calloc(ecs->capacity, sizeof(void *));
        ecs->components[compType].denseToEntity = calloc(ecs->capacity, sizeof(Entity));
        if (!ecs->components[compType].dense || !ecs->components[compType].denseToEntity) {
            fprintf(stderr, "Failed to allocate memory for ECS dense set\n");
            exit(EXIT_FAILURE);
        }
    }

    // Check if the entity ID will fit in the dense array
    if (id >= ecs->capacity) {
        printf("Reallocating memory for the dense array of the component %d's array\n", compType);
        // Resize the dense array if needed
        void **tmpDense = realloc(ecs->components[compType].dense, ecs->capacity * sizeof(void *));
        Entity *tmpDenseToEntity = realloc(ecs->components[compType].denseToEntity, ecs->capacity * sizeof(Entity));
        if (!tmpDense || !tmpDenseToEntity) {
            fprintf(stderr, "Failed to reallocate memory for ECS dense set\n");
            exit(EXIT_FAILURE);
        }
        ecs->components[compType].dense = tmpDense;
        ecs->components[compType].denseToEntity = tmpDenseToEntity;
    }

    // Allocations done, now add the component
    Uint64 denseIdx = ecs->components[compType].denseSize;
    ecs->components[compType].sparse[page][index] = denseIdx;
    ecs->components[compType].dense[denseIdx] = component;
    ecs->components[compType].denseToEntity[denseIdx] = id;  // map the component to its entity ID
    ecs->components[compType].denseSize++;

    // and set the corresponding bitmask
    ecs->componentsFlags[id] |= (1 << compType);
    printf("Added component %d to entity %ld from %s\n", compType, id, ecs->name);
}

/**
 * =====================================================================================================================
 */

void deleteEntity(ECS ecs, Entity id) {
    if (!ecs) {
        fprintf(stderr, "ECS is NULL, cannot delete entity\n");
        return;
    }

    if (id >= ecs->capacity || id < 0) {
        printf("Warning: Attempting to delete entity %ld which is out of bounds\n", id);
        return;
    }

    if (ecs->componentsFlags[id] == 00000000) {
        // entity has no components, just free the ID
        ecs->freeEntities[ecs->freeEntityCount++] = id;
        ecs->entityCount--;
        return;
    }
    
    // Free all components associated with this entity
    for (Uint64 i = 0; i < COMPONENT_COUNT; i++) {
        bitset componentFlag = 1 << i;
        
        // Check if entity has this component
        if ((ecs->componentsFlags[id] & componentFlag) == componentFlag) {
            // Find the component in the sparse set
            Uint64 page = id / PAGE_SIZE;
            Uint64 index = id % PAGE_SIZE;
            
            if (
                ecs->components[i].sparse && 
                page < ecs->components[i].pageCount &&
                ecs->components[i].dense
            ) {
                Uint64 denseIndex = ecs->components[i].sparse[page][index];
                void *component = ecs->components[i].dense[denseIndex];
                
                // Free the component data based on its type
                if (component) {
                    switch (i) {
                        case COLLISION_COMPONENT: {
                            CollisionComponent *colComp = (CollisionComponent*)component;
                            if (colComp->hitbox) free(colComp->hitbox);
                            break;
                        }
                        case TEXT_COMPONENT: {
                            TextComponent *textComp = (TextComponent*)component;
                            if (textComp->text) free(textComp->text);
                            if (textComp->destRect) free(textComp->destRect);
                            break;
                        }
                        case RENDER_COMPONENT: {
                            RenderComponent *render = (RenderComponent*)component;
                            if (render->destRect) free(render->destRect);
                            break;
                        }
                    }
                    free(component);
                    
                    // Remove from dense array by swapping with the last element
                    Uint64 lastDenseIndex = ecs->components[i].denseSize - 1;
                    if (denseIndex != lastDenseIndex) {
                        // Move the last element to the position of the removed element
                        ecs->components[i].dense[denseIndex] = ecs->components[i].dense[lastDenseIndex];
                        
                        // Use the reverse mapping
                        Entity lastEntity = ecs->components[i].denseToEntity[lastDenseIndex];
                        
                        // Update the sparse pointer for the moved entity
                        Uint64 lastPage = lastEntity / PAGE_SIZE;
                        Uint64 lastIndex = lastEntity % PAGE_SIZE;
                        ecs->components[i].sparse[lastPage][lastIndex] = denseIndex;

                        // update the denseToEntity mapping
                        ecs->components[i].denseToEntity[denseIndex] = lastEntity;
                    }
                    
                    // Decrease the size of the dense array
                    ecs->components[i].denseSize--;
                    ecs->components[i].dense[lastDenseIndex] = NULL;  // avoid dangling pointer
                }
            }
        }
    }
    
    // Reset the components flags for this entity
    ecs->componentsFlags[id] = 0;
    
    // Add the entity ID to the free list for reuse
    if (ecs->freeEntityCount >= ecs->freeEntityCapacity) {
        // Resize the free entities array if needed
        ecs->freeEntityCapacity *= 2;
        Entity *newFreeEntities = realloc(
            ecs->freeEntities,
            ecs->freeEntityCapacity * sizeof(Entity)
        );
        if (!newFreeEntities) {
            fprintf(stderr, "Failed to resize free entities array\n");
            return;
        }
        ecs->freeEntities = newFreeEntities;
    }

    // Remove the entity from the active entities array
    for (Uint64 i = 0; i < ecs->entityCount; i++) {
        if (ecs->activeEntities[i] == id) {
            ecs->activeEntities[i] = ecs->activeEntities[ecs->entityCount - 1];
            break;
        }
    }
     
    // Add this entity to the free list
    ecs->freeEntities[ecs->freeEntityCount++] = id;
    ecs->entityCount--;
    
    printf("Deleted entity with ID %ld from %s\n", id, ecs->name);
}

/**
 * =====================================================================================================================
 */

void freeECS(ECS ecs) {
    if (ecs) {
        if (ecs->name) {
            free(ecs->name);
        }
        if (ecs->componentsFlags) {
            free(ecs->componentsFlags);
        }
        if (ecs->activeEntities) {
            free(ecs->activeEntities);
        }
        if (ecs->components) {
            for (Uint64 i = 0; i < COMPONENT_COUNT; i++) {
                if (ecs->components[i].dense) {
                    for (Uint64 j = 0; j < ecs->components[i].denseSize; j++) {
                        void *curr = ecs->components[i].dense[j];
                        if (curr) {
                            switch (ecs->components[i].type) {
                                case TEXT_COMPONENT: {
                                    if ((*(TextComponent *)(curr)).texture) SDL_DestroyTexture((*(TextComponent *)(curr)).texture);
                                    if ((*(TextComponent *)(curr)).text) free((*(TextComponent *)(curr)).text);
                                    if ((*(TextComponent *)(curr)).destRect) free((*(TextComponent *)(curr)).destRect);
                                    break;
                                }
                                case RENDER_COMPONENT: {
                                    if ((*(RenderComponent *)(curr)).texture) SDL_DestroyTexture((*(RenderComponent *)(curr)).texture);
                                    if ((*(RenderComponent *)(curr)).destRect) free((*(RenderComponent *)(curr)).destRect);
                                    break;
                                }
                                case COLLISION_COMPONENT: {
                                    if ((*(CollisionComponent *)(curr)).hitbox) free((*(CollisionComponent *)(curr)).hitbox);
                                    break;
                                }
                            }
                            free(ecs->components[i].dense[j]);
                        }
                    }
                    free(ecs->components[i].dense);
                    free(ecs->components[i].denseToEntity);
                }
                if (ecs->components[i].sparse) {
                    for (Uint64 j = 0; j < ecs->components[i].pageCount; j++) {
                        if (ecs->components[i].sparse[j]) free(ecs->components[i].sparse[j]);
                    }
                    free(ecs->components[i].sparse);
                }
            }
            free(ecs->components);
        }
        if (ecs->freeEntities) {
            free(ecs->freeEntities);
        }
        free(ecs);
    }
}