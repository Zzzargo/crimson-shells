#include "include/mainLoop.h"

Entity PLAYER_ID = 0;  // will be set when the player is created

int main(int argc, char* argv[]) {
    ZENg zEngine = initGame();

    // prepare the main menu UI components
    onEnterMainMenu(zEngine);

    // time for some delta time (no pun intended)
    Uint64 lastFrameTime = SDL_GetTicks64();
    double_t deltaTime = 0.0;
    const double_t targetFrameTime = 1000.0 / 60.0; // capping at 60 fps

    // Main loop
    Uint8 running = 1;  // could have used bool, but it takes 8 bits anyway
    SDL_Event event;  // this will be used to poll events

    while (running) {
        Uint64 frameStart = SDL_GetTicks64();
        deltaTime = (frameStart - lastFrameTime) / 1000.0;  // ms to s
        lastFrameTime = frameStart;

        // Cap delta time to prevent spikes after lags
        if (deltaTime > 0.1) deltaTime = 0.1;  // max 100 ms per frame

        while (SDL_PollEvent(&event)) {
            running = handleEvents(&event, zEngine);

            if (event.type == SDL_QUIT || zEngine->state == STATE_EXIT) {
                running = 0;
            }
        }

        handleInput(zEngine);
        updateGameLogic(zEngine, deltaTime);
        renderFrame(zEngine);
        
        Uint64 frameTime = SDL_GetTicks64() - frameStart;
        if (frameTime < targetFrameTime) {
            // if the frame was loaded faster than the target FPS - wait a little
            SDL_Delay(targetFrameTime - frameTime);
        }
    }

    // Cleanup
    destroyEngine(&zEngine);
    return 0;
}
