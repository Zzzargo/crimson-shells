#ifndef PLAYSTATE_H
#define PLAYSTATE_H

#include "tanki.h"
#include "engine.h"
#include "mainLoop.h"

// handles in-game events
void handlePlayStateEvents(SDL_Event *e, ZENg zEngine);

// game logic driven through continuous input
void handlePlayStateInput(ZENg zEngine);

// renders the in-game entities
void renderPlayState(ZENg zEngine);

#endif // PLAYSTATE_H