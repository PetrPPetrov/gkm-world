// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include "gkm_world/gkm_world.h"
#include "gkm_world/coordinate_2d.h"

struct UnitLocation {
    Coordinate2D position;
    AngularType direction = 0;

    UnitLocation() = default;
    UnitLocation(Coordinate2D position_, AngularType direction_)
        : position(position_), direction(direction_) {}
};

struct UnitLocationToken : public UnitLocation {
    IndexType unit_token = 0;

    UnitLocationToken() = default;
    UnitLocationToken(const UnitLocation& unit_location, IndexType token) : UnitLocation(unit_location), unit_token(token) {}
};

struct KeyboardState {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
};

//inline UnitLocation delta(const UnitLocation& a, const UnitLocation& b) noexcept {
//    UnitLocation result(a.position.x - b.position.x, a.position.y - b.position.y, a.direction - b.direction);
//    return result;
//}
//
//inline UnitLocation add(const UnitLocation& a, const UnitLocation& b) noexcept {
//    UnitLocation result(a.position.x + b.position.x, a.position.y + b.position.y, a.direction + b.direction);
//    return result;
//}

inline void gameStep(UnitLocation& unit_location, const KeyboardState& keyboard_state) noexcept {
    constexpr AngularType ROTATION_ANGLE = 364;

    // TODO: Probably perform computations in integer numbers instead
    // TODO: Probably use some lookup tables for quick sin & cos calculation
    const double direction = static_cast<double>(unit_location.direction) / (static_cast<double>(std::numeric_limits<AngularType>::max()) + 1) * GKM_2PI;
    double x_pos = toFloatingPoint(unit_location.position.x);
    double y_pos = toFloatingPoint(unit_location.position.y);
    if (keyboard_state.up && !keyboard_state.down) {
        x_pos += sin(direction) * 10;
        y_pos += cos(direction) * 10;
    } else if (keyboard_state.down && !keyboard_state.up) {
        x_pos -= sin(direction) * 10;
        y_pos -= cos(direction) * 10;
    }
    // TODO: Use some atomic to atomically update unit_location.position
    unit_location.position.x = toFixedPoint(x_pos);
    unit_location.position.y = toFixedPoint(y_pos);

    if (keyboard_state.right && !keyboard_state.left) {
        unit_location.direction += ROTATION_ANGLE;
    } else if (keyboard_state.left && !keyboard_state.right) {
        unit_location.direction -= ROTATION_ANGLE;
    }
}
