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
    connect(&connection_thread, SIGNAL(started()), this, SLOT(onThreadStart()));
    connection_thread.start();
}

void MonitorUDPConnection::onClose()
{
    connection_thread.exit(0);
}

void MonitorUDPConnection::onGetBalancerServerInfo()
{
    Packet::MonitoringBalancerServerInfo request;
    socket->writeDatagram(reinterpret_cast<const char*>(&request), sizeof(request), balancer_server_host_address, balancer_server_port_number);
}

void MonitorUDPConnection::onThreadStart()
{
    main_window->message(tr("Connecting to balancer server- ") + balancer_server_host_name + ":" + std::to_string(balancer_server_port_number).c_str());
    QHostInfo::lookupHost(balancer_server_host_name, this, SLOT(onResolve(QHostInfo)));
    socket = new QUdpSocket(this);
    connect(socket, &QUdpSocket::readyRead, this, &MonitorUDPConnection::onReadyRead);
}

void MonitorUDPConnection::onResolve(QHostInfo host_info)
{
    if (host_info.error() != QHostInfo::NoError)
    {
        main_window->connectionFatal("DNS resolution failed");
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
        main_window->connectionFatal("DNS resolution failed");
    }
    else
    {
        main_window->message(tr("Resoved IP address- ") + balancer_server_host_address.toString());
    }
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
        switch (received_base_packet->type)
        {
        case Packet::EType::MonitoringBalancerServerInfoAnswer:
            if (buffer.size() >= sizeof(Packet::MonitoringBalancerServerInfoAnswer))
            {
                main_window->monitoringBalancerServerInfoAnswer(*reinterpret_cast<const Packet::MonitoringBalancerServerInfoAnswer*>(received_base_packet));
            }
            break;
        default:
            break;
        }
    }
}
