// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "protocol.h"
#include "logic.h"

class UDPConnection
{
public:
    UDPConnection(const std::string& ip_address, std::uint16_t port_number);
    void login(const std::string& email, const std::string& password, const std::string& full_name);
    void logout();
    void initializePosition(const PlayerLocation& player_location);
    void sendPlayerState(const KeyboardState& keyboard_state, PlayerLocation& player_location);
private:
    boost::asio::io_service io_service;
    boost::asio::ip::udp::endpoint end_point;
    boost::asio::ip::udp::socket socket;
    std::uint32_t cur_packet_number = 0;
    std::uint32_t token = 0;
};
