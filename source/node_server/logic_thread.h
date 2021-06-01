// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <list>
#include <boost/asio.hpp>
#include "protocol.h"
#include "user_location.h"

class LogicThread
{
    std::unique_ptr<boost::asio::io_service> io_service;
    std::unique_ptr<boost::asio::deadline_timer> timer;
    UserLocationBlockChain* user_location_chain = nullptr;

public:
    LogicThread() = default;
    LogicThread(const LogicThread&) = delete;
    LogicThread& operator=(const LogicThread&) = delete;

    void start();
    void execute();
    void run(const boost::system::error_code& error);
    void addNewUser(UserLocationBlockChain* new_user);
    void removeUser(UserLocationBlockChain* user);
    void fillOtherUserList(Packet::UserActionInternalAnswer& packet, std::uint32_t user_token);
};
