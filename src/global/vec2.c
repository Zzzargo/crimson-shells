#include "vec2.h"

double_t vec2_len(Vec2 v) {
    return sqrt(v.x * v.x + v.y * v.y);
}

/**
 * =====================================================================================================================
 */

Vec2 vec2_normalize(Vec2 v) {
    double_t len = sqrt(v.x * v.x + v.y * v.y);
    if (len > 0.0) {
        v.x /= len;
        v.y /= len;
    } else {
        v.x = 0.0;
        v.y = 0.0;
    }
    return v;
}