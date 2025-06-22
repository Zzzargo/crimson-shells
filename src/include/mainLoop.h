#ifndef MAINLOOP_H
#define MAINLOOP_H

#define COLOR_WHITE_TRANSPARENT (SDL_Color){255, 255, 255, 155}  // semi-transparent white
#define COLOR_WHITE (SDL_Color){255, 255, 255, 255}  // normal text color
#define COLOR_YELLOW (SDL_Color){255, 255, 0, 255}  // Highlight color

typedef enum {
    STATE_MAIN_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAME_OVER,
    STATE_EXIT
} GameState;

#include "ecs.h"
#include "fontManager.h"

// initialises SDL lib, the game window, the renderer
void initGame(SDL_Window **wdw, SDL_Renderer **rdr, GameECS *ecs, UIECS *uiEcs, FontManager *fonts);

// loads the main menu UI components into the ECS
void onEnterMainMenu(UIECS uiEcs, SDL_Renderer *rdr, FontManager fonts);

// clears the main menu UI components from the ECS
void onExitMainMenu(UIECS uiEcs, SDL_Renderer *rdr);

// updates the render components in the UI ECS
void updateMenuUI(UIECS uiEcs, SDL_Renderer *rdr);

// takes care of the events in the main menu
void handleMainMenuEvents(GameState *currState, SDL_Event *event, UIECS uiEcs, GameECS gEcs, SDL_Renderer *rdr);

// takes care of the rendering part in the main menu
void renderMainMenu(SDL_Renderer *rdr, UIECS uiEcs);

// loads the initial game entities into the ECS when entering the play state
void onEnterPlayState(GameECS ecs, SDL_Renderer *rdr);

// game loop
void renderPlayState(SDL_Renderer *rdr, GameState *currState, GameECS ecs, UIECS uiEcs, FontManager fonts);

#endif // MAINLOOP_H