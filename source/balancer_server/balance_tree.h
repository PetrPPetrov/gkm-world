// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include "global_types.h"
#include "transport.h"
#include "node.h"

class BalancerServer;

class BalanceTree
{
    enum EChildIndex : std::uint8_t
    {
        ChildUpperLeft,
        ChildUpperRight,
        ChildLowerRight,
        ChildLowerLeft,
        ChildLast,
        ChildFirst = ChildUpperLeft,
        CountOfChildren = ChildLast
    };

    BalancerServer& balancer_server;
    std::uint32_t token = 0;
    std::size_t level = 0;
    SquareCell bounding_box;
    std::int32_t size = MAXIMAL_NODE_SIZE;
    BalanceTree* parent = nullptr;
    bool leaf_node = true;
    std::array<BalanceTree*, CountOfChildren> children;
    std::array<BalanceTree*, 4 * (MAXIMAL_NODE_SIZE / MINIMAL_NODE_SIZE + 1)> neighbors;
    std::uint32_t user_count = 0;
    unsigned short node_server_port_number = 0;
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

    void startNodeServer();
    void startNodeServers();

    void staticSplit();
    void staticSplit(std::size_t required_level);

private:
    void setNeighbor
};
