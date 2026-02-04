#ifndef MELTY_LOGIC_H
#define MELTY_LOGIC_H

#include "dshot_rmt.h"

// Standard C helper for constraining values
static inline float constrain_f(float val, float min, float max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

/**
 * @brief Tank Drive mixing for 3D Mode ESCs
 * Now uses dshot_map_3d from the driver to avoid redundancy.
 */
static inline void calculate_tank_drive_3d(float x, float y, uint16_t *m1, uint16_t *m2) {
    // Simple mixing (X: Steering, Y: Throttle)
    float left = y + x;
    float right = y - x;

    // Constrain normalized values to [-1.0, 1.0]
    left = constrain_f(left, -1.0f, 1.0f);
    right = constrain_f(right, -1.0f, 1.0f);

    // Call utility from dshot_rmt.h
    *m1 = dshot_map_3d(left);
    *m2 = dshot_map_3d(right);
}

#endif // MELTY_LOGIC_H