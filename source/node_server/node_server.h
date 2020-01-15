// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <map>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/filesystem.hpp>
#include "packet_pool.h"
#include "fast_index_map.h"
#include "transport.h"
#include "user_location.h"
#include "logic_thread.h"

class NodeServer : public Transport
{
    std::uint16_t port_number;
    boost::asio::signal_set signals;

    std::string balancer_server_ip = "127.0.0.1";
    std::uint16_t balancer_server_port_number = 17013;
    boost::asio::ip::udp::endpoint balancer_server_end_point;

    typedef Memory::FastIndexMap<UserLocation> uuid_to_user_location_t;
    uuid_to_user_location_t uuid_to_user_location;
    std::uint32_t user_count = 0;

    boost::asio::ip::udp::endpoint node_server_end_point;
    std::uint32_t node_server_token;
    SquareCell global_bounding_box;
    SquareCell without_notify_bounding_box;
    std::array<boost::asio::ip::udp::endpoint, 12> neighbor_end_points;
    std::array<std::uint32_t, 12> neighbor_tokens;
    boost::asio::ip::udp::endpoint parent_end_point;
    std::uint32_t parent_token;

    LogicThread& logic_thread;
    boost::filesystem::path node_server_path;

public:
    NodeServer(std::uint16_t port_number, LogicThread& logic_thread);
    bool start();

private:
    void dumpParameters();
    void startImpl();
    bool onLogoutInternal(size_t received_bytes);
    bool onInitializePositionInternal(size_t received_bytes);
    bool onUserActionInternal(size_t received_bytes);
    bool onGetNodeInfoAnswer(size_t received_bytes);
    bool onSpawnNodeServer(size_t received_bytes);
    void getNodeInfo();
};
