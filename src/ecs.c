#include "include/ecs.h"
#include <stdlib.h>
#include <stdio.h>

void initECS(ECS *ecs) {
    (*ecs) = malloc(sizeof(struct ecs));
    if (!(*ecs)) {
        fprintf(stderr, "Failed to allocate memory for ECS\n");
        exit(EXIT_FAILURE);
    }

    (*ecs)->entityCount = 0;
    (*ecs)->capacity = INIT_CAPACITY;
    (*ecs)->healthComponents = malloc(INIT_CAPACITY * sizeof(HealthComponent));
    if (!(*ecs)->healthComponents) {
        fprintf(stderr, "Failed to allocate memory for health components\n");
        free(*ecs);
        exit(EXIT_FAILURE);
    }

    (*ecs)->speedComponents = malloc(INIT_CAPACITY * sizeof(SpeedComponent));
    if (!(*ecs)->speedComponents) {
        fprintf(stderr, "Failed to allocate memory for speed components\n");
        free((*ecs)->healthComponents);
        free(*ecs);
        exit(EXIT_FAILURE);
    }
}