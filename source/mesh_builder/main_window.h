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
#include <QMdiSubWindow>
#include <QListWidget>
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

    MeshBuilderWidget* open_gl_view = nullptr;
    QMdiSubWindow* open_gl_window = nullptr;
    QPlainTextEdit* log_view = nullptr;
    QMdiSubWindow* log_window = nullptr;
    TextureWidget* texture1_view = nullptr;
    QMdiSubWindow* texture1_window = nullptr;
    TextureWidget* texture2_view = nullptr;
    QMdiSubWindow* texture2_window = nullptr;
    QMdiSubWindow* texture_list_window = nullptr;
    QListWidget* texture_list_view = nullptr;

    std::vector<ImagePtr> images;
};

extern MainWindow* g_main_window;
