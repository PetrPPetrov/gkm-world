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
    connect(this, SIGNAL(connectionFatal(QString)), this, SLOT(onConnectionFatal(QString)));
    connect(this, SIGNAL(monitoringBalancerServerInfoAnswer(QByteArray)),
            this, SLOT(onMonitoringBalancerServerInfoAnswer(QByteArray)));
    connect(this, SIGNAL(monitoringBalanceTreeInfoAnswer(QByteArray)),
        this, SLOT(onMonitoringBalanceTreeInfoAnswer(QByteArray)));

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

    QDockWidget* tree_node_dock = new QDockWidget(tr("Node Tree"), this);
    tree_node_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    node_tree_view = new QTreeView(tree_node_dock);
    node_tree_view->setIndentation(10);
    node_tree_view->setHeaderHidden(true);
    node_tree_view->setUniformRowHeights(true);
    tree_node_dock->setWidget(node_tree_view);
    addDockWidget(Qt::RightDockWidgetArea, tree_node_dock);
    view_menu->addAction(tree_node_dock->toggleViewAction());

    QDockWidget* property_dock = new QDockWidget(tr("Properties"), this);
    property_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    property_view = new QTableView(property_dock);
    property_view->setAutoScroll(true);
    property_view->verticalHeader()->setVisible(false);
    property_view->horizontalHeader()->setVisible(false);
    property_dock->setWidget(property_view);
    addDockWidget(Qt::RightDockWidgetArea, property_dock);
    view_menu->addAction(property_dock->toggleViewAction());

    QDockWidget* log_dock = new QDockWidget(tr("Log"), this);
    log_dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    log = new QPlainTextEdit(log_dock);
    log->setReadOnly(true);
    log->ensureCursorVisible();
    log->setCenterOnScroll(true);
    log_dock->setWidget(log);
    addDockWidget(Qt::RightDockWidgetArea, log_dock);
    view_menu->addAction(log_dock->toggleViewAction());

    QAction* clear_log_act = new QAction(tr("Clear Log"), this);
    clear_log_act->setStatusTip(tr("Clear the log"));
    connect(clear_log_act, &QAction::triggered, this, &MainMonitorWindow::onClearLog);
    view_menu->addAction(clear_log_act);
}

QPlainTextEdit* MainMonitorWindow::getLog() const
{
    return log;
}

BalancerServerInfo::Ptr MainMonitorWindow::getServerInfo() const
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
        connect_act->setEnabled(false);
        close_act->setEnabled(true);
        connection = new MonitorUDPConnection(host_name_edit.text(), static_cast<unsigned short>(port_number_edit.value()), this);
    }
}

void MainMonitorWindow::onClose()
{
    assert(connection);
    connection->close();
    connection = nullptr;
    close_act->setEnabled(false);
    connect_act->setEnabled(true);
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

void MainMonitorWindow::onClearLog()
{
    log->clear();
}

void MainMonitorWindow::onNodeTreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    bool is_selected = false;
    if (!selected.empty() && !selected.front().indexes().empty())
    {
        QModelIndex index = selected.front().indexes().at(0);
        TreeItem* current_item = static_cast<TreeItem*>(index.internalPointer());
        if (current_item)
        {
            NodeTreeItem* current_node_tree = dynamic_cast<NodeTreeItem*>(current_item);
            if (current_node_tree && current_node_tree->getNode())
            {
                property_tree = std::make_shared<ListModel>(getPropertyList(current_node_tree->getNode()));
                property_view->setModel(property_tree.get());
                property_view->update();
                server_info->selected_node = current_node_tree->getNode();
                is_selected = true;
            }
            else if (current_node_tree && !current_node_tree->getNode())
            {
                property_tree = std::make_shared<ListModel>(getPropertyList(server_info));
                property_view->setModel(property_tree.get());
                property_view->update();
                server_info->selected_node = server_info->token_to_tree_node.find(server_info->tree_root_token);
                is_selected = true;
            }
        }
    }
    if (!is_selected)
    {
        property_tree = nullptr;
        property_view->setModel(nullptr);
        server_info->selected_node = nullptr;
    }
    update();
}

void MainMonitorWindow::onMessage(const QString& message)
{
    log->appendPlainText(message);
}

void MainMonitorWindow::onConnectionFatal(const QString& message)
{
    log->appendPlainText(message);
    onClose();
}

void MainMonitorWindow::onMonitoringBalancerServerInfoAnswer(QByteArray data)
{
    if (!server_info)
    {
        server_info = std::make_shared<BalancerServerInfo>();
    }
    const Packet::MonitoringBalancerServerInfoAnswer* answer = reinterpret_cast<const Packet::MonitoringBalancerServerInfoAnswer*>(data.data());
    if (data.size() >= Packet::getSize(answer))
    {
        log->appendPlainText(tr("received balancer server info"));
        server_info->bounding_box = answer->global_bounding_box;
        server_info->tree_root_token = answer->tree_root_token;
        if (server_info->tree_root_token)
        {
            server_info->wait_info_for_token.insert(server_info->tree_root_token);
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
        if (server_info->wait_info_for_token.find(answer->tree_node_token) == server_info->wait_info_for_token.end())
        {
            log->appendPlainText(tr("unexpected tree node token answer %1").arg(answer->tree_node_token));
            return;
        }
        if (!answer->success)
        {
            log->appendPlainText(tr("balance tree node with token %1 does not exist").arg(answer->tree_node_token));
            return;
        }
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
        server_info->wait_info_for_token.erase(answer->tree_node_token);
        if (!tree_info->leaf_node)
        {
            for (auto child_token : tree_info->children)
            {
                if (child_token)
                {
                    server_info->wait_info_for_token.insert(child_token);
                    connection->getBalanceTreeInfo(child_token);
                }
            }
        }
        if (server_info->wait_info_for_token.empty())
        {
            // We received all balancer node tree
            node_tree = std::make_shared<TreeModel>(std::make_shared<NodeTreeItem>(server_info));
            node_tree_view->setModel(node_tree.get());
            // TODO: Move this line somewhere to do not connect it each time
            connect(node_tree_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &MainMonitorWindow::onNodeTreeSelectionChanged);
            node_tree_view->update();
        }
    }
    else
    {
        log->appendPlainText(tr("invalid data size: %1").arg(data.size()));
    }
}
