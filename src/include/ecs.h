#ifndef ECS_H
#define ECS_H

#include <stdlib.h>
#include <SDL2/SDL.h>

#define INIT_CAPACITY 10  // initial capacity for the ECS

// ECS (Entity Component System) header file
// Little game - big ideas

// Health Component
typedef struct {
    char active;  // indicates if the health component is active
    int maxHealth; // maximum health of the entity
    int currentHealth; // current health of the entity
} HealthComponent;

typedef struct {
    char active;  // indicates if the speed component is active
    double velocity;  // speed of the entity
    double maxSpeed;  // maximum speed of the entity
} SpeedComponent;

typedef struct {
    char active;  // indicates if the render component is active
    SDL_Texture *texture;  // texture to render
    SDL_Rect *destRect;  // where to render the texture
} RenderComponent;

// Entity Component System (ECS) structure
typedef struct ecs {
    long long entityCount; // Number of entities currently in the ECS
    long long capacity;  // current capacity of the ECS
    HealthComponent *healthComponents;  // array to hold health components of each entity
    SpeedComponent *speedComponents;  // array to hold speed components of each entity
    RenderComponent *renderComponents;  // array to hold render components of each entity
} *ECS;

void initECS(ECS *ecs);

#endif // ECS_H