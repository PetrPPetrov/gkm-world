// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
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
#include "ui_main_window.h"
#include "texture_widget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

protected:
    void showEvent(QShowEvent* event) override;

private:
    bool first_show = true;
    Ui::MainWindow main_window;

    QDockWidget* log_dock = nullptr;
    QPlainTextEdit* log = nullptr;

    QDockWidget* texture1_dock = nullptr;
    TextureWidget* texture1_view = nullptr;
    QDockWidget* texture2_dock = nullptr;
    TextureWidget* texture2_view = nullptr;

    std::vector<ImagePtr> images;
};

extern MainWindow* g_main_window;
