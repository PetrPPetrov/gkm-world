// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
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

ProxyServer::ProxyServer(const std::string& cfg_file_name_) :
    cfg_file_name(cfg_file_name_), signals(io_service, SIGINT, SIGTERM)
{
    setServerType(Packet::EServerType::ProxyServer);

    std::ifstream config_file(cfg_file_name);
    ConfigurationReader config_reader;
    config_reader.addParameter("proxy_server_port_number", port_number);
    config_reader.addParameter("balancer_server_ip", balancer_server_ip);
    config_reader.addParameter("balancer_server_port_number", balancer_server_port_number);
    config_reader.addParameter("log_min_severity", minimum_level);
    config_reader.addParameter("log_to_screen", log_to_screen);
    config_reader.addParameter("log_to_file", log_to_file);
    config_reader.addParameter("registered_users_file_name", registered_users_file_name);
    config_reader.read(config_file);

    balancer_server_end_point = boost::asio::ip::udp::endpoint(IpAddress::from_string(balancer_server_ip), balancer_server_port_number);
    proxy_server_end_point = boost::asio::ip::udp::endpoint(IpAddress(), port_number);
    socket = boost::asio::ip::udp::socket(io_service, proxy_server_end_point);
}

ProxyServer::~ProxyServer()
{
    saveRegisteredUsers(registered_users_file_name);
}

extern Log::Logger* g_logger = nullptr;

bool ProxyServer::start()
{
    Log::Holder log_holder;
    g_logger = new Log::Logger(minimum_level, "proxy_server_" + std::to_string(port_number) + ".log", log_to_screen, log_to_file);
    LOG_INFO << "Proxy Server is starting...";

    try
    {
        dumpParameters();
        startImpl();
    }
    catch (boost::system::system_error & error)
    {
        LOG_FATAL << "boost::system::system_error: " << error.what();
        return false;
    }
    catch (const std::exception & exception)
    {
        LOG_FATAL << "std::exception: " << exception.what();
        return false;
    }
    catch (...)
    {
        LOG_FATAL << "Unknown error";
        return false;
    }
    return true;
}

void ProxyServer::dumpParameters()
{
    LOG_INFO << "configuration_file_name " << cfg_file_name;
    LOG_INFO << "balancer_server_ip " << balancer_server_ip;
    LOG_INFO << "balancer_server_port_number " << balancer_server_port_number;

    LOG_INFO << "proxy_server_ip " << proxy_server_end_point.address();
    LOG_INFO << "proxy_server_port_number " << proxy_server_end_point.port();

    LOG_INFO << "log_min_severity " << getText(minimum_level);
    LOG_INFO << "log_to_screen " << log_to_screen;
    LOG_INFO << "log_to_file " << log_to_file;

    LOG_INFO << "registered_users_file_name " << registered_users_file_name;
}

void ProxyServer::startImpl()
{
    loadRegisteredUsers(registered_users_file_name);
    doReceive();
    signals.async_wait(boost::bind(&ProxyServer::onQuit, this, _1, _2));
    setReceiveHandler(Packet::EType::Login, boost::bind(&ProxyServer::onLogin, this, _1));
    setReceiveHandler(Packet::EType::Logout, boost::bind(&ProxyServer::onLogout, this, _1));
    setReceiveHandler(Packet::EType::LogoutInternalAnswer, boost::bind(&ProxyServer::onLogoutInternalAnswer, this, _1));
    setReceiveHandler(Packet::EType::InitializePosition, boost::bind(&ProxyServer::onInitializePosition, this, _1));
    setReceiveHandler(Packet::EType::InitializePositionInternalAnswer, boost::bind(&ProxyServer::onInitializePositionInternalAnswer, this, _1));
    setReceiveHandler(Packet::EType::UserAction, boost::bind(&ProxyServer::onUserAction, this, _1));
    setReceiveHandler(Packet::EType::UserActionInternalAnswer, boost::bind(&ProxyServer::onUserActionInternalAnswer, this, _1));
    setReceiveHandler(Packet::EType::RegisterProxyAnswer, boost::bind(&ProxyServer::onRegisterProxyAnswer, this, _1));
#ifdef NETWORK_LOG
    setReceiveHandler(Packet::EType::MonitoringMessageCount, boost::bind(&Transport::onMonitoringMessageCount, this, _1));
    setReceiveHandler(Packet::EType::MonitoringPopMessage, boost::bind(&Transport::onMonitoringPopMessage, this, _1));
#endif
    auto register_proxy = createPacket<Packet::RegisterProxy>();
    guaranteedSendTo(register_proxy, balancer_server_end_point, boost::bind(&Transport::logError, this, _1, _2));
    io_service.run();
}

void ProxyServer::onQuit(const boost::system::error_code& error, int sig_number)
{
    if (!error)
    {
        saveRegisteredUsers(registered_users_file_name);
    }
    else
    {
        LOG_ERROR << "users were not saved in the file";
    }
    Log::onQuit(error, sig_number);
}

bool ProxyServer::onLogin(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onLogin";
#endif

    const auto packet = getReceiveBufferAs<Packet::Login>();

    std::string login = packet->getLogin();
    std::string password = packet->getPassword();
    std::string full_name = packet->getFullName();

#ifdef _DEBUG
    LOG_DEBUG << "end_point: " << remote_end_point;
    LOG_DEBUG << "password: " << login;
    LOG_DEBUG << "login: " << password;
    LOG_DEBUG << "full_name: " << full_name;
#endif

    auto fit_login_it = login_to_user_info.find(login);
    UserInfo::Ptr cur_user;
    if (fit_login_it != login_to_user_info.end())
    {
#ifdef _DEBUG
        LOG_DEBUG << "existing user";
#endif
        cur_user = fit_login_it->second;
        if (password != cur_user->password)
        {
#ifdef _DEBUG
            LOG_DEBUG << "authorization failure";
#endif
            auto answer = createPacket<Packet::LoginAnswer>(packet->packet_number);
            answer->success = false;
            standardSend(answer);
            return true;
        }
        else
        {
#ifdef _DEBUG
            LOG_DEBUG << "authorization OK";
#endif
        }
    }
    else
    {
#ifdef _DEBUG
        LOG_DEBUG << "new user";
#endif
        UserInfo::Ptr new_user = std::make_shared<UserInfo>();
        new_user->login = login;
        new_user->password = password;
        new_user->full_name = full_name;
        login_to_user_info.insert(LoginToUserInfo::value_type(login, new_user));
        cur_user = new_user;
    }

    if (!cur_user->online)
    {
#ifdef _DEBUG
        LOG_DEBUG << "user is logging";
#endif
        std::uint32_t id;
        UserOnlineInfo* allocated_user_online_info = id_to_user_info.allocate(id);
        online_user_count++;

#ifdef _DEBUG
        LOG_DEBUG << "generated new id " << id;
#endif
        cur_user->user_token = id;
        cur_user->online = true;
        UserOnlineInfo* user_online_info = new(allocated_user_online_info) UserOnlineInfo(cur_user.get());
        user_online_info->user_end_point = remote_end_point;
    }
    else
    {
#ifdef _DEBUG
        LOG_DEBUG << "user is already online";
#endif
    }

    auto answer = createPacket<Packet::LoginAnswer>(packet->packet_number);
    answer->success = true;
    answer->user_token = cur_user->user_token;
    standardSend(answer);

    return true;
}

bool ProxyServer::onLogout(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onLogout";
#endif

    const auto packet = getReceiveBufferAs<Packet::Logout>();

#ifdef _DEBUG
    LOG_DEBUG << "end point: " << remote_end_point;
#endif

    std::uint32_t user_id = packet->user_token;
    UserOnlineInfo* user_online_info = id_to_user_info.find(user_id);

    if (!user_online_info)
    {
        LOG_ERROR << "user does not exist";
        return true;
    }

    if (user_online_info->user_end_point != remote_end_point)
    {
        LOG_ERROR << "user sent incorrect id " << user_id << " ip: " << user_online_info->user_end_point << " and it should be ip: " << remote_end_point;
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
    LOG_DEBUG << "onLogoutInternalAnswer";
#endif

    if (!validateInternalServer(remote_end_point))
    {
        LOG_WARNING << "internal server token validation error";
        return true;
    }

    const auto packet = getReceiveBufferAs<Packet::LogoutInternalAnswer>();

    std::uint32_t user_id = packet->user_token;
    UserOnlineInfo* user_online_info = id_to_user_info.find(user_id);

    if (!user_online_info)
    {
        LOG_ERROR << "user does not exist";
        return true;
    }

    user_online_info->user_info->online = false;
    user_online_info->user_info->user_token = 0;
    user_online_info->in_game = false;
    user_online_info->user_info = nullptr;
    boost::asio::ip::udp::endpoint user_end_point = user_online_info->user_end_point;
    id_to_user_info.deallocate(user_id);
    online_user_count--;

    auto answer = createPacket<Packet::LogoutAnswer>(packet->client_packet_number);
    answer->success = true;
    standardSendTo(answer, user_end_point);

    return true;
}

bool ProxyServer::onInitializePosition(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onInitializePosition";
#endif

    const auto packet = getReceiveBufferAs<Packet::InitializePosition>();

    std::uint32_t user_id = packet->user_token;
    UserOnlineInfo* user_online_info = id_to_user_info.find(user_id);

    if (!user_online_info)
    {
        LOG_ERROR << "user does not exist";
        return true;
    }

    if (user_online_info->user_end_point != remote_end_point)
    {
        LOG_ERROR << "user sent incorrect id " << user_id << " ip: " << user_online_info->user_end_point << " and it should be ip: " << remote_end_point;
        return true;
    }

    if (!user_online_info->in_game)
    {
        auto request = createPacket<Packet::InitializePositionInternal>();
        request->user_token = user_online_info->user_info->user_token;
        request->client_packet_number = packet->packet_number;
        request->proxy_packet_number = request->packet_number;
        request->user_location = packet->user_location;
        request->proxy_server_address = socket.local_endpoint().address().TO_V().to_bytes();
        request->proxy_server_port_number = port_number;
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
    LOG_DEBUG << "onInitializePositionInternalAnswer";
#endif

    if (!validateInternalServer(remote_end_point))
    {
        LOG_WARNING << "internal server token validation error";
        return true;
    }

    const auto packet = getReceiveBufferAs<Packet::InitializePositionInternalAnswer>();

    std::uint32_t user_id = packet->user_token;
    UserOnlineInfo* user_online_info = id_to_user_info.find(user_id);

    if (!user_online_info)
    {
#ifdef _DEBUG
        LOG_ERROR << "user does not exist";
#endif
        return true;
    }

    if (!user_online_info->user_info->online)
    {
        LOG_ERROR << "user is not online or is already in game";
        return true;
    }

    if (user_online_info->in_game)
    {
        LOG_ERROR << "user is already in game";
        return true;
    }

    user_online_info->in_game = true;

    user_online_info->node_server_end_point = boost::asio::ip::udp::endpoint(IpAddress(packet->node_server_address), packet->node_server_port_number);

    if (user_online_info->node_server_end_point.address().is_loopback())
    {
        user_online_info->node_server_end_point = boost::asio::ip::udp::endpoint(remote_end_point.address().TO_V(), packet->node_server_port_number);
    }

    auto answer = createPacket<Packet::InitializePositionAnswer>(packet->client_packet_number);
    answer->success = packet->success;
    answer->corrected_location = packet->corrected_location;
    standardSendTo(answer, user_online_info->user_end_point);

    return true;
}

bool ProxyServer::onUserAction(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onUserAction";
#endif

    const auto packet = getReceiveBufferAs<Packet::UserAction>();

    std::uint32_t user_id = packet->user_token;
    UserOnlineInfo* user_online_info = id_to_user_info.find(user_id);

    if (!user_online_info)
    {
        LOG_ERROR << "user does not exist";
        return true;
    }

    if (user_online_info->user_end_point != remote_end_point)
    {
        LOG_ERROR << "user sent incorrect id " << user_id << " ip: " << user_online_info->user_end_point << " and it should be ip: " << remote_end_point;
        return true;
    }

    if (!user_online_info->in_game)
    {
        LOG_ERROR << "user is not in game";
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
    LOG_DEBUG << "onUserActionInternalAnswer";
#endif

    if (!validateInternalServer(remote_end_point))
    {
        LOG_WARNING << "fake internal server detected";
        return true;
    }

    const auto packet = getReceiveBufferAs<Packet::UserActionInternalAnswer>();

    std::uint32_t user_id = packet->user_token;
    UserOnlineInfo* user_online_info = id_to_user_info.find(user_id);

    if (!user_online_info)
    {
        LOG_ERROR << "user does not exist";
        return true;
    }

    if (!user_online_info->in_game)
    {
        LOG_ERROR << "user is not in game";
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

bool ProxyServer::onRegisterProxyAnswer(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onRegisterProxyAnswer";
#endif

    if (!validateInternalServer(remote_end_point))
    {
        LOG_WARNING << "fake internal server detected";
        return true;
    }

    const auto packet = getReceiveBufferAs<Packet::RegisterProxyAnswer>();
    proxy_index = packet->proxy_index;
    setServerToken(proxy_index);

#ifdef _DEBUG
    LOG_DEBUG << "proxy_index " << proxy_index;
#endif

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
    LOG_DEBUG << "loading users from " << file_name << "...";
#endif
    std::ifstream registered_users(file_name);
    while (!registered_users.bad() && !registered_users.eof())
    {
        UserInfo::Ptr new_user = std::make_shared<UserInfo>();
        registered_users >> *new_user;
        if (new_user->login.empty() || new_user->password.empty() || new_user->full_name.empty())
        {
            break;
        }
        login_to_user_info.insert_or_assign(new_user->login, new_user);
    }
#ifdef _DEBUG
    LOG_DEBUG << "users are loaded!";
#endif
}

void ProxyServer::saveRegisteredUsers(const std::string& file_name) const
{
#ifdef _DEBUG
    LOG_DEBUG << "saving users to " << file_name << "...";
#endif
    std::ofstream registered_users(file_name);
    for (auto cur_user : login_to_user_info)
    {
        registered_users << *cur_user.second.get();
    }
#ifdef _DEBUG
    LOG_DEBUG << "users are saved!";
#endif
}
