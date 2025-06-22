#include "include/mainLoop.h"

int main(int argc, char* argv[]) {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    FontManager fonts = NULL;
    GameECS ecs = NULL;
    UIECS uiEcs = NULL;

    initGame(&window, &renderer, &ecs, &uiEcs, &fonts);
    // start on the main menu
    GameState currState = STATE_MAIN_MENU;
    // prepare the main menu UI components
    onEnterMainMenu(uiEcs, renderer, fonts);

    // Main loop
    Uint8 running = 1;  // could have used bool, but it takes 8 bits anyway
    SDL_Event event;  // this will be used to poll events

    while (running) {
        while (SDL_PollEvent(&event)) {
            // here handle the events
            if (event.type == SDL_QUIT) {
                running = 0;  // quitting via the window close button
            }
            switch (currState) {
                case STATE_MAIN_MENU: {
                    handleMainMenuEvents(&currState, &event, uiEcs, renderer);
                    break;
                }
                case STATE_PAUSED: {
                    // todo
                    break;
                }
                case STATE_GAME_OVER: {
                    // todo
                    break;
                }
                case STATE_EXIT: {
                    running = 0;
                    break;
                }
            }
        }

        // here just render
        switch (currState) {
            case STATE_MAIN_MENU: {
                renderMainMenu(renderer, uiEcs);
                break;
            }
            case STATE_PAUSED: {
                // todo
                break;
            }
            case STATE_GAME_OVER: {
                // todo
                break;
            }
            case STATE_EXIT: {
                running = 0;
                break;
            }
            case STATE_PLAYING: {
                play(renderer, &currState, &ecs);
                break;
            }
        }
        SDL_Delay(16); // ~60 FPS cap
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    freeGECS(ecs);
    freeUIECS(uiEcs);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
