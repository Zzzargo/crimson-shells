#ifndef MAINLOOP_H
#define MAINLOOP_H

#include "engine.h"

// used to poll events like key presses each frame. Returns the value for <bool>running
Uint8 handleEvents(SDL_Event *e, ZENg zEngine);

//
void handleInput(ZENg zEngine);

// updates physics and game logic before rendering each frame
void updateGameLogic(ZENg zEngine, double_t deltaTime);

// renders each frame of the game according to the game state
void renderFrame(ZENg zEngine);

// loads the main menu UI components into the ECS
void onEnterMainMenu(ZENg zEngine);

// clears the main menu UI components from the ECS
void onExitMainMenu(ZENg zEngine);

// loads the initial game entities into the ECS when entering the play state
void onEnterPlayState(ZENg zEngine);

// deletes the game entities
void onExitPlayState(ZENg zEngine);

// these are included last because they depend on the above functions
// and we don't want to upset the compiler :)
#include "mainMenuState.h"
#include "playState.h"
#include "pauseState.h"

#endif // MAINLOOP_H