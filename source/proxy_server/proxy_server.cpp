// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <iostream>
#include <fstream>
#include <memory>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include "protocol.h"
#include "log.h"
#include "config.h"
#include "proxy_server.h"

ProxyServer::ProxyServer() :
    signals(io_service, SIGINT, SIGTERM)
{
    std::ifstream config_file("proxy_server.cfg");
    ConfigurationReader config_reader;
    config_reader.addParameter("proxy_server_port_number", port_number);
    config_reader.addParameter("balancer_server_ip", balancer_server_ip);
    config_reader.addParameter("balancer_server_port_number", balancer_server_port_number);
    config_reader.addParameter("registered_users_file_name", registered_users_file_name);
    config_reader.read(config_file);

#ifdef _DEBUG
    LOG_DEBUG << "proxy_server_port_number " << port_number << std::endl;
    LOG_DEBUG << "balancer_server_ip " << balancer_server_ip << std::endl;
    LOG_DEBUG << "balancer_server_port_number " << balancer_server_port_number << std::endl;
#endif

    socket = boost::asio::ip::udp::socket(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port_number));
    balancer_server_end_point = boost::asio::ip::udp::endpoint(ip_address_t::from_string(balancer_server_ip), balancer_server_port_number);

    loadRegisteredUsers(registered_users_file_name);
}

ProxyServer::~ProxyServer()
{
    saveRegisteredUsers(registered_users_file_name);
}

void ProxyServer::start()
{
    doReceive();
    signals.async_wait(boost::bind(&ProxyServer::onQuit, this, _1, _2));
    setReceiveHandler(Packet::EType::Login, boost::bind(&ProxyServer::onLogin, this, _1));
    setReceiveHandler(Packet::EType::Logout, boost::bind(&ProxyServer::onLogout, this, _1));
    setReceiveHandler(Packet::EType::LogoutInternalAnswer, boost::bind(&ProxyServer::onLogoutInternalAnswer, this, _1));
    setReceiveHandler(Packet::EType::InitializePosition, boost::bind(&ProxyServer::onInitializePosition, this, _1));
    setReceiveHandler(Packet::EType::InitializePositionInternalAnswer, boost::bind(&ProxyServer::onInitializePositionInternalAnswer, this, _1));
    setReceiveHandler(Packet::EType::UserAction, boost::bind(&ProxyServer::onUserAction, this, _1));
    setReceiveHandler(Packet::EType::UserActionInternalAnswer, boost::bind(&ProxyServer::onUserActionInternalAnswer, this, _1));
    io_service.run();
}

std::uint32_t ProxyServer::allocateId()
{
    online_user_count++;
    return id_to_user_info.allocateIndex();
}

void ProxyServer::deallocateId(std::uint32_t id)
{
    online_user_count--;
    id_to_user_info.deallocateIndex(id);
}

void ProxyServer::debugMonitor() const
{
#ifdef DEBUG_MONITOR
    COORD xy;
    xy.X = 0;
    xy.Y = 1;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), xy);
    const std::uint32_t max_id = id_to_user_info.getMaxAllocatedIndex();
    for (std::uint32_t i = 0; i <= max_id; ++i)
    {
        UserOnlineInfo* user_info = id_to_user_info.find(i);
        if (user_info)
        {
            std::cout << LINE_SEPARATOR << std::endl;
            std::cout << "user_token: " << i << std::endl;
            std::cout << "client_end_point: " << user_info->user_end_point << std::endl;
            std::cout << "node_server_end_point: " << user_info->node_server_end_point << std::endl;
        }
    }
    std::cout << LINE_SEPARATOR << std::endl;
    std::cout << ENDL_SEPARATOR << std::endl;
#endif
}

void ProxyServer::onQuit(const boost::system::error_code& error, int sig_number)
{
    if (!error)
    {
        saveRegisteredUsers(registered_users_file_name);
    }
    else
    {
        LOG_ERROR << "users were not saved in the file" << std::endl;
    }
    Log::onQuit(error, sig_number);
}

bool ProxyServer::onLogin(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onLogin" << std::endl;
#endif

    auto packet = getReceiveBufferAs<Packet::Login>();

    std::string login = packet->getLogin();
    std::string password = packet->getPassword();
    std::string full_name = packet->getFullName();

#ifdef _DEBUG
    LOG_DEBUG << "end_point: " << remote_end_point << std::endl;
    LOG_DEBUG << "password: " << login << std::endl;
    LOG_DEBUG << "login: " << password << std::endl;
    LOG_DEBUG << "full_name: " << full_name << std::endl;
#endif

    auto fit_login_it = login_to_user_info.find(login);
    UserInfo::ptr cur_user;
    if (fit_login_it != login_to_user_info.end())
    {
#ifdef _DEBUG
        LOG_DEBUG << "existing user" << std::endl;
#endif
        cur_user = fit_login_it->second;
        if (password != cur_user->password)
        {
#ifdef _DEBUG
            LOG_DEBUG << "authorization failure" << std::endl;
#endif
            auto answer = createPacket<Packet::LoginAnswer>(packet->packet_number);
            answer->success = false;
            standardSend(answer);
            return true;
        }
        else
        {
#ifdef _DEBUG
            LOG_DEBUG << "authorization OK" << std::endl;
#endif
        }
    }
    else
    {
#ifdef _DEBUG
        LOG_DEBUG << "new user" << std::endl;
#endif
        UserInfo::ptr new_user = std::make_shared<UserInfo>();
        new_user->login = login;
        new_user->password = password;
        new_user->full_name = full_name;
        login_to_user_info.insert(login_to_user_info_t::value_type(login, new_user));
        cur_user = new_user;
    }

    if (!cur_user->online)
    {
#ifdef _DEBUG
        LOG_DEBUG << "user is logging" << std::endl;
#endif
        std::uint32_t id = allocateId();

#ifdef _DEBUG
        LOG_DEBUG << "generated new id " << id << std::endl;
#endif
        cur_user->user_token = id;
        cur_user->online = true;
        UserOnlineInfo* user_online_info = new(id_to_user_info.allocate(id)) UserOnlineInfo(cur_user.get());
        user_online_info->user_end_point = remote_end_point;
    }
    else
    {
#ifdef _DEBUG
        LOG_DEBUG << "user is already online" << std::endl;
#endif
    }

    auto answer = createPacket<Packet::LoginAnswer>(packet->packet_number);
    answer->success = true;
    answer->user_token = cur_user->user_token;
    standardSend(answer);

#ifdef DEBUG_MONITOR
    debugMonitor();
#endif

    return true;
}

bool ProxyServer::onLogout(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onLogout" << std::endl;
#endif

    auto packet = getReceiveBufferAs<Packet::Logout>();

#ifdef _DEBUG
    LOG_DEBUG << "end point: " << remote_end_point << std::endl;
#endif

    std::uint32_t user_id = packet->user_token;
    UserOnlineInfo* user_online_info = id_to_user_info.find(user_id);

    if (!user_online_info)
    {
        LOG_ERROR << "user does not exist" << std::endl;
        return true;
    }

    if (user_online_info->user_end_point != remote_end_point)
    {
        LOG_ERROR << "user sent incorrect id " << user_id << " ip: " << user_online_info->user_end_point << " and it should be ip: " << remote_end_point << std::endl;
        return true;
    }

    auto request = createPacket<Packet::LogoutInternal>();
    request->user_token = user_online_info->user_info->user_token;
    request->client_packet_number = packet->packet_number;
    standardSendTo(request, user_online_info->node_server_end_point);
    return true;
}

bool ProxyServer::onLogoutInternalAnswer(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onLogoutInternalAnswer" << std::endl;
#endif

    auto packet = getReceiveBufferAs<Packet::LogoutInternalAnswer>();

    std::uint32_t user_id = packet->user_token;
    UserOnlineInfo* user_online_info = id_to_user_info.find(user_id);

    if (!user_online_info)
    {
        LOG_ERROR << "user does not exist" << std::endl;
        return true;
    }

    user_online_info->user_info->online = false;
    user_online_info->user_info->user_token = 0;
    user_online_info->in_game = false;
    user_online_info->user_info = nullptr;
    boost::asio::ip::udp::endpoint user_end_point = user_online_info->user_end_point;
    deallocateId(user_id);
    id_to_user_info.deallocate(user_id);

    auto answer = createPacket<Packet::LogoutAnswer>(packet->client_packet_number);
    answer->success = true;
    standardSendTo(answer, user_end_point);

#ifdef DEBUG_MONITOR
    debugMonitor();
#endif

    return true;
}

bool ProxyServer::onInitializePosition(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onInitializePosition" << std::endl;
#endif

    const auto packet = getReceiveBufferAs<Packet::InitializePosition>();

    std::uint32_t user_id = packet->user_token;
    UserOnlineInfo* user_online_info = id_to_user_info.find(user_id);

    if (!user_online_info)
    {
        LOG_ERROR << "user does not exist" << std::endl;
        return true;
    }

    if (user_online_info->user_end_point != remote_end_point)
    {
        LOG_ERROR << "user sent incorrect id " << user_id << " ip: " << user_online_info->user_end_point << " and it should be ip: " << remote_end_point << std::endl;
        return true;
    }

    if (!user_online_info->in_game)
    {
        auto request = createPacket<Packet::InitializePositionInternal>();
        request->user_token = user_online_info->user_info->user_token;
        request->client_packet_number = packet->packet_number;
        request->proxy_packet_number = request->packet_number;
        request->user_location = packet->user_location;
        standardSendTo(request, balancer_server_end_point);
        return true;
    }

    auto answer = createPacket<Packet::InitializePositionAnswer>(packet->packet_number);
    answer->success = true;
    answer->corrected_location = packet->user_location;
    standardSend(answer);

    return true;
}

bool ProxyServer::onInitializePositionInternalAnswer(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onInitializePositionInternalAnswer" << std::endl;
#endif

    if (!validateInternalServer(remote_end_point))
    {
        LOG_WARNING << "internal server token validation error" << std::endl;
        return true;
    }

    const auto packet = getReceiveBufferAs<Packet::InitializePositionInternalAnswer>();

    std::uint32_t user_id = packet->user_token;
    UserOnlineInfo* user_online_info = id_to_user_info.find(user_id);

    if (!user_online_info)
    {
#ifdef _DEBUG
        LOG_ERROR << "user does not exist" << std::endl;
#endif
        return true;
    }

    if (!user_online_info->user_info->online)
    {
        LOG_ERROR << "user is not online or is already in game" << std::endl;
        return true;
    }

    if (user_online_info->in_game)
    {
        LOG_ERROR << "user is already in game" << std::endl;
        return true;
    }

    user_online_info->in_game = true;

    user_online_info->node_server_end_point = boost::asio::ip::udp::endpoint(ip_address_t(packet->node_server_address), packet->node_server_port_number);

    if (user_online_info->node_server_end_point.address().is_loopback())
    {
        user_online_info->node_server_end_point = boost::asio::ip::udp::endpoint(remote_end_point.address().to_v4(), packet->node_server_port_number);
    }

    auto answer = createPacket<Packet::InitializePositionAnswer>(packet->client_packet_number);
    answer->success = packet->success;
    answer->corrected_location = packet->corrected_location;
    standardSendTo(answer, user_online_info->user_end_point);

#ifdef DEBUG_MONITOR
    debugMonitor();
#endif

    return true;
}

bool ProxyServer::onUserAction(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onUserAction" << std::endl;
#endif

#ifdef DEBUG_MONITOR
    debugMonitor();
#endif

    const auto packet = getReceiveBufferAs<Packet::UserAction>();

    std::uint32_t user_id = packet->user_token;
    UserOnlineInfo* user_online_info = id_to_user_info.find(user_id);

    if (!user_online_info)
    {
        LOG_ERROR << "user does not exist" << std::endl;
        return true;
    }

    if (user_online_info->user_end_point != remote_end_point)
    {
        LOG_ERROR << "user sent incorrect id " << user_id << " ip: " << user_online_info->user_end_point << " and it should be ip: " << remote_end_point << std::endl;
        return true;
    }

    if (!user_online_info->in_game)
    {
        LOG_ERROR << "user is not in game" << std::endl;
        return true;
    }

    auto request = createPacket<Packet::UserActionInternal>();
    request->user_token = user_online_info->user_info->user_token;
    request->keyboard_state = packet->keyboard_state;
    request->client_packet_number = packet->packet_number;
    standardSendTo(request, user_online_info->node_server_end_point);
    return true;
}

bool ProxyServer::onUserActionInternalAnswer(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onUserActionInternalAnswer" << std::endl;
#endif

    if (!validateInternalServer(remote_end_point))
    {
        LOG_WARNING << "fake internal server detected" << std::endl;
        return true;
    }

    const auto packet = getReceiveBufferAs<Packet::UserActionInternalAnswer>();

    std::uint32_t user_id = packet->user_token;
    UserOnlineInfo* user_online_info = id_to_user_info.find(user_id);

    if (!user_online_info)
    {
        LOG_ERROR << "user does not exist" << std::endl;
        return true;
    }

    if (!user_online_info->in_game)
    {
        LOG_ERROR << "user is not in game" << std::endl;
        return true;
    }

    auto answer = createPacket<Packet::UserActionAnswer>(packet->client_packet_number);
    answer->user_location = packet->user_location;
    answer->other_player_count = packet->other_player_count;
    if (answer->other_player_count >= Packet::MAX_USER_COUNT_IN_PACKET)
    {
        answer->other_player_count = Packet::MAX_USER_COUNT_IN_PACKET;
    }
    for (std::uint16_t i = 0; i < answer->other_player_count; ++i)
    {
        answer->other_player[i] = packet->other_player[i];
    }
    standardSendTo(answer, user_online_info->user_end_point);
    return true;
}

bool ProxyServer::validateInternalServer(const boost::asio::ip::udp::endpoint& end_point) const
{
    // TODO:

    return true;
}

void ProxyServer::loadRegisteredUsers(const std::string& file_name)
{
#ifdef _DEBUG
    LOG_DEBUG << "loading users from " << file_name << "..." << std::endl;
#endif
    std::ifstream registered_users(file_name);
    while (!registered_users.bad() && !registered_users.eof())
    {
        UserInfo::ptr new_user = std::make_shared<UserInfo>();
        registered_users >> *new_user;
        if (new_user->login.empty() || new_user->password.empty() || new_user->full_name.empty())
        {
            break;
        }
        login_to_user_info.insert_or_assign(new_user->login, new_user);
    }
#ifdef _DEBUG
    LOG_DEBUG << "users are loaded!" << std::endl;
#endif
}

void ProxyServer::saveRegisteredUsers(const std::string& file_name) const
{
#ifdef _DEBUG
    LOG_DEBUG << "saving users to " << file_name << "..." << std::endl;
#endif
    std::ofstream registered_users(file_name);
    for (auto cur_user : login_to_user_info)
    {
        registered_users << *cur_user.second.get();
    }
#ifdef _DEBUG
    LOG_DEBUG << "users are saved!" << std::endl;
#endif
}
