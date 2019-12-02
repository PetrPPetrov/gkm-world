// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
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
    unsigned short port_number = 17012;
    std::string registered_users_file_name = "users.txt";
    boost::asio::signal_set signals;

    std::string balancer_server_ip = "127.0.0.1";
    unsigned short balancer_server_port_number = 17013;
    boost::asio::ip::udp::endpoint balancer_server_end_point;

    typedef std::map<std::string, UserInfo::ptr> login_to_user_info_t;
    login_to_user_info_t login_to_user_info;

    typedef Memory::FastIndexMap<UserOnlineInfo> id_to_user_info_t;
    id_to_user_info_t id_to_user_info;
    std::uint32_t online_user_count = 0;

public:
    ProxyServer();
    ~ProxyServer();
    void start();

private:
    std::uint32_t allocateId();
    void deallocateId(std::uint32_t id);

    void debugMonitor() const;

    void onQuit(const boost::system::error_code& error, int sig_number);
    bool onLogin(size_t received_bytes);
    bool onLogout(size_t received_bytes);
    bool onLogoutInternalAnswer(size_t received_bytes);
    bool onInitializePosition(size_t received_bytes);
    bool onInitializePositionInternalAnswer(size_t received_bytes);
    bool onUserAction(size_t received_bytes);
    bool onUserActionInternalAnswer(size_t received_bytes);

    bool validateInternalServer(const boost::asio::ip::udp::endpoint& end_point) const;
    void loadRegisteredUsers(const std::string& file_name);
    void saveRegisteredUsers(const std::string& file_name) const;
};