// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <memory>
#include <set>
#include <map>
#include <unordered_map>
#include <vector>
#include <list>
#include <algorithm>
#include <QColor>
#include <QCryptographicHash>
#include <QDockWidget>
#include <QPlainTextEdit>
#include "gkm_world/gkm_world.h"
#include "gkm_world/fast_index.h"
#include "gkm_world/protocol.h"
#include "gkm_world/coordinate_2d.h"
#include "gkm_world/balance_tree/common.h"

struct BalancerTreeInfo {
    IndexType token = 0;
    std::size_t level = 0;
    Square2D bounding_box;
    bool leaf_node = true;
    std::array<IndexType, CountOfChildren> children;
    std::array<IndexType, 4 * (NEIGHBOR_COUNT_AT_SIDE + 1)> neighbors;
    IndexType user_count = 0;
    IpAddress node_server_address;
    PortNumberType node_server_port_number = 0;

    // (X, Y) -> neighbor token. (X,Y) is cell address of external cell
    std::map<std::pair<CoordinateType, CoordinateType>, IndexType> neighbor_nodes;

    std::uint8_t current_child_index_to_send = 0;
};

struct ServerInfo {
    typedef std::shared_ptr<ServerInfo> Ptr;

    // Balancer Server section
    Square2D bounding_box;
    IndexType tree_root_token;
    Memory::FastIndexMap<BalancerTreeInfo, 8> token_to_tree_node;
    BalancerTreeInfo* selected_node = nullptr;
    IndexType selected_proxy_index = 0;

    // Proxy Server section
    struct ProxyInfo {
        std::string ip_address;
        PortNumberType port_number = 0;
        QPlainTextEdit* log = nullptr;
        QDockWidget* log_dock = nullptr;
    };
    std::unordered_map<IndexType, ProxyInfo> id_to_proxy;

    // Temporary fields for receiving packets
    IndexType wait_token = 0;
    std::list<IndexType> parent_stack;

    struct NeighborRequest {
        unsigned token;
        int x, y;
    };
    std::list<NeighborRequest> neighbor_requests;
    std::list<IndexType> proxy_info_requests;
};

struct ServerTreeExpandStatus {
    typedef std::shared_ptr<ServerTreeExpandStatus> Ptr;

    std::unordered_map<IndexType, bool> tree_expand_status;
    bool is_selected_token_valid = false;
    IndexType selected_token = 0;
};

static inline QColor getColor(IndexType token) {
    QByteArray token_data;
    token_data.append(QString("%1").arg(token).toLatin1());
    QString hash = QString(QCryptographicHash::hash(token_data, QCryptographicHash::Md5).toHex());
    std::uint8_t color[6] = { 0 };
    for (int i = 0; i < hash.size(); ++i) {
        char symbol = hash[i].toUpper().toLatin1();
        char code = 0;
        if (symbol >= 'A') {
            code = symbol - 'A' + 10;
        } else {
            code = symbol - '0';
        }
        color[i % 6] ^= code;
    }
    std::uint8_t red = (color[1] << 4) + color[0];
    std::uint8_t green = (color[3] << 4) + color[2];
    std::uint8_t blue = (color[5] << 4) + color[4];
    return QColor(red, green, blue);
}

static inline bool isAnyNodeServerRunning(const ServerInfo::Ptr& server_info, IndexType tree_node_token) {
    if (server_info) {
        BalancerTreeInfo* tree_node = server_info->token_to_tree_node.find(tree_node_token);
        if (tree_node) {
            if (tree_node->leaf_node) {
                return tree_node->node_server_port_number != 0;
            } else {
                for (std::uint8_t i = ChildFirst; i < ChildLast; ++i) {
                    if (tree_node->children[i] && isAnyNodeServerRunning(server_info, tree_node->children[i])) {
                        return true;
                    }
                }
                return false;
            }
        } else {
            return false;
        }
    } else {
        return false;
    }
}
