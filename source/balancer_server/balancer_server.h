// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <list>
#include <map>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/core/noncopyable.hpp>
#include "protocol.h"
#include "transport.h"
#include "packet_pool.h"
#include "balance_tree.h"
#include "fast_index_map.h"
#include "mac_address.h"
#include "global_types.h"

struct NodeServerInfo
{
    typedef NodeServerInfo SelfType;
    typedef std::shared_ptr<SelfType> Ptr;

    Network::MacAddress mac_address;
    ip_address_t ip_address;
    unsigned short max_process_count = 1;
    bool power_on = false;

    std::unique_ptr<boost::asio::deadline_timer> timer;
};

struct NodeInfo
{
    Network::MacAddress mac_address;
    ip_address_t ip_address;
    std::uint16_t port_number = NODE_SERVER_PORT_NUMBER_BASE;
};

class BalancerServer : public Transport
{
    std::uint16_t port_number = 17013;
    boost::asio::signal_set signals;
    bool proxy_server_end_point_initialized = false;
    boost::asio::ip::udp::endpoint proxy_server_end_point;
    boost::filesystem::path node_server_path;

    SquareCell global_bounding_box;
    BalanceTree* balance_tree = nullptr;

    typedef Memory::FastIndexMap<BalanceTree> uuid_to_tree_t;
    uuid_to_tree_t uuid_to_tree;

    typedef std::map<boost::asio::ip::udp::endpoint, BalanceTree*> end_point_to_tree_t;
    end_point_to_tree_t end_point_to_tree;

    typedef std::map<ip_address_t::bytes_type, NodeServerInfo::Ptr> available_node_servers_t;
    available_node_servers_t available_node_servers;
    std::list<NodeInfo> available_nodes;

public:
    BalancerServer();
    void start();

    BalanceTree* createNewBalanceNode(const SquareCell& bounding_box, BalanceTree* parent);
    NodeInfo getAvailableNode();
    void startNode(NodeInfo& node_info, BalanceTree* balance_tree);
    void wakeUp(NodeServerInfo::Ptr node_server_info);
    void onWakeupTimeout(NodeServerInfo::Ptr node_server_info, const boost::system::error_code& error);

private:
    bool onInitializePositionInternal(size_t received_bytes);
    bool onInitializePositionInternalAnswer(size_t received_bytes);
    bool onGetNodeInfo(size_t received_bytes);
    bool onMonitoringBalancerServerInfo(size_t received_bytes);
    bool onMonitoringBalanceTreeInfo(size_t received_bytes);
    bool onMonitoringBalanceTreeNeighborInfo(size_t received_bytes);
    bool onMonitoringBalanceTreeStaticSplit(size_t received_bytes);
    bool onMonitoringBalanceTreeStaticMerge(size_t received_bytes);
    void initAvailableNodes();
};
