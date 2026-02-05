#ifndef VEC2_H
#define VEC2_H

#define _USE_MATH_DEFINES
#include <math.h>  // the almighty

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

// A small enough value to be considered zero with floating point tolerance
#define EPSILON 0.001

// 2D vector structure
typedef struct vec2D {
    double_t x;
    double_t y;
} Vec2;

/**
 * Get the length of a vector
 * @param v the vector
 * @return the length of the vector
 */
double_t vec2_len(Vec2 v);

/**
 * Normalize a vector
 * @param v the vector to normalize
 * @return the normalized vector (unit length)
 */
Vec2 vec2_normalize(Vec2 v);

/**
 * Get the angle (in degrees) between Ox axis and the vector
 * @param v the vector
 * @return angle in degrees (0 to 360)
 */
double vec2_to_angle(Vec2 v);

/**
 * Compare 2 vectors for equality
 */
#define VEC2_EQUAL(v1, v2) \
    ( \
        fabs((v1).x - (v2).x) < EPSILON && fabs((v1).y - (v2).y) < EPSILON \
    )

#endif // VEC2_H