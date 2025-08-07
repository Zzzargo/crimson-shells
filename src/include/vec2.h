#ifndef VEC2_H
#define VEC2_H

#include <math.h>  // the almighty

// 2D vector structure
typedef struct vec2D {
    double_t x;
    double_t y;
} Vec2;

// get the length of a vector
double_t vec2_len(Vec2 v);

// normalize a vector
Vec2 vec2_normalize(Vec2 v);

// compare 2 vectors for equality
#define VEC2_EQUAL(v1, v2) \
    ((v1).x == (v2).x && (v1).y == (v2).y)

#endif // VEC2_H