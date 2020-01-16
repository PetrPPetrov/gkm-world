// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <QObject>
#include <QUdpSocket>
#include <QHostInfo>
#include <QThread>
#include <QTimer>
#include "protocol.h"

class MainMonitorWindow;

class MonitorUDPConnection : public QObject
{
    Q_OBJECT

public:
    MonitorUDPConnection(const QString& host_name, std::uint16_t port_number, MainMonitorWindow* main_window);

signals:
    void close();
    void getBalancerServerInfo();
    void getBalanceTreeInfo(unsigned tree_node_token);
    void getBalanceTreeNeighborInfo(unsigned tree_node_token, int x, int y);
    void staticSplit(unsigned tree_node_token);
    void staticMerge(unsigned tree_node_token);

private slots:
    void onClose();
    void onGetBalancerServerInfo();
    void onGetBalanceTreeInfo(unsigned tree_node_token);
    void onGetBalanceTreeNeighborInfo(unsigned tree_node_token, int x, int y);
    void onStaticSplit(unsigned tree_node_token);
    void onStaticMerge(unsigned tree_node_token);

    void onThreadStart();
    void onResolve(QHostInfo);
    void onReadyRead();
    void onGetServerMessageTimer();
    void onMonitoringMessageCountAnswer(QByteArray data);
    void onMonitoringPopMessage();
    void onMonitoringPopMessageAnswer(QByteArray data);

private:
    MainMonitorWindow* main_window = nullptr;
    QThread connection_thread;
    QUdpSocket* socket = nullptr;
    QString balancer_server_host_name;
    QHostAddress balancer_server_host_address;
    std::uint16_t balancer_server_port_number;
    QTimer* get_server_message_timer = nullptr;
    std::uint32_t message_count = 0;
};
