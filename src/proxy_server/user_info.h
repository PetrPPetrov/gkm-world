// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <ostream>
#include <istream>

struct UserInfo {
    typedef std::shared_ptr<UserInfo> Ptr;

    std::string login;
    std::string password;
    std::string full_name;
    bool online = false;
    IndexType unit_token = 0;

    UserInfo() = default;
    UserInfo(const UserInfo&) = delete;
    UserInfo& operator=(const UserInfo&) = delete;
};

struct UserOnlineInfo {
    UserInfo* user_info;

    bool in_game = false;
    boost::asio::ip::udp::endpoint node_server_end_point;
    boost::asio::ip::udp::endpoint user_end_point;

    UserOnlineInfo(UserInfo* user_info_) : user_info(user_info_) {}

    UserOnlineInfo() = default;
    UserOnlineInfo(const UserOnlineInfo&) = delete;
    UserOnlineInfo& operator=(const UserOnlineInfo&) = delete;
};

inline std::istream& operator>>(std::istream& input_stream, UserInfo& user_info) {
    std::getline(input_stream, user_info.login, '\n');
    std::getline(input_stream, user_info.password, '\n');
    std::getline(input_stream, user_info.full_name, '\n');
    return input_stream;
}

inline std::ostream& operator<<(std::ostream& output_stream, const UserInfo& user_info) {
    output_stream << user_info.login << std::endl;
    output_stream << user_info.password << std::endl;
    output_stream << user_info.full_name << std::endl;
    return output_stream;
}
