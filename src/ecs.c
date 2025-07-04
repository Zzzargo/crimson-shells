#include "include/ecs.h"
#include <stdlib.h>
#include <stdio.h>

void initGECS(ECS *gEcs) {
    (*gEcs) = malloc(sizeof(struct EeSiEs));
    if (!(*gEcs)) {
        fprintf(stderr, "Failed to allocate memory for ECS\n");
        exit(EXIT_FAILURE);
    }

    (*gEcs)->nextEntityID = 0;
    (*gEcs)->entityCount = 0;
    (*gEcs)->capacity = INIT_CAPACITY;

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
        
        switch (i) {
            case HEALTH_COMPONENT: {
                (*gEcs)->components[i].type = HEALTH_COMPONENT;
                break;
            }
            case VELOCITY_COMPONENT: {
                (*gEcs)->components[i].type = VELOCITY_COMPONENT;
                break;
            }
            case TEXT_COMPONENT: {
                (*gEcs)->components[i].type = TEXT_COMPONENT;
                break;
            }
            case RENDER_COMPONENT: {
                (*gEcs)->components[i].type = RENDER_COMPONENT;
                break;
            }
            default: {
                fprintf(stderr, "Unknown component type %lu\n", i);
                free((*gEcs)->componentsFlags);
                free((*gEcs)->components);
                free(*gEcs);
                exit(EXIT_FAILURE);
            }
        }
    }
}

void initUIECS(ECS *uiEcs) {
    (*uiEcs) = malloc(sizeof(struct EeSiEs));
    if (!(*uiEcs)) {
        fprintf(stderr, "Failed to allocate memory for UI ECS\n");
        exit(EXIT_FAILURE);
    }

    (*uiEcs)->nextEntityID = 0;
    (*uiEcs)->entityCount = 0;
    (*uiEcs)->capacity = INIT_CAPACITY;

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
        
        switch (i) {
            case TEXT_COMPONENT: {
                (*uiEcs)->components[i].type = TEXT_COMPONENT;
                break;
            }
            case RENDER_COMPONENT: {
                (*uiEcs)->components[i].type = RENDER_COMPONENT;
                break;
            }
            default: {
                fprintf(stderr, "Unknown component type %lu\n", i);
                free((*uiEcs)->componentsFlags);
                free((*uiEcs)->components);
                free(*uiEcs);
                exit(EXIT_FAILURE);
            }
        }
    }
}

Entity createEntity(ECS ecs) {
    Entity entitty;
    if (ecs->freeEntityCount > 0) {
        // reuse an ID from the free entities array
        entitty = ecs->freeEntities[--ecs->freeEntityCount];
    } else {
        // no free entities, create a new one
        if (ecs->entityCount >= ecs->capacity) {
            // resize the ECS if needed
            ecs->capacity *= 2;
            bitset *tmpFlags = realloc(ecs->componentsFlags, ecs->capacity * sizeof(bitset));
            if (!tmpFlags) {
                fprintf(stderr, "Failed to reallocate memory for ECS components flags\n");
                free(ecs->componentsFlags);
                free(ecs->components);
                free(ecs);
                exit(EXIT_FAILURE);
            }
            ecs->componentsFlags = tmpFlags;

            Component *tmpComponents = realloc(ecs->components, COMPONENT_COUNT * ecs->capacity * sizeof(Component));
            if (!tmpComponents) {
                fprintf(stderr, "Failed to reallocate memory for ECS components\n");
                free(ecs->componentsFlags);
                free(ecs->components);
                free(ecs);
                exit(EXIT_FAILURE);
            }
            ecs->components = tmpComponents;

            // resize the components arrays
            for (Uint64 i = 0; i < COMPONENT_COUNT; i++) {
                // TODO
            }
        }
        entitty = ecs->nextEntityID++;
    }

    ecs->componentsFlags[entitty] = 00000000;  // the new entity has no components
    ecs->entityCount++;
    return entitty;
}

void addComponent(ECS ecs, Entity id, ComponentType compType, void *component) {
    Uint64 page = id / PAGE_SIZE;  // determine the page for the entity
    Uint64 index = id % PAGE_SIZE;  // determine the index within the page

    if (compType >= COMPONENT_COUNT) {
        fprintf(stderr, "Invalid component type %d\n", compType);
        return;
    }

    // Check if the page exists, if not, allocate it
    if (ecs->components[compType].sparse == NULL || ecs->components[compType].pageCount <= page) {
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
        // initially, it will have INIT_CAPACITY memory allocated
        ecs->components[compType].dense = calloc(ecs->capacity, sizeof(void *));
        if (!ecs->components[compType].dense) {
            fprintf(stderr, "Failed to allocate memory for ECS dense set\n");
            exit(EXIT_FAILURE);
        }
    }

    // Check if the entity ID will fit in the dense array
    if (id >= ecs->capacity) {
        // Resize the dense array if needed
        ecs->capacity *= 2;
        void **tmpDense = realloc(ecs->components[compType].dense, ecs->capacity * sizeof(void *));
        if (!tmpDense) {
            fprintf(stderr, "Failed to reallocate memory for ECS dense set\n");
            exit(EXIT_FAILURE);
        }
        ecs->components[compType].dense = tmpDense;
    }

    // Allocations done, now add the component
    ecs->components[compType].sparse[page][index] = ecs->components[compType].denseSize;
    ecs->components[compType].dense[ ecs->components[compType].denseSize ] = component;
    ecs->components[compType].denseSize++;

    // and set the corresponding bitmask
    ecs->componentsFlags[id] |= (1 << compType);
}

// void addUiTextEntity(UIECS uiEcs, TTF_Font *font, char *text, SDL_Color color, SDL_Texture *texture, SDL_Rect *destRect) {
//     if (uiEcs->entityCount >= uiEcs->capacity) {
//         // resize array if needed
//         uiEcs->capacity *= 2;
//         TextComponent *tmp = realloc(uiEcs->textComponents, uiEcs->capacity * sizeof(TextComponent));
//         if (!tmp) {
//             fprintf(stderr, "Failed to reallocate memory for UI render components\n");
//             exit(EXIT_FAILURE);
//         }
//         uiEcs->textComponents = tmp;

//         // reallocate the new text strings
//         for (Uint64 i = uiEcs->entityCount; i < uiEcs->capacity; i++) {
//             uiEcs->textComponents[i].text = calloc(256, sizeof(char));
//             if (!uiEcs->textComponents[i].text) {
//                 fprintf(stderr, "Failed to allocate memory for text in UI ECS\n");
//                 freeUIECS(uiEcs);
//                 exit(EXIT_FAILURE);
//             }
//             uiEcs->textComponents[i].active = 0;  // mark as inactive
//         }
//     }

//     // make sure the text buffer is valid (sometimes it can be freed earlier)
//     if (!uiEcs->textComponents[uiEcs->entityCount].text) {
//         uiEcs->textComponents[uiEcs->entityCount].text = calloc(256, sizeof(char));
//         if (!uiEcs->textComponents[uiEcs->entityCount].text) {
//             fprintf(stderr, "Failed to allocate memory for text in UI ECS\n");
//             freeUIECS(uiEcs);
//             exit(EXIT_FAILURE);
//         }
//     }

//     uiEcs->textComponents[uiEcs->entityCount].active = 1;  // mark as active
//     uiEcs->textComponents[uiEcs->entityCount].selected = 0;
//     uiEcs->textComponents[uiEcs->entityCount].font = font;  // store the surface
//     strcpy(uiEcs->textComponents[uiEcs->entityCount].text, text);  // copy the text
//     uiEcs->textComponents[uiEcs->entityCount].texture = texture;
//     uiEcs->textComponents[uiEcs->entityCount].color = color;  // store the color

//     // make a copy of the destination rectangle
//     SDL_Rect *rectCopy = calloc(1, sizeof(SDL_Rect));
//     if (!rectCopy) {
//         fprintf(stderr, "Failed to allocate memory for destination rectangle\n");
//         exit(EXIT_FAILURE);
//     }
//     *rectCopy = *destRect;  // copy the contents of destRect
//     uiEcs->textComponents[uiEcs->entityCount].destRect = rectCopy;
//     uiEcs->entityCount++;
// }

// void deleteUiTextEntity(UIECS uiEcs, Uint64 index) {
//     if (index >= uiEcs->entityCount) {
//         fprintf(stderr, "Index out of bounds when deleting UI text entity\n");
//         return;
//     }

//     // Free the resources of the text component
//     if (uiEcs->textComponents[index].texture) {
//         SDL_DestroyTexture(uiEcs->textComponents[index].texture);
//         uiEcs->textComponents[index].texture = NULL;  // avoid dangling pointer
//     }
//     if (uiEcs->textComponents[index].destRect) {
//         free(uiEcs->textComponents[index].destRect);
//         uiEcs->textComponents[index].destRect = NULL;  // avoid dangling pointer
//     }
//     if (uiEcs->textComponents[index].text) {
//         free(uiEcs->textComponents[index].text);
//         uiEcs->textComponents[index].text = NULL;  // avoid dangling pointer
//     }
//     uiEcs->textComponents[index].active = 0;

//     // superduper optimisation alert: swap the last element with the one deleted
//     if (index < uiEcs->entityCount) {
//         uiEcs->textComponents[index] = uiEcs->textComponents[uiEcs->entityCount - 1];
//     }

//     uiEcs->entityCount--;  // Decrease the entity count
// }

// void spawnGameEntity(GameECS ecs, HealthComponent health, SpeedComponent speed, RenderComponent render) {
//     if (ecs->entityCount >= ecs->capacity) {
//         // resize array if needed
//         ecs->capacity *= 2;
//         HealthComponent *tmpHealth = realloc(ecs->healthComponents, ecs->capacity * sizeof(HealthComponent));
//         SpeedComponent *tmpSpeed = realloc(ecs->speedComponents, ecs->capacity * sizeof(SpeedComponent));
//         RenderComponent *tmpRender = realloc(ecs->renderComponents, ecs->capacity * sizeof(RenderComponent));
//         if (!tmpHealth || !tmpSpeed || !tmpRender) {
//             fprintf(stderr, "Failed to reallocate memory for game components\n");
//             exit(EXIT_FAILURE);
//         }
//         ecs->healthComponents = tmpHealth;
//         ecs->speedComponents = tmpSpeed;
//         ecs->renderComponents = tmpRender;
//     }

//     // Add the new entity
//     ecs->healthComponents[ecs->entityCount] = health;
//     ecs->speedComponents[ecs->entityCount] = speed;
//     ecs->renderComponents[ecs->entityCount] = render;
//     ecs->entityCount++;
// }

void freeECS(ECS ecs) {
    if (ecs) {
        if (ecs->componentsFlags) {
            free(ecs->componentsFlags);
        }
        if (ecs->components) {
            for (Uint64 i = 0; i < COMPONENT_COUNT; i++) {
                if (ecs->components[i].dense) {
                    for (Uint64 j = 0; j < ecs->components[i].denseSize; j++) {
                        void *curr = ecs->components[i].dense[j];
                        switch (ecs->components[i].type) {
                            case RENDER_COMPONENT: {
                                if ((*(RenderComponent *)(curr)).texture) SDL_DestroyTexture((*(RenderComponent *)(curr)).texture);
                                if ((*(RenderComponent *)(curr)).destRect) free((*(RenderComponent *)(curr)).destRect);
                                break;
                            }
                            case TEXT_COMPONENT: {
                                if ((*(TextComponent *)(curr)).texture) SDL_DestroyTexture((*(TextComponent *)(curr)).texture);
                                if ((*(TextComponent *)(curr)).text) free((*(TextComponent *)(curr)).text);
                                if ((*(TextComponent *)(curr)).destRect) free((*(TextComponent *)(curr)).destRect);
                                break;
                            }
                        }
                        free(ecs->components[i].dense[j]);
                    }
                    free(ecs->components[i].dense);
                }
                if (ecs->components[i].sparse) {
                    for (Uint64 j = 0; j < ecs->components[i].pageCount; j++) {
                        free(ecs->components[i].sparse[j]);
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