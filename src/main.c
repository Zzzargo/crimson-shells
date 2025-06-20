#include "include/mainLoop.h"

int main(int argc, char* argv[]) {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    ECS ecs = NULL;

    initGame(&window, &renderer, &ecs);
    // start on the main menu
    GameState currState = STATE_MAIN_MENU;

    // Main loop
    Uint8 running = 1;
    SDL_Event event;
    SDL_Rect dot = {400, 300, 30, 30}; // x, y, width, height

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;  // quitting via the window close button
            }
            switch (currState) {
                case STATE_MAIN_MENU: {
                    mainMenu(renderer, &currState, &event);
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
        switch (currState) {
            case STATE_PLAYING: {
                play(renderer, &currState, &dot);
                break;
            }
        }
        SDL_Delay(16); // ~60 FPS cap
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
