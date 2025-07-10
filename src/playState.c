#include "include/playState.h"

void handlePlayStateEvents(SDL_Event *e, ZENg zEngine) {
    if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
            case SDLK_ESCAPE: {
                zEngine->state = STATE_PAUSED;
            }
        }
    }
}

void updatePlayState(ZENg zEngine) {
    // Keyboard state
    const Uint8* keys = SDL_GetKeyboardState(NULL);

    VelocityComponent *playerSpeed = (VelocityComponent *)(zEngine->gEcs->components[VELOCITY_COMPONENT].dense[PLAYER_ID]);
    PositionComponent *playerPos = (PositionComponent *)(zEngine->gEcs->components[POSITION_COMPONENT].dense[PLAYER_ID]);
    SDL_Rect *playerRect = (*(RenderComponent *)(zEngine->gEcs->components[RENDER_COMPONENT].dense[PLAYER_ID])).destRect;
    printf("%f %f\n", playerPos->x, playerPos->y);
    playerSpeed->currVelocity = 0.0;
    if (keys[SDL_SCANCODE_W]) {
        if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_D]) {
            // Diagonal movement - Speed / 1.414
            playerSpeed->currVelocity = playerSpeed->maxVelocity / 1.414;  // 1 / sqrt(2) - normalization
        } else {
            playerSpeed->currVelocity = playerSpeed->maxVelocity;  // Normal speed
        }
        playerPos->y -= playerSpeed->currVelocity;
        playerRect->y -= playerSpeed->currVelocity;
    }
    if (keys[SDL_SCANCODE_S]) {
        if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_D]) {
            // Diagonal movement - Speed / 1.414
            playerSpeed->currVelocity = playerSpeed->maxVelocity / 1.414;  // 1 / sqrt(2) - normalization
        } else {
            playerSpeed->currVelocity = playerSpeed->maxVelocity;  // Normal speed
        }
        playerPos->y += playerSpeed->currVelocity;
        playerRect->y += playerSpeed->currVelocity;
    }
    if (keys[SDL_SCANCODE_A]) {
        if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_S]) {
            // Diagonal movement - Speed / 1.414
            playerSpeed->currVelocity = playerSpeed->maxVelocity / 1.414;  // 1 / sqrt(2) - normalization
        } else {
            playerSpeed->currVelocity = playerSpeed->maxVelocity;  // Normal speed
        }
        playerPos->x -= playerSpeed->currVelocity;
        playerRect->x -= playerSpeed->currVelocity;
    }
    if (keys[SDL_SCANCODE_D]) {
        if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_S]) {
            // Diagonal movement - Speed / 1.414
            playerSpeed->currVelocity = playerSpeed->maxVelocity / 1.414;  // 1 / sqrt(2) - normalization
        } else {
            playerSpeed->currVelocity = playerSpeed->maxVelocity;  // Normal speed
        }
        playerPos->x += playerSpeed->currVelocity;
        playerRect->x += playerSpeed->currVelocity;
    }
}

void renderPlayState(ZENg zEngine) {
    // Clear the screen
    SDL_SetRenderDrawColor(zEngine->renderer, 20, 20, 20, 200);  // background color - grey
    SDL_RenderClear(zEngine->renderer);

    // Render game entities
    for (Uint64 i = 0; i < zEngine->gEcs->entityCount; i++) {
        RenderComponent *render = (RenderComponent *)(zEngine->gEcs->components[RENDER_COMPONENT].dense[i]);
        if (render) {
            SDL_RenderCopy(zEngine->renderer, render->texture, NULL, render->destRect);
        }
    }

    SDL_RenderPresent(zEngine->renderer);  // render the current frame
}