#ifndef MAINLOOP_H
#define MAINLOOP_H

#include "ecs.h"

// initialises SDL lib, the game window, the renderer
// void initGame(SDL_Window **wdw, SDL_Renderer **rdr, GameECS *ecs, UIECS *uiEcs, FontManager *fonts);

// loads the main menu UI components into the ECS
// void onEnterMainMenu(UIECS uiEcs, SDL_Renderer *rdr, FontManager fonts);

// clears the main menu UI components from the ECS
// void onExitMainMenu(UIECS uiEcs, SDL_Renderer *rdr);

// loads the initial game entities into the ECS when entering the play state
// void onEnterPlayState(GameECS ecs, SDL_Renderer *rdr);

// these are included last because they depend on the above functions
// and we don't want to upset the compiler :)
#include "mainMenuState.h"
#include "playState.h"

#endif // MAINLOOP_H