// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include "monitor_udp_connection.h"
#include "main_monitor_window.h"

MonitorUDPConnection::MonitorUDPConnection(const QString& host_name, std::uint16_t port_number, MainMonitorWindow* main_window_)
    : QObject(nullptr), main_window(main_window_), balancer_server_host_name(host_name), balancer_server_port_number(port_number)
{
    moveToThread(&connection_thread);
    connect(this, SIGNAL(close()), this, SLOT(onClose()));
    connect(this, SIGNAL(getBalancerServerInfo()), this, SLOT(onGetBalancerServerInfo()));
    connect(this, SIGNAL(getBalanceTreeInfo(unsigned)), this, SLOT(onGetBalanceTreeInfo(unsigned)));
    connect(this, SIGNAL(getBalanceTreeNeighborInfo(unsigned, int, int)), this, SLOT(onGetBalanceTreeNeighborInfo(unsigned, int, int)));
    connect(this, SIGNAL(staticSplit(unsigned)), this, SLOT(onStaticSplit(unsigned)));
    connect(this, SIGNAL(staticMerge(unsigned)), this, SLOT(onStaticMerge(unsigned)));
    connect(this, SIGNAL(getProxyInfo(unsigned)), this, SLOT(onGetProxyInfo(unsigned)));
    connect(this, SIGNAL(addProxyLog(unsigned, QString, unsigned)), this, SLOT(onAddProxyLog(unsigned, QString, unsigned)));
    connect(&connection_thread, SIGNAL(started()), this, SLOT(onThreadStart()));
    connection_thread.start();
}

QString MonitorUDPConnection::getBalancerIpAddress()
{
    return balancer_server_host_address.toString();
}

unsigned MonitorUDPConnection::getBalancerPortNumber()
{
    return balancer_server_port_number;
}

void MonitorUDPConnection::onClose()
{
    main_window->message(tr("disconnected"));
    main_window->message(tr("================"));
    if (get_server_message_timer)
    {
        get_server_message_timer->stop();
    }
    connection_thread.exit(0);
}

void MonitorUDPConnection::onGetBalancerServerInfo()
{
    Packet::MonitoringBalancerServerInfo request1;
    socket->writeDatagram(reinterpret_cast<const char*>(&request1), sizeof(request1), balancer_server_host_address, balancer_server_port_number);
    Packet::MonitoringGetProxyCount request2;
    socket->writeDatagram(reinterpret_cast<const char*>(&request2), sizeof(request2), balancer_server_host_address, balancer_server_port_number);
}

void MonitorUDPConnection::onGetBalanceTreeInfo(unsigned tree_node_token)
{
    Packet::MonitoringBalanceTreeInfo request;
    request.tree_node_token = tree_node_token;
    socket->writeDatagram(reinterpret_cast<const char*>(&request), sizeof(request), balancer_server_host_address, balancer_server_port_number);
}

void MonitorUDPConnection::onGetBalanceTreeNeighborInfo(unsigned tree_node_token, int x, int y)
{
    Packet::MonitoringBalanceTreeNeighborInfo request;
    request.tree_node_token = tree_node_token;
    request.neighbor_cell.x = x;
    request.neighbor_cell.y = y;
    socket->writeDatagram(reinterpret_cast<const char*>(&request), sizeof(request), balancer_server_host_address, balancer_server_port_number);
}

void MonitorUDPConnection::onStaticSplit(unsigned tree_node_token)
{
    Packet::MonitoringBalanceTreeStaticSplit request;
    request.tree_node_token = tree_node_token;
    socket->writeDatagram(reinterpret_cast<const char*>(&request), sizeof(request), balancer_server_host_address, balancer_server_port_number);
}

void MonitorUDPConnection::onStaticMerge(unsigned tree_node_token)
{
    Packet::MonitoringBalanceTreeStaticMerge request;
    request.tree_node_token = tree_node_token;
    socket->writeDatagram(reinterpret_cast<const char*>(&request), sizeof(request), balancer_server_host_address, balancer_server_port_number);
}

void MonitorUDPConnection::onGetProxyInfo(unsigned proxy_index)
{
    Packet::MonitoringGetProxyInfo request;
    request.proxy_index = proxy_index;
    socket->writeDatagram(reinterpret_cast<const char*>(&request), sizeof(request), balancer_server_host_address, balancer_server_port_number);
}

void MonitorUDPConnection::onAddProxyLog(unsigned proxy_index, QString ip_adddress, unsigned port_number)
{
    auto find_it = proxies_info.find(proxy_index);
    if (find_it == proxies_info.end())
    {
        auto proxy_server_info = std::make_shared<ServerAddressInfo>();
        proxy_server_info->server_type = Packet::EServerType::ProxyServer;
        proxy_server_info->server_token = static_cast<std::uint32_t>(proxy_index);
        proxy_server_info->ip_address.setAddress(ip_adddress);
        proxy_server_info->port_number = static_cast<std::uint16_t>(port_number);
        servers.push_back(proxy_server_info);
        proxies_info.emplace(proxy_index, proxy_server_info);
    }
    else
    {
        auto proxy_server_info = find_it->second;
        proxy_server_info->ip_address.setAddress(ip_adddress);
        proxy_server_info->port_number = static_cast<std::uint16_t>(port_number);
    }
}

void MonitorUDPConnection::onThreadStart()
{
    main_window->message(tr("================"));
    main_window->message(tr("connecting to balancer server..."));
    main_window->message(tr("host name - '%1'").arg(balancer_server_host_name));
    main_window->message(tr("port number - %1").arg(balancer_server_port_number));
    QHostInfo::lookupHost(balancer_server_host_name, this, SLOT(onResolve(QHostInfo)));
    socket = new QUdpSocket(this);
    connect(socket, &QUdpSocket::readyRead, this, &MonitorUDPConnection::onReadyRead);
    get_server_message_timer = new QTimer(this);
    connect(get_server_message_timer, &QTimer::timeout, this, &MonitorUDPConnection::onGetServerMessageTimer);
}

void MonitorUDPConnection::onResolve(QHostInfo host_info)
{
    if (host_info.error() != QHostInfo::NoError)
    {
        main_window->connectionFatal(tr("DNS resolution failed."));
        return;
    }
    bool found = false;
    for (auto& host_address : host_info.addresses())
    {
        balancer_server_host_address = host_address;
        found = true;
        auto balancer_server_info = std::make_shared<ServerAddressInfo>();
        balancer_server_info->server_type = Packet::EServerType::BalancerServer;
        balancer_server_info->ip_address = balancer_server_host_address;
        balancer_server_info->port_number = balancer_server_port_number;
        servers.push_back(balancer_server_info);
        break;
    }
    if (!found)
    {
        main_window->connectionFatal(tr("DNS resolution failed."));
    }
    else
    {
        main_window->message(tr("resoved IP address - %1").arg(balancer_server_host_address.toString()));
    }
    onGetBalancerServerInfo();
    get_server_message_timer->start(100);
}

void MonitorUDPConnection::onReadyRead()
{
    QByteArray buffer;
    buffer.resize(socket->pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;

    socket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

    if (buffer.size() >= sizeof(Packet::Base))
    {
        auto received_base_packet = reinterpret_cast<const Packet::Base*>(buffer.data());
        if (buffer.size() >= Packet::getSize(received_base_packet->type))
        {
            switch (received_base_packet->type)
            {
            case Packet::EType::MonitoringBalancerServerInfoAnswer:
                main_window->monitoringBalancerServerInfoAnswer(buffer);
                break;
            case Packet::EType::MonitoringBalanceTreeInfoAnswer:
                main_window->monitoringBalanceTreeInfoAnswer(buffer);
                break;
            case Packet::EType::MonitoringBalanceTreeNeighborInfoAnswer:
                main_window->monitoringBalanceTreeNeighborInfoAnswer(buffer);
                break;
            case Packet::EType::MonitoringBalanceTreeStaticSplitAnswer:
                main_window->monitoringBalanceTreeStaticSplitAnswer(buffer);
                break;
            case Packet::EType::MonitoringBalanceTreeStaticMergeAnswer:
                main_window->monitoringBalanceTreeStaticMergeAnswer(buffer);
                break;
            case Packet::EType::MonitoringGetProxyCountAnswer:
                main_window->monitoringGetProxyCountAnswer(buffer);
                break;
            case Packet::EType::MonitoringGetProxyInfoAnswer:
                main_window->monitoringGetProxyInfoAnswer(buffer);
                break;
            case Packet::EType::MonitoringMessageCountAnswer:
                onMonitoringMessageCountAnswer(buffer, sender, senderPort);
                break;
            case Packet::EType::MonitoringPopMessageAnswer:
                onMonitoringPopMessageAnswer(buffer, sender, senderPort);
                break;
            default:
                break;
            }
        }
        else
        {
            main_window->message(tr("invalid data size: %1").arg(buffer.size()));
        }
    }
}

void MonitorUDPConnection::onGetServerMessageTimer()
{
    if (cur_server_to_ask_log != servers.end())
    {
        auto& cur_server_info = *cur_server_to_ask_log;
        Packet::MonitoringMessageCount request;
        socket->writeDatagram(reinterpret_cast<const char*>(&request), sizeof(request), cur_server_info->ip_address, cur_server_info->port_number);
        ++cur_server_to_ask_log;
    }
    else
    {
        cur_server_to_ask_log = servers.begin();
    }
}

void MonitorUDPConnection::onMonitoringMessageCountAnswer(QByteArray data, QHostAddress ip_address, quint16 port_number)
{
    const Packet::MonitoringMessageCountAnswer* answer = reinterpret_cast<const Packet::MonitoringMessageCountAnswer*>(data.data());
    if (data.size() >= Packet::getSize(answer))
    {
        if (answer->message_count > 0)
        {
            onMonitoringPopMessage(ip_address, port_number);
        }
    }
}

void MonitorUDPConnection::onMonitoringPopMessage(QHostAddress ip_address, quint16 port_number)
{
    Packet::MonitoringPopMessage request;
    socket->writeDatagram(reinterpret_cast<const char*>(&request), sizeof(request), ip_address, port_number);
}

void MonitorUDPConnection::onMonitoringPopMessageAnswer(QByteArray data, QHostAddress ip_address, quint16 port_number)
{
    const Packet::MonitoringPopMessageAnswer* answer = reinterpret_cast<const Packet::MonitoringPopMessageAnswer*>(data.data());
    if (data.size() >= Packet::getSize(answer))
    {
        if (answer->success)
        {
            std::string message_text = std::string("[") + getText(answer->severity_type) + "] ";
            message_text += answer->getMessage();
            switch (answer->server_type)
            {
            case Packet::EServerType::BalancerServer:
                g_main_window->balancerServerMessage(message_text.c_str());
                break;
            case Packet::EServerType::ProxyServer:
                g_main_window->proxyServerMessage(answer->token, message_text.c_str());
            default:
                break;
            }
            if (answer->message_count > 0)
            {
                onMonitoringPopMessage(ip_address, port_number);
            }
        }
    }
}
