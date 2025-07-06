#include "include/mainLoop.h"

int main(int argc, char* argv[]) {
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    FontManager fonts = NULL;
    ECS gEcs = NULL;
    ECS uiEcs = NULL;

    initGame(&window, &renderer, &gEcs, &uiEcs, &fonts);
    // start on the main menu
    GameState currState = STATE_MAIN_MENU;
    // prepare the main menu UI components
    onEnterMainMenu(uiEcs, renderer, fonts);

    // Main loop
    Uint8 running = 1;  // could have used bool, but it takes 8 bits anyway
    SDL_Event event;  // this will be used to poll events

    while (running) {
        while (SDL_PollEvent(&event)) {
            running = handleEvents(&event, &currState, renderer, uiEcs, gEcs);

            if (event.type == SDL_QUIT || currState == STATE_EXIT) {
                running = 0;  // quitting via the window close button
            }
        }

        renderFrame(&currState, renderer, uiEcs, gEcs);
        SDL_Delay(16); // ~60 FPS cap
    }

    // Cleanup
    freeECS(gEcs);
    freeECS(uiEcs);
    freeFonts(&fonts);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
