#ifndef ZENG_H
#define ZENG_H

#include "ecs.h"
#include "inputManager.h"
#include <math.h>  // of course we need math, vectors be vectoring

typedef struct engine {
    // Window

    SDL_Window *window;
    SDL_Renderer *renderer;
    FontManager fonts;

    // Core engine systems

    ECS uiEcs;
    ECS gEcs;
    InputManager inputMng;
    GameState state;
} *ZENg;

// initialises the engine
ZENg initGame();

// updates the entities' positions based on their velocity
void velocitySystem(ZENg zEngine, double_t deltaTime);

// updates the rendered entities based on their position
void transformSystem(ECS ecs);

// frees the memory used by the engine
void destroyEngine(ZENg *zEngine);

#endif // ZENG_H