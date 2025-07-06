#ifndef MAINLOOP_H
#define MAINLOOP_H

#include "ecs.h"

// initialises SDL lib, the game window, the renderer
void initGame(SDL_Window **wdw, SDL_Renderer **rdr, ECS *gEcs, ECS *uiEcs, FontManager *fonts);

// used to poll events like key presses each frame. Returns the value for <bool>running
Uint8 handleEvents(SDL_Event *e, GameState *currState, SDL_Renderer *rdr, ECS uiEcs, ECS gEcs);

// renders each frame of the game according to the game state
void renderFrame(GameState *currState, SDL_Renderer *rdr, ECS uiEcs, ECS gEcs);

// loads the main menu UI components into the ECS
void onEnterMainMenu(ECS uiEcs, SDL_Renderer *rdr, FontManager fonts);

// clears the main menu UI components from the ECS
void onExitMainMenu(ECS uiEcs, SDL_Renderer *rdr);

// loads the initial game entities into the ECS when entering the play state
// void onEnterPlayState(GameECS ecs, SDL_Renderer *rdr);

// these are included last because they depend on the above functions
// and we don't want to upset the compiler :)
#include "mainMenuState.h"
#include "playState.h"

#endif // MAINLOOP_H