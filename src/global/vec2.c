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

/**
 * =====================================================================================================================
 */

double vec2_to_angle(Vec2 v) {
    // Flip Y axis because of SDL coordinate system
    double angle = atan2(-v.y, v.x) * (180.0 / M_PI);

    // Convert from "right is 0deg" to "up is 0deg"
    angle -= 90.0;

    // SDL_RenderCopyEx uses clockwise rotation => negate angle
    angle = -angle;
    
    // Normalize to [0, 360)
    if (angle < 0) angle += 360.0;
    return angle;
}