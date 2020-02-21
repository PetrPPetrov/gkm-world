// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include "global_parameters.h"

struct KeyboardState
{
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
};

inline double normalizeDirection(double direction) noexcept
{
    while (direction < 0.0)
    {
        direction += FULL_CIRCLE;
    }
    while (direction > FULL_CIRCLE)
    {
        direction -= FULL_CIRCLE;
    }
    return direction;
}

inline double normalizeDelta(double direction) noexcept
{
    while (direction < -FULL_CIRCLE / 2.0)
    {
        direction += FULL_CIRCLE;
    }
    while (direction > FULL_CIRCLE / 2.0)
    {
        direction -= FULL_CIRCLE;
    }
    return direction;
}

struct PlayerLocation
{
    double x_pos = 0;
    double y_pos = 0;
    double direction = 0;

    PlayerLocation()
    {
    }
    PlayerLocation(double x_pos_, double y_pos_, double direction_)
        : x_pos(x_pos_), y_pos(y_pos_), direction(direction_)
    {
    }

    void normalize() noexcept
    {
        while (direction < 0.0)
        {
            direction += FULL_CIRCLE;
        }
        while (direction > FULL_CIRCLE)
        {
            direction -= FULL_CIRCLE;
        }
    }

    void normalizeDelta() noexcept
    {
        while (direction < -FULL_CIRCLE / 2.0)
        {
            direction += FULL_CIRCLE;
        }
        while (direction > FULL_CIRCLE / 2.0)
        {
            direction -= FULL_CIRCLE;
        }
    }
};

struct PlayerUuidLocation : public PlayerLocation
{
    std::uint32_t user_token = 0;
};

inline PlayerLocation delta(const PlayerLocation& a, const PlayerLocation& b) noexcept
{
    PlayerLocation result(a.x_pos - b.x_pos, a.y_pos - b.y_pos, a.direction - b.direction);
    result.normalizeDelta();
    return result;
}

inline PlayerLocation add(const PlayerLocation& a, const PlayerLocation& b) noexcept
{
    PlayerLocation result(a.x_pos + b.x_pos, a.y_pos + b.y_pos, a.direction + b.direction);
    result.normalize();
    return result;
}

inline void gameStep(PlayerLocation& user_location, const KeyboardState& keyboard_state) noexcept
{
    if (keyboard_state.up && !keyboard_state.down)
    {
        user_location.x_pos += sin(user_location.direction * GRAD_TO_RAD) * 10;
        user_location.y_pos += cos(user_location.direction * GRAD_TO_RAD) * 10;
    }
    else if (keyboard_state.down && !keyboard_state.up)
    {
        user_location.x_pos -= sin(user_location.direction * GRAD_TO_RAD) * 10;
        user_location.y_pos -= cos(user_location.direction * GRAD_TO_RAD) * 10;
    }
    if (keyboard_state.right && !keyboard_state.left)
    {
        user_location.direction += ROTATION_ANGLE;
    }
    else if (keyboard_state.left && !keyboard_state.right)
    {
        user_location.direction -= ROTATION_ANGLE;
    }
    user_location.normalize();
}
