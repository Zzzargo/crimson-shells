#include "include/playState.h"

// void renderPlayState(SDL_Renderer *rdr, GameState *currState, GameECS ecs, UIECS uiEcs, FontManager fonts) {
//     // Clear the screen
//     SDL_SetRenderDrawColor(rdr, 100, 50, 0, 200);  // background color - brownish
//     SDL_RenderClear(rdr);

//     // Keyboard state
//     const Uint8* keys = SDL_GetKeyboardState(NULL);
//     if (keys[SDL_SCANCODE_ESCAPE]) {
//         *currState = STATE_MAIN_MENU;  // Go back to main menu
//         onEnterMainMenu(uiEcs, rdr, fonts);
//     }
//     if (keys[SDL_SCANCODE_W]) {
//         SpeedComponent *dotSpeed = &ecs->speedComponents[0];
//         if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_D]) {
//             // Diagonal movement Speed / 1.414
//             dotSpeed->velocity = dotSpeed->maxSpeed / 1.414;  // 1 / sqrt(2) - normalization
//         } else {
//             dotSpeed->velocity = dotSpeed->maxSpeed;  // Normal speed
//         }
//         ecs->renderComponents[0].destRect->y -= dotSpeed->velocity;  // Move up
//     }
//     if (keys[SDL_SCANCODE_S]) {
//         SpeedComponent *dotSpeed = &ecs->speedComponents[0];
//         if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_D]) {
//             // Diagonal movement Speed / 1.414
//             dotSpeed->velocity = dotSpeed->maxSpeed / 1.414;
//         } else {
//             dotSpeed->velocity = dotSpeed->maxSpeed;
//         }
//         ecs->renderComponents[0].destRect->y += dotSpeed->velocity;  // Move down
//     }
//     if (keys[SDL_SCANCODE_A]) {
//         SpeedComponent *dotSpeed = &ecs->speedComponents[0];
//         if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_W]) {
//             dotSpeed->velocity = dotSpeed->maxSpeed / 1.414;
//         } else {
//             dotSpeed->velocity = dotSpeed->maxSpeed;
//         }
//         ecs->renderComponents[0].destRect->x -= dotSpeed->velocity;  // Move left
//     }
//     if (keys[SDL_SCANCODE_D]) {
//         SpeedComponent *dotSpeed = &ecs->speedComponents[0];
//         if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_W]) {
//             dotSpeed->velocity = dotSpeed->maxSpeed / 1.414;
//         } else {
//             dotSpeed->velocity = dotSpeed->maxSpeed;
//         }
//         ecs->renderComponents[0].destRect->x += dotSpeed->velocity;  // Move right
//     }

//     // Render game entities
//     // printf("Rendering %lu entities\n", ecs->entityCount);
//     for (Uint64 i = 0; i < ecs->entityCount; i++) {
//         RenderComponent *render = &ecs->renderComponents[i];
//         if (render->active) {
//             SDL_RenderCopy(rdr, render->texture, NULL, render->destRect);
//         }
//     }

//     SDL_RenderPresent(rdr);  // render the current frame
// }