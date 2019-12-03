// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/core/noncopyable.hpp>
#include "transport.h"
#include "node.h"
#include "bounding_box.h"

class BalancerServer;

class BalanceTree : private boost::noncopyable
{
    const std::size_t MAX_USER = 1000;
    const std::size_t MIN_USER = 100;

    enum EChildIndex : std::uint8_t
    {
        ChildUpperLeft,
        ChildUpperRight,
        ChildLowerRight,
        ChildLowerLeft,
        ChildLast,
        ChildFirst = ChildUpperLeft
    };

    BalancerServer& balancer_server;
    std::uint32_t token = 0;
    std::size_t level = 0;
    box2i_t bounding_box;
    BalanceTree* parent = nullptr;
    bool leaf_node = true;
    std::array<BalanceTree*, 4> children;
    std::array<BalanceTree*, 12> neighbors;
    std::size_t user_count = 0;
    unsigned short node_server_port_number = 0;
    boost::asio::ip::udp::endpoint node_server_end_point;

public:
    BalanceTree(BalancerServer& balancer_server, std::uint32_t token, const box2i_t& bounding_box);
    BalanceTree(BalancerServer& balancer_server, std::uint32_t token, const box2i_t& bounding_box, BalanceTree* parent);

    std::uint32_t getToken() const;
    bool registerNewUser(const Packet::InitializePositionInternal& packet);
    void increaseUserCount();
    void decreaseUserCount();
    void balance();
    void getInfo(Packet::GetNodeInfoAnswer* answer) const;
    void startNodeServer();
    void dump() const;

    void split();
    void splitChildren();
    void startChildrenNodeServer();
};
