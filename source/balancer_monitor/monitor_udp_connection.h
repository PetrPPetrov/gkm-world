// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <QObject>
#include <QUdpSocket>
#include <QHostInfo>
#include <QThread>
#include "protocol.h"

class MainMonitorWindow;

class MonitorUDPConnection : public QObject
{
    Q_OBJECT

public:
    MonitorUDPConnection(const QString& host_name, unsigned short port_number, MainMonitorWindow* main_window);

signals:
    void close();
    void getBalancerServerInfo();
    void getBalanceTreeInfo(unsigned tree_node_token);

private slots:
    void onClose();
    void onGetBalancerServerInfo();
    void onGetBalanceTreeInfo(unsigned tree_node_token);

    void onThreadStart();
    void onResolve(QHostInfo);
    void onReadyRead();

private:
    MainMonitorWindow* main_window = nullptr;
    QThread connection_thread;
    QUdpSocket* socket = nullptr;
    QString balancer_server_host_name;
    QHostAddress balancer_server_host_address;
    unsigned short balancer_server_port_number;
};