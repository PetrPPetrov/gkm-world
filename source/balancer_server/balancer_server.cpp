// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <cassert>
#include <iostream>
#include <fstream>
#include <memory>
#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include "protocol.h"
#include "log.h"
#include "config.h"
#include "process_spawn.h"
#include "balancer_server.h"

BalancerServer::BalancerServer(const std::string& cfg_file_name_) :
    cfg_file_name(cfg_file_name_), signals(io_service, SIGINT, SIGTERM)
{
    setServerType(Packet::EServerType::BalancerServer);

    std::int32_t global_bounding_box_start_x = 0;
    std::int32_t global_bounding_box_start_y = 0;
    std::int32_t global_bounding_box_size = MAXIMAL_NODE_SIZE;
    std::list<Network::MacAddress> node_server_mac_address;
    std::list<std::string> node_server_ip_address;
    std::list<unsigned short> node_server_max_process_count;

    std::ifstream config_file(cfg_file_name);
    ConfigurationReader config_reader;
    config_reader.addParameter("balancer_server_port_number", port_number);
    config_reader.addParameter("global_bounding_box_start_x", global_bounding_box_start_x);
    config_reader.addParameter("global_bounding_box_start_y", global_bounding_box_start_y);
    config_reader.addParameter("global_bounding_box_size", global_bounding_box_size);
    config_reader.addParameter("log_min_severity", minimum_level);
    config_reader.addParameter("log_to_screen", log_to_screen);
    config_reader.addParameter("log_to_file", log_to_file);
    config_reader.addParameter("node_server_mac_address", node_server_mac_address);
    config_reader.addParameter("node_server_ip_address", node_server_ip_address);
    config_reader.addParameter("node_server_max_process_count", node_server_max_process_count);
    config_reader.read(config_file);

    auto ip_address_it = node_server_ip_address.begin();
    auto max_process_count_it = node_server_max_process_count.begin();

    for (auto mac_address_it = node_server_mac_address.begin(); mac_address_it != node_server_mac_address.end(); ++mac_address_it)
    {
        NodeServerInfo::Ptr new_node_server_info = std::make_shared<NodeServerInfo>();
        new_node_server_info->mac_address = *mac_address_it;
        new_node_server_info->ip_address = IpAddress::from_string(*ip_address_it++);
        if (new_node_server_info->ip_address.is_loopback())
        {
            // Local node server is always power on
            new_node_server_info->power_on = true;
        }
        new_node_server_info->max_process_count = *max_process_count_it++;
        available_node_servers.insert(AvailableNodeServers::value_type(new_node_server_info->ip_address.to_bytes(), new_node_server_info));
    }

    balancer_server_end_point = boost::asio::ip::udp::endpoint(IpAddress(), port_number);
    socket = boost::asio::ip::udp::socket(io_service, balancer_server_end_point);

    global_bounding_box.start.x = global_bounding_box_start_x;
    global_bounding_box.start.y = global_bounding_box_start_y;
    global_bounding_box.size = global_bounding_box_size;

    if (MINIMAL_NODE_SIZE > MAXIMAL_NODE_SIZE)
    {
        throw std::runtime_error("MINIMAL_NODE_SIZE > MAXIMAL_NODE_SIZE");
    }
    if (global_bounding_box_size > MAXIMAL_NODE_SIZE)
    {
        throw std::runtime_error("global_bounding_box_size > MAXIMAL_NODE_SIZE");
    }
    while (global_bounding_box_size > MINIMAL_NODE_SIZE)
    {
        if (global_bounding_box_size % 2)
        {
            throw std::runtime_error("global_bounding_box_size must be even, not odd!");
        }
        global_bounding_box_size /= 2;
    }

    node_server_path = findNodeServerPath();

    initAvailableNodes();

    uuid_to_tree.allocate(uuid_to_tree.allocateIndex()); // Allocate BalanceTree with 0 token
    balance_tree = createNewBalanceNode(global_bounding_box, nullptr);
    balance_tree->staticSplit(1);
    //balance_tree->startNodeServers();
}

extern Log::Logger* g_logger = nullptr;

bool BalancerServer::start()
{
    Log::Holder log_holder;
    g_logger = new Log::Logger(minimum_level, "balancer_server_" + std::to_string(port_number) + ".log", log_to_screen, log_to_file);
    LOG_INFO << "Balancer Server is starting...";

    try
    {
        dumpParameters();
        startImpl();
    }
    catch (boost::system::system_error & error)
    {
        LOG_FATAL << "boost::system::system_error: " << error.what();
        return false;
    }
    catch (const std::exception & exception)
    {
        LOG_FATAL << "std::exception: " << exception.what();
        return false;
    }
    catch (...)
    {
        LOG_FATAL << "Unknown error";
        return false;
    }
    return true;
}

BalanceTree* BalancerServer::createNewBalanceNode(const SquareCell& bounding_box, BalanceTree* parent)
{
    std::uint32_t new_node_token = uuid_to_tree.allocateIndex();
    BalanceTree* new_node = uuid_to_tree.allocate(new_node_token);
    return new(new_node) BalanceTree(*this, new_node_token, bounding_box, parent);
}

void BalancerServer::destroyBalanceNode(BalanceTree* node)
{
    std::uint32_t node_token = node->getToken();
    node->~BalanceTree();
    uuid_to_tree.deallocate(node_token);
    uuid_to_tree.deallocateIndex(node_token);
}

NodeInfo BalancerServer::getAvailableNode()
{
    if (available_nodes.empty())
    {
        LOG_FATAL << "no more nodes are available";
        throw std::runtime_error("no more nodes are available");
    }

    NodeInfo node_info = available_nodes.front();
    available_nodes.pop_front();
    return node_info;
}

void BalancerServer::startNode(NodeInfo& node_info, BalanceTree* balance_tree)
{
    boost::asio::ip::udp::endpoint end_point(node_info.ip_address, node_info.port_number);

    auto it = end_point_to_tree.find(end_point);
    if (it == end_point_to_tree.end())
    {
        end_point_to_tree.insert(EndPointToTree::value_type(end_point, balance_tree));
    }
    else
    {
        assert(false);
        LOG_FATAL << "node server is already running";
        throw std::runtime_error("node server is already running");
    }

    if (node_info.ip_address.is_loopback())
    {
        processSpawn(node_server_path.generic_string(), std::to_string(node_info.port_number));
        return;
    }

    auto found_node_server_info_it = available_node_servers.find(node_info.ip_address.to_bytes());
    if (found_node_server_info_it != available_node_servers.end())
    {
        if (!found_node_server_info_it->second->power_on)
        {
            wakeUp(found_node_server_info_it->second);
        }
        else
        {
            auto request = createPacket<Packet::SpawnNodeServer>();
            request->node_server_port = node_info.port_number;
            // TODO: Select randomly port number from the running node servers pool of the specified node server host
            // In the current case we overload the first node server (which is running on port number NODE_SERVER_PORT_NUMBER_BASE)
            standardSendTo(request, boost::asio::ip::udp::endpoint(node_info.ip_address, NODE_SERVER_PORT_NUMBER_BASE));
        }
    }
    else
    {
        assert(false);
        LOG_FATAL << "can not find node server information";
        throw std::runtime_error("can not find node server information");
    }
}

void BalancerServer::wakeUp(NodeServerInfo::Ptr node_server_info)
{
    if (!node_server_info->power_on)
    {
        LOG_INFO << "waking-up " << to_string(node_server_info->mac_address);
        auto request = createPacket<Packet::WakeOnLan>();
        request->setMacAddress(node_server_info->mac_address);
        socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        socket.set_option(boost::asio::socket_base::broadcast(true));
        standardSendTo(request, boost::asio::ip::udp::endpoint(node_server_info->ip_address, 9));
        socket.set_option(boost::asio::ip::udp::socket::reuse_address(false));
        socket.set_option(boost::asio::socket_base::broadcast(false));

        node_server_info->timer = std::make_unique<boost::asio::deadline_timer>(io_service, boost::posix_time::seconds(60));
        node_server_info->timer->async_wait(boost::bind(&BalancerServer::onWakeupTimeout, this, node_server_info, _1));
    }
    else
    {
        node_server_info->timer->cancel();
        node_server_info->timer.reset(nullptr);
    }
}

void BalancerServer::onWakeupTimeout(NodeServerInfo::Ptr node_server_info, const boost::system::error_code& error)
{
    // TODO: Introduce some time-out
    // TODO: If server does not respond when exclude this server from available servers list
    if (!node_server_info->power_on)
    {
        wakeUp(node_server_info);
    }
    else
    {
        node_server_info->timer->cancel();
        node_server_info->timer.reset(nullptr);
    }
}

void BalancerServer::dumpParameters()
{
    LOG_INFO << "configuration_file_name " << cfg_file_name;
    LOG_INFO << "balancer_server_ip " << balancer_server_end_point.address();
    LOG_INFO << "balancer_server_port_number " << balancer_server_end_point.port();

    LOG_INFO << "global_bounding_box_start_x " << global_bounding_box.start.x;
    LOG_INFO << "global_bounding_box_start_y " << global_bounding_box.start.y;
    LOG_INFO << "global_bounding_box_size " << global_bounding_box.size;

    LOG_INFO << "log_min_severity " << getText(minimum_level);
    LOG_INFO << "log_to_screen " << log_to_screen;
    LOG_INFO << "log_to_file " << log_to_file;

    for (auto node_server_info : available_node_servers)
    {
        LOG_INFO << "node_server_mac_address " << to_string(node_server_info.second->mac_address);
        LOG_INFO << "node_server_ip_address " << node_server_info.second->ip_address;
        LOG_INFO << "node_server_max_process_count " << node_server_info.second->max_process_count;
    }

    LOG_INFO << "node_server_path " << node_server_path;
}

void BalancerServer::startImpl()
{
    doReceive();
    signals.async_wait(Log::onQuit);
    setReceiveHandler(Packet::EType::InitializePositionInternal, boost::bind(&BalancerServer::onInitializePositionInternal, this, _1));
    setReceiveHandler(Packet::EType::GetNodeInfo, boost::bind(&BalancerServer::onGetNodeInfo, this, _1));
    setReceiveHandler(Packet::EType::RegisterProxy, boost::bind(&BalancerServer::onRegisterProxy, this, _1));
    setReceiveHandler(Packet::EType::MonitoringBalancerServerInfo, boost::bind(&BalancerServer::onMonitoringBalancerServerInfo, this, _1));
    setReceiveHandler(Packet::EType::MonitoringBalanceTreeInfo, boost::bind(&BalancerServer::onMonitoringBalanceTreeInfo, this, _1));
    setReceiveHandler(Packet::EType::MonitoringBalanceTreeNeighborInfo, boost::bind(&BalancerServer::onMonitoringBalanceTreeNeighborInfo, this, _1));
    setReceiveHandler(Packet::EType::MonitoringBalanceTreeStaticSplit, boost::bind(&BalancerServer::onMonitoringBalanceTreeStaticSplit, this, _1));
    setReceiveHandler(Packet::EType::MonitoringBalanceTreeStaticMerge, boost::bind(&BalancerServer::onMonitoringBalanceTreeStaticMerge, this, _1));
    setReceiveHandler(Packet::EType::MonitoringGetProxyCount, boost::bind(&BalancerServer::onMonitoringGetProxyCount, this, _1));
    setReceiveHandler(Packet::EType::MonitoringGetProxyInfo, boost::bind(&BalancerServer::onMonitoringGetProxyInfo, this, _1));
#ifdef NETWORK_LOG
    setReceiveHandler(Packet::EType::MonitoringMessageCount, boost::bind(&Transport::onMonitoringMessageCount, this, _1));
    setReceiveHandler(Packet::EType::MonitoringPopMessage, boost::bind(&Transport::onMonitoringPopMessage, this, _1));
#endif
    io_service.run();
}

bool BalancerServer::onInitializePositionInternal(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onInitializePositionInternal";
#endif

    const auto packet = getReceiveBufferAs<Packet::InitializePositionInternal>();
#ifdef _DEBUG
    LOG_DEBUG << "user token " << packet->user_token;
#endif
    if (!balance_tree->registerNewUser(*packet))
    {
        // Send negative result, possitive result will be sent by a node server
        auto answer = createPacket<Packet::InitializePositionInternalAnswer>(packet->packet_number);
        answer->user_token = packet->user_token;
        answer->client_packet_number = packet->client_packet_number;
        answer->proxy_packet_number = packet->proxy_packet_number;
        answer->corrected_location = packet->user_location;
        answer->success = false;
        standardSend(answer);
    }

    return true;
}

bool BalancerServer::onGetNodeInfo(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onGetNodeInfo";
    LOG_DEBUG << "node_server_end_point " << remote_end_point;
#endif

    const auto packet = getReceiveBufferAs<Packet::GetNodeInfo>();
    auto answer = createPacket<Packet::GetNodeInfoAnswer>(packet->packet_number);
    auto node_it = end_point_to_tree.find(remote_end_point);
    if (node_it != end_point_to_tree.end())
    {
        auto found_node_info_it = available_node_servers.find(remote_end_point.address().TO_V().to_bytes());
        if (found_node_info_it != available_node_servers.end())
        {
            if (!found_node_info_it->second->power_on)
            {
                found_node_info_it->second->power_on = true;
                // Add other node info for this node server
                for (std::uint16_t port_number = NODE_SERVER_PORT_NUMBER_BASE + 1; port_number < NODE_SERVER_PORT_NUMBER_BASE + found_node_info_it->second->max_process_count; ++port_number)
                {
                    NodeInfo new_node_info;
                    new_node_info.ip_address = found_node_info_it->second->ip_address;
                    new_node_info.port_number = port_number;
                    new_node_info.mac_address = found_node_info_it->second->mac_address;
                    available_nodes.push_front(new_node_info);
                }
            }
        }

        node_it->second->getInfo(answer);
    }
    else
    {
        answer->success = false;
    }

    standardSend(answer);
    return true;
}

bool BalancerServer::onRegisterProxy(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onRegisterProxy";
#endif

    const auto packet = getReceiveBufferAs<Packet::RegisterProxy>();
    auto answer = createPacket<Packet::RegisterProxyAnswer>(packet->packet_number);

    auto find_it = id_to_proxy.find(packet->proxy_index);
    if (find_it == id_to_proxy.end())
    {
        std::uint32_t new_proxy_index = static_cast<std::uint32_t>(id_to_proxy.size() + 1);
        answer->proxy_index = new_proxy_index;
        id_to_proxy.emplace(new_proxy_index, remote_end_point);
    }
    else
    {
        id_to_proxy[packet->proxy_index] = remote_end_point;
        answer->proxy_index = packet->proxy_index;
    }

    standardSend(answer);
    return true;
}

bool BalancerServer::onMonitoringBalancerServerInfo(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onMonitoringBalancerServerInfo";
#endif

    const auto packet = getReceiveBufferAs<Packet::MonitoringBalancerServerInfo>();
    auto answer = createPacket<Packet::MonitoringBalancerServerInfoAnswer>(packet->packet_number);
    answer->global_bounding_box = global_bounding_box;
    answer->tree_root_token = balance_tree ? balance_tree->getToken() : 0;
    standardSend(answer);
    return true;
}

bool BalancerServer::onMonitoringBalanceTreeInfo(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onMonitoringBalanceTreeInfo";
#endif

    const auto packet = getReceiveBufferAs<Packet::MonitoringBalanceTreeInfo>();
    auto answer = createPacket<Packet::MonitoringBalanceTreeInfoAnswer>(packet->packet_number);
    BalanceTree* found_tree = uuid_to_tree.find(packet->tree_node_token);
    if (found_tree)
    {
        found_tree->getMonitoringInfo(answer);
    }
    else
    {
        answer->success = false;
    }
    standardSend(answer);
    return true;
}

bool BalancerServer::onMonitoringBalanceTreeNeighborInfo(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onMonitoringBalanceTreeNeighborInfo";
#endif

    const auto packet = getReceiveBufferAs<Packet::MonitoringBalanceTreeNeighborInfo>();
    auto answer = createPacket<Packet::MonitoringBalanceTreeNeighborInfoAnswer>(packet->packet_number);
    BalanceTree* found_tree = uuid_to_tree.find(packet->tree_node_token);
    if (found_tree)
    {
        found_tree->getMonitoringNeighborInfo(answer, packet->neighbor_cell);
    }
    else
    {
        answer->success = false;
    }
    standardSend(answer);
    return true;
}

bool BalancerServer::onMonitoringBalanceTreeStaticSplit(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onMonitoringBalanceTreeStaticSplit";
#endif

    const auto packet = getReceiveBufferAs<Packet::MonitoringBalanceTreeStaticSplit>();
    auto answer = createPacket<Packet::MonitoringBalanceTreeStaticSplitAnswer>(packet->packet_number);

    if (balance_tree->isAnyNodeServerRunning())
    {
        answer->node_server_running = true;
        answer->success = false;
    }
    else
    {
        BalanceTree* found_tree = uuid_to_tree.find(packet->tree_node_token);
        if (found_tree)
        {
            found_tree->monitoringBalanceTreeStaticSplit(answer);
        }
        else
        {
            answer->success = false;
        }
    }
    standardSend(answer);
    return true;
}

bool BalancerServer::onMonitoringBalanceTreeStaticMerge(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onMonitoringBalanceTreeStaticMerge";
#endif

    const auto packet = getReceiveBufferAs<Packet::MonitoringBalanceTreeStaticMerge>();
    auto answer = createPacket<Packet::MonitoringBalanceTreeStaticMergeAnswer>(packet->packet_number);

    if (balance_tree->isAnyNodeServerRunning())
    {
        answer->success = false;
    }
    else
    {
        BalanceTree* found_tree = uuid_to_tree.find(packet->tree_node_token);
        if (found_tree)
        {
            found_tree->monitoringBalanceTreeStaticMerge(answer);
        }
        else
        {
            answer->success = false;
        }
    }
    standardSend(answer);
    return true;
}

bool BalancerServer::onMonitoringGetProxyCount(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onMonitoringGetProxyCount";
#endif

    const auto packet = getReceiveBufferAs<Packet::MonitoringGetProxyCount>();
    auto answer = createPacket<Packet::MonitoringGetProxyCountAnswer>(packet->packet_number);
    answer->proxy_count = static_cast<std::uint32_t>(id_to_proxy.size());
    standardSend(answer);
    return true;
}

bool BalancerServer::onMonitoringGetProxyInfo(size_t received_bytes)
{
#ifdef _DEBUG
    LOG_DEBUG << "onMonitoringGetProxyInfo";
#endif

    const auto packet = getReceiveBufferAs<Packet::MonitoringGetProxyInfo>();
    auto answer = createPacket<Packet::MonitoringGetProxyInfoAnswer>(packet->packet_number);
    answer->success = false;
    answer->proxy_index = packet->proxy_index;

    auto find_it = id_to_proxy.find(packet->proxy_index);
    if (find_it != id_to_proxy.end())
    {
        answer->proxy_server_address = find_it->second.address().TO_V().to_bytes();
        answer->proxy_server_port_number = find_it->second.port();
        answer->success = true;
    }

    standardSend(answer);
    return true;
}

void BalancerServer::initAvailableNodes()
{
    available_nodes.clear();
    for (auto node_server_info : available_node_servers)
    {
        if (node_server_info.second->ip_address.is_loopback())
        {
            for (std::uint16_t port_number = NODE_SERVER_PORT_NUMBER_BASE; port_number < NODE_SERVER_PORT_NUMBER_BASE + node_server_info.second->max_process_count; ++port_number)
            {
                NodeInfo new_node_info;
                new_node_info.ip_address = node_server_info.second->ip_address;
                new_node_info.port_number = port_number;
                new_node_info.mac_address = node_server_info.second->mac_address;
                available_nodes.push_front(new_node_info);
            }
        }
        else
        {
            // Standard case
            NodeInfo new_node_info;
            new_node_info.ip_address = node_server_info.second->ip_address;
            new_node_info.port_number = NODE_SERVER_PORT_NUMBER_BASE;
            new_node_info.mac_address = node_server_info.second->mac_address;
            available_nodes.push_back(new_node_info);
        }
    }
}
