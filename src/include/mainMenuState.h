#ifndef MAINMENUSTATE_H
#define MAINMENUSTATE_H

#include "ecs.h"
#include "tanki.h"

// updates the components in the UI ECS
void updateMenuUI(ECS uiEcs, SDL_Renderer *rdr);

// takes care of the events in the main menu
void handleMainMenuEvents(SDL_Event *event, GameState *currState, SDL_Renderer *rdr, ECS uiEcs, ECS gEcs);

// takes care of the rendering part in the main menu
void renderMainMenu(SDL_Renderer *rdr, ECS uiEcs);

#endif // MAINMENUSTATE_H