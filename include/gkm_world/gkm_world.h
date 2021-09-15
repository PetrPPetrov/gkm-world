// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstdint>
#include <limits>
#include <boost/date_time/posix_time/posix_time_types.hpp>

// We use fixed-point numbers for coordinates
typedef std::int32_t CoordinateType;
// The lower FRACTION_BITS is used for fraction part, the rest part is used as integer part
constexpr std::uint8_t FRACTION_BITS = 8;
constexpr CoordinateType UNIT_SCALE = 1 << FRACTION_BITS;
// So, 256 coordinate units are 1 meter
// 1 coordinate unit is 1/256 meter

inline double toFloatingPoint(CoordinateType value) {
    return static_cast<double>(value) / UNIT_SCALE;
}

inline CoordinateType toFixedPoint(double value) {
    return static_cast<CoordinateType>(value * UNIT_SCALE);
}

constexpr CoordinateType CELL_SIZE = 16 * UNIT_SCALE; // Cell size is 16 x 16 meters
constexpr CoordinateType NODE_SIZE_MIN = 4 * CELL_SIZE; // Minimum node size is 4 x 4 cells (64 x 64 meters)
constexpr CoordinateType NODE_SIZE_MAX = 2048 * CELL_SIZE; // Maximum node size is 2048 x 2048 cells (32768 x 32768 meters)
constexpr CoordinateType NOTIFY_DISTANCE = CELL_SIZE; // Notify distance is 16 meters
constexpr CoordinateType TRIGGER_DISTANCE = CELL_SIZE / 2; // Trigger distance is 8 meters

// We use IndexType as index (or in other words, token) for game units
typedef std::uint32_t IndexType;
constexpr IndexType INVALID_INDEX = static_cast<IndexType>(std::numeric_limits<IndexType>::max());
constexpr IndexType NODE_GUNIT_MAX = 1024; // Maxumum number of game units on node server is 1024
constexpr IndexType NODE_GUNIT_MIN = 64; // Minumum number of game units of node server is 64

// We use fixed-point numbers for angles; 0-360 range is mapped into 0-65535 range
typedef std::uint16_t AngularType;

// We us 16-bit unsigned integer as port number
typedef std::uint16_t PortNumberType;

constexpr PortNumberType NODE_SERVER_PORT_NUMBER_BASE = 17014;
constexpr PortNumberType NODE_SERVER_PORT_NUMBER_MAX = 50000;

// Game logic computation interval
const boost::posix_time::milliseconds GAME_TIME_INTERVAL(50);
const boost::posix_time::milliseconds LOGGING_TIME_INTERVAL(1);

constexpr double GKM_PI = 3.14159265358979323846264338327950288;
constexpr double GKM_2PI = 2 * GKM_PI;
