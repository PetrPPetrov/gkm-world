﻿// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <memory>
#include <boost/geometry/arithmetic/arithmetic.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/name_generator.hpp>
#include <boost/uuid/random_generator.hpp>
#include "Eigen/Eigen"
#include "protocol.h"
#include "log.h"
#include "config.h"
#include "node.h"
#include "process_spawn.h"
#include "node_server.h"

#ifdef DEBUG_MONITOR
#include <windows.h>
#endif

NodeServer::NodeServer(unsigned short port_number_, LogicThread& logic_thread_) :
    port_number(port_number_), logic_thread(logic_thread_), signals(io_service, SIGINT, SIGTERM)
{
    socket = boost::asio::ip::udp::socket(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port_number));
    std::ifstream config_file("node_server.cfg");
    ConfigurationReader config_reader;
    config_reader.addParameter("balancer_server_ip", balancer_server_ip);
    config_reader.addParameter("balancer_server_port_number", balancer_server_port_number);
    config_reader.read(config_file);

#ifdef _DEBUG
    LOG_DEBUG << "balancer_server_ip " << balancer_server_ip << std::endl;
    LOG_DEBUG << "balancer_server_port_number " << balancer_server_port_number << std::endl;
#endif

    balancer_server_end_point = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::from_string(balancer_server_ip), balancer_server_port_number);

    node_server_end_point = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4(), port_number);
    node_server_token = 0;

    node_server_path = findNodeServerPath();
    LOG_INFO << "node_server_path " << node_server_path << std::endl;

#ifdef _DEBUG
    LOG_DEBUG << "node_server_ip " << node_server_end_point.address() << std::endl;
    LOG_DEBUG << "node_server_port_number " << node_server_end_point.port() << std::endl;
    LOG_DEBUG << "node_server_token " << node_server_token << std::endl;
#endif
}

void NodeServer::start()
{
    doReceive();
    getNodeInfo();
    signals.async_wait(Log::onQuit);
    setReceiveHandler(Packet::EType::LogoutInternal, boost::bind(&NodeServer::onLogoutInternal, this, _1));
    setReceiveHandler(Packet::EType::InitializePositionInternal, boost::bind(&NodeServer::onInitializePositionInternal, this, _1));
    setReceiveHandler(Packet::EType::UserActionInternal, boost::bind(&NodeServer::onUserActionInternal, this, _1));
    setReceiveHandler(Packet::EType::GetNodeInfoAnswer, boost::bind(&NodeServer::onGetNodeInfoAnswer, this, _1));
    io_service.run();
}

bool NodeServer::onLogoutInternal(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onLogoutInternal" << std::endl;
#endif

    const auto packet = getReceiveBufferAs<Packet::LogoutInternal>();
    UserLocation* user_location = uuid_to_user_location.find(packet->user_token);
    if (user_location)
    {
        logic_thread.removeUser(user_location);
        // TODO: remove user_location from uuid_to_user_location with some delay, to avoid race condition
        user_count--;
        uuid_to_user_location.deallocate(packet->user_token);
    }

    auto answer = createPacket<Packet::LogoutInternalAnswer>(packet->packet_number);
    answer->success = true;
    answer->user_token = packet->user_token;
    answer->client_packet_number = packet->client_packet_number;
    standardSend(answer);
    return true;
}

bool NodeServer::onInitializePositionInternal(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onInitializePositionInternal" << std::endl;
#endif

    const auto packet = getReceiveBufferAs<Packet::InitializePositionInternal>();
    UserLocation* user_location = uuid_to_user_location.find(packet->user_token);
    if (!user_location)
    {
        // Register new user
        if (inside(packet->user_location.x_pos, packet->user_location.y_pos, global_bounding_box))
        {
            user_count++;
            UserLocation* new_user = new(uuid_to_user_location.allocate(packet->user_token)) UserLocation;
            new_user->user_location.user_token = packet->user_token;
            new_user->user_location.x_pos = packet->user_location.x_pos;
            new_user->user_location.y_pos = packet->user_location.y_pos;
            new_user->user_location.direction = packet->user_location.direction;
            logic_thread.addNewUser(new_user);

#ifdef _DEBUG
            LOG_DEBUG << "initialized position of user " << new_user->user_location.user_token << std::endl;
#endif
            auto answer = createPacket<Packet::InitializePositionInternalAnswer>(packet->packet_number);
            answer->user_token = new_user->user_location.user_token;
            answer->client_packet_number = packet->client_packet_number;
            answer->corrected_location = new_user->user_location;
            answer->node_server_address = node_server_end_point.address().to_v4().to_bytes();
            answer->node_server_port_number = node_server_end_point.port();
            answer->proxy_packet_number = packet->proxy_packet_number;
            answer->success = true;
            standardSend(answer);
            return true;
        }
        else
        {
#ifdef _DEBUG
            LOG_DEBUG << "can not initialize position of user " << packet->user_token << " because position is out of bounding box" << std::endl;
#endif
        }
    }
    else
    {
#ifdef _DEBUG
        LOG_DEBUG << "can not initialize position of user " << packet->user_token << " because user already has a position, however, return success" << std::endl;
#endif
        // However return success for guaranteed delivery service
        auto answer = createPacket<Packet::InitializePositionInternalAnswer>(packet->packet_number);
        answer->user_token = packet->user_token;
        answer->client_packet_number = packet->client_packet_number;
        answer->corrected_location = user_location->user_location;
        answer->node_server_address = node_server_end_point.address().to_v4().to_bytes();
        answer->node_server_port_number = node_server_end_point.port();
        answer->proxy_packet_number = packet->proxy_packet_number;
        answer->success = true;
        standardSend(answer);
        return true;
    }

    auto answer = createPacket<Packet::InitializePositionInternalAnswer>(packet->packet_number);
    answer->user_token = packet->user_token;
    answer->client_packet_number = packet->client_packet_number;
    answer->node_server_address = node_server_end_point.address().to_v4().to_bytes();
    answer->node_server_port_number = node_server_end_point.port();
    answer->proxy_packet_number = packet->proxy_packet_number;
    answer->success = false;
    standardSend(answer);
    return true;
}

bool NodeServer::onUserActionInternal(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onUserActionInternal" << std::endl;
#endif

    const auto packet = getReceiveBufferAs<Packet::UserActionInternal>();
    auto user_location = uuid_to_user_location.find(packet->user_token);
    if (user_location)
    {
        // If user is in the notify zone, then need to send message neighbors node servers
        if (!inside(user_location->user_location.x_pos, user_location->user_location.y_pos, without_notify_bounding_box))
        {
            ENeighborIndex neighbor_index = getNeihgborByPosition(user_location->user_location.x_pos, user_location->user_location.y_pos, global_bounding_box);
            boost::asio::ip::udp::endpoint neighbor_end_point = neighbor_end_points[neighbor_index];
            auto user_action_internal_packet = createPacket<Packet::UserActionInternal>(packet->packet_number);
            user_action_internal_packet->keyboard_state = packet->keyboard_state;
            user_action_internal_packet->user_token = packet->user_token;
            standardSendTo(user_action_internal_packet, neighbor_end_point);
        }

        user_location->state = packet->keyboard_state;
        auto answer = createPacket<Packet::UserActionInternalAnswer>(packet->packet_number);
        answer->user_token = packet->user_token;
        answer->user_location = user_location->user_location;
        answer->client_packet_number = packet->client_packet_number;
        logic_thread.fillOtherUserList(*answer, packet->user_token);
        standardSend(answer);
        return true;
    }

    LOG_WARNING << "user with token " << packet->user_token << " not found" << std::endl;
    return true;
}

bool NodeServer::onGetNodeInfoAnswer(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onGetNodeInfoAnswer" << std::endl;
#endif

    const auto packet = getReceiveBufferAs<Packet::GetNodeInfoAnswer>();
    if (!packet->success)
    {
        LOG_FATAL << "error receiving node_server information" << std::endl;
        throw std::runtime_error("error receiving node_server information");
    }

    global_bounding_box = packet->bounding_box;
    without_notify_bounding_box.min_corner().set<0>(global_bounding_box.min_corner().get<0>() + NOTIFY_DISTANCE);
    without_notify_bounding_box.min_corner().set<1>(global_bounding_box.min_corner().get<1>() + NOTIFY_DISTANCE);
    without_notify_bounding_box.max_corner().set<0>(global_bounding_box.max_corner().get<0>() - NOTIFY_DISTANCE);
    without_notify_bounding_box.max_corner().set<1>(global_bounding_box.max_corner().get<1>() - NOTIFY_DISTANCE);

    for (size_t i = NeighborFirst; i < NeighborLast; ++i)
    {
        neighbor_end_points[i] = boost::asio::ip::udp::endpoint(
            boost::asio::ip::address_v4(packet->neighbor_addresses[i]), packet->neighbor_ports[i]);
        neighbor_tokens[i] = packet->neighbor_tokens[i];
#ifdef _DEBUG
        LOG_DEBUG << "neighbor_end_points[ " << i << "] " << neighbor_end_points[i] << std::endl;
        LOG_DEBUG << "neighbor_tokens[ " << i << "] " << neighbor_tokens[i] << std::endl;
#endif
    }
    parent_end_point = boost::asio::ip::udp::endpoint(
        boost::asio::ip::address_v4(packet->parent_address), packet->parent_port);
    parent_token = packet->parent_token;
#ifdef _DEBUG
    LOG_DEBUG << "parent_end_point " << parent_end_point << std::endl;
    LOG_DEBUG << "parent_token " << parent_token << std::endl;
#endif

    return true;
}

bool NodeServer::onSpawnNodeServer(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onSpawnNodeServer" << std::endl;
#endif

    const auto packet = getReceiveBufferAs<Packet::SpawnNodeServer>();

    processSpawn(node_server_path.generic_string(), std::to_string(packet->node_server_port));

    return true;
}

void NodeServer::getNodeInfo()
{
#ifdef _DEBUG
    LOG_DEBUG << "getNodeInfo" << std::endl;
#endif
    auto request = createPacket<Packet::GetNodeInfo>();
    standardSendTo(request, balancer_server_end_point);
}
