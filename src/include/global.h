#ifndef GLOBAL_H
#define GLOBAL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vec2.h"

typedef int32_t Int32;
typedef int64_t Int64;

#define COLOR_WHITE_TRANSPARENT (SDL_Color){255, 255, 255, 155}  // Semi-transparent white
#define COLOR_WHITE (SDL_Color){255, 255, 255, 255}  // Normal text color
#define COLOR_YELLOW (SDL_Color){255, 255, 0, 255}  // Highlight color

// Compare two SDL_Color structs for equal R,G,B,A values
#define CMP_COLORS(c1, c2) \
    ((c1).r == (c2).r && (c1).g == (c2).g && (c1).b == (c2).b && (c1).a == (c2).a)

// Coordinate system axis
typedef enum {
    AXIS_NONE,  // For initializations
    AXIS_X,
    AXIS_Y
} Axis;

// A small enough value to be considered zero with floating point tolerance
#define EPSILON 0.001

#endif // GLOBAL_H