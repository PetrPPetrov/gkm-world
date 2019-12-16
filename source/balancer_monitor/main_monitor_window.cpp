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
#include <QPushButton>
#include "main_monitor_window.h"

extern MainMonitorWindow* g_main_window = nullptr;

MainMonitorWindow::MainMonitorWindow()
{
    g_main_window = this;
    main_monitor_window.setupUi(this);

    connect(this, SIGNAL(message(QString)), this, SLOT(onMessage(QString)));
    connect(this, SIGNAL(connectionFatal(QString)), this, SLOT(onConnectionFatal(QString)));
    connect(this, SIGNAL(monitoringBalancerServerInfoAnswer(Packet::MonitoringBalancerServerInfoAnswer)),
            this, SLOT(onMonitoringBalancerServerInfoAnswer(Packet::MonitoringBalancerServerInfoAnswer)));

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
}

void MainMonitorWindow::onQuit()
{
    QMainWindow::close();
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

void MainMonitorWindow::onMonitoringBalancerServerInfoAnswer(Packet::MonitoringBalancerServerInfoAnswer packet)
{
}
