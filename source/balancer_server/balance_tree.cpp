// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <cassert>
#include <boost/asio.hpp>
#include "balance_tree.h"
#include "balancer_server.h"
#include "log.h"

BalanceTree::BalanceTree(BalancerServer& balancer_server_, std::uint32_t token_, SquareCell bounding_box_) :
    BalanceTree(balancer_server_, token_, bounding_box_, nullptr)
{
}

BalanceTree::BalanceTree(BalancerServer& balancer_server_, std::uint32_t token_, SquareCell bounding_box_, BalanceTree* parent_) :
    balancer_server(balancer_server_)
{
    token = token_;
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

bool BalanceTree::isLeafNode() const
{
    return leaf_node;
}

bool BalanceTree::isNodeServerRunning() const
{
    if (leaf_node)
    {
        return node_server_port_number;
    }

    for (auto& child : children)
    {
        if (child)
        {
            if (child->isNodeServerRunning())
            {
                return true;
            }
        }
    }

    return false;
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

bool BalanceTree::registerNewUser(const Packet::InitializePositionInternal& packet)
{
    if (!inside(bounding_box, packet.user_location.x_pos, packet.user_location.y_pos))
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
        user_count++;
        return true;
    }
    for (auto child : children)
    {
        if (child && child->registerNewUser(packet))
            return true;
    }
    return false;
}

void BalanceTree::getInfo(Packet::GetNodeInfoAnswer* answer) const
{
    assert(answer);
    // TODO:
}

void BalanceTree::getMonitoringInfo(Packet::MonitoringBalanceTreeInfoAnswer* answer) const
{
    answer->tree_node_token = token;
    answer->level = level;
    answer->bounding_box = bounding_box;
    answer->leaf_node = leaf_node;
    for (std::uint8_t i = ChildFirst; i < ChildLast; ++i)
    {
        answer->children[i] = children[i] ? children[i]->getToken() : 0;
    }
    answer->user_count = user_count;
    answer->node_server_port_number = node_server_port_number;
    answer->success = true;
}

void BalanceTree::getMonitoringNeighborInfo(Packet::MonitoringBalanceTreeNeighborInfoAnswer* answer, CellIndex neighbor_cell) const
{
    answer->tree_node_token = token;
    answer->neighbor_cell = neighbor_cell;
    BalanceTree* neighbor_tree = getNeighbor(neighbor_cell);
    answer->neighbor_node_token = neighbor_tree ? neighbor_tree->getToken() : 0;
    answer->success = true;
}

void BalanceTree::monitoringBalanceTreeStaticSplit(Packet::MonitoringBalanceTreeStaticSplitAnswer* answer)
{
    answer->tree_node_token = token;
    if (node_server_port_number)
    {
        answer->node_server_running = true;
        answer->success = false;
        return;
    }
    if (!leaf_node)
    {
        answer->not_leaf_node = true;
        answer->success = false;
        return;
    }
    answer->success = staticSplit();
}

void BalanceTree::monitoringBalanceTreeStaticMerge(Packet::MonitoringBalanceTreeStaticMergeAnswer* answer)
{
    answer->tree_node_token = token;
    answer->success = true;
}

void BalanceTree::startNodeServer()
{
    if (!leaf_node)
    {
        return;
    }

    if (node_server_port_number == 0)
    {
        NodeInfo node_info = balancer_server.getAvailableNode();
        node_server_end_point = boost::asio::ip::udp::endpoint(node_info.ip_address, node_info.port_number);
        node_server_port_number = node_info.port_number;

#ifdef _DEBUG
        LOG_DEBUG << "starting node_server " << node_info.ip_address << ":" << node_info.port_number << " " << token << std::endl;
#endif

        balancer_server.startNode(node_info, this);
    }
    else
    {
        // TODO: Report a warning that node server is already started
    }
}

void BalanceTree::startNodeServers()
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
            child->startNodeServers();
        }
    }
}

bool BalanceTree::staticSplit()
{
    if (!leaf_node)
    {
        return false;
    }

    if (node_server_port_number)
    {
        // Static split can not be applied, use dynamic split instead
        return false;
    }

    if (bounding_box.size % 2)
    {
        // Error: odd size of node area
        assert(false);
        return false;
    }

    if (bounding_box.size / 2 < MINIMAL_NODE_SIZE)
    {
        // Warning: minimum  size of node has been reached
        return false;
    }

    SquareCell lower_left_box = bounding_box;
    lower_left_box.size /= 2;

    SquareCell upper_left_box = lower_left_box;
    upper_left_box.start.y += lower_left_box.size;

    SquareCell upper_right_box = lower_left_box;
    upper_right_box.start.x += lower_left_box.size;
    upper_right_box.start.y += lower_left_box.size;

    SquareCell lower_right_box = lower_left_box;
    lower_right_box.start.x += lower_left_box.size;

    leaf_node = false;

    BalanceTree* lower_left_sub_tree = balancer_server.createNewBalanceNode(lower_left_box, this);
    BalanceTree* upper_left_sub_tree = balancer_server.createNewBalanceNode(upper_left_box, this);
    BalanceTree* upper_right_sub_tree = balancer_server.createNewBalanceNode(upper_right_box, this);
    BalanceTree* lower_right_sub_tree = balancer_server.createNewBalanceNode(lower_right_box, this);

    children[ChildLowerLeft] = lower_left_sub_tree;
    children[ChildUpperLeft] = upper_left_sub_tree;
    children[ChildUpperRight] = upper_right_sub_tree;
    children[ChildLowerRight] = lower_right_sub_tree;

    {
        CellIndex lower_left_index = lower_left_sub_tree->getBottomLeftNeighborCell();
        lower_left_sub_tree->setNeighbor(lower_left_index, getNeighbor(lower_left_index));
        CellIndex upper_left_index = lower_left_sub_tree->getTopLeftNeighborCell();
        lower_left_sub_tree->setNeighbor(upper_left_index, getNeighbor(upper_left_index));
        lower_left_sub_tree->setNeighbor(lower_left_sub_tree->getTopRightNeighborCell(), upper_right_sub_tree);
        CellIndex lower_right_index = lower_left_sub_tree->getBottomRightNeighborCell();
        lower_left_sub_tree->setNeighbor(lower_right_index, getNeighbor(lower_right_index));
        for (std::int32_t i = 0; i < lower_left_box.size; ++i)
        {
            CellIndex bottom_index = lower_left_sub_tree->getBottomNeighborCell(i);
            lower_left_sub_tree->setNeighbor(bottom_index, getNeighbor(bottom_index));
            CellIndex left_index = lower_left_sub_tree->getLeftNeighborCell(i);
            lower_left_sub_tree->setNeighbor(left_index, getNeighbor(left_index));
            lower_left_sub_tree->setNeighbor(lower_left_sub_tree->getTopNeighborCell(i), upper_left_sub_tree);
            lower_left_sub_tree->setNeighbor(lower_left_sub_tree->getRightNeighborCell(i), lower_right_sub_tree);
        }
    }

    CellIndex lower_left_neg = lower_left_box.start;
    lower_left_neg.x--;
    lower_left_neg.y--;
    lower_left_sub_tree->setNeighbor(lower_left_neg, getNeighbor(lower_left_neg));
    for (std::int32_t i = 0; i < lower_left_box.size + 1; ++i)
    {
        CellIndex cur_x_cell = lower_left_neg;
        cur_x_cell.x += (i + 1);
        lower_left_sub_tree->setNeighbor(cur_x_cell, getNeighbor(cur_x_cell));
        CellIndex cur_y_cell = lower_left_neg;
        cur_y_cell.y += (i + 1);
        lower_left_sub_tree->setNeighbor(cur_y_cell, getNeighbor(cur_y_cell));
    }
    for (std::int32_t i = 0; i < lower_left_box.size; ++i)
    {
        CellIndex cur_x_cell = upper_left_box.start;
        cur_x_cell.x += i;
        lower_left_sub_tree->setNeighbor(cur_x_cell, upper_left_sub_tree);
        CellIndex cur_y_cell = lower_right_box.start;
        cur_y_cell.y += i;
        lower_left_sub_tree->setNeighbor(cur_y_cell, lower_right_sub_tree);
    }
    lower_left_sub_tree->setNeighbor(upper_right_box.start, upper_right_sub_tree);

    CellIndex upper_left_neg = upper_left_box.start;
    upper_left_neg.x--;
    upper_left_neg.y--;
    upper_left_sub_tree->setNeighbor(upper_left_neg, getNeighbor(upper_left_neg));
    for (std::int32_t i = 0; i < upper_left_box.size + 1; ++i)
    {
        CellIndex cur_x_cell = upper_left_neg;
        cur_x_cell.x += (i + 1);
        cur_x_cell.y += (upper_left_box.size + 1);
        upper_left_sub_tree->setNeighbor(cur_x_cell, getNeighbor(cur_x_cell));
        CellIndex cur_y_cell = upper_left_neg;
        cur_y_cell.y += (i + 1);
        upper_left_sub_tree->setNeighbor(cur_y_cell, getNeighbor(cur_y_cell));
    }
    for (std::int32_t i = 0; i < upper_left_box.size; ++i)
    {
        CellIndex cur_x_cell = upper_left_neg;
        cur_x_cell.x += (i + 1);
        upper_left_sub_tree->setNeighbor(cur_x_cell, lower_left_sub_tree);
        CellIndex cur_y_cell = upper_right_box.start;
        cur_y_cell.y += i;
        upper_left_sub_tree->setNeighbor(cur_y_cell, upper_right_sub_tree);
    }
    CellIndex upper_left_lr = upper_right_box.start;
    upper_left_lr.y--;
    upper_left_sub_tree->setNeighbor(upper_left_lr, lower_right_sub_tree);

    CellIndex upper_right_neg = upper_right_box.start;
    upper_right_neg.x--;
    upper_right_neg.y--;
    upper_right_sub_tree->setNeighbor(upper_right_neg, lower_left_sub_tree);
    for (std::int32_t i = 0; i < upper_right_box.size; ++i)
    {
        CellIndex cur_x_cell = upper_right_neg;
        cur_x_cell.x += (i + 1);
        upper_right_sub_tree->setNeighbor(cur_x_cell, lower_right_sub_tree);
        CellIndex cur_y_cell = upper_right_neg;
        cur_y_cell.y += (i + 1);
        upper_right_sub_tree->setNeighbor(cur_y_cell, upper_left_sub_tree);
    }
    for (std::int32_t i = 0; i < upper_right_box.size + 1; ++i)
    {
        CellIndex cur_x_cell = upper_right_neg;
        cur_x_cell.x += i;
        cur_x_cell.y += (upper_right_box.size + 1);
        upper_right_sub_tree->setNeighbor(cur_x_cell, getNeighbor(cur_x_cell));
        CellIndex cur_y_cell = upper_right_neg;
        cur_y_cell.x += (upper_right_box.size + 1);
        cur_y_cell.y += i;
        upper_right_sub_tree->setNeighbor(cur_y_cell, getNeighbor(cur_y_cell));
    }
    CellIndex upper_right_ur = upper_right_box.start;
    upper_right_ur.x += upper_right_box.size;
    upper_right_ur.y += upper_right_box.size;
    upper_right_sub_tree->setNeighbor(upper_right_ur, getNeighbor(upper_right_ur));

    CellIndex lower_right_neg = lower_right_box.start;
    lower_right_neg.x--;
    lower_right_neg.y--;
    lower_right_sub_tree->setNeighbor(lower_right_neg, getNeighbor(lower_right_neg));
    for (std::int32_t i = 0; i < lower_right_box.size + 1; ++i)
    {
        CellIndex cur_x_cell = lower_right_neg;
        cur_x_cell.x += (i + 1);
        lower_right_sub_tree->setNeighbor(cur_x_cell, getNeighbor(cur_x_cell));
        CellIndex cur_y_cell = lower_right_neg;
        cur_y_cell.x += (lower_right_box.size + 1);
        cur_y_cell.y += (i + 1);
        lower_right_sub_tree->setNeighbor(cur_y_cell, getNeighbor(cur_y_cell));
    }
    for (std::int32_t i = 0; i < lower_right_box.size; ++i)
    {
        CellIndex cur_x_cell = lower_right_box.start;
        cur_x_cell.x += i;
        cur_x_cell.y += lower_right_box.size;
        lower_right_sub_tree->setNeighbor(cur_x_cell, upper_right_sub_tree);
        CellIndex cur_y_cell = lower_right_neg;
        cur_y_cell.y += (i + 1);
        lower_right_sub_tree->setNeighbor(cur_y_cell, lower_left_sub_tree);
    }
    CellIndex lower_right_ul = lower_right_neg;
    lower_right_ul.y += (lower_right_box.size + 1);
    lower_right_sub_tree->setNeighbor(lower_right_ul, upper_left_sub_tree);

    for (std::int32_t i = 0; i < bounding_box.size; ++i)
    {
        CellIndex left_cell = bounding_box.start;
        left_cell.y += i;
        CellIndex left_cell_neighbor = left_cell;
        left_cell_neighbor.x--;
        BalanceTree* left_neighbor = getNeighbor(left_cell_neighbor);
        if (left_neighbor)
        {
            left_neighbor->setNeighbor(left_cell, i < lower_left_box.size ? lower_left_sub_tree : upper_left_sub_tree);
        }

        CellIndex top_cell_neighbor = bounding_box.start;
        top_cell_neighbor.x += i;
        top_cell_neighbor.y += bounding_box.size;
        CellIndex top_cell = top_cell_neighbor;
        top_cell.y--;
        BalanceTree* top_neighbor = getNeighbor(top_cell_neighbor);
        if (top_neighbor)
        {
            top_neighbor->setNeighbor(top_cell, i < upper_left_box.size ? upper_left_sub_tree : upper_right_sub_tree);
        }

        CellIndex right_cell_neighbor = bounding_box.start;
        right_cell_neighbor.x += bounding_box.size;
        right_cell_neighbor.y += i;
        CellIndex right_cell = right_cell_neighbor;
        right_cell.x--;
        BalanceTree* right_neighbor = getNeighbor(right_cell_neighbor);
        if (right_neighbor)
        {
            right_neighbor->setNeighbor(right_cell, i < lower_right_box.size ? lower_right_sub_tree : upper_right_sub_tree);
        }

        CellIndex bottom_cell = bounding_box.start;
        bottom_cell.x += i;
        CellIndex bottom_cell_neighbor = bottom_cell;
        bottom_cell_neighbor.y--;
        BalanceTree* bottom_neighbor = getNeighbor(bottom_cell_neighbor);
        if (bottom_neighbor)
        {
            bottom_neighbor->setNeighbor(bottom_cell, i < lower_left_box.size ? lower_left_sub_tree : lower_right_sub_tree);
        }
    }

    CellIndex bottom_left_cell_neighbor = bounding_box.start;
    bottom_left_cell_neighbor.x--;
    bottom_left_cell_neighbor.y--;
    CellIndex bottom_left_cell = bounding_box.start;
    BalanceTree* bottom_left_neighbor = getNeighbor(bottom_left_cell_neighbor);
    if (bottom_left_neighbor)
    {
        bottom_left_neighbor->setNeighbor(bottom_left_cell, lower_left_sub_tree);
    }

    CellIndex top_left_cell_neighbor = bounding_box.start;
    top_left_cell_neighbor.x--;
    top_left_cell_neighbor.y += bounding_box.size;
    CellIndex top_left_cell = bounding_box.start;
    top_left_cell.y += (bounding_box.size - 1);
    BalanceTree* top_left_neighbor = getNeighbor(top_left_cell_neighbor);
    if (top_left_neighbor)
    {
        top_left_neighbor->setNeighbor(top_left_cell, upper_left_sub_tree);
    }

    CellIndex top_right_cell_neighbor = bounding_box.start;
    top_right_cell_neighbor.x += bounding_box.size;
    top_right_cell_neighbor.y += bounding_box.size;
    CellIndex top_right_cell = top_right_cell_neighbor;
    top_right_cell.x--;
    top_right_cell.y--;
    BalanceTree* top_right_neighbor = getNeighbor(top_right_cell_neighbor);
    if (top_right_neighbor)
    {
        top_right_neighbor->setNeighbor(top_right_cell, upper_right_sub_tree);
    }

    CellIndex bottom_right_cell_neighbor = bounding_box.start;
    bottom_right_cell_neighbor.x += bounding_box.size;
    bottom_right_cell_neighbor.y--;
    CellIndex bottom_right_cell = bounding_box.start;
    bottom_right_cell.x += (bounding_box.size - 1);
    BalanceTree* bottom_right_neighbor = getNeighbor(bottom_right_cell_neighbor);
    if (bottom_right_neighbor)
    {
        bottom_right_neighbor->setNeighbor(bottom_right_cell, lower_right_sub_tree);
    }

    return true;
}

void BalanceTree::staticSplit(std::size_t required_level)
{
    if (level >= required_level)
    {
        return;
    }

    if (leaf_node)
    {
        staticSplit();
    }

    for (auto& child : children)
    {
        if (child)
        {
            child->staticSplit(required_level);
        }
    }
}

bool BalanceTree::staticMerge()
{
    if (leaf_node)
    {
        return false;
    }

    if (isNodeServerRunning())
    {
        // Static merge can not be applied, use dynamic merge instead
        return false;
    }

    destroyChildren();

    for (std::uint8_t i = ChildFirst; i < ChildLast; ++i)
    {
        children[i] = nullptr;
    }
    leaf_node = true;

    CellIndex bottom_left_cell_neighbor = bounding_box.start;
    bottom_left_cell_neighbor.x--;
    bottom_left_cell_neighbor.y--;
    CellIndex bottom_left_cell = bounding_box.start;
    BalanceTree* bottom_left_neighbor = getNeighbor(bottom_left_cell_neighbor);
    if (bottom_left_neighbor)
    {
        bottom_left_neighbor->setNeighbor(bottom_left_cell, this);
    }

    CellIndex top_left_cell_neighbor = bounding_box.start;
    top_left_cell_neighbor.x--;
    top_left_cell_neighbor.y += bounding_box.size;
    CellIndex top_left_cell = bounding_box.start;
    top_left_cell.y += (bounding_box.size - 1);
    BalanceTree* top_left_neighbor = getNeighbor(top_left_cell_neighbor);
    if (top_left_neighbor)
    {
        top_left_neighbor->setNeighbor(top_left_cell, this);
    }

    CellIndex top_right_cell_neighbor = bounding_box.start;
    top_right_cell_neighbor.x += bounding_box.size;
    top_right_cell_neighbor.y += bounding_box.size;
    CellIndex top_right_cell = top_right_cell_neighbor;
    top_right_cell.x--;
    top_right_cell.y--;
    BalanceTree* top_right_neighbor = getNeighbor(top_right_cell_neighbor);
    if (top_right_neighbor)
    {
        top_right_neighbor->setNeighbor(top_right_cell, this);
    }

    CellIndex bottom_right_cell_neighbor = bounding_box.start;
    bottom_right_cell_neighbor.x += bounding_box.size;
    bottom_right_cell_neighbor.y--;
    CellIndex bottom_right_cell = bounding_box.start;
    bottom_right_cell.x += (bounding_box.size - 1);
    BalanceTree* bottom_right_neighbor = getNeighbor(bottom_right_cell_neighbor);
    if (bottom_right_neighbor)
    {
        bottom_right_neighbor->setNeighbor(bottom_right_cell, this);
    }

}

void BalanceTree::destroyChildren()
{
    for (auto& child : children)
    {
        if (child)
        {
            if (child->leaf_node)
            {
                balancer_server.destroyBalanceNode(child);
            }
            else
            {
                child->destroyChildren();
            }
        }
    }
}

inline std::uint32_t BalanceTree::getNeighborIndex(const SquareCell& box, CellIndex neighbor)
{
    if (neighbor.x < box.start.x && neighbor.y < box.start.y)
    {
        return 0;
    }
    if (neighbor.x < box.start.x)
    {
        if (neighbor.y < box.start.y + box.size)
        {
            return 1 + (neighbor.y - box.start.y) / MINIMAL_NODE_SIZE;
        }
        else
        {
            return 1 + NEIGHBOR_COUNT_AT_SIDE;
        }
    }
    if (neighbor.y >= box.start.y + box.size)
    {
        if (neighbor.x < box.start.x + box.size)
        {
            return 2 + NEIGHBOR_COUNT_AT_SIDE + (neighbor.x - box.start.x) / MINIMAL_NODE_SIZE;
        }
        else
        {
            return 2 * (NEIGHBOR_COUNT_AT_SIDE + 1);
        }
    }
    if (neighbor.x >= box.start.x + box.size)
    {
        if (neighbor.y >= box.start.y)
        {
            return 3 + 2 * NEIGHBOR_COUNT_AT_SIDE + (neighbor.y - box.start.y) / MINIMAL_NODE_SIZE;
        }
        else
        {
            return 3 * (NEIGHBOR_COUNT_AT_SIDE + 1);
        }
    }
    return 4 + 3 * NEIGHBOR_COUNT_AT_SIDE + (neighbor.x - box.start.x) / MINIMAL_NODE_SIZE;
}

inline void BalanceTree::setNeighbor(CellIndex neighbor_cell, BalanceTree* neighbor)
{
    assert(!inside(bounding_box, neighbor_cell));
    auto neighbor_index = getNeighborIndex(bounding_box, neighbor_cell);
    neighbors[neighbor_index] = neighbor;
}

inline BalanceTree* BalanceTree::getNeighbor(CellIndex neighbor_cell) const
{
    assert(!inside(bounding_box, neighbor_cell));
    auto neighbor_index = getNeighborIndex(bounding_box, neighbor_cell);
    return neighbors[neighbor_index];
}

inline CellIndex BalanceTree::getBottomLeftNeighborCell() const
{
    CellIndex bottom_left_cell_neighbor = bounding_box.start;
    bottom_left_cell_neighbor.x--;
    bottom_left_cell_neighbor.y--;
    return bottom_left_cell_neighbor;
}

inline CellIndex BalanceTree::getTopLeftNeighborCell() const
{
    CellIndex top_left_cell_neighbor = bounding_box.start;
    top_left_cell_neighbor.x--;
    top_left_cell_neighbor.y += bounding_box.size;
    return top_left_cell_neighbor;
}

inline CellIndex BalanceTree::getTopRightNeighborCell() const
{
    CellIndex top_right_cell_neighbor = bounding_box.start;
    top_right_cell_neighbor.x += bounding_box.size;
    top_right_cell_neighbor.y += bounding_box.size;
    return top_right_cell_neighbor;
}

inline CellIndex BalanceTree::getBottomRightNeighborCell() const
{
    CellIndex bottom_right_cell_neighbor = bounding_box.start;
    bottom_right_cell_neighbor.x += bounding_box.size;
    bottom_right_cell_neighbor.y--;
    return bottom_right_cell_neighbor;
}

inline CellIndex BalanceTree::getBottomNeighborCell(std::int32_t index) const
{
    CellIndex result = getBottomLeftNeighborCell();
    result.x += (index + 1);
    return result;
}

inline CellIndex BalanceTree::getTopNeighborCell(std::int32_t index) const
{
    CellIndex result = getTopLeftNeighborCell();
    result.x += (index + 1);
    return result;
}

inline CellIndex BalanceTree::getLeftNeighborCell(std::int32_t index) const
{
    CellIndex result = getBottomLeftNeighborCell();
    result.y += (index + 1);
    return result;
}

inline CellIndex BalanceTree::getRightNeighborCell(std::int32_t index) const
{
    CellIndex result = getBottomRightNeighborCell();
    result.y += (index + 1);
    return result;
}

inline CellIndex BalanceTree::getBottomLeftCell() const
{
    return bounding_box.start;
}

inline CellIndex BalanceTree::getTopLeftCell() const
{
    CellIndex top_left_cell = bounding_box.start;
    top_left_cell.y += (bounding_box.size - 1);
    return top_left_cell;
}

inline CellIndex BalanceTree::getTopRightCell() const
{
    CellIndex top_right_cell = bounding_box.start;
    top_right_cell.x += (bounding_box.size - 1);
    top_right_cell.y += (bounding_box.size - 1);
    return top_right_cell;
}

inline CellIndex BalanceTree::getBottomRightCell() const
{
    CellIndex bottom_right_cell = bounding_box.start;
    bottom_right_cell.x += (bounding_box.size -1);
    return bottom_right_cell;
}

inline CellIndex BalanceTree::getBottomCell(std::int32_t index) const
{
    CellIndex result = getBottomLeftCell();
    result.x += index;
    return result;
}

inline CellIndex BalanceTree::getTopCell(std::int32_t index) const
{
    CellIndex result = getTopLeftCell();
    result.x += index;
    return result;
}

inline CellIndex BalanceTree::getLeftCell(std::int32_t index) const
{
    CellIndex result = getBottomLeftCell();
    result.y += index;
    return result;
}

inline CellIndex BalanceTree::getRightCell(std::int32_t index) const
{
    CellIndex result = getBottomRightCell();
    result.y += index;
    return result;
}

inline void BalanceTree::setBottomLeftMe(BalanceTree* node)
{
    BalanceTree* neighbor = getNeighbor(getBottomLeftNeighborCell());
    if (neighbor)
    {
        neighbor->setNeighbor(getBottomLeftCell(), node);
    }
}

inline void BalanceTree::setTopLeftMe(BalanceTree* node)
{
    BalanceTree* neighbor = getNeighbor(getTopLeftNeighborCell());
    if (neighbor)
    {
        neighbor->setNeighbor(getTopLeftCell(), node);
    }
}

inline void BalanceTree::setTopRightMe(BalanceTree* node)
{
    BalanceTree* neighbor = getNeighbor(getTopRightNeighborCell());
    if (neighbor)
    {
        neighbor->setNeighbor(getTopRightCell(), node);
    }
}

inline void BalanceTree::setBottomRightMe(BalanceTree* node)
{
    BalanceTree* neighbor = getNeighbor(getBottomRightNeighborCell());
    if (neighbor)
    {
        neighbor->setNeighbor(getBottomRightCell(), node);
    }
}

inline void BalanceTree::setBottomMe(std::int32_t index, BalanceTree* node)
{
    BalanceTree* neighbor = getNeighbor(getBottomNeighborCell(index));
    if (neighbor)
    {
        neighbor->setNeighbor(getBottomCell(index), node);
    }
}

inline void BalanceTree::setTopMe(std::int32_t index, BalanceTree* node)
{
    BalanceTree* neighbor = getNeighbor(getTopNeighborCell(index));
    if (neighbor)
    {
        neighbor->setNeighbor(getTopCell(index), node);
    }
}

inline void BalanceTree::setLeftMe(std::int32_t index, BalanceTree* node)
{
    BalanceTree* neighbor = getNeighbor(getLeftNeighborCell(index));
    if (neighbor)
    {
        neighbor->setNeighbor(getLeftCell(index), node);
    }
}

inline void BalanceTree::setRightMe(std::int32_t index, BalanceTree* node)
{
    BalanceTree* neighbor = getNeighbor(getRightNeighborCell(index));
    if (neighbor)
    {
        neighbor->setNeighbor(getRightCell(index), node);
    }
}
