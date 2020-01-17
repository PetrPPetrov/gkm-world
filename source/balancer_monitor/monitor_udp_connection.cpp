// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
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
    connect(&connection_thread, SIGNAL(started()), this, SLOT(onThreadStart()));
    connection_thread.start();
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
    Packet::MonitoringBalancerServerInfo request;
    socket->writeDatagram(reinterpret_cast<const char*>(&request), sizeof(request), balancer_server_host_address, balancer_server_port_number);
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
    get_server_message_timer->start(1000);
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
            case Packet::EType::MonitoringMessageCountAnswer:
                onMonitoringMessageCountAnswer(buffer);
                break;
            case Packet::EType::MonitoringPopMessageAnswer:
                onMonitoringPopMessageAnswer(buffer);
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
    Packet::MonitoringMessageCount request;
    socket->writeDatagram(reinterpret_cast<const char*>(&request), sizeof(request), balancer_server_host_address, balancer_server_port_number);
}

void MonitorUDPConnection::onMonitoringMessageCountAnswer(QByteArray data)
{
    const Packet::MonitoringMessageCountAnswer* answer = reinterpret_cast<const Packet::MonitoringMessageCountAnswer*>(data.data());
    if (data.size() >= Packet::getSize(answer))
    {
        message_count = answer->message_count;
        if (message_count > 0)
        {
            onMonitoringPopMessage();
        }
    }
}

void MonitorUDPConnection::onMonitoringPopMessage()
{
    Packet::MonitoringPopMessage request;
    socket->writeDatagram(reinterpret_cast<const char*>(&request), sizeof(request), balancer_server_host_address, balancer_server_port_number);
}

void MonitorUDPConnection::onMonitoringPopMessageAnswer(QByteArray data)
{
    const Packet::MonitoringPopMessageAnswer* answer = reinterpret_cast<const Packet::MonitoringPopMessageAnswer*>(data.data());
    if (data.size() >= Packet::getSize(answer))
    {
        if (answer->success)
        {
            if (message_count > 0)
            {
                --message_count;
            }
            std::string message_text = getText(answer->server_type) + " ";
            if (answer->server_type == Packet::EServerType::NodeServer)
            {
                message_text += std::string("[Token=") + std::to_string(answer->token) + "] ";
            }
            message_text += std::string("[") + getText(answer->severity_type) + "] ";
            message_text += answer->getMessage();
            g_main_window->serverMessage(message_text.c_str());
            if (message_count > 0)
            {
                onMonitoringPopMessage();
            }
        }
    }
}
