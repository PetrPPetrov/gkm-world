// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include "monitor_udp_connection.h"

MonitorUDPConnection::MonitorUDPConnection(const QString& ip_address, unsigned short port_number, QObject* parent)
    : QObject(parent), balancer_server_ip_address(ip_address), balancer_server_port_number(port_number)
{
    socket = new QUdpSocket(this);
    //socket->bind(QHostAddress(ip_address), port_number);
    connect(socket, &QUdpSocket::readyRead, this, &MonitorUDPConnection::readyRead);
}

void MonitorUDPConnection::getBalancerServerInfo()
{
    Packet::MonitoringBalancerServerInfo request;
    socket->writeDatagram(reinterpret_cast<const char*>(&request), sizeof(request), balancer_server_ip_address, balancer_server_port_number);
}

void MonitorUDPConnection::readyRead()
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
                emit onMonitoringBalancerServerInfoAnswer(*reinterpret_cast<const Packet::MonitoringBalancerServerInfoAnswer*>(received_base_packet));
            }
            break;
        default:
            break;
        }
    }
}
