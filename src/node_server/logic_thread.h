// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <boost/asio.hpp>
#include "gkm_world/protocol.h"
#include "user_location.h"

class LogicThread {
    std::unique_ptr<boost::asio::io_service> io_service;
    std::unique_ptr<boost::asio::deadline_timer> timer;
    const UnitList& unit_list;

public:
    LogicThread(const UnitList& unit_list);
    LogicThread(const LogicThread&) = delete;
    LogicThread& operator=(const LogicThread&) = delete;

    void start();
    void execute();
    void run(const boost::system::error_code& error);
};
