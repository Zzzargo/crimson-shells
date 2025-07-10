#ifndef ZENG_H
#define ZENG_H

#include "ecs.h"

typedef struct engine {
    // Window

    SDL_Window *window;
    SDL_Renderer *renderer;
    FontManager fonts;

    // Core engine systems

    ECS uiEcs;
    ECS gEcs;
    GameState state;
} *ZENg;

// initialises the engine
ZENg initGame();

// frees the memory used by the engine
void destroyEngine(ZENg *zEngine);

#endif // ZENG_H