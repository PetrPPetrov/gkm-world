// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <cassert>
#include <boost/asio.hpp>
#include <boost/geometry.hpp>
#include "balance_tree.h"
#include "balancer_server.h"
#include "log.h"

BalanceTree::BalanceTree(BalancerServer& balancer_server_, std::uint32_t index_, const box2i_t& bounding_box_) :
    BalanceTree(balancer_server_, index_, bounding_box_, nullptr)
{
}

BalanceTree::BalanceTree(BalancerServer& balancer_server_, std::uint32_t index_, const box2i_t& bounding_box_, BalanceTree* parent_) :
    balancer_server(balancer_server_), token(index_)
{
    bounding_box = bounding_box_;
    parent = parent_;
    if (parent)
    {
        level = parent->level + 1;
    }
    children.fill(nullptr);
    neighbors.fill(nullptr);
}

std::uint32_t BalanceTree::getToken() const
{
    return token;
}

bool BalanceTree::registerNewUser(const Packet::InitializePositionInternal& packet)
{
    if (!inside(packet.user_location.x_pos, packet.user_location.y_pos, bounding_box))
    {
        return false;
    }
    if (leaf_node)
    {
        auto create_new_user_request = balancer_server.createPacket<Packet::InitializePositionInternal>();
        create_new_user_request->client_packet_number = packet.client_packet_number;
        create_new_user_request->proxy_packet_number = packet.proxy_packet_number;
        create_new_user_request->user_location = packet.user_location;
        create_new_user_request->user_token = packet.user_token;
#ifdef _DEBUG
        LOG_DEBUG << "sending request to " << node_server_end_point.address().to_string() << ":" << node_server_end_point.port() << std::endl;
#endif
        balancer_server.standardSendTo(create_new_user_request, node_server_end_point);
        increaseUserCount();
        balance();
        return true;
    }
    for (auto child : children)
    {
        if (child && child->registerNewUser(packet))
            return true;
    }
    return false;
}

void BalanceTree::addUser()
{
    // TODO:
}

void BalanceTree::deleteUser()
{
    // TODO:
}

void BalanceTree::increaseUserCount()
{
    user_count++;
    if (parent)
    {
        parent->increaseUserCount();
    }
}

void BalanceTree::decreaseUserCount()
{
    user_count--;
    if (parent)
    {
        parent->decreaseUserCount();
    }
}

void BalanceTree::balance()
{
    if (leaf_node)
    {
        if (user_count >= MAX_USER)
        {
            split();
        }
        return;
    }
    else
    {
        if (user_count < MIN_USER)
        {
            merge();
        }
    }
    for (auto child : children)
    {
        child->balance();
    }
}

void BalanceTree::getInfo(Packet::GetNodeInfoAnswer* answer) const
{
    assert(answer);

    answer->bounding_box = bounding_box;

    for (size_t i = NeighborFirst; i < NeighborLast; ++i)
    {
        answer->neighbor_addresses[i] = boost::asio::ip::address_v4::from_string("127.0.0.1").to_bytes();
        auto neighbor = neighbors[i];
        if (neighbor)
        {
            answer->neighbor_ports[i] = neighbor->node_server_port_number;
            answer->neighbor_tokens[i] = neighbor->token;
        }
        else
        {
            answer->neighbor_ports[i] = 0;
            answer->neighbor_tokens[i] = { 0 };
        }
    }

    if (parent)
    {
        answer->parent_port = parent->node_server_port_number;
        answer->parent_token = parent->token;
    }
    else
    {
        answer->parent_port = 0;
        answer->parent_token = { 0 };
    }
    answer->parent_address = boost::asio::ip::address_v4::from_string("127.0.0.1").to_bytes();
    answer->success = true;
}

void BalanceTree::startNodeServer()
{
    if (!leaf_node)
    {
        return;
    }

    NodeInfo node_info = balancer_server.getAvailableNode();
    node_server_end_point = boost::asio::ip::udp::endpoint(node_info.ip_address, node_info.port_number);
    node_server_port_number = node_info.port_number;

#ifdef _DEBUG
    LOG_DEBUG << "starting node_server " << node_info.ip_address << ":" << node_info.port_number << " " << token << std::endl;
#endif

    balancer_server.startNode(node_info, this);
}

void BalanceTree::dump() const
{
    const std::string indent(level, ' ');
    std::cout << indent << "node_server_token: " << token << std::endl;
    std::cout << indent << "node_server_end_point: " << node_server_end_point << std::endl;
    std::cout << indent << "leaf_node: " << leaf_node << std::endl;
    std::cout << indent << "level: " << level << std::endl;
    std::cout << indent << "user_count: " << user_count << std::endl;
}

void BalanceTree::split()
{
    if (!leaf_node)
    {
        return;
    }

    const std::int32_t min_x = bounding_box.min_corner().get<0>();
    const std::int32_t min_y = bounding_box.min_corner().get<1>();
    const std::int32_t max_x = bounding_box.max_corner().get<0>();
    const std::int32_t max_y = bounding_box.max_corner().get<1>();
    const std::int32_t middle_x = (max_x - min_x) / 2 + min_x;
    const std::int32_t middle_y = (max_y - min_y) / 2 + min_y;

    box2i_t upper_left_bb;
    upper_left_bb.min_corner().set<0>(min_x);
    upper_left_bb.min_corner().set<1>(middle_y);
    upper_left_bb.max_corner().set<0>(middle_x);
    upper_left_bb.max_corner().set<1>(max_y);

    box2i_t upper_right_bb;
    upper_right_bb.min_corner().set<0>(middle_x);
    upper_right_bb.min_corner().set<1>(middle_y);
    upper_right_bb.max_corner().set<0>(max_x);
    upper_right_bb.max_corner().set<1>(max_y);

    box2i_t lower_right_bb;
    lower_right_bb.min_corner().set<0>(middle_x);
    lower_right_bb.min_corner().set<1>(min_y);
    lower_right_bb.max_corner().set<0>(max_x);
    lower_right_bb.max_corner().set<1>(middle_y);

    box2i_t lower_left_bb = bounding_box;
    lower_left_bb.max_corner().set<0>(middle_x);
    lower_left_bb.max_corner().set<1>(middle_y);

    leaf_node = false;

    BalanceTree* upper_left_sub_tree = balancer_server.createNewBalanceNode(upper_left_bb, this);
    BalanceTree* upper_right_sub_tree = balancer_server.createNewBalanceNode(upper_right_bb, this);
    BalanceTree* lower_right_sub_tree = balancer_server.createNewBalanceNode(lower_right_bb, this);
    BalanceTree* lower_left_sub_tree = balancer_server.createNewBalanceNode(lower_left_bb, this);

    children[ChildUpperLeft] = upper_left_sub_tree;
    children[ChildUpperRight] = upper_right_sub_tree;
    children[ChildLowerRight] = lower_right_sub_tree;
    children[ChildLowerLeft] = lower_left_sub_tree;

    upper_left_sub_tree->neighbors[NeighborUpperLeftCorner] = neighbors[NeighborUpperLeftCorner];
    upper_left_sub_tree->neighbors[NeighborUpperLeft] = neighbors[NeighborUpperLeft];
    upper_left_sub_tree->neighbors[NeighborUpperRight] = neighbors[NeighborUpperLeft];
    upper_left_sub_tree->neighbors[NeighborUpperRightCorner] = neighbors[NeighborUpperRight];
    upper_left_sub_tree->neighbors[NeighborRightTop] = upper_right_sub_tree;
    upper_left_sub_tree->neighbors[NeighborRightBottom] = upper_right_sub_tree;
    upper_left_sub_tree->neighbors[NeighborLowerRightCorner] = lower_right_sub_tree;
    upper_left_sub_tree->neighbors[NeighborLowerRight] = lower_left_sub_tree;
    upper_left_sub_tree->neighbors[NeighborLowerLeft] = lower_left_sub_tree;
    upper_left_sub_tree->neighbors[NeighborLowerLeftCorner] = neighbors[NeighborLeftBottom];
    upper_left_sub_tree->neighbors[NeighborLeftBottom] = neighbors[NeighborLeftTop];
    upper_left_sub_tree->neighbors[NeighborLeftTop] = neighbors[NeighborLeftTop];

    upper_right_sub_tree->neighbors[NeighborUpperLeftCorner] = neighbors[NeighborUpperLeft];
    upper_right_sub_tree->neighbors[NeighborUpperLeft] = neighbors[NeighborUpperRight];
    upper_right_sub_tree->neighbors[NeighborUpperRight] = neighbors[NeighborUpperRight];
    upper_right_sub_tree->neighbors[NeighborUpperRightCorner] = neighbors[NeighborUpperRightCorner];
    upper_right_sub_tree->neighbors[NeighborRightTop] = neighbors[NeighborRightTop];
    upper_right_sub_tree->neighbors[NeighborRightBottom] = neighbors[NeighborRightTop];
    upper_right_sub_tree->neighbors[NeighborLowerRightCorner] = neighbors[NeighborRightBottom];
    upper_right_sub_tree->neighbors[NeighborLowerRight] = lower_right_sub_tree;
    upper_right_sub_tree->neighbors[NeighborLowerLeft] = lower_right_sub_tree;
    upper_right_sub_tree->neighbors[NeighborLowerLeftCorner] = lower_left_sub_tree;
    upper_right_sub_tree->neighbors[NeighborLeftBottom] = upper_left_sub_tree;
    upper_right_sub_tree->neighbors[NeighborLeftTop] = upper_left_sub_tree;

    lower_right_sub_tree->neighbors[NeighborUpperLeftCorner] = upper_left_sub_tree;
    lower_right_sub_tree->neighbors[NeighborUpperLeft] = upper_right_sub_tree;
    lower_right_sub_tree->neighbors[NeighborUpperRight] = upper_right_sub_tree;
    lower_right_sub_tree->neighbors[NeighborUpperRightCorner] = neighbors[NeighborRightTop];
    lower_right_sub_tree->neighbors[NeighborRightTop] = neighbors[NeighborRightBottom];
    lower_right_sub_tree->neighbors[NeighborRightBottom] = neighbors[NeighborRightBottom];
    lower_right_sub_tree->neighbors[NeighborLowerRightCorner] = neighbors[NeighborLowerRightCorner];
    lower_right_sub_tree->neighbors[NeighborLowerRight] = neighbors[NeighborLowerRight];
    lower_right_sub_tree->neighbors[NeighborLowerLeft] = neighbors[NeighborLowerRight];
    lower_right_sub_tree->neighbors[NeighborLowerLeftCorner] = neighbors[NeighborLowerLeft];
    lower_right_sub_tree->neighbors[NeighborLeftBottom] = lower_left_sub_tree;
    lower_right_sub_tree->neighbors[NeighborLeftTop] = lower_left_sub_tree;

    lower_left_sub_tree->neighbors[NeighborUpperLeftCorner] = neighbors[NeighborLeftTop];
    lower_left_sub_tree->neighbors[NeighborUpperLeft] = upper_left_sub_tree;
    lower_left_sub_tree->neighbors[NeighborUpperRight] = upper_left_sub_tree;
    lower_left_sub_tree->neighbors[NeighborUpperRightCorner] = upper_right_sub_tree;
    lower_left_sub_tree->neighbors[NeighborRightTop] = lower_right_sub_tree;
    lower_left_sub_tree->neighbors[NeighborRightBottom] = lower_right_sub_tree;
    lower_left_sub_tree->neighbors[NeighborLowerRightCorner] = neighbors[NeighborLowerRight];
    lower_left_sub_tree->neighbors[NeighborLowerRight] = neighbors[NeighborLowerLeft];
    lower_left_sub_tree->neighbors[NeighborLowerLeft] = neighbors[NeighborLowerLeft];
    lower_left_sub_tree->neighbors[NeighborLowerLeftCorner] = neighbors[NeighborLowerLeftCorner];
    lower_left_sub_tree->neighbors[NeighborLeftBottom] = neighbors[NeighborLeftBottom];
    lower_left_sub_tree->neighbors[NeighborLeftTop] = neighbors[NeighborLeftBottom];

    {
        auto upper_left_corner_neighbor = neighbors[NeighborUpperLeftCorner];
        if (upper_left_corner_neighbor)
        {
            upper_left_corner_neighbor->neighbors[NeighborLowerRightCorner] = upper_left_sub_tree;
        }
    }

    {
        auto upper_left_neighbor = neighbors[NeighborUpperLeft];
        if (upper_left_neighbor)
        {
            upper_left_neighbor->neighbors[NeighborLowerRightCorner] = upper_right_sub_tree;
            upper_left_neighbor->neighbors[NeighborLowerRight] = upper_left_sub_tree;
            upper_left_neighbor->neighbors[NeighborLowerLeft] = upper_left_sub_tree;
        }
    }

    {
        auto upper_right_neighbor = neighbors[NeighborUpperRight];
        if (upper_right_neighbor)
        {
            upper_right_neighbor->neighbors[NeighborLowerRight] = upper_right_sub_tree;
            upper_right_neighbor->neighbors[NeighborLowerLeft] = upper_right_sub_tree;
            upper_right_neighbor->neighbors[NeighborLowerLeftCorner] = upper_left_sub_tree;
        }
    }

    {
        auto upper_right_corner_neighbor = neighbors[NeighborUpperRightCorner];
        if (upper_right_corner_neighbor)
        {
            upper_right_corner_neighbor->neighbors[NeighborLowerLeftCorner] = upper_right_sub_tree;
        }
    }

    {
        auto right_top_neighbor = neighbors[NeighborRightTop];
        if (right_top_neighbor)
        {
            right_top_neighbor->neighbors[NeighborLowerLeftCorner] = lower_right_sub_tree;
            right_top_neighbor->neighbors[NeighborLeftBottom] = upper_right_sub_tree;
            right_top_neighbor->neighbors[NeighborLeftTop] = upper_right_sub_tree;
        }
    }

    {
        auto right_bottom_neighbor = neighbors[NeighborRightBottom];
        if (right_bottom_neighbor)
        {
            right_bottom_neighbor->neighbors[NeighborLeftBottom] = lower_right_sub_tree;
            right_bottom_neighbor->neighbors[NeighborLeftTop] = lower_right_sub_tree;
            right_bottom_neighbor->neighbors[NeighborUpperLeftCorner] = upper_right_sub_tree;
        }
    }

    {
        auto lower_right_corner_neighbor = neighbors[NeighborLowerRightCorner];
        if (lower_right_corner_neighbor)
        {
            lower_right_corner_neighbor->neighbors[NeighborUpperLeftCorner] = lower_right_sub_tree;
        }
    }

    {
        auto lower_right_neighbor = neighbors[NeighborLowerRight];
        if (lower_right_neighbor)
        {
            lower_right_neighbor->neighbors[NeighborUpperLeftCorner] = lower_left_sub_tree;
            lower_right_neighbor->neighbors[NeighborUpperLeft] = lower_right_sub_tree;
            lower_right_neighbor->neighbors[NeighborUpperRight] = lower_right_sub_tree;
        }
    }

    {
        auto lower_left_neighbor = neighbors[NeighborLowerLeft];
        if (lower_left_neighbor)
        {
            lower_left_neighbor->neighbors[NeighborUpperLeft] = lower_left_sub_tree;
            lower_left_neighbor->neighbors[NeighborUpperRight] = lower_left_sub_tree;
            lower_left_neighbor->neighbors[NeighborUpperRightCorner] = lower_right_sub_tree;
        }
    }

    {
        auto lower_left_corner_neighbor = neighbors[NeighborLowerLeftCorner];
        if (lower_left_corner_neighbor)
        {
            lower_left_corner_neighbor->neighbors[NeighborUpperRightCorner] = lower_left_sub_tree;
        }
    }

    {
        auto left_bottom_neighbor = neighbors[NeighborLeftBottom];
        if (left_bottom_neighbor)
        {
            left_bottom_neighbor->neighbors[NeighborUpperRightCorner] = upper_left_sub_tree;
            left_bottom_neighbor->neighbors[NeighborRightTop] = lower_left_sub_tree;
            left_bottom_neighbor->neighbors[NeighborRightBottom] = lower_left_sub_tree;
        }
    }

    {
        auto left_top_neighbor = neighbors[NeighborLeftTop];
        if (left_top_neighbor)
        {
            left_top_neighbor->neighbors[NeighborRightTop] = upper_left_sub_tree;
            left_top_neighbor->neighbors[NeighborRightBottom] = upper_left_sub_tree;
            left_top_neighbor->neighbors[NeighborLowerRightCorner] = lower_left_sub_tree;
        }
    }

    //upper_left_sub_tree->startNodeServer();
    //upper_right_sub_tree->startNodeServer();
    //lower_right_sub_tree->startNodeServer();
    //lower_left_sub_tree->node_server_port_number = node_server_port_number;

    //for (auto& neighbor : neighbors)
    //{
    //    if (neighbor && neighbor->level + 1 < level)
    //    {
    //        neighbor->split();
    //    }
    //    neighbor = nullptr;
    //}
}

void BalanceTree::splitChildren()
{
    for (auto& child : children)
    {
        if (child)
        {
            child->split();
        }
    }
}

void BalanceTree::startChildrenNodeServer()
{
    if (leaf_node)
    {
        startNodeServer();
        return;
    }

    for (auto& child : children)
    {
        if (child)
        {
            child->startChildrenNodeServer();
        }
    }
}

void BalanceTree::merge()
{
    // TODO:
}
