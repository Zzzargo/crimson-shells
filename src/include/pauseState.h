#ifndef PAUSE_STATE_H
#define PAUSE_STATE_H

#include "tanki.h"
#include "engine.h"
#include "mainLoop.h"

void updatePauseUI(ZENg zEngine);

void handlePauseStateEvents(SDL_Event *e, ZENg zEngine);

void renderPauseState(ZENg zEngine);

#endif // PAUSE_STATE_H