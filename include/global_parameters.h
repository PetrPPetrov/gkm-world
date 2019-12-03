// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <boost/date_time/posix_time/posix_time_types.hpp>

#define DEBUG_MONITOR

const double RAD_TO_GRAD = 57.295779513082320876798154814105;
const double GRAD_TO_RAD = 0.01745329251994329576923690768489;
const double ROTATION_ANGLE = 2.0;
const double FULL_CIRCLE = 360;

const double NOTIFY_DISTANCE = 12.5;
const double TRANSITION_DISTANCE = NOTIFY_DISTANCE / 2.0;
const double CELL_UNIT_SIZE = TRANSITION_DISTANCE;
const std::int32_t MINIMAL_CELL_SIZE = 8;
const std::int32_t MAXIMAL_CELL_SIZE = 2048;

const size_t PACKET_POOL_SIZE = 4096;

const boost::posix_time::milliseconds TIMER_INTERVAL(50);

const unsigned short NODE_SERVER_PORT_NUMBER_BASE = 17014;
const unsigned short NODE_SERVER_PORT_NUMBER_MAX = 50000;
