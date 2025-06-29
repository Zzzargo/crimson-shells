#ifndef PLAYSTATE_H
#define PLAYSTATE_H

#include "ecs.h"
#include "tanki.h"

// game loop
void renderPlayState(SDL_Renderer *rdr, GameState *currState, GameECS ecs, UIECS uiEcs, FontManager fonts);

#endif // PLAYSTATE_H