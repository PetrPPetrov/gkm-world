// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <list>
#include <boost/core/noncopyable.hpp>
#include <boost/asio.hpp>
#include "protocol.h"
#include "user_location.h"

class LogicThread : private boost::noncopyable
{
    std::unique_ptr<boost::asio::io_service> io_service;
    std::unique_ptr<boost::asio::deadline_timer> timer;
    UserLocation* user_location_chain = nullptr;

public:
    void start();
    void execute();
    void run(const boost::system::error_code& error);
    void addNewUser(UserLocation* new_user);
    void removeUser(UserLocation* user);
    void fillOtherUserList(Packet::UserActionInternalAnswer& packet, std::uint32_t user_token);
};
