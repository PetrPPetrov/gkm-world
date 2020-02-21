// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include "logic.h"

struct UserLocation
{
    PlayerUuidLocation user_location;
    KeyboardState state;

    UserLocation* previous = nullptr;
    UserLocation* next = nullptr;

    UserLocation(const UserLocation&) = delete;
    UserLocation& operator=(const UserLocation&) = delete;
};
