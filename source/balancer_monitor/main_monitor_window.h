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
#include <QThread>
#include "ui_main_monitor_window.h"
#include "monitor_udp_connection.h"
#include "balancer_server_info.h"

class MainMonitorWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainMonitorWindow();
    QPlainTextEdit* getLog() const;

    BalancerServerInfo::Ptr server_info;

protected:
    void showEvent(QShowEvent* event) override;

private:
    void onConnect();
    void onClose();
    void onQuit();

signals:
    void message(const QString& message);
    void connectionFatal(const QString& message);
    void monitoringBalancerServerInfoAnswer(QByteArray data);

private slots:
    void onMessage(const QString& message);
    void onConnectionFatal(const QString& message);
    void onMonitoringBalancerServerInfoAnswer(QByteArray data);

private:
    bool first_show = true;
    Ui::MainMonitorWindow main_monitor_window;
    QPlainTextEdit* log = nullptr;
    QAction* connect_act = nullptr;
    QAction* close_act = nullptr;
    MonitorUDPConnection* connection = nullptr;
};

extern MainMonitorWindow* g_main_window;
