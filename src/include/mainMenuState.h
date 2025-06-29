#ifndef MAINMENUSTATE_H
#define MAINMENUSTATE_H

#include "ecs.h"
#include "tanki.h"

// updates the render components in the UI ECS
void updateMenuUI(UIECS uiEcs, SDL_Renderer *rdr);

// takes care of the events in the main menu
void handleMainMenuEvents(GameState *currState, SDL_Event *event, UIECS uiEcs, GameECS gEcs, SDL_Renderer *rdr);

// takes care of the rendering part in the main menu
void renderMainMenu(SDL_Renderer *rdr, UIECS uiEcs);

#endif // MAINMENUSTATE_H