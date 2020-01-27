// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <map>
#include <unordered_map>
#include <list>
#include <memory>
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

    QString getBalancerIpAddress();
    unsigned getBalancerPortNumber();

signals:
    void close();
    void getBalancerServerInfo();
    void getBalanceTreeInfo(unsigned tree_node_token);
    void getBalanceTreeNeighborInfo(unsigned tree_node_token, int x, int y);
    void staticSplit(unsigned tree_node_token);
    void staticMerge(unsigned tree_node_token);
    void getProxyInfo(unsigned proxy_index);
    void addProxyLog(unsigned proxy_index, QString ip_adddress, unsigned port_number);

private slots:
    void onClose();
    void onGetBalancerServerInfo();
    void onGetBalanceTreeInfo(unsigned tree_node_token);
    void onGetBalanceTreeNeighborInfo(unsigned tree_node_token, int x, int y);
    void onStaticSplit(unsigned tree_node_token);
    void onStaticMerge(unsigned tree_node_token);
    void onGetProxyInfo(unsigned proxy_index);
    void onAddProxyLog(unsigned proxy_index, QString ip_adddress, unsigned port_number);

    void onThreadStart();
    void onResolve(QHostInfo);
    void onReadyRead();
    void onGetServerMessageTimer();
    void onMonitoringMessageCountAnswer(QByteArray data, QHostAddress ip_address, quint16 port_number);
    void onMonitoringPopMessage(QHostAddress ip_address, quint16 port_number);
    void onMonitoringPopMessageAnswer(QByteArray data, QHostAddress ip_address, quint16 port_number);

private:
    MainMonitorWindow* main_window = nullptr;
    QThread connection_thread;
    QUdpSocket* socket = nullptr;
    QString balancer_server_host_name;
    QHostAddress balancer_server_host_address;
    std::uint16_t balancer_server_port_number;
    QTimer* get_server_message_timer = nullptr;

    struct ServerAddressInfo
    {
        typedef std::shared_ptr<ServerAddressInfo> Ptr;
        Packet::EServerType server_type;
        std::uint32_t server_token = 0;
        QHostAddress ip_address;
        std::uint16_t port_number = 0;
    };
    std::unordered_map<unsigned, ServerAddressInfo::Ptr> proxies_info;
    std::list<ServerAddressInfo::Ptr> servers;
    std::list<ServerAddressInfo::Ptr>::const_iterator cur_server_to_ask_log = servers.begin();
};
