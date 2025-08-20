#ifndef VEC2_H
#define VEC2_H

#include <math.h>  // the almighty

// A small enough value to be considered zero with floating point tolerance
#define EPSILON 0.001

// 2D vector structure
typedef struct vec2D {
    double_t x;
    double_t y;
} Vec2;

/**
 * Get the length of a vector
 */
double_t vec2_len(Vec2 v);

/**
 * Normalize a vector
 */
Vec2 vec2_normalize(Vec2 v);

/**
 * Compare 2 vectors for equality
 */
#define VEC2_EQUAL(v1, v2) \
    ( \
        fabs((v1).x - (v2).x) < EPSILON && fabs((v1).y - (v2).y) < EPSILON \
    )

#endif // VEC2_H