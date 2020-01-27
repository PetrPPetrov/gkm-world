// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <list>
#include <set>
#include <map>
#include <unordered_map>
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
    const std::string cfg_file_name;
    std::uint16_t port_number = 17013;
    boost::asio::signal_set signals;
    boost::asio::ip::udp::endpoint balancer_server_end_point;
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

    typedef std::unordered_map<std::uint32_t, boost::asio::ip::udp::endpoint> id_to_proxy_t;
    id_to_proxy_t id_to_proxy;

    Packet::ESeverityType minimum_level = Packet::ESeverityType::DebugMessage;
    bool log_to_screen = false;
    bool log_to_file = true;

public:
    BalancerServer(const std::string& cfg_file_name);
    bool start();

    BalanceTree* createNewBalanceNode(const SquareCell& bounding_box, BalanceTree* parent);
    void destroyBalanceNode(BalanceTree* node);
    NodeInfo getAvailableNode();
    void startNode(NodeInfo& node_info, BalanceTree* balance_tree);
    void wakeUp(NodeServerInfo::Ptr node_server_info);
    void onWakeupTimeout(NodeServerInfo::Ptr node_server_info, const boost::system::error_code& error);

private:
    void dumpParameters();
    void startImpl();
    bool onInitializePositionInternal(size_t received_bytes);
    bool onGetNodeInfo(size_t received_bytes);
    bool onRegisterProxy(size_t received_bytes);
    bool onMonitoringBalancerServerInfo(size_t received_bytes);
    bool onMonitoringBalanceTreeInfo(size_t received_bytes);
    bool onMonitoringBalanceTreeNeighborInfo(size_t received_bytes);
    bool onMonitoringBalanceTreeStaticSplit(size_t received_bytes);
    bool onMonitoringBalanceTreeStaticMerge(size_t received_bytes);
    bool onMonitoringGetProxyCount(size_t received_bytes);
    bool onMonitoringGetProxyInfo(size_t received_bytes);
    void initAvailableNodes();
};
