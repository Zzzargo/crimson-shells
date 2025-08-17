#ifndef ARENA_H
#define ARENA_H

#include "global.h"

#define ARENA_WIDTH 48
#define ARENA_HEIGHT 27
extern Uint32 TILE_SIZE;  // Size of a tile, will be initialized once the display is initialized

typedef enum {
    TILE_EMPTY,
    TILE_GRASS,
    TILE_WATER,
    TILE_ROCK,
    TILE_BRICKS,
    TILE_WOOD,
    TILE_SPAWN,
    TILE_COUNT  // Automatically counts
} TileType;

typedef struct {
    SDL_Texture *texture;  // Tile sprite
    double_t speedMod;  // Speed modifier for entities on this tile
    Int32 damage;  // Damage dealt to entities on this tile
    TileType type;  // Type of the tile
    Uint32 idx;  // Index of the tile inside the arena grid
    Uint8 isWalkable;
    Uint8 isSolid;  // If true, projectiles cannot pass through
} Tile;

typedef struct map {
    Tile tiles[ARENA_WIDTH][ARENA_HEIGHT];
} *Arena;

/**
 * Converts a tile's index to vector coordinates
 */
Vec2 tileToWorld(Int32 idx);

/**
 * Converts world coordinates to a tile index
 */
Int32 worldToTile(Vec2 pos);

#endif // ARENA_H