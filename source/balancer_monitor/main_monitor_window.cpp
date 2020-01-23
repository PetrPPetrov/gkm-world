// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <QMenu>
#include <QToolBar>
#include <QDockWidget>
#include <QTextEdit>
#include <QStatusBar>
#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QHeaderView>
#include <QPushButton>
#include "main_monitor_window.h"

extern MainMonitorWindow* g_main_window = nullptr;

MainMonitorWindow::MainMonitorWindow()
{
    g_main_window = this;
    main_monitor_window.setupUi(this);

    connect(this, SIGNAL(message(QString)), this, SLOT(onMessage(QString)));
    connect(this, SIGNAL(balancerServerMessage(QString)), this, SLOT(onBalancerServerMessage(QString)));
    connect(this, SIGNAL(proxyServerMessage(unsigned, QString)), this, SLOT(onProxyServerMessage(unsigned, QString)));
    connect(this, SIGNAL(connectionFatal(QString)), this, SLOT(onConnectionFatal(QString)));
    connect(this, SIGNAL(monitoringBalancerServerInfoAnswer(QByteArray)),
            this, SLOT(onMonitoringBalancerServerInfoAnswer(QByteArray)));
    connect(this, SIGNAL(monitoringBalanceTreeInfoAnswer(QByteArray)),
        this, SLOT(onMonitoringBalanceTreeInfoAnswer(QByteArray)));
    connect(this, SIGNAL(monitoringBalanceTreeNeighborInfoAnswer(QByteArray)),
        this, SLOT(onMonitoringBalanceTreeNeighborInfoAnswer(QByteArray)));
    connect(this, SIGNAL(monitoringBalanceTreeStaticSplitAnswer(QByteArray)),
        this, SLOT(onMonitoringBalanceTreeStaticSplitAnswer(QByteArray)));
    connect(this, SIGNAL(monitoringBalanceTreeStaticMergeAnswer(QByteArray)),
        this, SLOT(onMonitoringBalanceTreeStaticMergeAnswer(QByteArray)));
    connect(this, SIGNAL(monitoringGetProxyCountAnswer(QByteArray)),
        this, SLOT(onMonitoringGetProxyCountAnswer(QByteArray)));
    connect(this, SIGNAL(monitoringGetProxyInfoAnswer(QByteArray)),
        this, SLOT(onMonitoringGetProxyInfoAnswer(QByteArray)));

    statusBar()->showMessage(tr("Gkm-World Position: Unknown"));

    QMenu* balancer_server_menu = menuBar()->addMenu(tr("Server"));

    connect_act = new QAction(tr("C&onnect"), this);
    connect_act->setShortcuts(QKeySequence::Open);
    connect_act->setStatusTip(tr("Connect to balancer server"));
    connect(connect_act, &QAction::triggered, this, &MainMonitorWindow::onConnect);
    balancer_server_menu->addAction(connect_act);

    close_act = new QAction(tr("&Close"), this);
    close_act->setEnabled(false);
    close_act->setShortcuts(QKeySequence::Close);
    close_act->setStatusTip(tr("Close connection"));
    connect(close_act, &QAction::triggered, this, &MainMonitorWindow::onClose);
    balancer_server_menu->addAction(close_act);

    balancer_server_menu->addSeparator();
    QAction* quit_act = new QAction(tr("&Quit"), this);
    quit_act->setShortcuts(QKeySequence::Quit);
    quit_act->setStatusTip(tr("Quit the application"));
    connect(quit_act, &QAction::triggered, this, &MainMonitorWindow::onQuit);
    balancer_server_menu->addAction(quit_act);

    QMenu* view_menu = menuBar()->addMenu(tr("&View"));

    QDockWidget* server_tree_dock = new QDockWidget(tr("Server Tree"), this);
    server_tree_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    server_tree_view = new QTreeView(server_tree_dock);
    server_tree_view->setIndentation(10);
    server_tree_view->setHeaderHidden(true);
    server_tree_view->setUniformRowHeights(true);
    server_tree_dock->setWidget(server_tree_view);
    addDockWidget(Qt::LeftDockWidgetArea, server_tree_dock);
    view_menu->addAction(server_tree_dock->toggleViewAction());
    connect(server_tree_view, &QTreeView::collapsed, this, &MainMonitorWindow::onServerTreeCollapsed);
    connect(server_tree_view, &QTreeView::expanded, this, &MainMonitorWindow::onServerTreeExpanded);

    QDockWidget* property_dock = new QDockWidget(tr("Properties"), this);
    property_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    property_view = new QTableView(property_dock);
    property_view->setAutoScroll(true);
    property_view->verticalHeader()->setVisible(false);
    property_view->horizontalHeader()->setVisible(false);
    property_dock->setWidget(property_view);
    addDockWidget(Qt::RightDockWidgetArea, property_dock);
    view_menu->addAction(property_dock->toggleViewAction());

    log_dock = new QDockWidget(tr("Log"), this);
    log_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    log = new QPlainTextEdit(log_dock);
    log->setReadOnly(true);
    log->ensureCursorVisible();
    log->setCenterOnScroll(true);
    log_dock->setWidget(log);
    addDockWidget(Qt::BottomDockWidgetArea, log_dock);
    view_menu->addAction(log_dock->toggleViewAction());

    QDockWidget* balancer_server_log_dock = new QDockWidget(tr("Balancer Server Log"), this);
    balancer_server_log_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    balancer_server_log = new QPlainTextEdit(balancer_server_log_dock);
    balancer_server_log->setReadOnly(true);
    balancer_server_log->ensureCursorVisible();
    balancer_server_log->setCenterOnScroll(true);
    balancer_server_log_dock->setWidget(balancer_server_log);
    addDockWidget(Qt::BottomDockWidgetArea, balancer_server_log_dock);
    view_menu->addAction(balancer_server_log_dock->toggleViewAction());
    tabifyDockWidget(log_dock, balancer_server_log_dock);

    show_leaf_nodes_act = new QAction(tr("Show Leaf Nodes"), this);
    show_leaf_nodes_act->setStatusTip(tr("Show Leaf Nodes"));
    show_leaf_nodes_act->setCheckable(true);
    show_leaf_nodes_act->setChecked(true);
    connect(show_leaf_nodes_act, &QAction::triggered, this, &MainMonitorWindow::onShowLeafNodes);
    view_menu->addAction(show_leaf_nodes_act);

    show_selected_node_act = new QAction(tr("Show Selected Node"), this);
    show_selected_node_act->setStatusTip(tr("Show Selected Node"));
    show_selected_node_act->setCheckable(true);
    show_selected_node_act->setChecked(false);
    connect(show_selected_node_act, &QAction::triggered, this, &MainMonitorWindow::onShowSelectedNode);
    view_menu->addAction(show_selected_node_act);

    show_neighbor_act = new QAction(tr("Show Neighbor Nodes"), this);
    show_neighbor_act->setStatusTip(tr("Show Neighbor Nodes"));
    show_neighbor_act->setCheckable(true);
    show_neighbor_act->setChecked(false);
    connect(show_neighbor_act, &QAction::triggered, this, &MainMonitorWindow::onShowNeighbor);
    view_menu->addAction(show_neighbor_act);

    QAction* clear_log_act = new QAction(tr("Clear Log"), this);
    clear_log_act->setStatusTip(tr("Clear the log"));
    connect(clear_log_act, &QAction::triggered, this, &MainMonitorWindow::onClearLog);
    view_menu->addAction(clear_log_act);

    QAction* clear_balancer_server_log_act = new QAction(tr("Clear Balancer Server Log"), this);
    clear_balancer_server_log_act->setStatusTip(tr("Clear the balancer server log"));
    connect(clear_balancer_server_log_act, &QAction::triggered, this, &MainMonitorWindow::onClearBalancerServerLog);
    view_menu->addAction(clear_balancer_server_log_act);

    QAction* refresh_server_tree_act = new QAction(tr("Refresh Server Tree"), this);
    refresh_server_tree_act->setStatusTip(tr("Refresh Server Tree"));
    connect(refresh_server_tree_act, &QAction::triggered, this, &MainMonitorWindow::onRefreshServerTree);
    view_menu->addAction(refresh_server_tree_act);

    QMenu* action_menu = menuBar()->addMenu(tr("&Action"));

    static_split_act = new QAction(tr("Static Split"), this);
    static_split_act->setStatusTip(tr("Static Split of the Selected Node Server"));
    static_split_act->setEnabled(false);
    connect(static_split_act, &QAction::triggered, this, &MainMonitorWindow::onStaticSplit);
    action_menu->addAction(static_split_act);

    static_merge_act = new QAction(tr("Static Merge"), this);
    static_merge_act->setStatusTip(tr("Static Merge of the Selected Group Node"));
    static_merge_act->setEnabled(false);
    connect(static_merge_act, &QAction::triggered, this, &MainMonitorWindow::onStaticMerge);
    action_menu->addAction(static_merge_act);

    show_server_log = new QAction(tr("Show Log"), this);
    show_server_log->setStatusTip(tr("Show log of the selected server"));
    show_server_log->setEnabled(false);
    connect(show_server_log, &QAction::triggered, this, &MainMonitorWindow::onShowServerLog);
    action_menu->addAction(show_server_log);

    server_tree_view->setContextMenuPolicy(Qt::ActionsContextMenu);
    server_tree_view->addAction(static_split_act);
    server_tree_view->addAction(static_merge_act);
    server_tree_view->addAction(show_server_log);
}

QPlainTextEdit* MainMonitorWindow::getLog() const
{
    return log;
}

bool MainMonitorWindow::isShowLeafNodes() const
{
    return show_leaf_nodes_act->isChecked();
}

bool MainMonitorWindow::isShowSelectedNode() const
{
    return show_selected_node_act->isChecked();
}

bool MainMonitorWindow::isShowNeighbor() const
{
    return show_neighbor_act->isChecked();
}

void MainMonitorWindow::connectedState()
{
    connect_act->setEnabled(false);
    close_act->setEnabled(true);
    static_split_act->setEnabled(false);
    static_merge_act->setEnabled(false);
}

void MainMonitorWindow::disconnectedState()
{
    connect_act->setEnabled(true);
    close_act->setEnabled(false);
    static_split_act->setEnabled(false);
    static_merge_act->setEnabled(false);
}

ServerInfo::Ptr MainMonitorWindow::getServerInfo() const
{
    return server_info;
}

void MainMonitorWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);

    if (first_show)
    {
        first_show = false;
    }
}

void MainMonitorWindow::onConnect()
{
    QDialog dialog(this);
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    dialog.setWindowTitle(tr("Connect to balancer server"));
    dialog.setModal(true);
    QGridLayout layout(&dialog);

    QLabel label_host_name(&dialog);
    label_host_name.setText(tr("Enter host name or IP address"));
    layout.addWidget(&label_host_name);

    QLineEdit host_name_edit(&dialog);
    host_name_edit.setText(tr("localhost"));
    layout.addWidget(&host_name_edit);

    QLabel label_port_number(&dialog);
    label_port_number.setText(tr("Enter port number"));
    layout.addWidget(&label_port_number);

    QSpinBox port_number_edit(&dialog);
    port_number_edit.setRange(100, 20000);
    port_number_edit.setSingleStep(1);
    port_number_edit.setValue(17013);
    layout.addWidget(&port_number_edit);

    QPushButton ok(tr("Connect"), &dialog);
    ok.setDefault(true);
    ok.setAutoDefault(true);
    connect(&ok, &QPushButton::pressed, &dialog, &QDialog::accept);
    layout.addWidget(&ok);

    if (dialog.exec())
    {
        connectedState();
        connection = new MonitorUDPConnection(host_name_edit.text(), static_cast<std::uint16_t>(port_number_edit.value()), this);
    }
}

void MainMonitorWindow::onClose()
{
    assert(connection);
    connection->close();
    connection = nullptr;
    disconnectedState();
    server_tree = nullptr;
    server_tree_view->setModel(nullptr);
    property_tree = nullptr;
    property_view->setModel(nullptr);
    server_info = nullptr;
    update();
}

void MainMonitorWindow::onQuit()
{
    if (connection)
    {
        onClose();
    }
    QMainWindow::close();
}

void MainMonitorWindow::onShowLeafNodes(bool checked)
{
    update();
}

void MainMonitorWindow::onShowSelectedNode(bool checked)
{
    update();
}

void MainMonitorWindow::onShowNeighbor(bool checked)
{
    update();
}

void MainMonitorWindow::onClearLog()
{
    log->clear();
}

void MainMonitorWindow::onClearBalancerServerLog()
{
    balancer_server_log->clear();
}

void MainMonitorWindow::onRefreshServerTree()
{
    if (server_info && server_info->tree_root_token)
    {
        server_info->wait_token = server_info->tree_root_token;
        connection->getBalanceTreeInfo(server_info->tree_root_token);
    }
}

void MainMonitorWindow::onStaticSplit()
{
    if (server_info)
    {
        if (server_info->tree_root_token && isAnyNodeServerRunning(server_info, server_info->tree_root_token))
        {
            log->appendPlainText(tr("can not use static split method, some node servers are running"));
            return;
        }
        if (server_info->selected_node)
        {
            auto selected_node = server_info->selected_node;
            if (!selected_node->leaf_node)
            {
                log->appendPlainText(tr("can not use static split method, balance tree node with token %1 is not leaf node").arg(selected_node->token));
                return;
            }
            if (selected_node->node_server_port_number)
            {
                log->appendPlainText(tr("can not use static split method, balance tree node with token %1 has node server running").arg(selected_node->token));
                return;
            }
            connection->staticSplit(selected_node->token);
        }
    }
}

void MainMonitorWindow::onStaticMerge()
{
    if (server_info)
    {
        if (server_info->tree_root_token && isAnyNodeServerRunning(server_info, server_info->tree_root_token))
        {
            log->appendPlainText(tr("can not use static merge method, some node servers are running"));
            return;
        }
        if (server_info->selected_node)
        {
            auto selected_node = server_info->selected_node;
            if (selected_node->leaf_node)
            {
                log->appendPlainText(tr("can not use static merge method, balance tree node with token %1 is leaf node").arg(selected_node->token));
                return;
            }
            if (selected_node->node_server_port_number)
            {
                log->appendPlainText(tr("can not use static merge method, balance tree node with token %1 has node server running").arg(selected_node->token));
                return;
            }
            connection->staticMerge(selected_node->token);
        }
    }
}

void MainMonitorWindow::onShowServerLog()
{
    if (server_info)
    {
        if (server_info->selected_node && server_info->selected_node->leaf_node && server_info->selected_node->node_server_port_number != 0)
        {
            return;
        }
        if (server_info->selected_proxy_index)
        {
            auto find_it = server_info->id_to_proxy.find(server_info->selected_proxy_index);
            if (find_it != server_info->id_to_proxy.end())
            {
                if (find_it->second.log && find_it->second.log_dock)
                {
                    find_it->second.log_dock->show();
                }else
                {
                    QDockWidget* server_log_dock = new QDockWidget(tr("Proxy Server %1 Log").arg(server_info->selected_proxy_index), this);
                    server_log_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
                    QPlainTextEdit* server_log = new QPlainTextEdit(server_log_dock);
                    server_log->setReadOnly(true);
                    server_log->ensureCursorVisible();
                    server_log->setCenterOnScroll(true);
                    server_log_dock->setWidget(server_log);
                    addDockWidget(Qt::BottomDockWidgetArea, server_log_dock);
                    tabifyDockWidget(log_dock, server_log_dock);
                    find_it->second.log = server_log;
                    find_it->second.log_dock = server_log_dock;
                    connection->addProxyLog(server_info->selected_proxy_index, find_it->second.ip_address.c_str(), find_it->second.port_number);
                }
            }
        }
    }
}

void MainMonitorWindow::onServerTreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    bool has_properties = false;
    if (!selected.empty() && !selected.front().indexes().empty())
    {
        QModelIndex index = selected.front().indexes().at(0);
        TreeItem* current_item = static_cast<TreeItem*>(index.internalPointer());
        if (current_item)
        {
            BalancerItem* current_balancer = dynamic_cast<BalancerItem*>(current_item);
            if (current_balancer)
            {
                // It is Balancer Server, so, split and merge are unavailable
                has_properties = true;
                property_tree = std::make_shared<ListModel>(buildBalancerPropertyList(server_info, connection));
                property_view->setModel(property_tree.get());
                property_view->update();
                server_info->selected_proxy_index = 0;
                server_info->selected_node = nullptr;
                expand_status.selected_token = 0;
                expand_status.is_selected_token_valid = false;
                static_split_act->setEnabled(false);
                static_merge_act->setEnabled(false);
                show_server_log->setEnabled(false);
            }
            NodeItem* current_node = dynamic_cast<NodeItem*>(current_item);
            if (current_node && current_node->getNode())
            {
                has_properties = true;
                property_tree = std::make_shared<ListModel>(buildNodePropertyList(current_node->getNode()));
                property_view->setModel(property_tree.get());
                property_view->update();
                server_info->selected_proxy_index = 0;
                server_info->selected_node = current_node->getNode();
                expand_status.selected_token = current_node->getNode()->token;
                expand_status.is_selected_token_valid = true;
                static_split_act->setEnabled(current_node->getNode()->leaf_node);
                static_merge_act->setEnabled(!current_node->getNode()->leaf_node);
                show_server_log->setEnabled(current_node->getNode()->leaf_node && server_info->selected_node->node_server_port_number != 0);
            }
            ProxyItem* current_proxy = dynamic_cast<ProxyItem*>(current_item);
            if (current_proxy)
            {
                // It is Proxy Server, so, split and merge are unavailable
                has_properties = true;
                property_tree = std::make_shared<ListModel>(buildProxyPropertyList(server_info, current_proxy->getProxyIndex()));
                property_view->setModel(property_tree.get());
                property_view->update();
                server_info->selected_proxy_index = current_proxy->getProxyIndex();
                server_info->selected_node = nullptr;
                expand_status.selected_token = 0;
                expand_status.is_selected_token_valid = false;
                static_split_act->setEnabled(false);
                static_merge_act->setEnabled(false);
                show_server_log->setEnabled(true);
            }
        }
    }
    if (!has_properties)
    {
        property_tree = nullptr;
        property_view->setModel(nullptr);
        server_info->selected_node = nullptr;
    }
    update();
}

void MainMonitorWindow::onServerTreeCollapsed(const QModelIndex& index)
{
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item)
    {
        NodeItem* node_item = dynamic_cast<NodeItem*>(item);
        if (node_item && node_item->getNode())
        {
            expand_status.tree_expand_status.erase(node_item->getNode()->token);
        }
        BalancerItem* balancer_item = dynamic_cast<BalancerItem*>(item);
        if (balancer_item)
        {
            expand_status.tree_expand_status.erase(0);
        }
    }
}

void MainMonitorWindow::onServerTreeExpanded(const QModelIndex& index)
{
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item)
    {
        NodeItem* node_item = dynamic_cast<NodeItem*>(item);
        if (node_item && node_item->getNode())
        {
            expand_status.tree_expand_status.emplace(node_item->getNode()->token, true);
        }
        BalancerItem* balancer_item = dynamic_cast<BalancerItem*>(item);
        if (balancer_item)
        {
            expand_status.tree_expand_status.emplace(0, true);
        }
    }
}

void MainMonitorWindow::onMessage(QString message)
{
    log->appendPlainText(message);
}

void MainMonitorWindow::onBalancerServerMessage(QString message)
{
    balancer_server_log->appendPlainText(message);
    update();
}

void MainMonitorWindow::onProxyServerMessage(unsigned proxy_index, QString message)
{
    if (server_info)
    {
        auto find_it = server_info->id_to_proxy.find(proxy_index);
        if (find_it != server_info->id_to_proxy.end())
        {
            if (find_it->second.log)
            {
                find_it->second.log->appendPlainText(message);
            }
            update();
        }
    }
}

void MainMonitorWindow::onConnectionFatal(QString message)
{
    log->appendPlainText(message);
    onClose();
}

void MainMonitorWindow::onMonitoringBalancerServerInfoAnswer(QByteArray data)
{
    if (!server_info)
    {
        server_info = std::make_shared<ServerInfo>();
    }
    const Packet::MonitoringBalancerServerInfoAnswer* answer = reinterpret_cast<const Packet::MonitoringBalancerServerInfoAnswer*>(data.data());
    if (data.size() >= Packet::getSize(answer))
    {
        log->appendPlainText(tr("received balancer server info"));
        server_info->bounding_box = answer->global_bounding_box;
        server_info->tree_root_token = answer->tree_root_token;
        if (server_info->tree_root_token)
        {
            server_info->wait_token = server_info->tree_root_token;
            connection->getBalanceTreeInfo(server_info->tree_root_token);
        }
        else
        {
            log->appendPlainText(tr("root tree does not exist"));
        }
    }
    else
    {
        log->appendPlainText(tr("invalid data size: %1").arg(data.size()));
    }
    update();
}

void MainMonitorWindow::onMonitoringBalanceTreeInfoAnswer(QByteArray data)
{
    const Packet::MonitoringBalanceTreeInfoAnswer* answer = reinterpret_cast<const Packet::MonitoringBalanceTreeInfoAnswer*>(data.data());
    if (data.size() >= Packet::getSize(answer))
    {
        if (server_info->wait_token == answer->tree_node_token)
        {
            server_info->wait_token = 0;
            if (answer->success)
            {
                log->appendPlainText(tr("received balance tree info, token %1").arg(answer->tree_node_token));
                BalancerTreeInfo* tree_info = server_info->token_to_tree_node.find(answer->tree_node_token);
                if (!tree_info)
                {
                    tree_info = new(server_info->token_to_tree_node.allocate(answer->tree_node_token)) BalancerTreeInfo;
                }
                tree_info->token = answer->tree_node_token;
                tree_info->bounding_box = answer->bounding_box;
                tree_info->children = answer->children;
                tree_info->leaf_node = answer->leaf_node;
                tree_info->level = answer->level;
                tree_info->user_count = answer->user_count;
                tree_info->node_server_address = ip_address_t(answer->node_server_address);
                tree_info->node_server_port_number = answer->node_server_port_number;
                tree_info->current_child_index_to_send = 0;

                bool sent_new_request = false;
                if (!tree_info->leaf_node)
                {
                    server_info->parent_stack.push_back(tree_info->token);
                    for (tree_info->current_child_index_to_send = 0; tree_info->current_child_index_to_send < ChildLast; ++tree_info->current_child_index_to_send)
                    {
                        std::uint32_t child_token = tree_info->children[tree_info->current_child_index_to_send];
                        if (child_token)
                        {
                            server_info->wait_token = child_token;
                            connection->getBalanceTreeInfo(child_token);
                            sent_new_request = true;
                            break;
                        }
                    }
                }
                else
                {
                    generateNeighborRequests(tree_info->token);
                }

                while (!sent_new_request && !server_info->parent_stack.empty())
                {
                    std::uint32_t parent_token = server_info->parent_stack.back();
                    BalancerTreeInfo* parent_tree_info = server_info->token_to_tree_node.find(parent_token);
                    if (parent_tree_info)
                    {
                        ++parent_tree_info->current_child_index_to_send;
                        if (parent_tree_info->current_child_index_to_send < ChildLast)
                        {
                            std::uint32_t sibling_token = parent_tree_info->children[parent_tree_info->current_child_index_to_send];
                            server_info->wait_token = sibling_token;
                            connection->getBalanceTreeInfo(sibling_token);
                            sent_new_request = true;
                        }
                        else
                        {
                            server_info->parent_stack.pop_back();
                        }
                    }
                }

                if (server_info->parent_stack.empty())
                {
                    // We received all balancer node tree
                    buildServerTree();

                    if (!server_info->neighbor_requests.empty())
                    {
                        ServerInfo::NeighborRequest request = server_info->neighbor_requests.front();
                        connection->getBalanceTreeNeighborInfo(request.token, request.x, request.y);
                        server_info->neighbor_requests.pop_front();
                    }
                }
            }
            else
            {
                log->appendPlainText(tr("balance tree node with token %1 does not exist").arg(answer->tree_node_token));
            }
        }
        else
        {
            log->appendPlainText(tr("unexpected tree node token answer %1, waiting for %2").arg(answer->tree_node_token, server_info->wait_token));
        }
    }
    else
    {
        log->appendPlainText(tr("invalid data size: %1").arg(data.size()));
    }
}

void MainMonitorWindow::onMonitoringBalanceTreeNeighborInfoAnswer(QByteArray data)
{
    const Packet::MonitoringBalanceTreeNeighborInfoAnswer* answer = reinterpret_cast<const Packet::MonitoringBalanceTreeNeighborInfoAnswer*>(data.data());
    if (data.size() >= Packet::getSize(answer))
    {
        if (answer->success)
        {
            log->appendPlainText(tr("received balance tree neighbor info, token %1").arg(answer->tree_node_token));
            BalancerTreeInfo* tree_info = server_info->token_to_tree_node.find(answer->tree_node_token);
            if (tree_info)
            {
                if (tree_info->leaf_node)
                {
                    std::pair<std::int32_t, std::int32_t> coordinate(answer->neighbor_cell.x, answer->neighbor_cell.y);
                    if (tree_info->neighbor_nodes.find(coordinate) == tree_info->neighbor_nodes.end())
                    {
                        tree_info->neighbor_nodes.emplace(coordinate, answer->neighbor_node_token);
                    }
                    else
                    {
                        log->appendPlainText(tr("neighbor with coordinates %1, %2 (tree node with token %3) is already received").arg(answer->neighbor_cell.x, answer->neighbor_cell.y, answer->tree_node_token));
                    }
                    if (!server_info->neighbor_requests.empty())
                    {
                        ServerInfo::NeighborRequest request = server_info->neighbor_requests.front();
                        connection->getBalanceTreeNeighborInfo(request.token, request.x, request.y);
                        server_info->neighbor_requests.pop_front();
                    }
                }
                else
                {
                    log->appendPlainText(tr("balance tree node with token %1 is not a leaf node").arg(answer->tree_node_token));
                }
            }
            else
            {
                log->appendPlainText(tr("balance tree node with token %1 is null").arg(answer->tree_node_token));
            }
        }
        else
        {
            log->appendPlainText(tr("balance tree node with token %1 does not exist").arg(answer->tree_node_token));
        }
    }
    else
    {
        log->appendPlainText(tr("invalid data size: %1").arg(data.size()));
    }
}

void MainMonitorWindow::onMonitoringBalanceTreeStaticSplitAnswer(QByteArray data)
{
    const Packet::MonitoringBalanceTreeStaticSplitAnswer* answer = reinterpret_cast<const Packet::MonitoringBalanceTreeStaticSplitAnswer*>(data.data());
    if (data.size() >= Packet::getSize(answer))
    {
        if (answer->node_server_running)
        {
            log->appendPlainText(tr("can not use static split method, balance tree node with token %1 has node server running").arg(answer->tree_node_token));
            return;
        }
        if (answer->not_leaf_node)
        {
            log->appendPlainText(tr("can not use static split method, balance tree node with token %1 is not leaf node").arg(answer->tree_node_token));
            return;
        }
        if (!answer->success)
        {
            log->appendPlainText(tr("balance tree node with token %1 does not exist").arg(answer->tree_node_token));
            return;
        }
        log->appendPlainText(tr("received balance tree static split success answer, token %1").arg(answer->tree_node_token));
        connection->getBalancerServerInfo();
    }
    else
    {
        log->appendPlainText(tr("invalid data size: %1").arg(data.size()));
    }
}

void MainMonitorWindow::onMonitoringBalanceTreeStaticMergeAnswer(QByteArray data)
{
    const Packet::MonitoringBalanceTreeStaticMergeAnswer* answer = reinterpret_cast<const Packet::MonitoringBalanceTreeStaticMergeAnswer*>(data.data());
    if (data.size() >= Packet::getSize(answer))
    {
        if (!answer->success)
        {
            log->appendPlainText(tr("static merge was failed on node with token %1").arg(answer->tree_node_token));
            return;
        }
        log->appendPlainText(tr("received balance tree static merge success answer, token %1").arg(answer->tree_node_token));
        connection->getBalancerServerInfo();
    }
    else
    {
        log->appendPlainText(tr("invalid data size: %1").arg(data.size()));
    }
}

void MainMonitorWindow::onMonitoringGetProxyCountAnswer(QByteArray data)
{
    const Packet::MonitoringGetProxyCountAnswer* answer = reinterpret_cast<const Packet::MonitoringGetProxyCountAnswer*>(data.data());
    if (data.size() >= Packet::getSize(answer))
    {
        log->appendPlainText(tr("proxy count = %1").arg(answer->proxy_count));
        server_info->id_to_proxy.clear();
        server_info->proxy_info_requests.clear();
        for (std::uint32_t index = 1; index <= answer->proxy_count; ++index)
        {
            server_info->proxy_info_requests.push_back(index);
        }
        if (server_info->proxy_info_requests.empty())
        {
            // We have received all information about proxy servers
            buildServerTree();
        }
        else
        {
            std::uint32_t cur_proxy_index = server_info->proxy_info_requests.front();
            server_info->proxy_info_requests.pop_front();
            connection->getProxyInfo(cur_proxy_index);
        }
    }
    else
    {
        log->appendPlainText(tr("invalid data size: %1").arg(data.size()));
    }
}

void MainMonitorWindow::onMonitoringGetProxyInfoAnswer(QByteArray data)
{
    const Packet::MonitoringGetProxyInfoAnswer* answer = reinterpret_cast<const Packet::MonitoringGetProxyInfoAnswer*>(data.data());
    if (data.size() >= Packet::getSize(answer))
    {
        log->appendPlainText(tr("proxy info, proxy index = %1").arg(answer->proxy_index));
        ServerInfo::ProxyInfo proxy_server_address;
        proxy_server_address.ip_address = ip_address_t(answer->proxy_server_address).to_string();
        proxy_server_address.port_number = answer->proxy_server_port_number;
        server_info->id_to_proxy.emplace(answer->proxy_index, proxy_server_address);
        if (server_info->proxy_info_requests.empty())
        {
            // We have received all information about proxy servers
            buildServerTree();
        }else
        {
            std::uint32_t cur_proxy_index = server_info->proxy_info_requests.front();
            server_info->proxy_info_requests.pop_front();
            connection->getProxyInfo(cur_proxy_index);
        }
    }
    else
    {
        log->appendPlainText(tr("invalid data size: %1").arg(data.size()));
    }
}

void MainMonitorWindow::generateNeighborRequests(std::uint32_t token)
{
    if (isShowNeighbor())
    {
        BalancerTreeInfo* tree_info = server_info->token_to_tree_node.find(token);
        if (tree_info->leaf_node)
        {
            ServerInfo::NeighborRequest new_request;
            new_request.token = token;
            for (int x = 0; x < tree_info->bounding_box.size + 2; ++x)
            {
                int cur_x = tree_info->bounding_box.start.x - 1 + x;
                new_request.x = cur_x;
                new_request.y = tree_info->bounding_box.start.y - 1;
                server_info->neighbor_requests.push_back(new_request);
                new_request.x = cur_x;
                new_request.y = tree_info->bounding_box.start.y + tree_info->bounding_box.size;
                server_info->neighbor_requests.push_back(new_request);
            }
            for (int y = 0; y < tree_info->bounding_box.size; ++y)
            {
                int cur_y = tree_info->bounding_box.start.y + y;
                new_request.x = tree_info->bounding_box.start.x - 1;
                new_request.y = cur_y;
                server_info->neighbor_requests.push_back(new_request);
                new_request.x = tree_info->bounding_box.start.x + tree_info->bounding_box.size;
                new_request.y = cur_y;
                server_info->neighbor_requests.push_back(new_request);
            }
        }
    }
}

void MainMonitorWindow::restoreExpandStatus()
{
    QAbstractItemModel* model = server_tree_view->model();
    if (model)
    {
        QModelIndex root_index = model->index(0, 0);
        restoreExpandStatus(root_index);
    }
}

void MainMonitorWindow::restoreExpandStatus(const QModelIndex& index)
{
    QAbstractItemModel* model = server_tree_view->model();
    if (model)
    {
        TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
        if (item)
        {
            std::uint32_t token = 0;
            BalancerItem* balancer_item = dynamic_cast<BalancerItem*>(item);
            NodeItem* node_item = dynamic_cast<NodeItem*>(item);
            if (node_item || balancer_item)
            {
                if (node_item)
                {
                    token = node_item->getNode()->token;
                }
                if (expand_status.tree_expand_status.find(token) != expand_status.tree_expand_status.end())
                {
                    server_tree_view->expand(index);
                }
                if (expand_status.is_selected_token_valid && expand_status.selected_token == token)
                {
                    server_tree_view->selectionModel()->select(index, QItemSelectionModel::Select);
                }
                for (int i = 0; i < model->rowCount(index); ++i)
                {
                    QModelIndex child = model->index(i, 0, index);
                    restoreExpandStatus(child);
                }
            }
        }
    }
}

void MainMonitorWindow::buildServerTree()
{
    server_tree = std::make_shared<TreeModel>(buildServerHierarchy(server_info));
    server_tree_view->setModel(server_tree.get());
    restoreExpandStatus();
    // TODO: Move this line somewhere to do not connect it each time
    connect(server_tree_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainMonitorWindow::onServerTreeSelectionChanged);
    update();
}
