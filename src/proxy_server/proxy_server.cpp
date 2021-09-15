// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <iostream>
#include <fstream>
#include <memory>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include "gkm_world/protocol.h"
#include "gkm_world/logger.h"
#include "gkm_world/configuration_reader.h"
#include "proxy_server.h"

ProxyServer::ProxyServer(const std::string& cfg_file_name_) :
    cfg_file_name(cfg_file_name_), signals(io_service, SIGINT, SIGTERM) {
    setApplicationType(Packet::EApplicationType::ProxyServer);

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

ProxyServer::~ProxyServer() {
    saveRegisteredUsers(registered_users_file_name);
}

bool ProxyServer::start() {
    Logger::setLoggingToScreen(log_to_screen);
    Logger::setLoggingToFile(log_to_file, "proxy_server_" + std::to_string(port_number) + ".log");
    Logger::setMinimumLevel(minimum_level);
    Logger::registerThisThread();

    LOG_INFO << "Proxy Server is starting...";

    try {
        dumpParameters();
        startImpl();
    } catch (boost::system::system_error& error) {
        LOG_FATAL << "boost::system::system_error: " << error.what();
        return false;
    } catch (const std::exception& exception) {
        LOG_FATAL << "std::exception: " << exception.what();
        return false;
    } catch (...) {
        LOG_FATAL << "Unknown error";
        return false;
    }
    return true;
}

void ProxyServer::dumpParameters() {
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

void ProxyServer::startImpl() {
    using namespace boost::placeholders;

    loadRegisteredUsers(registered_users_file_name);
    doReceive();
    signals.async_wait(boost::bind(&ProxyServer::onQuit, this, _1, _2));
    setReceiveHandler(Packet::EType::Login, boost::bind(&ProxyServer::onLogin, this, _1));
    setReceiveHandler(Packet::EType::Logout, boost::bind(&ProxyServer::onLogout, this, _1));
    setReceiveHandler(Packet::EType::LogoutInternalAnswer, boost::bind(&ProxyServer::onLogoutInternalAnswer, this, _1));
    setReceiveHandler(Packet::EType::InitializePosition, boost::bind(&ProxyServer::onInitializePosition, this, _1));
    setReceiveHandler(Packet::EType::InitializePositionInternalAnswer, boost::bind(&ProxyServer::onInitializePositionInternalAnswer, this, _1));
    setReceiveHandler(Packet::EType::UnitAction, boost::bind(&ProxyServer::onUnitAction, this, _1));
    setReceiveHandler(Packet::EType::UnitActionInternalAnswer, boost::bind(&ProxyServer::onUnitActionInternalAnswer, this, _1));
    setReceiveHandler(Packet::EType::RegisterProxyAnswer, boost::bind(&ProxyServer::onRegisterProxyAnswer, this, _1));

    auto register_proxy = createPacket<Packet::RegisterProxy>();
    guaranteedSendTo(register_proxy, balancer_server_end_point, boost::bind(&Transport::logError, this, _1, _2));
    io_service.run();
}

void ProxyServer::onQuit(const boost::system::error_code& error, int sig_number) {
    if (!error) {
        saveRegisteredUsers(registered_users_file_name);
    } else {
        LOG_ERROR << "users were not saved in the file";
    }
    onQuitSignal(error, sig_number);
}

bool ProxyServer::onLogin(size_t received_bytes) {
    LOG_DEBUG << "onLogin";

    const auto packet = getReceiveBufferAs<Packet::Login>();

    std::string login = packet->login.str();
    std::string password = packet->password.str();
    std::string full_name = packet->full_name.str();

    LOG_DEBUG << "end_point: " << remote_end_point;
    LOG_DEBUG << "password: " << login;
    LOG_DEBUG << "login: " << password;
    LOG_DEBUG << "full_name: " << full_name;

    auto fit_login_it = login_to_user_info.find(login);
    UserInfo::Ptr cur_user;
    if (fit_login_it != login_to_user_info.end()) {
        LOG_DEBUG << "existing user";

        cur_user = fit_login_it->second;
        if (password != cur_user->password) {
            LOG_DEBUG << "authorization failure";

            auto answer = createPacket<Packet::LoginAnswer>(packet->packet_number);
            answer->success = false;
            standardSend(answer);
            return true;
        } else {
            LOG_DEBUG << "authorization OK";
        }
    } else {
        LOG_DEBUG << "new user";

        UserInfo::Ptr new_user = std::make_shared<UserInfo>();
        new_user->login = login;
        new_user->password = password;
        new_user->full_name = full_name;
        login_to_user_info.emplace(login, new_user);
        cur_user = new_user;
    }

    if (!cur_user->online) {
        LOG_DEBUG << "user is logging";

        IndexType new_token;
        UserOnlineInfo* allocated_user_online_info = token_to_unit_info.allocate(new_token);
        online_user_count++;

        LOG_DEBUG << "generated new token " << new_token;

        cur_user->unit_token = new_token;
        cur_user->online = true;
        UserOnlineInfo* user_online_info = new(allocated_user_online_info) UserOnlineInfo(cur_user.get());
        user_online_info->user_end_point = remote_end_point;
    } else {
#ifdef _DEBUG
        LOG_DEBUG << "user is already online";
#endif
    }

    auto answer = createPacket<Packet::LoginAnswer>(packet->packet_number);
    answer->success = true;
    answer->unit_token = cur_user->unit_token;
    standardSend(answer);

    return true;
}

bool ProxyServer::onLogout(size_t received_bytes) {
    LOG_DEBUG << "onLogout";

    const auto packet = getReceiveBufferAs<Packet::Logout>();

    LOG_DEBUG << "end point: " << remote_end_point;

    IndexType unit_token = packet->unit_token;
    UserOnlineInfo* user_online_info = token_to_unit_info.find(unit_token);

    if (!user_online_info) {
        LOG_ERROR << "user does not exist";
        return true;
    }

    if (user_online_info->user_end_point != remote_end_point) {
        LOG_ERROR << "user sent incorrect token " << unit_token << " ip: " << user_online_info->user_end_point << " and it should be ip: " << remote_end_point;
        return true;
    }

    auto request = createPacket<Packet::LogoutInternal>();
    request->unit_token = user_online_info->user_info->unit_token;
    request->client_packet_number = packet->packet_number;
    standardSendTo(request, user_online_info->node_server_end_point);
    return true;
}

bool ProxyServer::onLogoutInternalAnswer(size_t received_bytes) {
    LOG_DEBUG << "onLogoutInternalAnswer";

    if (!validateInternalServer(remote_end_point)) {
        LOG_WARNING << "internal server token validation error";
        return true;
    }

    const auto packet = getReceiveBufferAs<Packet::LogoutInternalAnswer>();

    IndexType unit_token = packet->unit_token;
    UserOnlineInfo* user_online_info = token_to_unit_info.find(unit_token);

    if (!user_online_info) {
        LOG_ERROR << "user does not exist";
        return true;
    }

    user_online_info->user_info->online = false;
    user_online_info->user_info->unit_token = 0;
    user_online_info->in_game = false;
    user_online_info->user_info = nullptr;
    boost::asio::ip::udp::endpoint user_end_point = user_online_info->user_end_point;
    token_to_unit_info.deallocate(unit_token);
    online_user_count--;

    auto answer = createPacket<Packet::LogoutAnswer>(packet->client_packet_number);
    answer->success = true;
    standardSendTo(answer, user_end_point);

    return true;
}

bool ProxyServer::onInitializePosition(size_t received_bytes) {
    LOG_DEBUG << "onInitializePosition";

    const auto packet = getReceiveBufferAs<Packet::InitializePosition>();

    IndexType unit_token = packet->unit_location.unit_token;
    UserOnlineInfo* user_online_info = token_to_unit_info.find(unit_token);

    if (!user_online_info) {
        LOG_ERROR << "user does not exist";
        return true;
    }

    if (user_online_info->user_end_point != remote_end_point) {
        LOG_ERROR << "unit sent incorrect token " << unit_token << " ip: " << user_online_info->user_end_point << " and it should be ip: " << remote_end_point;
        return true;
    }

    if (!user_online_info->in_game) {
        auto request = createPacket<Packet::InitializePositionInternal>();
        request->client_packet_number = packet->packet_number;
        request->proxy_packet_number = request->packet_number;
        request->unit_location = packet->unit_location;
        request->proxy_server_address = socket.local_endpoint().address().TO_V().to_bytes();
        request->proxy_server_port_number = port_number;
        standardSendTo(request, balancer_server_end_point);
        return true;
    }

    auto answer = createPacket<Packet::InitializePositionAnswer>(packet->packet_number);
    answer->success = true;
    answer->corrected_location = packet->unit_location;
    standardSend(answer);

    return true;
}

bool ProxyServer::onInitializePositionInternalAnswer(size_t received_bytes) {
    LOG_DEBUG << "onInitializePositionInternalAnswer";

    if (!validateInternalServer(remote_end_point)) {
        LOG_WARNING << "internal server token validation error";
        return true;
    }

    const auto packet = getReceiveBufferAs<Packet::InitializePositionInternalAnswer>();

    IndexType unit_token = packet->corrected_location.unit_token;
    UserOnlineInfo* user_online_info = token_to_unit_info.find(unit_token);

    if (!user_online_info) {
        LOG_ERROR << "user does not exist";
        return true;
    }

    if (!user_online_info->user_info->online) {
        LOG_ERROR << "user is not online or is already in game";
        return true;
    }

    if (user_online_info->in_game) {
        LOG_ERROR << "user is already in game";
        return true;
    }

    user_online_info->in_game = true;

    user_online_info->node_server_end_point = boost::asio::ip::udp::endpoint(IpAddress(packet->node_server_address), packet->node_server_port_number);

    if (user_online_info->node_server_end_point.address().is_loopback()) {
        user_online_info->node_server_end_point = boost::asio::ip::udp::endpoint(remote_end_point.address().TO_V(), packet->node_server_port_number);
    }

    auto answer = createPacket<Packet::InitializePositionAnswer>(packet->client_packet_number);
    answer->success = packet->success;
    answer->corrected_location = packet->corrected_location;
    standardSendTo(answer, user_online_info->user_end_point);

    return true;
}

bool ProxyServer::onUnitAction(size_t received_bytes) {
    LOG_DEBUG << "onUnitAction";

    const auto packet = getReceiveBufferAs<Packet::UnitAction>();

    IndexType unit_token = packet->unit_token;
    UserOnlineInfo* user_online_info = token_to_unit_info.find(unit_token);

    if (!user_online_info) {
        LOG_ERROR << "user does not exist";
        return true;
    }

    if (user_online_info->user_end_point != remote_end_point) {
        LOG_ERROR << "user sent incorrect token " << unit_token << " ip: " << user_online_info->user_end_point << " and it should be ip: " << remote_end_point;
        return true;
    }

    if (!user_online_info->in_game) {
        LOG_ERROR << "user is not in game";
        return true;
    }

    auto request = createPacket<Packet::UnitActionInternal>();
    request->unit_token = user_online_info->user_info->unit_token;
    request->keyboard_state = packet->keyboard_state;
    request->client_packet_number = packet->packet_number;
    standardSendTo(request, user_online_info->node_server_end_point);
    return true;
}

bool ProxyServer::onUnitActionInternalAnswer(size_t received_bytes) {
    LOG_DEBUG << "onUserActionInternalAnswer";

    if (!validateInternalServer(remote_end_point)) {
        LOG_WARNING << "fake internal server detected";
        return true;
    }

    const auto packet = getReceiveBufferAs<Packet::UnitActionInternalAnswer>();

    IndexType unit_token = packet->unit_location.unit_token;
    UserOnlineInfo* user_online_info = token_to_unit_info.find(unit_token);

    if (!user_online_info) {
        LOG_ERROR << "user does not exist";
        return true;
    }

    if (!user_online_info->in_game) {
        LOG_ERROR << "user is not in game";
        return true;
    }

    auto answer = createPacket<Packet::UnitActionAnswer>(packet->client_packet_number);
    answer->unit_location = packet->unit_location;
    answer->other_unit_count = packet->other_unit_count;
    if (answer->other_unit_count >= Packet::MAX_UNIT_COUNT_IN_PACKET) {
        answer->other_unit_count = Packet::MAX_UNIT_COUNT_IN_PACKET;
    }
    for (std::uint16_t i = 0; i < answer->other_unit_count; ++i) {
        answer->other_unit[i] = packet->other_unit[i];
    }
    standardSendTo(answer, user_online_info->user_end_point);
    return true;
}

bool ProxyServer::onRegisterProxyAnswer(size_t received_bytes) {
    LOG_DEBUG << "onRegisterProxyAnswer";

    if (!validateInternalServer(remote_end_point)) {
        LOG_WARNING << "fake internal server detected";
        return true;
    }

    const auto packet = getReceiveBufferAs<Packet::RegisterProxyAnswer>();
    proxy_token = packet->proxy_token;
    setApplicationToken(proxy_token);

    LOG_DEBUG << "proxy_token " << proxy_token;

    return true;
}

bool ProxyServer::validateInternalServer(const boost::asio::ip::udp::endpoint& end_point) const {
    // TODO:

    return true;
}

void ProxyServer::loadRegisteredUsers(const std::string& file_name) {
    LOG_DEBUG << "loading users from " << file_name << "...";

    std::ifstream registered_users(file_name);
    while (!registered_users.bad() && !registered_users.eof()) {
        UserInfo::Ptr new_user = std::make_shared<UserInfo>();
        registered_users >> *new_user;
        if (new_user->login.empty() || new_user->password.empty() || new_user->full_name.empty()) {
            break;
        }
        login_to_user_info.insert_or_assign(new_user->login, new_user);
    }

    LOG_DEBUG << "users are loaded!";
}

void ProxyServer::saveRegisteredUsers(const std::string& file_name) const {
    LOG_DEBUG << "saving users to " << file_name << "...";

    std::ofstream registered_users(file_name);
    for (auto cur_user : login_to_user_info) {
        registered_users << *cur_user.second.get();
    }

    LOG_DEBUG << "users are saved!";
}
