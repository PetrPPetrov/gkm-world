// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <vector>
#include <memory>
#include <QObject>
#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QDockWidget>
#include <QPlainTextEdit>
#include <QTreeView>
#include <QTableView>
#include <QThread>
#include "ui_main_monitor_window.h"
#include "monitor_udp_connection.h"
#include "server_info.h"
#include "server_tree.h"

class MainMonitorWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainMonitorWindow();
    QPlainTextEdit* getLog() const;
    bool isShowLeafNodes() const;
    bool isShowSelectedNode() const;
    bool isShowNeighbor() const;
    void connectedState();
    void disconnectedState();
    ServerInfo::Ptr getServerInfo() const;

protected:
    void showEvent(QShowEvent* event) override;

private:
    void onConnect();
    void onClose();
    void onQuit();
    void onShowLeafNodes(bool checked);
    void onShowSelectedNode(bool checked);
    void onShowNeighbor(bool checked);
    void onClearLog();
    void onClearBalancerServerLog();
    void onRefreshServerTree();
    void onStaticSplit();
    void onStaticMerge();
    void onShowServerLog();
    void onServerTreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void onServerTreeCollapsed(const QModelIndex& index);
    void onServerTreeExpanded(const QModelIndex& index);

signals:
    void message(QString message);
    void balancerServerMessage(QString message);
    void proxyServerMessage(unsigned proxy_index, QString message);
    void connectionFatal(QString message);
    void monitoringBalancerServerInfoAnswer(QByteArray data);
    void monitoringBalanceTreeInfoAnswer(QByteArray data);
    void monitoringBalanceTreeNeighborInfoAnswer(QByteArray data);
    void monitoringBalanceTreeStaticSplitAnswer(QByteArray data);
    void monitoringBalanceTreeStaticMergeAnswer(QByteArray data);
    void monitoringGetProxyCountAnswer(QByteArray data);
    void monitoringGetProxyInfoAnswer(QByteArray data);

private slots:
    void onMessage(QString message);
    void onBalancerServerMessage(QString message);
    void onProxyServerMessage(unsigned proxy_index, QString message);
    void onConnectionFatal(QString message);
    void onMonitoringBalancerServerInfoAnswer(QByteArray data);
    void onMonitoringBalanceTreeInfoAnswer(QByteArray data);
    void onMonitoringBalanceTreeNeighborInfoAnswer(QByteArray data);
    void onMonitoringBalanceTreeStaticSplitAnswer(QByteArray data);
    void onMonitoringBalanceTreeStaticMergeAnswer(QByteArray data);
    void onMonitoringGetProxyCountAnswer(QByteArray data);
    void onMonitoringGetProxyInfoAnswer(QByteArray data);

private:
    void generateNeighborRequests(std::uint32_t token);
    void restoreExpandStatus();
    void restoreExpandStatus(const QModelIndex& index);
    void buildServerTree();

private:
    bool first_show = true;
    Ui::MainMonitorWindow main_monitor_window;
    QDockWidget* log_dock = nullptr;
    QPlainTextEdit* log = nullptr;
    QPlainTextEdit* balancer_server_log = nullptr;
    QTreeView* server_tree_view = nullptr;
    QTableView* property_view = nullptr;
    QAction* connect_act = nullptr;
    QAction* close_act = nullptr;
    QAction* show_leaf_nodes_act = nullptr;
    QAction* show_selected_node_act = nullptr;
    QAction* show_neighbor_act = nullptr;
    QAction* static_split_act = nullptr;
    QAction* static_merge_act = nullptr;
    QAction* show_server_log = nullptr;
    MonitorUDPConnection* connection = nullptr;

    ServerInfo::Ptr server_info;
    TreeModel::Ptr server_tree = nullptr;
    ListModel::Ptr property_tree = nullptr;
    ServerTreeExpandStatus expand_status;
};

extern MainMonitorWindow* g_main_window;
