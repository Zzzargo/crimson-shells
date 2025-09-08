#ifndef ARENA_H
#define ARENA_H

#include "global.h"

#define ARENA_WIDTH 64  // Arena width, in tiles
#define ARENA_HEIGHT 36  // Arena height, in tiles
extern Uint32 TILE_SIZE;  // Size of a tile, in pixels

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
    Tile tileDefs[TILE_COUNT];  // Definitions for each tile type, initialized at engine start
    Tile **tiles;  // 2D array of tiles representing the arena
} *Arena;

/**
 * Converts a tile's index to vector coordinates
 * @param arena pointer to the arena structure
 * @param idx the index of the tile
 * @return a Vec2 representing the tile's position in the arena
 */
Vec2 tileToWorld(Arena arena, Uint32 idx);

/**
 * Converts world coordinates to a tile index
 * @param arena pointer to the arena structure
 * @param pos the position in world coordinates
 * @return the index of the tile in the arena grid
 */
Uint32 worldToTile(Arena arena, Vec2 pos);

#endif // ARENA_H