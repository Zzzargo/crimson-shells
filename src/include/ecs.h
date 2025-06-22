#ifndef ECS_H
#define ECS_H

#include <stdlib.h>
#include <SDL2/SDL.h>
#include "fontManager.h"

#define INIT_CAPACITY 10  // initial capacity for the ECS

// ECS (Entity Component System) header file
// Little game - big ideas

// Health Component
typedef struct {
    Uint8 active;  // indicates if the health component is active
    Uint32 maxHealth; // maximum health of the entity
    Uint32 currentHealth; // current health of the entity
} HealthComponent;

typedef struct {
    Uint8 active;  // indicates if the speed component is active
    double_t velocity;  // speed of the entity
    double_t maxSpeed;  // maximum speed of the entity
} SpeedComponent;

typedef struct {
    Uint8 active;  // indicates if the render component is active
    Uint8 selected;  // for UI components
    SDL_Texture *texture;  // texture to render
    SDL_Rect *destRect;  // where to render the texture
} RenderComponent;

typedef struct {
    Uint8 active;  // indicates if the text component is active
    Uint8 selected;  // for UI components
    TTF_Font *font;  // font to use for rendering text
    char *text;
    SDL_Texture *texture;  // texture for the text
    SDL_Rect *destRect;  // where to render the text
} TextComponent;

// Entity Component System (ECS) structure
typedef struct gameecs {
    Uint64 entityCount; // Number of entities currently in the ECS
    Uint64 capacity;  // current capacity of the ECS
    HealthComponent *healthComponents;  // array to hold health components of each entity
    SpeedComponent *speedComponents;  // array to hold speed components of each entity
    RenderComponent *renderComponents;  // array to hold render components of each entity
} *GameECS;

typedef struct uiecs {
    Uint64 entityCount; // Number of entities currently in the ECS
    Uint64 capacity;  // current capacity of the ECS
    TextComponent *textComponents;  // array to hold text components of each entity
} *UIECS;

// initialises the game ECS
void initGECS(GameECS *ecs);

// initialises the UI ECS
void initUIECS(UIECS *uiEcs);

// adds a text entity to the UI ECS
void addUiTextEntity(UIECS uiEcs, TTF_Font *font, char *text, SDL_Texture *texture, SDL_Rect *destRect);

// removes a text entity from the UI ECS
void deleteUiTextEntity(UIECS uiEcs, Uint64 index);

// frees the game ECS memory
void freeGECS(GameECS ecs);

// frees the UI ECS memory
void freeUIECS(UIECS uiEcs);

#endif // ECS_H