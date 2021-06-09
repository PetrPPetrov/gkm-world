// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <map>
#include <set>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "gkm_world/transport.h"
#include "gkm_world/fast_index.h"
#include "user_info.h"

class ProxyServer : public Transport
{
    const std::string cfg_file_name;
    PortNumberType port_number = 17012;
    std::string registered_users_file_name = "users.txt";
    boost::asio::signal_set signals;

    std::string balancer_server_ip = "127.0.0.1";
    PortNumberType balancer_server_port_number = 17013;
    boost::asio::ip::udp::endpoint balancer_server_end_point;
    boost::asio::ip::udp::endpoint proxy_server_end_point;

    typedef std::map<std::string, UserInfo::Ptr> LoginToUserInfo;
    LoginToUserInfo login_to_user_info;

    typedef Memory::FastIndexRegistry<UserOnlineInfo> IdToUnitInfo;
    IdToUnitInfo id_to_unit_info;
    IndexType online_user_count = 0;

    IndexType proxy_token = 0;

    Packet::ESeverityType minimum_level = Packet::ESeverityType::DebugMessage;
    bool log_to_screen = false;
    bool log_to_file = true;

public:
    ProxyServer(const std::string& cfg_file_name);
    ~ProxyServer();
    bool start();

private:
    void dumpParameters();
    void startImpl();

    void onQuit(const boost::system::error_code& error, int sig_number);
    bool onLogin(size_t received_bytes);
    bool onLogout(size_t received_bytes);
    bool onLogoutInternalAnswer(size_t received_bytes);
    bool onInitializePosition(size_t received_bytes);
    bool onInitializePositionInternalAnswer(size_t received_bytes);
    bool onUnitAction(size_t received_bytes);
    bool onUnitActionInternalAnswer(size_t received_bytes);
    bool onRegisterProxyAnswer(size_t received_bytes);

    bool validateInternalServer(const boost::asio::ip::udp::endpoint& end_point) const;
    void loadRegisteredUsers(const std::string& file_name);
    void saveRegisteredUsers(const std::string& file_name) const;
};
