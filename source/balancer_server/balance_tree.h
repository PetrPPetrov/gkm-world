// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include "global_types.h"
#include "transport.h"
#include "node.h"
#include "balance_tree/common.h"

class BalancerServer;

class BalanceTree
{
    BalancerServer& balancer_server;
    std::uint32_t token = 0;
    std::size_t level = 0;
    SquareCell bounding_box;
    BalanceTree* parent = nullptr;
    bool leaf_node = true;
    std::array<BalanceTree*, CountOfChildren> children;
    std::array<BalanceTree*, 4 * (NEIGHBOR_COUNT_AT_SIDE + 1)> neighbors;
    std::uint32_t user_count = 0;
    std::uint16_t node_server_port_number = 0;
    boost::asio::ip::udp::endpoint node_server_end_point;

public:
    BalanceTree(const BalanceTree&) = delete;
    BalanceTree& operator=(const BalanceTree&) = delete;
    BalanceTree(BalancerServer& balancer_server, std::uint32_t token, SquareCell bounding_box);
    BalanceTree(BalancerServer& balancer_server, std::uint32_t token, SquareCell bounding_box, BalanceTree* parent);

    std::uint32_t getToken() const;
    void dump() const;

    bool registerNewUser(const Packet::InitializePositionInternal& packet);
    void getInfo(Packet::GetNodeInfoAnswer* answer) const;
    void getMonitoringInfo(Packet::MonitoringBalanceTreeInfoAnswer* answer) const;
    void getMonitoringNeighborInfo(Packet::MonitoringBalanceTreeNeighborInfoAnswer* answer, CellIndex neighbor_cell) const;

    void startNodeServer();
    void startNodeServers();

    void staticSplit();
    void staticSplit(std::size_t required_level);

private:
    static std::uint32_t getNeighborIndex(const SquareCell& box, CellIndex neighbor);
    void setNeighbor(CellIndex neighbor_cell, BalanceTree* neighbor);
    BalanceTree* getNeighbor(CellIndex neighbor_cell) const;
};
