#ifndef PLAYSTATE_H
#define PLAYSTATE_H

#include "global.h"
#include "engine.h"
#include "mainLoop.h"

// spawns a bullet with a given owner entity, in the same direction the owner is looking
void spawnBulletProjectile(ZENg zEngine, Entity owner);

// handles in-game events
void handlePlayStateEvents(SDL_Event *e, ZENg zEngine);

// game logic driven through continuous input
void handlePlayStateInput(ZENg zEngine);

// renders the in-game entities
void renderPlayState(ZENg zEngine);

#endif // PLAYSTATE_H