#include "include/fontManager.h"

void initFonts(FontManager *fonts) {
    // Initialize TTF and fonts
    if (TTF_Init() < 0) {
        printf("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    (*fonts) = malloc(sizeof(struct fontmng));
    if (!(*fonts)) {
        printf("Failed to allocate memory for FontManager\n");
        TTF_Quit();
        SDL_Quit();
        exit(EXIT_FAILURE);
    }
    // Load fonts
    (*fonts)->titleFont = TTF_OpenFont("assets/fonts/ByteBounce.ttf", 48);
    (*fonts)->menuFont = TTF_OpenFont("assets/fonts/ByteBounce.ttf", 28);
    (*fonts)->gameFont = TTF_OpenFont("assets/fonts/ByteBounce.ttf", 20);

    if (!(*fonts)->titleFont || !(*fonts)->menuFont || !(*fonts)->gameFont) {
        printf("Failed to load font! TTF_Error: %s\n", TTF_GetError());
        return;
    }
}

void freeFonts(FontManager *fonts) {
    TTF_CloseFont((*fonts)->titleFont);
    TTF_CloseFont((*fonts)->menuFont);
    TTF_CloseFont((*fonts)->gameFont);

    free(*fonts);
    TTF_Quit();
}