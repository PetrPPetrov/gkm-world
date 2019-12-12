// Copyright 2018-2019 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <vector>
#include <QObject>
#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QDockWidget>
#include <QTreeView>
#include "ui_main_monitor_window.h"

class MainMonitorWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainMonitorWindow();

protected:
    void showEvent(QShowEvent* event) override;

private:
    bool first_show = true;
    Ui::MainMonitorWindow main_monitor_window;
};

extern MainMonitorWindow* g_main_window;
