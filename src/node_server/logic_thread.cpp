// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <boost/bind/bind.hpp>
#include "gkm_world/gkm_world.h"
#include "gkm_world/logger.h"
#include "gkm_world/game_logic.h"
#include "logic_thread.h"

LogicThread::LogicThread(const UnitList& unit_list_) : unit_list(unit_list_) {}

void LogicThread::start() {
    std::thread separate_thread(&LogicThread::execute, this);
    separate_thread.detach();
}

void LogicThread::execute() {
    using namespace boost::placeholders;

    io_service = std::make_unique<boost::asio::io_service>();
    timer = std::make_unique<boost::asio::deadline_timer>(*io_service, GAME_TIME_INTERVAL);
    timer->async_wait(boost::bind(&LogicThread::run, this, _1));
    io_service->run();
}

void LogicThread::run(const boost::system::error_code& error) {
    using namespace boost::placeholders;

    if (error) {
        LOG_ERROR << "error";
        return;
    }

    IndexType cur_unit_index = unit_list.getChainHeadIndex();
    while (cur_unit_index != INVALID_INDEX) {
        UnitLocationInfo* unit_info = unit_list.get(cur_unit_index);
        gameStep(unit_info->unit_location, unit_info->state);
        cur_unit_index = unit_list.getChainNextIndex(cur_unit_index);
    }

    timer->expires_at(timer->expires_at() + GAME_TIME_INTERVAL);
    timer->async_wait(boost::bind(&LogicThread::run, this, _1));

#ifdef DEBUG_MONITOR
    COORD xy;
    xy.X = 0;
    xy.Y = 1;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), xy);
    cur_user_location = user_location_chain;
    while (cur_user_location) {
        std::cout << LINE_SEPARATOR;
        std::cout << "user_token: " << cur_user_location->user_location.user_token;
        std::cout << "x_pos: " << std::fixed << std::setw(10) << std::setprecision(3) << cur_user_location->user_location.x_pos;
        std::cout << "y_pos: " << cur_user_location->user_location.y_pos;
        std::cout << "direction: " << cur_user_location->user_location.direction;
        cur_user_location = cur_user_location->next;
    }
    std::cout << LINE_SEPARATOR;
    std::cout << ENDL_SEPARATOR;
#endif
}
