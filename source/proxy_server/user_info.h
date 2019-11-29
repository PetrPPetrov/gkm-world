// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <ostream>
#include <istream>
#include <boost/core/noncopyable.hpp>

struct UserInfo : private boost::noncopyable
{
    typedef UserInfo self_type;
    typedef std::shared_ptr<self_type> ptr;
    std::string login;
    std::string password;
    std::string full_name;
    bool online = false;
    std::uint32_t user_token = 0;
};

struct UserOnlineInfo : private boost::noncopyable
{
    UserInfo* user_info;

    bool in_game = false;
    boost::asio::ip::udp::endpoint node_server_end_point;
    boost::asio::ip::udp::endpoint user_end_point;

    UserOnlineInfo(UserInfo* user_info_) : user_info(user_info_)
    {
    }
};

inline std::istream& operator>>(std::istream& input_stream, UserInfo& user_info)
{
    std::getline(input_stream, user_info.login, '\n');
    std::getline(input_stream, user_info.password, '\n');
    std::getline(input_stream, user_info.full_name, '\n');
    return input_stream;
}

inline std::ostream& operator<<(std::ostream& output_stream, const UserInfo& user_info)
{
    output_stream << user_info.login << std::endl;
    output_stream << user_info.password << std::endl;
    output_stream << user_info.full_name << std::endl;
    return output_stream;
}
