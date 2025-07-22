#ifndef GLOBAL_H
#define GLOBAL_H

#define COLOR_WHITE_TRANSPARENT (SDL_Color){255, 255, 255, 155}  // semi-transparent white
#define COLOR_WHITE (SDL_Color){255, 255, 255, 255}  // normal text color
#define COLOR_YELLOW (SDL_Color){255, 255, 0, 255}  // Highlight color

#define CMP_COLORS(c1, c2) \
    ((c1).r == (c2).r && (c1).g == (c2).g && (c1).b == (c2).b && (c1).a == (c2).a)

typedef enum {
    STATE_MAIN_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAME_OVER,
    STATE_EXIT
} GameState;

#endif // GLOBAL_H