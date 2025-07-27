#include "include/engine.h"

Entity PLAYER_ID = 0;  // will be set when the player is created

int main(int argc, char* argv[]) {
    ZENg zEngine = initGame();

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

        GameState *currState = zEngine->stateMng->states[zEngine->stateMng->top - 1];
        while (SDL_PollEvent(&event)) {
            running = currState->handleEvents(&event, zEngine);
            currState = getCurrState(zEngine->stateMng);  // get the current state after handling events
            if (!running) printf("Event handler returned 0 for the main loop\n");

            if (event.type == SDL_QUIT || (currState && currState->type) == STATE_EXIT) {
                running = 0;
            }
        }

        // currState can be NULL if the stack was popped, so check it
        if (currState && currState->handleInput) currState->handleInput(zEngine);
        if (currState && currState->update) currState->update(zEngine, deltaTime);
        if (currState && currState->render) currState->render(zEngine);
        
        SDL_RenderPresent(zEngine->display->renderer);
        
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
