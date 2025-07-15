#include "include/engine.h"

ZENg initGame() {
    ZENg zEngine = calloc(1, sizeof(struct engine));
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Initialize the input manager
    zEngine->inputMng = calloc(1, sizeof(struct inputmng));
    if (!zEngine->inputMng) {
        printf("Failed to allocate memory for input manager\n");
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    loadKeyBindings(zEngine->inputMng, "keys.cfg");
    zEngine->inputMng->keyboardState = SDL_GetKeyboardState(NULL);  // get the current keyboard state

    // Initialize font manager
    initFonts(&zEngine->fonts);

    // Initialize ECS
    initGECS(&zEngine->gEcs);
    initUIECS(&zEngine->uiEcs);

    // Create a window
    (zEngine->window) = SDL_CreateWindow(
        "GOAT Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280, 720,  // a medium 16:9 window in the center of the monitor if windowed mode
        SDL_WINDOW_FULLSCREEN_DESKTOP
    );
    if (!zEngine->window) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Create a renderer
    (zEngine->renderer) = SDL_CreateRenderer(zEngine->window, -1, SDL_RENDERER_ACCELERATED);
    if (!zEngine->renderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(zEngine->window);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // start on the main menu
    zEngine->state = STATE_MAIN_MENU;
    return zEngine;
}

void velocitySystem(ZENg zEngine, double_t deltaTime) {
    // for each entity with a VELOCITY_COMPONENT, update its position based on the current velocity
    for (Uint64 i = 0; i < zEngine->gEcs->components[VELOCITY_COMPONENT].denseSize; i++) {
        VelocityComponent *velComp = (VelocityComponent *)(zEngine->gEcs->components[VELOCITY_COMPONENT].dense[i]);
        Entity entitty = zEngine->gEcs->components[VELOCITY_COMPONENT].denseToEntity[i];  // get the owner entity

        if ((zEngine->gEcs->componentsFlags[entitty] & (1 << POSITION_COMPONENT)) == (1 << POSITION_COMPONENT)) {
            // if the entity has also the position component
            Uint64 page = entitty / PAGE_SIZE;  // determine the page for the entity
            Uint64 index = entitty % PAGE_SIZE;  // determine the index within the page
            Uint64 denseIndex = zEngine->gEcs->components[POSITION_COMPONENT].sparse[page][index];
            if (denseIndex >= zEngine->gEcs->components[POSITION_COMPONENT].denseSize) {
                printf("Warning: Entity %ld has a position component with invalid dense index %lu\n", entitty, denseIndex);
                continue;
            }
            PositionComponent *posComp = (PositionComponent *)(zEngine->gEcs->components[POSITION_COMPONENT].dense[denseIndex]);

            // Update the position based on the velocity
            posComp->x += velComp->currVelocity.x * deltaTime;
            posComp->y += velComp->currVelocity.y * deltaTime;

            // clamp the position to the window bounds
            if (posComp->x < 0) posComp->x = 0;  // prevent going out of bounds
            if (posComp->y < 0) posComp->y = 0;  // prevent going out of bounds

            int wW, wH;
            SDL_GetWindowSize(zEngine->window, &wW, &wH);
            Uint64 denseRenderIndex = zEngine->gEcs->components[RENDER_COMPONENT].sparse[page][index];
            SDL_Rect *entityRect = ((RenderComponent *)(zEngine->gEcs->components[RENDER_COMPONENT].dense[denseRenderIndex]))->destRect;
            if (entityRect) {
                if (posComp->x + entityRect->w > wW) {
                    posComp->x = wW - entityRect->w;  // prevent going out of bounds
                }
                if (posComp->y + entityRect->h > wH) {
                    posComp->y = wH - entityRect->h;  // prevent going out of bounds
                }
            }
        }
    }
}

void transformSystem(ECS ecs) {
    // iterate through all entities with POSITION_COMPONENT and update their rendered textures
    for (Uint64 i = 0; i < ecs->components[POSITION_COMPONENT].denseSize; i++) {
        PositionComponent *posComp = (PositionComponent *)(ecs->components[POSITION_COMPONENT].dense[i]);
        Entity entitty = ecs->components[POSITION_COMPONENT].denseToEntity[i];  // get the owner entity

        if ((ecs->componentsFlags[entitty] & (1 << RENDER_COMPONENT)) == (1 << RENDER_COMPONENT)) {
            // if the entity has also the render component
            Uint64 page = entitty / PAGE_SIZE;  // determine the page for the entity
            Uint64 index = entitty % PAGE_SIZE;  // determine the index within the page
            Uint64 denseIndex = ecs->components[RENDER_COMPONENT].sparse[page][index];
            if (denseIndex >= ecs->components[RENDER_COMPONENT].denseSize) {
                printf("Warning: Entity %ld has a render component with invalid dense index %lu\n", entitty, denseIndex);
                continue;
            }
            // Update the render component's destination rectangle based on the position component
            RenderComponent *renderComp = (RenderComponent *)(ecs->components[RENDER_COMPONENT].dense[denseIndex]);

            if (renderComp) {  // sanity check cause I've been pretty insane lately
                renderComp->destRect->x = (int)posComp->x;
                renderComp->destRect->y = (int)posComp->y;
            }
        }
    }
}

void destroyEngine(ZENg *zEngine) {
    // save the current input bindings to the config file
    saveKeyBindings((*zEngine)->inputMng, "keys.cfg");
    free((*zEngine)->inputMng);

    freeECS((*zEngine)->gEcs);
    freeECS((*zEngine)->uiEcs);
    freeFonts(&((*zEngine)->fonts));

    SDL_DestroyRenderer((*zEngine)->renderer);
    SDL_DestroyWindow((*zEngine)->window);
    SDL_Quit();
    free(*zEngine);
}