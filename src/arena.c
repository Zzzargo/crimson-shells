#include "include/arena.h"

Uint32 TILE_SIZE = 0;  // Size of a tile, will be initialized once the display is initialized

Vec2 tileToWorld(Int32 idx) {
    return (Vec2){
        .x = (double_t)(idx % ARENA_WIDTH) * TILE_SIZE,
        .y = (double_t)(idx / ARENA_WIDTH) * TILE_SIZE
    };
}

Int32 worldToTile(Vec2 pos) {
    Int32 x = (Int32)(pos.x / TILE_SIZE);
    Int32 y = (Int32)(pos.y / TILE_SIZE);
    
    // Ensure the coordinates are within bounds
    if (x < 0 || x >= ARENA_WIDTH || y < 0 || y >= ARENA_HEIGHT) {
        return -1;  // Invalid tile index
    }
    
    return y * ARENA_WIDTH + x;
}
