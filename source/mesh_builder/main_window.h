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
#include "mesh_project.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    void updateWindowTitle();

protected:
    void showEvent(QShowEvent* event) override;

private:
    void addPhoto(const char* filename);
    void loadPhotos();
    void updateProject();
    void updatePhotoListWidget();

    void onPhotoChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void closeEvent(QCloseEvent* event);

    void onNewProject();
    void onOpenProject();
    void onSaveProject();
    void onSaveAsProject();
    void onQuit();

    void onAddPhoto();
    void onRemovePhoto();

private:
    bool first_show = true;
    Ui::MainWindow main_window;

    QAction* new_project_act = nullptr;
    QAction* open_project_act = nullptr;
    QAction* save_project_act = nullptr;
    QAction* save_project_as_act = nullptr;
    QAction* close_project_act = nullptr;
    QAction* quit_act = nullptr;

    QAction* add_photo_act = nullptr;
    QAction* remove_photo_act = nullptr;

    MeshBuilderWidget* camera_orientation_widget = nullptr;
    QMdiSubWindow* camera_orientation_window = nullptr;

    QPlainTextEdit* log_widget = nullptr;
    QMdiSubWindow* log_window = nullptr;

    QListWidget* photo_list_widget = nullptr;
    QMdiSubWindow* photo_list_window = nullptr;

    const std::string auto_save_file_name = "autosave.gmb";

    MeshProject::Ptr mesh_project;
};

extern MainWindow* g_main_window;
