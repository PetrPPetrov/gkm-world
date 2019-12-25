// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <cstddef>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#define DEBUG_MONITOR

const double CELL_SIZE = 25.0;
const std::int32_t NOTIFICATION_BAND_HALF_WIDTH = 1;
const double TRANSITION_TRIGGER_DISTANCE = 10.0;
const std::int32_t MINIMAL_NODE_SIZE = 4;
const std::int32_t MAXIMAL_NODE_SIZE = 2048;

const std::size_t PACKET_POOL_SIZE = 4096;
const std::uint16_t NODE_SERVER_PORT_NUMBER_BASE = 17014;
const std::uint16_t NODE_SERVER_PORT_NUMBER_MAX = 50000;

const std::uint32_t NODE_MAX_USER = 1000;
const std::uint32_t NODE_MIN_USER = 100;

const boost::posix_time::milliseconds TIMER_INTERVAL(50);

const double RAD_TO_GRAD = 57.295779513082320876798154814105;
const double GRAD_TO_RAD = 0.01745329251994329576923690768489;
const double ROTATION_ANGLE = 2.0;
const double FULL_CIRCLE = 360;
