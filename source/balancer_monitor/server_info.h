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
#include "global_types.h"
#include "balance_tree/common.h"
#include "fast_index_map.h"
#include "protocol.h"

struct BalancerTreeInfo
{
    std::uint32_t token = 0;
    std::size_t level = 0;
    SquareCell bounding_box;
    bool leaf_node = true;
    std::array<std::uint32_t, CountOfChildren> children;
    std::array<std::uint32_t, 4 * (NEIGHBOR_COUNT_AT_SIDE + 1)> neighbors;
    std::uint32_t user_count = 0;
    ip_address_t node_server_address;
    std::uint16_t node_server_port_number = 0;

    // (X, Y) -> neighbor token. (X,Y) is cell address of external cell
    std::map<std::pair<std::int32_t, std::int32_t>, std::uint32_t> neighbor_nodes;

    std::uint8_t current_child_index_to_send = 0;
};

struct ServerInfo
{
    typedef std::shared_ptr<ServerInfo> Ptr;

    // Balancer Server section
    SquareCell bounding_box;
    std::uint32_t tree_root_token;
    Memory::FastIndexMap<BalancerTreeInfo> token_to_tree_node;
    BalancerTreeInfo* selected_node = nullptr;
    std::uint32_t selected_proxy_index = 0;

    // Proxy Server section
    struct ProxyInfo
    {
        std::string ip_address;
        std::uint16_t port_number = 0;
        QPlainTextEdit* log = nullptr;
        QDockWidget* log_dock = nullptr;
    };
    std::unordered_map<std::uint32_t, ProxyInfo> id_to_proxy;

    // Temporary fields for receiving packets
    std::uint32_t wait_token = 0;
    std::list<std::uint32_t> parent_stack;

    struct NeighborRequest
    {
        unsigned token;
        int x, y;
    };
    std::list<NeighborRequest> neighbor_requests;
    std::list<std::uint32_t> proxy_info_requests;
};

struct ServerTreeExpandStatus
{
    typedef std::shared_ptr<ServerTreeExpandStatus> Ptr;

    std::unordered_map<std::uint32_t, bool> tree_expand_status;
    bool is_selected_token_valid = false;
    std::uint32_t selected_token = 0;
};

static inline QColor getColor(std::uint32_t token)
{
    QByteArray token_data;
    token_data.append(QString("%1").arg(token));
    QString hash = QString(QCryptographicHash::hash(token_data, QCryptographicHash::Md5).toHex());
    std::uint8_t color[6] = { 0 };
    for (int i = 0; i < hash.size(); ++i)
    {
        char symbol = hash[i].toUpper().toLatin1();
        char code = 0;
        if (symbol >= 'A')
        {
            code = symbol - 'A' + 10;
        }
        else
        {
            code = symbol - '0';
        }
        color[i % 6] ^= code;
    }
    std::uint8_t red = (color[1] << 4) + color[0];
    std::uint8_t green = (color[3] << 4) + color[2];
    std::uint8_t blue = (color[5] << 4) + color[4];
    return QColor(red, green, blue);
}

static inline bool isAnyNodeServerRunning(const ServerInfo::Ptr& server_info, std::uint32_t tree_node_token)
{
    if (server_info)
    {
        BalancerTreeInfo* tree_node = server_info->token_to_tree_node.find(tree_node_token);
        if (tree_node)
        {
            if (tree_node->leaf_node)
            {
                return tree_node->node_server_port_number != 0;
            }
            else
            {
                for (std::uint8_t i = ChildFirst; i < ChildLast; ++i)
                {
                    if (tree_node->children[i] && isAnyNodeServerRunning(server_info, tree_node->children[i]))
                    {
                        return true;
                    }
                }
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}
