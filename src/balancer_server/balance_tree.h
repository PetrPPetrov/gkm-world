// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include "gkm_world/gkm_world.h"
#include "gkm_world/transport.h"
#include "gkm_world/coordinate_2d.h"
#include "gkm_world/balance_tree/common.h"

class BalancerServer;

class BalanceTree {
    BalancerServer& balancer_server;
    IndexType token = 0;
    std::size_t level = 0;
    Square2D bounding_box;
    BalanceTree* parent = nullptr;
    bool leaf_node = true;
    std::array<BalanceTree*, CountOfChildren> children;
    std::array<BalanceTree*, 4 * (NEIGHBOR_COUNT_AT_SIDE + 1)> neighbors;
    IndexType unit_count = 0;
    PortNumberType node_server_port_number = 0;
    boost::asio::ip::udp::endpoint node_server_end_point;

public:
    BalanceTree(const BalanceTree&) = delete;
    BalanceTree& operator=(const BalanceTree&) = delete;
    BalanceTree(BalancerServer& balancer_server, IndexType token, Square2D bounding_box);
    BalanceTree(BalancerServer& balancer_server, IndexType token, Square2D bounding_box, BalanceTree* parent);

    IndexType getToken() const;
    bool isLeafNode() const;
    bool isAnyNodeServerRunning() const;
    void dump() const;

    bool registerNewUser(const Packet::InitializePositionInternal& packet);
    void getInfo(Packet::GetNodeInfoAnswer* answer) const;
    void getMonitoringInfo(Packet::MonitoringBalanceTreeInfoAnswer* answer) const;
    void getMonitoringNeighborInfo(Packet::MonitoringBalanceTreeNeighborInfoAnswer* answer, Coordinate2D neighbor_cell) const;
    void monitoringBalanceTreeStaticSplit(Packet::MonitoringBalanceTreeStaticSplitAnswer* answer);
    void monitoringBalanceTreeStaticMerge(Packet::MonitoringBalanceTreeStaticMergeAnswer* answer);

    void startNodeServer();
    void startNodeServers();

    bool staticSplit();
    bool staticSplit(std::size_t required_level);
    bool staticMerge();

private:
    void destroyChildren();
    static inline IndexType getNeighborIndex(const Square2D& box, Coordinate2D neighbor);
    inline void setNeighbor(Coordinate2D neighbor_cell, BalanceTree* neighbor);
    inline BalanceTree* getNeighbor(Coordinate2D neighbor_cell) const;
    inline void specifyNeighbor(Coordinate2D neighbor_cell, BalanceTree* neighbor);
    inline BalanceTree* obtainNeighbor(Coordinate2D neighbor_cell) const;

    inline Coordinate2D getBottomLeftNeighborCell() const;
    inline Coordinate2D getTopLeftNeighborCell() const;
    inline Coordinate2D getTopRightNeighborCell() const;
    inline Coordinate2D getBottomRightNeighborCell() const;

    inline Coordinate2D getBottomNeighborCell(CoordinateType index) const;
    inline Coordinate2D getTopNeighborCell(CoordinateType index) const;
    inline Coordinate2D getLeftNeighborCell(CoordinateType index) const;
    inline Coordinate2D getRightNeighborCell(CoordinateType index) const;

    inline Coordinate2D getBottomLeftCell() const;
    inline Coordinate2D getTopLeftCell() const;
    inline Coordinate2D getTopRightCell() const;
    inline Coordinate2D getBottomRightCell() const;

    inline Coordinate2D getBottomCell(CoordinateType index) const;
    inline Coordinate2D getTopCell(CoordinateType index) const;
    inline Coordinate2D getLeftCell(CoordinateType index) const;
    inline Coordinate2D getRightCell(CoordinateType index) const;

    inline void setBottomLeftMe(BalanceTree* node);
    inline void setTopLeftMe(BalanceTree* node);
    inline void setTopRightMe(BalanceTree* node);
    inline void setBottomRightMe(BalanceTree* node);

    inline void setBottomMe(CoordinateType index, BalanceTree* node);
    inline void setTopMe(CoordinateType index, BalanceTree* node);
    inline void setLeftMe(CoordinateType index, BalanceTree* node);
    inline void setRightMe(CoordinateType index, BalanceTree* node);
};
