#ifndef MAINLOOP_H
#define MAINLOOP_H

typedef enum {
    STATE_MAIN_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAME_OVER,
    STATE_EXIT
} GameState;

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>

// initialises SDL lib, the game window, the renderer
void initGame(SDL_Window **wdw, SDL_Renderer **rdr);

// main menu loop
void mainMenu(SDL_Renderer *rdr, GameState *currState, SDL_Event *event);

// game loop
void play(SDL_Renderer *rdr, GameState *currState, SDL_Rect *dot);

#endif // MAINLOOP_H