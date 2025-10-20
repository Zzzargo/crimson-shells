#ifndef GLOBAL_H
#define GLOBAL_H

#define DEBUG
#define DEBUGPP
#define DEBUGSYSTEMS

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include "../thirdparty/cJSON.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vec2.h"
#include "DLinkList.h"
#include "hashMap.h"

typedef struct engine *ZENg;  // Forward declaration of the engine struct

/**
 * This enum is used to differentiate between entity types only at creation, for example in arena initialization
 * It should never be added as a component to an entity for the sake of ECS purity
*/
typedef enum {
    ENTITY_PLAYER,  // Your tank
    ENTITY_TANK_BASIC,
    ENTITY_TANK_LIGHT,
    ENTITY_TANK_HEAVY,
    ENTITY_TYPE_COUNT  // Automatically counts
} EntityType;

typedef int32_t Int32;
typedef int64_t Int64;

#define LOGICAL_WIDTH 1920
#define LOGICAL_HEIGHT 1080

/**
 * 1920x1080 => tilesize = 1920 / (16 * k) = 120 / k
 * An integer k makes the tiles square and integer sized
 * I chose k = 4 => TILE_SIZE = 30
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
#define COLOR_BLUE_DARK (SDL_Color){28, 22, 43, 255}
#define COLOR_YELLOW (SDL_Color){255, 255, 0, 255}
#define COLOR_CYAN (SDL_Color){0, 255, 255, 255}
#define COLOR_MAGENTA (SDL_Color){255, 0, 255, 255}
#define COLOR_CRIMSON (SDL_Color){180, 0, 40, 255}
#define COLOR_PINK (SDL_Color){255, 0, 120, 255}
#define COLOR_CRIMSON_DARK (SDL_Color){150, 0, 20, 255}
#define COLOR_BROWN (SDL_Color){160, 82, 45, 255}
#define COLOR_PURPLE (SDL_Color){128, 0, 128, 255}
#define COLOR_GOLD (SDL_Color){210, 105, 30, 255}

typedef enum {
    IDX_WHITE,
    IDX_BLACK,
    IDX_GRAY,
    IDX_RED,
    IDX_GREEN,
    IDX_BLUE,
    IDX_BLUE_DARK,
    IDX_YELLOW,
    IDX_CYAN,
    IDX_MAGENTA,
    IDX_CRIMSON,
    IDX_PINK,
    IDX_CRIMSON_DARK,
    IDX_BROWN,
    IDX_PURPLE,
    IDX_GOLD,
    COLOR_TABLE_SIZE
} ColorIndex;

// Sorry about this
static const SDL_Color COLOR_TABLE[] = {
    [IDX_WHITE] = COLOR_WHITE,
    [IDX_BLACK] = COLOR_BLACK,
    [IDX_GRAY] = COLOR_GRAY,
    [IDX_RED] = COLOR_RED,
    [IDX_GREEN] = COLOR_GREEN,
    [IDX_BLUE] = COLOR_BLUE,
    [IDX_BLUE_DARK] = COLOR_BLUE_DARK,
    [IDX_YELLOW] = COLOR_YELLOW,
    [IDX_CYAN] = COLOR_CYAN,
    [IDX_MAGENTA] = COLOR_MAGENTA,
    [IDX_CRIMSON] = COLOR_CRIMSON,
    [IDX_PINK] = COLOR_PINK,
    [IDX_CRIMSON_DARK] = COLOR_CRIMSON_DARK,
    [IDX_BROWN] = COLOR_BROWN,
    [IDX_PURPLE] = COLOR_PURPLE,
    [IDX_GOLD] = COLOR_GOLD
};

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

// I just found out about those sorceries

#define THROW_ERROR_AND_RETURN(msg, ret) \
    do { \
        fprintf(stderr, "ERROR: %s\n", msg); \
        return ret; \
    } while (0)

#define THROW_ERROR_AND_RETURN_VOID(msg) \
    do { \
        fprintf(stderr, "ERROR: %s\n", msg); \
        return; \
    } while (0)

#define THROW_ERROR_AND_CONTINUE(msg) \
    do { \
        fprintf(stderr, "ERROR: %s\n", msg); \
        continue; \
    } while (0)

#define THROW_ERROR_AND_EXIT(msg) \
    do { \
        fprintf(stderr, "ERROR: %s\n", msg); \
        exit(EXIT_FAILURE); \
    } while (0)

// This one doesn't write a newline at the end of the error message
#define THROW_ERROR_AND_DO(msg, code) \
    do { \
        fprintf(stderr, "ERROR: %s", msg); \
        code \
    } while (0)

#endif // GLOBAL_H
