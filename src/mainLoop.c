#include "include/mainLoop.h"

void initGame(SDL_Window **wdw, SDL_Renderer **rdr, ECS *ecs) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // Initialize SDL TTF
    if (TTF_Init() < 0) {
        printf("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Initialize ECS
    initECS(ecs);

    // Create a window
    (*wdw) = SDL_CreateWindow(
        "GOAT Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1600, 900,
        SDL_WINDOW_SHOWN
    );
    if (!wdw) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    // Create a renderer
    (*rdr) = SDL_CreateRenderer(*wdw, -1, SDL_RENDERER_ACCELERATED);
    if (!rdr) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(*wdw);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
}

void mainMenu(SDL_Renderer *rdr, GameState *currState, SDL_Event *event) {
    // Clear the screen
    SDL_SetRenderDrawColor(rdr, 100, 50, 0, 200);  // background color - brownish
    SDL_RenderClear(rdr);
    
    static int selectedOption = 0; // 0 = Play, 1 = Exit

    // Handle input
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_UP:
            case SDLK_w: {
                selectedOption = (selectedOption - 1 + 2) % 2; // Wrap around
                break;
            }
            case SDLK_DOWN:
            case SDLK_s: {
                selectedOption = (selectedOption + 1) % 2; // Wrap around
                break;
            }
            case SDLK_RETURN:
            case SDLK_SPACE: {
                if (selectedOption == 0) {
                    *currState = STATE_PLAYING;
                } else {
                    *currState = STATE_EXIT;
                }
                break;
            }
        }
    }

    TTF_Font *titleFont = TTF_OpenFont("assets/fonts/ByteBounce.ttf", 48);
    TTF_Font *menuFont = TTF_OpenFont("assets/fonts/ByteBounce.ttf", 24);
    
    if (!titleFont || !menuFont) {
        printf("Failed to load font! TTF_Error: %s\n", TTF_GetError());
        return;
    }

    SDL_Color whiteColor = {255, 255, 255, 255};  // normal text color
    SDL_Color yellowColor = {255, 255, 0, 255};  // Highlight color

    // Render title
    SDL_Color textColor = {255, 255, 255, 155};  // white color, semi-transparent
    SDL_Surface *titleSurface = TTF_RenderText_Solid(titleFont, "GOAT Game", textColor);    
    if (!titleSurface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_Texture *titleTexture = SDL_CreateTextureFromSurface(rdr, titleSurface);
    if (!titleTexture){
        printf("Failed to create text texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_Rect titleRect = {800 - titleSurface->w/2, 200, titleSurface->w, titleSurface->h};
    SDL_RenderCopy(rdr, titleTexture, NULL, &titleRect);
    
    // Render "Play" option
    SDL_Color playColor = (selectedOption == 0) ? yellowColor : whiteColor;
    SDL_Surface *playSurface = TTF_RenderText_Solid(menuFont, "Play", playColor);
    SDL_Texture *playTexture = SDL_CreateTextureFromSurface(rdr, playSurface);
    SDL_Rect playRect = {800 - playSurface->w/2, 400, playSurface->w, playSurface->h};
    SDL_RenderCopy(rdr, playTexture, NULL, &playRect);
    
    // Render "Exit" option
    SDL_Color exitColor = (selectedOption == 1) ? yellowColor : whiteColor;
    SDL_Surface *exitSurface = TTF_RenderText_Solid(menuFont, "Exit", exitColor);
    SDL_Texture *exitTexture = SDL_CreateTextureFromSurface(rdr, exitSurface);
    SDL_Rect exitRect = {800 - exitSurface->w/2, 450, exitSurface->w, exitSurface->h};
    SDL_RenderCopy(rdr, exitTexture, NULL, &exitRect);
    
    // Render instructions
    SDL_Surface *instrSurface = TTF_RenderText_Solid(menuFont, "Use W/S or Arrow Keys to navigate, Enter/Space to select", whiteColor);
    SDL_Texture *instrTexture = SDL_CreateTextureFromSurface(rdr, instrSurface);
    SDL_Rect instrRect = {800 - instrSurface->w/2, 600, instrSurface->w, instrSurface->h};
    SDL_RenderCopy(rdr, instrTexture, NULL, &instrRect);

    SDL_RenderPresent(rdr);  // render the current frame
}

void play(SDL_Renderer *rdr, GameState *currState, SDL_Rect *dot) {
    // Clear the screen
    SDL_SetRenderDrawColor(rdr, 100, 50, 0, 200);  // background color - brownish
    SDL_RenderClear(rdr);

    // Dot
    int VSpeed = 5, HSpeed = 5;

    // Keyboard state
    const Uint8* keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_ESCAPE]) {
        *currState = STATE_MAIN_MENU;  // Go back to main menu
    }
    if (keys[SDL_SCANCODE_W]) {
        if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_D]) {
            // Diagonal movement
            dot->y -= VSpeed / 1.414; // 1/sqrt(2) - normalisation of speed vector
        } else {
            dot->y -= VSpeed;
        }
    }
    if (keys[SDL_SCANCODE_S]) {
        if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_D]) {
            dot->y += VSpeed / 1.414;
        } else {
            dot->y += VSpeed;
        }
    }
    if (keys[SDL_SCANCODE_A]) {
        if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_W]) {
            dot->x -= HSpeed / 1.414;
        } else {
            dot->x -= HSpeed;
        }
    }
    if (keys[SDL_SCANCODE_D]) {
        if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_W]) {
            dot->x += HSpeed / 1.414;
        } else {
            dot->x += HSpeed;
        }
    }

     // Draw the dot (white)
    SDL_SetRenderDrawColor(rdr, 255, 255, 255, 255);
    SDL_RenderFillRect(rdr, dot);
    SDL_RenderPresent(rdr);  // render the current frame
}