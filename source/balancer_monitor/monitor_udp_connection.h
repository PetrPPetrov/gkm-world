// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <QObject>
#include <QUdpSocket>
#include "protocol.h"

class MonitorUDPConnection : public QObject
{
    Q_OBJECT

public:
    explicit MonitorUDPConnection(const QString& ip_address, unsigned short port_number, QObject* parent = 0);

signals:
    void onMonitoringBalancerServerInfoAnswer(Packet::MonitoringBalancerServerInfoAnswer packet);

public slots:
    void getBalancerServerInfo();
    void readyRead();

private:
    QUdpSocket* socket;
    QHostAddress balancer_server_ip_address;
    unsigned short balancer_server_port_number;
};
