#ifndef FONT_MANAGER_H
#define FONT_MANAGER_H

// I needed a way to have the fonts at hand at all times

#include <SDL2/SDL_ttf.h>

typedef struct fontmng {
    TTF_Font *titleFont;  // Headers and big text
    TTF_Font *menuFont;   // Slightly smaller text, for menus and options
    TTF_Font *gameFont;   // In-game text, smaller than menu font
} *FontManager;

// initializes TTF and loads the predefined fonts
void initFonts(FontManager *fonts);

void freeFonts(FontManager *fonts);

#endif