#include "include/mainMenuState.h"

// void updateMenuUI(UIECS uiEcs, SDL_Renderer *rdr) {
//     // Rerender the UI based on the entities' current state

//     SDL_SetRenderDrawColor(rdr, 100, 50, 0, 200);  // background color - brownish
//     SDL_RenderClear(rdr);  // clear the renderer

//     for (Uint64 i = 0; i < uiEcs->entityCount; i++) {
//         TextComponent *curr = &uiEcs->textComponents[i];
//         if (
//             CMP_COLORS(curr->color, COLOR_WHITE)
//             || CMP_COLORS(curr->color, COLOR_YELLOW)
//         ) {
//             if (curr->active) {
//                 // Update the texture or other properties of selectables if active
//                 SDL_DestroyTexture(curr->texture);
//                 SDL_Surface *surface = TTF_RenderText_Solid(
//                     curr->font,
//                     curr->text,
//                     curr->selected ? COLOR_YELLOW : COLOR_WHITE
//                 );
//                 curr->texture = SDL_CreateTextureFromSurface(rdr, surface);
//                 SDL_RenderCopy(rdr, curr->texture, NULL, curr->destRect);
//                 SDL_FreeSurface(surface);
//             }
//         }
//     }
// }

// void handleMainMenuEvents(GameState *currState, SDL_Event *event, UIECS uiEcs, GameECS gEcs, SDL_Renderer *rdr) {
//     // Handle input
//     if (event->type == SDL_KEYDOWN) {
//         switch (event->key.keysym.sym) {
//             case SDLK_UP:
//             case SDLK_w: {
//                 for (Uint64 i = 0; i < uiEcs->entityCount; i++) {
//                     TextComponent *curr = &uiEcs->textComponents[i];
//                     if (curr->selected) {
//                         curr->selected = 0;  // Deselect current
//                         if (strcmp(curr->text, "Play") == 0) {
//                             // "Play" was selected, wrap around to "Exit"
//                             for (Uint64 j = i; j < uiEcs->entityCount; j++) {
//                                 if (strcmp(uiEcs->textComponents[j].text, "Exit") == 0) {
//                                     uiEcs->textComponents[j].selected = 1;  // select "Exit"
//                                     break;
//                                 }
//                             }
//                         } else {
//                             // select the previous item
//                             uiEcs->textComponents[i - 1].selected = 1;
//                         }
//                         updateMenuUI(uiEcs, rdr);
//                         break;
//                     }
//                 }
//                 break;
//             }
//             case SDLK_DOWN:
//             case SDLK_s: {
//                 for (Uint64 i = 0; i < uiEcs->entityCount; i++) {
//                     TextComponent *curr = &uiEcs->textComponents[i];
//                     if (curr->selected) {
//                         curr->selected = 0;  // Deselect current
//                         if (strcmp(curr->text, "Exit") == 0) {
//                             // "Exit" was selected, wrap around to "Play"
//                             for (Uint64 j = 0; j < uiEcs->entityCount; j++) {
//                                 if (strcmp(uiEcs->textComponents[j].text, "Play") == 0) {
//                                     uiEcs->textComponents[j].selected = 1;  // select "Play"
//                                     break;
//                                 }
//                             }
//                         } else {
//                             // select the next item
//                             uiEcs->textComponents[i + 1].selected = 1;
//                         }
//                         updateMenuUI(uiEcs, rdr);
//                         break;
//                     }
//                 }
//                 break;
//             }
//             case SDLK_RETURN:
//             case SDLK_SPACE: {
//                 for (Uint64 i = 0; i < uiEcs->entityCount; i++) {
//                     TextComponent *curr = &uiEcs->textComponents[i];
//                     if (curr->selected) {
//                         if (strcmp(curr->text, "Play") == 0) {
//                             onExitMainMenu(uiEcs, rdr);  // Clear the main menu UI
//                             onEnterPlayState(gEcs, rdr);
//                             *currState = STATE_PLAYING;  // Start the game
//                         } else if (strcmp(curr->text, "Exit") == 0) {
//                             *currState = STATE_EXIT;  // Exit the game
//                         }
//                     }
//                 }
//                 break;
//             }
//         }
//     }
// }

void renderMainMenu(SDL_Renderer *rdr, ECS uiEcs) {
    // Clear the screen
    SDL_SetRenderDrawColor(rdr, 100, 50, 0, 200);  // background color - brownish
    SDL_RenderClear(rdr);

    bitset targetMask = 1 << TEXT_COMPONENT;

    for (Uint64 i = 0; i < uiEcs->entityCount; i++) {
        // printf("Component %ld's flag: %d\nTarget flag: %d\n", i, uiEcs->componentsFlags[i], targetMask);
        // printf("uiEcs->componentsFlags[i] (%d) & targetMask (%d) == targetMask (%d): %d\n", uiEcs->componentsFlags[i], targetMask, targetMask, (uiEcs->componentsFlags[i] & targetMask) == targetMask);
        if ((uiEcs->componentsFlags[i] & targetMask) == targetMask) {
            // if the entity has the text component
            SDL_RenderCopy(
                rdr,
                (*(TextComponent *)(uiEcs->components[TEXT_COMPONENT].dense[i])).texture,
                NULL,
                (*(TextComponent *)(uiEcs->components[TEXT_COMPONENT].dense[i])).destRect
            );
        }
    }

    SDL_RenderPresent(rdr);  // render current frame
}