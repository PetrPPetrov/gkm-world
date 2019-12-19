// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include "monitor_udp_connection.h"
#include "main_monitor_window.h"

MonitorUDPConnection::MonitorUDPConnection(const QString& host_name, unsigned short port_number, MainMonitorWindow* main_window_)
    : QObject(nullptr), main_window(main_window_), balancer_server_host_name(host_name), balancer_server_port_number(port_number)
{
    moveToThread(&connection_thread);
    connect(this, SIGNAL(close()), this, SLOT(onClose()));
    connect(this, SIGNAL(getBalancerServerInfo()), this, SLOT(onGetBalancerServerInfo()));
    connect(this, SIGNAL(getBalanceTreeInfo(unsigned)), this, SLOT(onGetBalanceTreeInfo(unsigned)));
    connect(&connection_thread, SIGNAL(started()), this, SLOT(onThreadStart()));
    connection_thread.start();
}

void MonitorUDPConnection::onClose()
{
    main_window->message(tr("disconnected"));
    main_window->message(tr("================"));
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

void MonitorUDPConnection::onThreadStart()
{
    main_window->message(tr("================"));
    main_window->message(tr("connecting to balancer server..."));
    main_window->message(tr("host name - '%1'").arg(balancer_server_host_name));
    main_window->message(tr("port number - %1").arg(balancer_server_port_number));
    QHostInfo::lookupHost(balancer_server_host_name, this, SLOT(onResolve(QHostInfo)));
    socket = new QUdpSocket(this);
    connect(socket, &QUdpSocket::readyRead, this, &MonitorUDPConnection::onReadyRead);
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
