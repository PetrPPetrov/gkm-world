// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include "gkm_world/protocol.h"
#include "gkm_world/game_logic.h"

class UDPConnection {
public:
    UDPConnection(const std::string& ip_address, PortNumberType port_number);
    void login(const std::string& email, const std::string& password, const std::string& full_name);
    void logout();
    void initializePosition(const UnitLocation& player_location);
    void sendPlayerState(const KeyboardState& keyboard_state, UnitLocation& player_location);
private:
    boost::asio::io_service io_service;
    boost::asio::ip::udp::endpoint end_point;
    boost::asio::ip::udp::socket socket;
    IndexType cur_packet_number = 0;
    IndexType token = 0;
};
