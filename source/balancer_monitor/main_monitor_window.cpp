// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#include <QMenu>
#include <QToolBar>
#include <QDockWidget>
#include <QTextEdit>
#include <QStatusBar>
#include "main_monitor_window.h"

extern MainMonitorWindow* g_main_window = nullptr;

MainMonitorWindow::MainMonitorWindow()
{
    g_main_window = this;
    main_monitor_window.setupUi(this);
    statusBar()->showMessage(tr("Gkm-World Position: Unknown"));

    QMenu* balancer_server_menu = menuBar()->addMenu(tr("Server"));

    QAction* connect_act = new QAction(tr("C&onnect"), this);
    connect_act->setShortcuts(QKeySequence::Open);
    connect_act->setStatusTip(tr("Connect to balancer server"));
    //connect(connect_act, &QAction::triggered, this, &MainMonitorWindow::connect_balancer_server);
    balancer_server_menu->addAction(connect_act);

    QAction* close_act = new QAction(tr("&Close"), this);
    close_act->setShortcuts(QKeySequence::Close);
    close_act->setStatusTip(tr("Close connection"));
    //connect(close_act, &QAction::triggered, this, &MainMonitorWindow::close_balancer_server);
    balancer_server_menu->addAction(close_act);

    balancer_server_menu->addSeparator();
    QAction* quit_act = new QAction(tr("&Quit"), this);
    quit_act->setShortcuts(QKeySequence::Quit);
    quit_act->setStatusTip(tr("Quit the application"));
    connect(quit_act, &QAction::triggered, this, &QMainWindow::close);
    balancer_server_menu->addAction(quit_act);

    QMenu* view_menu = menuBar()->addMenu(tr("&View"));

    QDockWidget* dock = new QDockWidget(tr("Log"), this);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    log = new QPlainTextEdit(dock);
    log->setReadOnly(true);
    dock->setWidget(log);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    view_menu->addAction(dock->toggleViewAction());
}

QPlainTextEdit* MainMonitorWindow::getLog() const
{
    return log;
}

void MainMonitorWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);

    if (first_show)
    {
        first_show = false;
    }
}
