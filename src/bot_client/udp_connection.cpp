// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include "udp_connection.h"

UDPConnection::UDPConnection(const std::string& ip_address, PortNumberType port_number)
    : end_point(boost::asio::ip::address::from_string(ip_address), port_number), socket(io_service) {
    socket.connect(end_point);
}

void UDPConnection::login(const std::string& email, const std::string& password, const std::string& full_name) {
    Packet::Login login;
    login.packet_number = cur_packet_number++;
    login.login = email;
    login.password = password;
    login.full_name = full_name;
    socket.send(boost::asio::buffer(static_cast<void*>(&login), sizeof(login)));

    Packet::LoginAnswer login_answer;
    socket.receive(boost::asio::buffer(static_cast<void*>(&login_answer), sizeof(login_answer)));
    token = login_answer.unit_token;
    if (!login_answer.success) {
        throw std::runtime_error("Login is not successfull");
    }
}

void UDPConnection::logout() {
    Packet::Logout logout;
    logout.unit_token = token;
    logout.packet_number = cur_packet_number++;
    socket.send(boost::asio::buffer(static_cast<void*>(&logout), sizeof(logout)));

    Packet::LogoutAnswer logout_answer;
    socket.receive(boost::asio::buffer(static_cast<void*>(&logout_answer), sizeof(logout_answer)));
}

void UDPConnection::initializePosition(const UnitLocation& player_location) {
    Packet::InitializePosition initialize_position;
    initialize_position.packet_number = cur_packet_number++;
    initialize_position.unit_location.position = player_location.position;
    initialize_position.unit_location.direction = player_location.direction;
    initialize_position.unit_location.unit_token = token;

    socket.send(boost::asio::buffer(static_cast<void*>(&initialize_position), sizeof(initialize_position)));

    Packet::InitializePositionAnswer initialize_position_answer;
    socket.receive(boost::asio::buffer(static_cast<void*>(&initialize_position_answer), sizeof(initialize_position_answer)));
    if (!initialize_position_answer.success) {
        throw std::runtime_error("Initialize position is not successfull");
    }
}

void UDPConnection::sendPlayerState(const KeyboardState& keyboard_state, UnitLocation& player_location) {
    Packet::UnitAction user_action;
    user_action.unit_token = token;
    user_action.packet_number = cur_packet_number++;
    user_action.keyboard_state = keyboard_state;
    socket.send(boost::asio::buffer(static_cast<void*>(&user_action), sizeof(user_action)));

    Packet::UnitActionAnswer user_action_answer;
    socket.receive(boost::asio::buffer(static_cast<void*>(&user_action_answer), sizeof(user_action_answer)));
    player_location = user_action_answer.unit_location.toUnitLocation();
}
