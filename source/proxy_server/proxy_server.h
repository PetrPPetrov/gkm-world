// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <map>
#include <set>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "packet_pool.h"
#include "transport.h"
#include "fast_index_map.h"
#include "user_info.h"

class ProxyServer : public Transport
{
    const std::string cfg_file_name;
    std::uint16_t port_number = 17012;
    std::string registered_users_file_name = "users.txt";
    boost::asio::signal_set signals;

    std::string balancer_server_ip = "127.0.0.1";
    std::uint16_t balancer_server_port_number = 17013;
    boost::asio::ip::udp::endpoint balancer_server_end_point;
    boost::asio::ip::udp::endpoint proxy_server_end_point;

    typedef std::map<std::string, UserInfo::Ptr> LoginToUserInfo;
    LoginToUserInfo login_to_user_info;

    typedef Memory::FastIndexMap<UserOnlineInfo> IdToUserInfo;
    IdToUserInfo id_to_user_info;
    std::uint32_t online_user_count = 0;

    std::uint32_t proxy_index = 0;

    Packet::ESeverityType minimum_level = Packet::ESeverityType::DebugMessage;
    bool log_to_screen = false;
    bool log_to_file = true;

public:
    ProxyServer(const std::string& cfg_file_name);
    ~ProxyServer();
    bool start();

private:
    std::uint32_t allocateId();
    void deallocateId(std::uint32_t id);

    void dumpParameters();
    void startImpl();

    void onQuit(const boost::system::error_code& error, int sig_number);
    bool onLogin(size_t received_bytes);
    bool onLogout(size_t received_bytes);
    bool onLogoutInternalAnswer(size_t received_bytes);
    bool onInitializePosition(size_t received_bytes);
    bool onInitializePositionInternalAnswer(size_t received_bytes);
    bool onUserAction(size_t received_bytes);
    bool onUserActionInternalAnswer(size_t received_bytes);
    bool onRegisterProxyAnswer(size_t received_bytes);

    bool validateInternalServer(const boost::asio::ip::udp::endpoint& end_point) const;
    void loadRegisteredUsers(const std::string& file_name);
    void saveRegisteredUsers(const std::string& file_name) const;
};
