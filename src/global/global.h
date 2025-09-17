#ifndef GLOBAL_H
#define GLOBAL_H

// #define DEBUG

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vec2.h"
#include "DLinkList.h"

typedef struct engine *ZENg;  // Forward declaration of the engine struct

typedef int32_t Int32;
typedef int64_t Int64;

#define LOGICAL_WIDTH 1280
#define LOGICAL_HEIGHT 720

/**
 * 1280x720 => tilesize = 1280 / (16 * k) = 80 / k
 * An integer k makes the tiles square and integer sized
 * I chose k = 4 => TILE_SIZE = 20
 * So the arena will be 64x36 tiles
 * Keep in mind that the level files are going to be written for this arena size only
 */

// Compare two SDL_Color structs for equal R,G,B,A values
#define CMP_COLORS(c1, c2) \
    ((c1).r == (c2).r && (c1).g == (c2).g && (c1).b == (c2).b && (c1).a == (c2).a)

#define OPACITY_FULL 255
#define OPACITY_HIGH 200
#define OPACITY_MEDIUM 150
#define OPACITY_LOW 100
#define OPACITY_VERYLOW 50
#define OPACITY_TRANSPARENT 0

#define COLOR_WITH_ALPHA(c, a) (SDL_Color){(c).r, (c).g, (c).b, (a)}

#define COLOR_WHITE (SDL_Color){255, 255, 255, 255}
#define COLOR_BLACK (SDL_Color){0, 0, 0, 255}
#define COLOR_GRAY (SDL_Color){128, 128, 128, 255}
#define COLOR_RED (SDL_Color){255, 0, 0, 255}
#define COLOR_GREEN (SDL_Color){0, 255, 0, 255}
#define COLOR_BLUE (SDL_Color){0, 0, 255, 255}
#define COLOR_YELLOW (SDL_Color){255, 255, 0, 255}
#define COLOR_CYAN (SDL_Color){0, 255, 255, 255}
#define COLOR_MAGENTA (SDL_Color){255, 0, 255, 255}
#define COLOR_CRIMSON (SDL_Color){220, 20, 60, 255}
#define COLOR_PURPLE (SDL_Color){128, 0, 128, 255}

// Coordinate system axis
typedef enum {
    AXIS_NONE,  // For initializations
    AXIS_X,
    AXIS_Y
} Axis;

// Moving directions - pay attention to the direction of the Y axis in SDL

#define DIR_LEFT (Vec2){-1.0, 0.0}
#define DIR_RIGHT (Vec2){1.0, 0.0}
#define DIR_UP (Vec2){0.0, -1.0}
#define DIR_DOWN (Vec2){0.0, 1.0}

#endif // GLOBAL_H