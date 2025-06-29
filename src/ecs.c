#include "include/ecs.h"
#include <stdlib.h>
#include <stdio.h>

void initGECS(GameECS *ecs) {
    (*ecs) = malloc(sizeof(struct gameecs));
    if (!(*ecs)) {
        fprintf(stderr, "Failed to allocate memory for ECS\n");
        exit(EXIT_FAILURE);
    }

    (*ecs)->entityCount = 0;
    (*ecs)->capacity = INIT_CAPACITY;

    // make sure the health components are initialized as inactive
    (*ecs)->healthComponents = calloc(INIT_CAPACITY, sizeof(HealthComponent));
    if (!(*ecs)->healthComponents) {
        fprintf(stderr, "Failed to allocate memory for health components\n");
        free(*ecs);
        exit(EXIT_FAILURE);
    }

    // make sure the speed components are initialized as inactive
    (*ecs)->speedComponents = calloc(INIT_CAPACITY, sizeof(SpeedComponent));
    if (!(*ecs)->speedComponents) {
        fprintf(stderr, "Failed to allocate memory for speed components\n");
        free((*ecs)->healthComponents);
        free(*ecs);
        exit(EXIT_FAILURE);
    }

    // make sure the render components are initialized as inactive
    (*ecs)->renderComponents = calloc(INIT_CAPACITY, sizeof(RenderComponent));
    if (!(*ecs)->renderComponents) {
        fprintf(stderr, "Failed to allocate memory for render components\n");
        free((*ecs)->speedComponents);
        free((*ecs)->healthComponents);
        free(*ecs);
        exit(EXIT_FAILURE);
    }
}

void initUIECS(UIECS *uiEcs) {
    (*uiEcs) = malloc(sizeof(struct uiecs));
    if (!(*uiEcs)) {
        fprintf(stderr, "Failed to allocate memory for UI ECS\n");
        exit(EXIT_FAILURE);
    }

    (*uiEcs)->entityCount = 0;
    (*uiEcs)->capacity = INIT_CAPACITY;
    // make sure the render components are initialized as inactive
    (*uiEcs)->textComponents = calloc(INIT_CAPACITY, sizeof(TextComponent));
    if (!(*uiEcs)->textComponents) {
        fprintf(stderr, "Failed to allocate memory for UI text components\n");
        free(*uiEcs);
        exit(EXIT_FAILURE);
    }

    for (Uint64 i = 0; i < INIT_CAPACITY; i++) {
        (*uiEcs)->textComponents[i].text = calloc(256, sizeof(char));  // allocate space for text
        if (!(*uiEcs)->textComponents[i].text) {
            fprintf(stderr, "Failed to allocate memory for text in UI ECS\n");
            freeUIECS(*uiEcs);
            exit(EXIT_FAILURE);
        }
    }
}

void addUiTextEntity(UIECS uiEcs, TTF_Font *font, char *text, SDL_Color color, SDL_Texture *texture, SDL_Rect *destRect) {
    if (uiEcs->entityCount >= uiEcs->capacity) {
        // resize array if needed
        uiEcs->capacity *= 2;
        TextComponent *tmp = realloc(uiEcs->textComponents, uiEcs->capacity * sizeof(TextComponent));
        if (!tmp) {
            fprintf(stderr, "Failed to reallocate memory for UI render components\n");
            exit(EXIT_FAILURE);
        }
        uiEcs->textComponents = tmp;

        // reallocate the new text strings
        for (Uint64 i = uiEcs->entityCount; i < uiEcs->capacity; i++) {
            uiEcs->textComponents[i].text = calloc(256, sizeof(char));
            if (!uiEcs->textComponents[i].text) {
                fprintf(stderr, "Failed to allocate memory for text in UI ECS\n");
                freeUIECS(uiEcs);
                exit(EXIT_FAILURE);
            }
            uiEcs->textComponents[i].active = 0;  // mark as inactive
        }
    }

    // make sure the text buffer is valid (sometimes it can be freed earlier)
    if (!uiEcs->textComponents[uiEcs->entityCount].text) {
        uiEcs->textComponents[uiEcs->entityCount].text = calloc(256, sizeof(char));
        if (!uiEcs->textComponents[uiEcs->entityCount].text) {
            fprintf(stderr, "Failed to allocate memory for text in UI ECS\n");
            freeUIECS(uiEcs);
            exit(EXIT_FAILURE);
        }
    }

    uiEcs->textComponents[uiEcs->entityCount].active = 1;  // mark as active
    uiEcs->textComponents[uiEcs->entityCount].selected = 0;
    uiEcs->textComponents[uiEcs->entityCount].font = font;  // store the surface
    strcpy(uiEcs->textComponents[uiEcs->entityCount].text, text);  // copy the text
    uiEcs->textComponents[uiEcs->entityCount].texture = texture;
    uiEcs->textComponents[uiEcs->entityCount].color = color;  // store the color

    // make a copy of the destination rectangle
    SDL_Rect *rectCopy = calloc(1, sizeof(SDL_Rect));
    if (!rectCopy) {
        fprintf(stderr, "Failed to allocate memory for destination rectangle\n");
        exit(EXIT_FAILURE);
    }
    *rectCopy = *destRect;  // copy the contents of destRect
    uiEcs->textComponents[uiEcs->entityCount].destRect = rectCopy;
    uiEcs->entityCount++;
}

void deleteUiTextEntity(UIECS uiEcs, Uint64 index) {
    if (index >= uiEcs->entityCount) {
        fprintf(stderr, "Index out of bounds when deleting UI text entity\n");
        return;
    }

    // Free the resources of the text component
    if (uiEcs->textComponents[index].texture) {
        SDL_DestroyTexture(uiEcs->textComponents[index].texture);
        uiEcs->textComponents[index].texture = NULL;  // avoid dangling pointer
    }
    if (uiEcs->textComponents[index].destRect) {
        free(uiEcs->textComponents[index].destRect);
        uiEcs->textComponents[index].destRect = NULL;  // avoid dangling pointer
    }
    if (uiEcs->textComponents[index].text) {
        free(uiEcs->textComponents[index].text);
        uiEcs->textComponents[index].text = NULL;  // avoid dangling pointer
    }
    uiEcs->textComponents[index].active = 0;

    // superduper optimisation alert: swap the last element with the one deleted
    if (index < uiEcs->entityCount) {
        uiEcs->textComponents[index] = uiEcs->textComponents[uiEcs->entityCount - 1];
    }

    uiEcs->entityCount--;  // Decrease the entity count
}

void spawnGameEntity(GameECS ecs, HealthComponent health, SpeedComponent speed, RenderComponent render) {
    if (ecs->entityCount >= ecs->capacity) {
        // resize array if needed
        ecs->capacity *= 2;
        HealthComponent *tmpHealth = realloc(ecs->healthComponents, ecs->capacity * sizeof(HealthComponent));
        SpeedComponent *tmpSpeed = realloc(ecs->speedComponents, ecs->capacity * sizeof(SpeedComponent));
        RenderComponent *tmpRender = realloc(ecs->renderComponents, ecs->capacity * sizeof(RenderComponent));
        if (!tmpHealth || !tmpSpeed || !tmpRender) {
            fprintf(stderr, "Failed to reallocate memory for game components\n");
            exit(EXIT_FAILURE);
        }
        ecs->healthComponents = tmpHealth;
        ecs->speedComponents = tmpSpeed;
        ecs->renderComponents = tmpRender;
    }

    // Add the new entity
    ecs->healthComponents[ecs->entityCount] = health;
    ecs->speedComponents[ecs->entityCount] = speed;
    ecs->renderComponents[ecs->entityCount] = render;
    ecs->entityCount++;
}

void freeGECS(GameECS ecs) {
    if (ecs) {
        if (ecs->healthComponents) {
            free(ecs->healthComponents);
            ecs->healthComponents = NULL;
        }
        free(ecs->speedComponents);
        free(ecs->renderComponents);
        free(ecs);
    }
}

void freeUIECS(UIECS uiEcs) {
    if (uiEcs) {
        // free each active text component's resources
        for (Uint64 i = 0; i < uiEcs->capacity; i++) {
            if (uiEcs->textComponents[i].active) {
                if (uiEcs->textComponents[i].texture) {
                    SDL_DestroyTexture(uiEcs->textComponents[i].texture);
                    uiEcs->textComponents[i].texture = NULL;
                }
                if (uiEcs->textComponents[i].destRect) {
                    free(uiEcs->textComponents[i].destRect);
                    uiEcs->textComponents[i].destRect = NULL;
                }
            }
            if (uiEcs->textComponents[i].text) {
                free(uiEcs->textComponents[i].text);
                uiEcs->textComponents[i].text = NULL;
            }
        }
        free(uiEcs->textComponents);
        free(uiEcs);
    }
}