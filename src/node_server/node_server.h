// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <map>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/filesystem.hpp>
#include "gkm_world/fast_index.h"
#include "gkm_world/coordinate_2d.h"
#include "gkm_world/transport.h"
#include "user_location.h"

class NodeServer : public Transport {
    const std::string cfg_file_name;
    PortNumberType port_number;
    boost::asio::signal_set signals;

    std::string balancer_server_ip = "127.0.0.1";
    PortNumberType balancer_server_port_number = 17013;
    boost::asio::ip::udp::endpoint balancer_server_end_point;

    UnitList unit_list;
    IndexType unit_count = 0;

    boost::asio::ip::udp::endpoint node_server_end_point;
    IndexType node_server_token;
    Square2D global_bounding_box;
    Square2D without_notify_bounding_box;
    std::array<boost::asio::ip::udp::endpoint, 12> neighbor_end_points;
    std::array<IndexType, 12> neighbor_tokens;
    boost::asio::ip::udp::endpoint parent_end_point;
    IndexType parent_token;

    boost::filesystem::path node_server_path;

    Packet::ESeverityType minimum_level = Packet::ESeverityType::DebugMessage;
    bool log_to_screen = false;
    bool log_to_file = true;

public:
    NodeServer(PortNumberType port_number, const std::string& cfg_file_name);
    bool start();
    const UnitList& getNodeList() const;

private:
    void dumpParameters();
    void startImpl();
    bool onLogoutInternal(size_t received_bytes);
    bool onInitializePositionInternal(size_t received_bytes);
    bool onUnitActionInternal(size_t received_bytes);
    bool onGetNodeInfoAnswer(size_t received_bytes);
    bool onSpawnNodeServer(size_t received_bytes);
    void getNodeInfo();
};
