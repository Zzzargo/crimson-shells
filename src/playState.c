#include "include/playState.h"

void handlePlayStateEvents(SDL_Event *e, ZENg zEngine) {
    if (e->type == SDL_KEYDOWN) {
        InputAction action = scancodeToAction(zEngine->inputMng, e->key.keysym.scancode);
        if (action == INPUT_UNKNOWN) {
            printf("Unknown input action for scancode %d\n", e->key.keysym.scancode);
            return;
        }
        switch (action) {
            case INPUT_BACK: {
                zEngine->state = STATE_PAUSED;
                break;
            }
            case INPUT_SHOOT: {
                // spawn a bullet entity
                Entity bulletID = createEntity(zEngine->gEcs);
                PositionComponent *bulletPos = calloc(1, sizeof(PositionComponent));
                if (!bulletPos) {
                    printf("Failed to allocate memory for bullet position component\n");
                    exit(EXIT_FAILURE);
                }

                SDL_Rect *playerRect = (*(RenderComponent *)(zEngine->gEcs->components[RENDER_COMPONENT].dense[PLAYER_ID])).destRect;
                PositionComponent *playerPos = (PositionComponent *)(zEngine->gEcs->components[POSITION_COMPONENT].dense[PLAYER_ID]);
                *bulletPos = (PositionComponent) {
                    playerPos->x + playerRect->w / 2,  // center the bullet on the player
                    playerPos->y + playerRect->h / 2
                };

                addComponent(zEngine->gEcs, bulletID, POSITION_COMPONENT, (void *)bulletPos);

                DirectionComponent *bulletDir = calloc(1, sizeof(DirectionComponent));
                if (!bulletDir) {
                    printf("Failed to allocate memory for bullet direction component\n");
                    exit(EXIT_FAILURE);
                }
                *bulletDir = *(DirectionComponent *)(zEngine->gEcs->components[DIRECTION_COMPONENT].dense[PLAYER_ID]);  // bullet inherits the player's direction
                addComponent(zEngine->gEcs, bulletID, DIRECTION_COMPONENT, (void *)bulletDir);

                VelocityComponent *bulletSpeed = calloc(1, sizeof(VelocityComponent));
                if (!bulletSpeed) {
                    printf("Failed to allocate memory for bullet velocity component\n");
                    exit(EXIT_FAILURE);
                }
                *bulletSpeed = (VelocityComponent) {
                    (Vec2) {bulletDir->x * 300, bulletDir->y * 300},  // speed is 300 units per second
                    300.0,  // max speed
                    1  // active
                };
                addComponent(zEngine->gEcs, bulletID, VELOCITY_COMPONENT, (void *)bulletSpeed);

                RenderComponent *bulletRender = calloc(1, sizeof(RenderComponent));
                if (!bulletRender) {
                    printf("Failed to allocate memory for bullet render component\n");
                    exit(EXIT_FAILURE);
                }

                bulletRender->active = 1;
                bulletRender->selected = 0;
                bulletRender->destRect = calloc(1, sizeof(SDL_Rect));
                if (!bulletRender->destRect) {
                    printf("Failed to allocate memory for bullet rectangle\n");
                    exit(EXIT_FAILURE);
                }
                *bulletRender->destRect = (SDL_Rect){
                    .x = (int)bulletPos->x,
                    .y = (int)bulletPos->y,
                    .w = 10,  // bullet size
                    .h = 10
                };
                bulletRender->texture = SDL_CreateTexture(
                    zEngine->renderer,
                    SDL_PIXELFORMAT_RGBA8888,
                    SDL_TEXTUREACCESS_TARGET,
                    bulletRender->destRect->w,
                    bulletRender->destRect->h
                );
                if (!bulletRender->texture) {
                    printf("Failed to create bullet texture: %s\n", SDL_GetError());
                    exit(EXIT_FAILURE);
                }
                addComponent(zEngine->gEcs, bulletID, RENDER_COMPONENT, (void *)bulletRender);
            }
        }
    }
}

void handlePlayStateInput(ZENg zEngine) {
    // Keyboard state
    zEngine->inputMng->keyboardState = SDL_GetKeyboardState(NULL);

    VelocityComponent *playerSpeed = (VelocityComponent *)(zEngine->gEcs->components[VELOCITY_COMPONENT].dense[PLAYER_ID]);
    PositionComponent *playerPos = (PositionComponent *)(zEngine->gEcs->components[POSITION_COMPONENT].dense[PLAYER_ID]);
    DirectionComponent *playerDir = (DirectionComponent *)(zEngine->gEcs->components[DIRECTION_COMPONENT].dense[PLAYER_ID]);

    Uint8 moving = 0;  // flag to check if the player is moving
    if (isActionPressed(zEngine->inputMng, INPUT_MOVE_UP)) {
        *playerDir = DIR_UP;
        moving = 1;
    }
    if (isActionPressed(zEngine->inputMng, INPUT_MOVE_DOWN)) {
        *playerDir = DIR_DOWN;
        moving = 1;
    }
    if (isActionPressed(zEngine->inputMng, INPUT_MOVE_LEFT)) {
        *playerDir = DIR_LEFT;
        moving = 1;
    }
    if (isActionPressed(zEngine->inputMng, INPUT_MOVE_RIGHT)) {
        *playerDir = DIR_RIGHT;
        moving = 1;
    }

    if (moving) {
        // set the velocity vector
        playerSpeed->currVelocity.x = playerSpeed->maxVelocity * playerDir->x * 150;
        playerSpeed->currVelocity.y = playerSpeed->maxVelocity * playerDir->y * 150;
    } else {
        // If no movement input, stop the player
        playerSpeed->currVelocity = (Vec2) { .x = 0.0, .y = 0.0 };
    }
}

void renderPlayState(ZENg zEngine) {
    // Clear the screen
    SDL_SetRenderDrawColor(zEngine->renderer, 20, 20, 20, 200);  // background color - grey
    SDL_RenderClear(zEngine->renderer);

    // Render game entities
    for (Uint64 i = 0; i < zEngine->gEcs->components[RENDER_COMPONENT].denseSize; i++) {
        RenderComponent *render = (RenderComponent *)(zEngine->gEcs->components[RENDER_COMPONENT].dense[i]);
        if (render) {
            SDL_RenderCopy(zEngine->renderer, render->texture, NULL, render->destRect);
        }
    }
}