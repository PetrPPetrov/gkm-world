// Copyright 2018-2020 Petr Petrovich Petrov. All rights reserved.
// License: https://github.com/PetrPPetrov/gkm-world/blob/master/LICENSE

#pragma once

#include <vector>
#include <memory>
#include <string>
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
#include <QCloseEvent>
#include "ui_main_window.h"
#include "common.h"
#include "aux_geometry.h"
#include "build_info.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

protected:
    void showEvent(QShowEvent* event) override;

private:
    void createDefaultProject();
    void addPhoto(const char* filename);
    void loadPhotos();
    void updatePhotoListWidget();

    void onPhotoChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void closeEvent(QCloseEvent* event);

private:
    bool first_show = true;
    Ui::MainWindow main_window;

    MeshBuilderWidget* camera_orientation_widget = nullptr;
    QMdiSubWindow* camera_orientation_window = nullptr;

    QPlainTextEdit* log_widget = nullptr;
    QMdiSubWindow* log_window = nullptr;

    QListWidget* photo_list_widget = nullptr;
    QMdiSubWindow* photo_list_window = nullptr;

    const std::string auto_save_file_name = "autosave.gmb";

    AuxGeometry::Ptr aux_geometry;
    BuildInfo::Ptr build_info;
};

extern MainWindow* g_main_window;
