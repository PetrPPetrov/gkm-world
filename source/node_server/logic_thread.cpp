// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <boost/bind.hpp>
#include "Eigen/Eigen"
#include "log.h"
#include "logic.h"
#include "logic_thread.h"
#include "block_chain.h"
#include "global_parameters.h"

void LogicThread::start()
{
    std::thread separate_thread(&LogicThread::execute, this);
    separate_thread.detach();
}

void LogicThread::execute()
{
    io_service = std::make_unique<boost::asio::io_service>();
    timer = std::make_unique<boost::asio::deadline_timer>(*io_service, TIMER_INTERVAL);
    timer->async_wait(boost::bind(&LogicThread::run, this, _1));
    io_service->run();
}

void LogicThread::run(const boost::system::error_code& error)
{
    if (error)
    {
        LOG_ERROR << "error";
        return;
    }

    UserLocation* cur_user_location = user_location_chain;
    while (cur_user_location)
    {
        gameStep(cur_user_location->user_location, cur_user_location->state);
        cur_user_location = cur_user_location->next;
    }

    timer->expires_at(timer->expires_at() + TIMER_INTERVAL);
    timer->async_wait(boost::bind(&LogicThread::run, this, _1));

#ifdef DEBUG_MONITOR
    COORD xy;
    xy.X = 0;
    xy.Y = 1;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), xy);
    cur_user_location = user_location_chain;
    while (cur_user_location)
    {
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

void LogicThread::addNewUser(UserLocation* new_user)
{
    pushFrontBlock(new_user, user_location_chain);
}

void LogicThread::removeUser(UserLocation* user)
{
    removeBlock(user, user_location_chain);
}

void LogicThread::fillOtherUserList(Packet::UserActionInternalAnswer& packet, std::uint32_t user_token)
{
    packet.other_player_count = 0;
    UserLocation* cur_user_location = user_location_chain;
    // TODO: Optimize this; use cell spacing for the visible users selection
    while (cur_user_location)
    {
        if (cur_user_location->user_location.user_token != user_token)
        {
            packet.other_player[packet.other_player_count].x_pos = cur_user_location->user_location.x_pos;
            packet.other_player[packet.other_player_count].y_pos = cur_user_location->user_location.y_pos;
            packet.other_player[packet.other_player_count].direction = cur_user_location->user_location.direction;
            packet.other_player[packet.other_player_count].user_token = cur_user_location->user_location.user_token;
            packet.other_player_count++;
        }
        if (packet.other_player_count >= Packet::MAX_USER_COUNT_IN_PACKET)
        {
            break;
        }

        cur_user_location = cur_user_location->next;
    }
}
