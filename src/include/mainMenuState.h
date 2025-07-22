#ifndef MAINMENUSTATE_H
#define MAINMENUSTATE_H

#include "global.h"
#include "engine.h"
#include "mainLoop.h"

// updates the components in the UI ECS
void updateMenuUI(ZENg zEngine);

// takes care of the events in the main menu
void handleMainMenuEvents(SDL_Event *event, ZENg zEngine);

// takes care of the rendering part in the main menu
void renderMainMenu(ZENg zEngine);

#endif // MAINMENUSTATE_H